#include <grid/Grid.h>

namespace grid {
  
  /* ---------------------------------------------------------------------------------- */

  static void on_image_loaded(mos::ImageTask* task, void* user);
  static void on_dir_changed(std::string dir, std::string filename, void* user);
  static bool source_sorter(const Source& a, const Source& b);

  /* ---------------------------------------------------------------------------------- */

  Vertex::Vertex() 
    :size(0.0f)
  {
    pos.set(0.0f, 0.0f);
  }

  Vertex::~Vertex() {
    size = 0.0f;
    pos.set(0.0f, 0.0f);
  }

  /* ---------------------------------------------------------------------------------- */

  Cell::Cell()
    :row(0)
    ,col(0)
    ,dx(0)
    ,time_added(0)
    ,state(CELL_STATE_NONE)
    ,pixels(NULL)
    ,capacity(0)
  {

  }

  Cell::~Cell() {

    row = 0;
    col = 0;
    dx = 0;
    time_added = 0;
    state = CELL_STATE_NONE;
    capacity = 0;

    if (NULL != pixels) {
      delete[] pixels;
    }
    pixels = NULL;
  }

  /* ---------------------------------------------------------------------------------- */

  Grid::Grid(int dir) 
    :is_init(false)
    ,direction(dir)
    ,img_width(0)
    ,img_height(0)
    ,rows(0)
    ,cols(0)
    ,tex_id(0)
    ,tex_width(0)
    ,tex_height(0)
    ,col_hidden(0)
    ,prev_col_hidden(-1)
    ,filled_cells(0)
    ,preloaded_timeout(0)
    ,vert(0)
    ,frag(0)
    ,prog(0)
    ,vao(0)
    ,vbo(0)
  {
    if (0 != pthread_mutex_init(&mutex, NULL)) {
      RX_ERROR("Something went wrong when trying to create the mutex.");
    }
  }

  Grid::~Grid() {

    if (true == is_init) {
      shutdown();
    }

    if (0 != pthread_mutex_destroy(&mutex)) {
      RX_ERROR("Something went wrong when trying to destroy the mutex.");
    }
  }

  int Grid::init(std::string path, int imgWidth, int imgHeight, int rws, int cls) {

    if (true == is_init) {
      RX_ERROR("Grid is already initialized, first shut it down.");
      return -97;
    }

    img_width = imgWidth; 
    img_height = imgHeight;
    rows = rws;
    cols = cls * 2; /* we need to duplicate the number of columns so we can update cells which are hidden. */
    tex_width = img_width * cols;
    tex_height = img_height * rows;
    filled_cells = 0;
    
    if (GRID_DIR_LEFT != direction && GRID_DIR_RIGHT != direction) {
      RX_ERROR("Invalid direction value.");
      return -98;
    }

    if (0 == path.size()) {
      RX_ERROR("Path is empty");
      return -99;
    }

    if (false == rx_is_dir(path)) {
      RX_ERROR("Not a path: %s", path.c_str());
      return -100;
    }

    if (0 == img_width || 0 == img_height || 0 == rows || 0 == cols) {
      RX_ERROR("Invalid params, widht/height/rows or cols");
      return -103;
    }

    /* get viewport info; needed to create the projection matrix. */
    glGetIntegerv(GL_VIEWPORT, vp);
    if (0 == vp[2] || 0 == vp[3]) {
      RX_ERROR("Invalid viewport sizes");
      return -102;
    }

    /* check if the texture size will fit. */
    GLint tex_size = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tex_size);

    if (tex_width > tex_size) {
      RX_ERROR("The total width that we need to is too large; we can only handle %d and you asked %d", tex_size, tex_width);
      return -104;
    }

    if (tex_height > tex_size) {
      RX_ERROR("The total height that we need is to too large; we can only handle %d and you asked %d", tex_size, tex_height);
      return -105;
    }

    /* we allocate a temp buffer for the grid texture so it has some color when we start. */
    unsigned char* tmp_pixels = new unsigned char[(tex_width * tex_height * 4)];
    if (NULL == tmp_pixels) {
      RX_ERROR("Cannot allocate a temp buffer for the grid pixels");
      return -130; 
    }
    memset(tmp_pixels, 0x00, (tex_width * tex_height * 4));

    /* initialize the image loader. */
    if (0 != img_loader.init()) {
      RX_ERROR("Cannot init the image loader");
      return -101;
    }

    img_loader.on_loaded = on_image_loaded;
    img_loader.user = this;

    if (0 != dir_watcher.init(path, on_dir_changed, this)) {
      RX_ERROR("Cannot ini the dir watcher");
      return -102;
    }

    /* create a texture */
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_width, tex_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, tmp_pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] tmp_pixels;
    tmp_pixels = NULL;

    /* create the cells and vertices */
    cells.resize(rows * cols);
    vertices.resize(rows *cols);
    index_order.resize(rows * cols);

    /* allocate the pixel buffer for each cell. */
    int bytes_per_cell = img_width * img_height * 4;
    size_t c = 0;

    for (int i = 0; i < cols; ++i) {
      for (int j = 0; j < rows; ++j) {

        /* set the cell + vertex col/row */
        int dx = (j * cols) + i;
        cells[dx].pixels = new unsigned char[bytes_per_cell];

        if (GRID_DIR_LEFT == direction) {
          index_order[c++] = j * cols + i;
          cells[dx].col = i;
        }
        else {
          index_order[c++] = j * cols + ((cols-1) - i);
          cells[dx].col = (cols-1) - i;
        }

        cells[dx].row = j;
        vertices[dx].col = i;
        vertices[dx].row = j;

        if (NULL == cells[dx].pixels) {
          RX_ERROR("Couldn't allocate buffer for grid cells, bytes: %d. Out of mem?", bytes_per_cell);
          continue;
        }
        cells[dx].capacity = bytes_per_cell;
      }
    }

    /* setup GL. */
    vert = rx_create_shader(GL_VERTEX_SHADER, GRID_VS);
    frag = rx_create_shader(GL_FRAGMENT_SHADER, GRID_FS);
    prog = rx_create_program(vert, frag, true);
    glUseProgram(prog);

    /* set projection matrix */
    GLint u_pm = glGetUniformLocation(prog, "u_pm");
    if (-1 == u_pm) {
      RX_ERROR("u_pm not used!");
      exit(0);
    }
    pm.ortho(0, vp[2], vp[3], 0,  0.0f, 100.0f);
    glUniformMatrix4fv(u_pm, 1, GL_FALSE, pm.ptr());

    /* get position uniform. */
    u_pos = glGetUniformLocation(prog, "u_pos");
    if (-1 == u_pos) {
      RX_ERROR("Cannot get the u_pos for the grid shader.");
      exit(0);
    }

    glUniform1f(glGetUniformLocation(prog, "u_scalex"), 1.0 / cols); /* texture scaling */
    glUniform1f(glGetUniformLocation(prog, "u_scaley"), 1.0 / rows); /* texture scaling */
    glUniform1i(glGetUniformLocation(prog, "u_tex"), 0);

    /* create buffers. */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), NULL, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0); /* pos */
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)8); /* size */
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)12); /* row */
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)16); /* col */
    glEnableVertexAttribArray(0); /* pos */
    glEnableVertexAttribArray(1); /* size */
    glEnableVertexAttribArray(2); /* row */
    glEnableVertexAttribArray(3); /* col */
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);

    /* load initial files. */
    std::vector<std::string> files = rx_get_files(path, "*");
    for (size_t i = 0; i < files.size(); ++i) {
      Source src;
      src.filepath = files[i];
      src.mtime = rx_get_file_mtime(files[i]);
      preloaded.push_back(src);
      if (i > cells.size()) {
        break;
      }
    }
    std::sort(preloaded.begin(), preloaded.end(), source_sorter);
    if (0 != preloaded.size()) {
      preloaded_timeout = rx_hrtime() + 300e6; /* XXe6 millis */
    }

    is_init = true;

    return 0;
  }

  void Grid::update() {

    if (0 == tex_width) {
      RX_WARNING("No tex_width set, probably not yet initialized.");
      return;
    }

    /* do we still have preloaded images that we can load? */
    if (0 != preloaded.size()) {
      uint64_t now = rx_hrtime();
      if (now > preloaded_timeout) {
        preloaded_timeout = now + 50e6; /* XXe6 millis */
        Source& preloaded_source = preloaded.front();
        on_dir_changed(dir_watcher.directory, rx_strip_dir(preloaded_source.filepath), this);
        preloaded.pop_front();
      }
    }

    dir_watcher.update();

    /* make sure we only use the last X when images are coming in too fast. */
    int max_sources = rows * cols;
    if (max_sources < sources.size()) {
      while(sources.size() > max_sources) {
        sources.erase(sources.begin());
        RX_VERBOSE("Erased a source as we've got too many sources");
      }
    }

    /* as long as we can fill cells, load the image in another thread. */
    size_t dx;
    while (0 != sources.size() && true == getUsableCell(dx, CELL_STATE_NONE, CELL_STATE_RESERVED)) {
      Source& source = *sources.begin();
      img_loader.load(source.filepath);
      sources.erase(sources.begin());
    }

    /* for each of the loaded images, we need to update the texture. */
    glBindTexture(GL_TEXTURE_2D, tex_id);
    
    lock();
    {
      for (size_t i = 0; i < index_order.size(); ++i) {

        size_t cdx = index_order[i];
        if (cdx >= cells.size()) {
          RX_ERROR("The index_order contains an invalid index.");
          continue;
        }
        Cell& cell = cells[cdx];

        if (CELL_STATE_LOADED == cell.state) {

          if (NULL == cell.pixels) {
            RX_ERROR("We got a cell in loaded state, but pixels haven't been set :(");
            cell.state = CELL_STATE_NONE;
            continue;
          }

          cell.state = CELL_STATE_IN_USE;

          /* upload the loaded texture into the correct position. */
          int x = cell.col * img_width;
          int y = cell.row * img_height;
          glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, img_width, img_height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, cell.pixels);
        }
      }
    }
    unlock();
     
    /* @todo - we could optimize this a bit and keep track if we need to reposition; though this takes basically no time atm. */

    /* position the images. */
    if (GRID_DIR_LEFT == direction) {
      for (size_t i = 0; i < vertices.size(); ++i) {
        Vertex& v = vertices[i];
        v.size = img_width; /* @todo - for now the cells are square */
        v.pos.set(offset.x + img_width * 0.5 + v.col * img_width + v.col * padding.x,
                  offset.y + img_height * 0.5 + v.row * img_height + v.row * padding.y);

      }
    }
    else {
      for (size_t i = 0; i < vertices.size(); ++i) {
        Vertex& v = vertices[i];
        v.size = img_width; /* @todo - for now the cells are square */
        v.pos.set(offset.x - img_width * 0.5 - v.col * img_width - v.col * padding.x,
                  offset.y + img_height * 0.5 + v.row * img_height + v.row * padding.y);
      }
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertices.size(), vertices[0].pos.ptr());
  }

  void Grid::draw() {

    if (0 == tex_width) {
      return;
    }

    /* check if we need to switch set A with B */
    if (GRID_DIR_LEFT == direction && (fabs(pos_a.x) >= (cols * img_width + cols * padding.x))) {
      vec2 tmp = pos_b;
      pos_b = pos_a;
      pos_a = tmp;
    }
    else if (GRID_DIR_RIGHT == direction && pos_a.x >= (cols * img_width + cols * padding.x)) {
      vec2 tmp = pos_b;
      pos_b = pos_a;
      pos_a = tmp;
    }

    /* move into the correct direction. */
    if (GRID_DIR_LEFT == direction) {
      pos_a.x -= 3.5; // 1.5, 3.5
    }
    else {
      pos_a.x += 3.5; // 1.5, 3.5
    }

    /* Detect which column is hidden, because the hidden column 
       can be reused. When a column is hidden we also need to disable
       the previously hidden column because else a newly loaded image
       would be positioned into the previous one which will change the
       cell contents of a cell which is currently visible.
    */
    int scroll_width = img_width + padding.x;
    col_hidden = (abs(int(pos_a.x)) / scroll_width) - 1;

    lock();
       uint64_t fc = filled_cells;
    unlock();

    if (fc >= cells.size()  &&  col_hidden >= 0 && col_hidden != prev_col_hidden) {
      lock();
      {
        for (size_t i = 0; i < cells.size(); ++i) {
          Cell& cell = cells[i];
          if (cell.col == prev_col_hidden) {
            cell.state = CELL_STATE_IN_USE;
          }
          else if (cell.col == col_hidden) {
            cell.state = CELL_STATE_NONE;
          }
        }
      }
      unlock();
      prev_col_hidden = col_hidden;
    }

    /* cutout the visible area */
    int half_grid_width = (cols * img_width + cols * padding.x) * 0.5;
    int grid_height = rows * img_height + rows * padding.y;

    glEnable(GL_SCISSOR_TEST);
    if (GRID_DIR_LEFT == direction) {
      glScissor(offset.x, (vp[3] - offset.y) - grid_height, half_grid_width, grid_height); 
    }
    else {
      glScissor(offset.x - half_grid_width, (vp[3] - offset.y) - grid_height, half_grid_width, grid_height); 
    }

    /* draw all the rects */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glBindVertexArray(vao);
    glUseProgram(prog);

    /* draw first set */
    glUniform2fv(u_pos, 1, pos_a.ptr());
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, vertices.size());

    /* draw second set (copy) */
    if (GRID_DIR_LEFT == direction) {
      pos_b.x = pos_a.x + cols * img_width + cols * padding.x;
    }
    else {
      pos_b.x = pos_a.x - cols * img_width - cols * padding.x;
    }

    glUniform2fv(u_pos, 1, pos_b.ptr());
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, vertices.size());
    glDisable(GL_SCISSOR_TEST);
  }

  int Grid::shutdown() {

    if (false == is_init) {
      RX_ERROR("Trying to shutdown but we're not yet initialized.");
      return -1;
    }

    if (0 != vao)        {   glDeleteVertexArrays(1, &vao);    }
    if (0 != vert)       {   glDeleteShader(vert);             }
    if (0 != frag)       {   glDeleteShader(frag);             }
    if (0 != prog)       {   glDeleteProgram(prog);            }
    if (0 != tex_id)     {   glDeleteTextures(1, &tex_id);     }
    if (0 != vbo)        {   glDeleteBuffers(1, &vbo);         }

    vert = 0;
    frag = 0;
    prog = 0;
    vao = 0;
    vbo = 0;
    tex_id = 0;
    tex_width = 0;
    tex_height = 0;
    cols = 0;
    rows = 0;
    img_width = 0;
    img_height = 0;
    col_hidden = 0;
    prev_col_hidden = -1;
    filled_cells = 0;
    preloaded_timeout = 0; 
    pos_a.set(0,0);
    pos_b.set(0,0);

    if (0 != img_loader.shutdown()) {
      RX_ERROR("Error while trying to shutdown the image loader.");
    }

    if (0 != dir_watcher.shutdown()) {
      RX_ERROR("Error while trying to shutdown the dir watcher.");
    }

    /* just make sure that all these vectors are sync'd */
    lock();
    {
      cells.clear();
      sources.clear();
      vertices.clear();
      index_order.clear();
    }
    unlock();
    
    is_init = false;

    return 0;
  }
  
  bool Grid::getUsableCell(size_t& dx, int currState, int newState) {
    bool result = false;

    /* @todo - we maybe need to add a lock here? */
    if (index_order.size() != cells.size()) {
      RX_ERROR("Invalid index order size");
      return false;
    }

    lock();
    {
      for (size_t i = 0; i < index_order.size(); ++i) {
        
        size_t order_dx = index_order[i];
        if (order_dx >= cells.size()) {
          RX_ERROR("The index we got from the index_order list is invalid: %lu", order_dx);
          continue;
        }

        Cell& cell = cells[order_dx];
        if (cell.state == currState) {
          dx = order_dx;
          cell.state = newState;
          result = true;
          break;
        }
      }
    }
    unlock();
    
    return result;
  }

  /* ---------------------------------------------------------------------------------- */

  static void on_image_loaded(mos::ImageTask* task, void* user) {

    /* validate */
    if (NULL == task) {
      RX_ERROR("Invalid task == NULL.");
      return;
    }
    if (0 == task->filepath.size()) {
      RX_ERROR("Filepath size is 0; not supposed to happen!");
      return;
    }
    if (false == rx_file_exists(task->filepath)) {
      RX_ERROR("The image was loaded, but it was removed afterwards; not supposed to happen!");
      return;
    }

    Grid* grid = static_cast<Grid*>(user);
    if (NULL == user) {
      RX_ERROR("Cannot get grid handle. Not supposed to happen!");
      return;
    }

    if (grid->img_width != task->width) {
      RX_ERROR("The added image has a different width then we expect: %d != %d", grid->img_width, task->width);
      return;
    }
    if (grid->img_height != task->height) {
      RX_ERROR("The added image has a different height then we expect: %d != %d", grid->img_height, task->height);
      return;
    }
    if (4 != task->channels) {
      RX_ERROR("The given image is not a 4 channel image which we expect it to be.");
      return;
    }

    int nbytes = grid->img_width * grid->img_height * 4;
    if (nbytes != task->nbytes) {
      RX_ERROR("The number of bytes in the loaded images isn't correct.");
      return;
    }

    /* find a free cell. */
    int found_cell = 0;
    size_t cdx = 0;
    int tries = 0;
    int max_tries = 3;
    while (tries < max_tries) {
      if (false == grid->getUsableCell(cdx, CELL_STATE_RESERVED, CELL_STATE_LOADED)) {
        RX_ERROR("Cannot find a cell that we need to fill");
      }
      else {
        found_cell = 1;
        break;
      }
      RX_VERBOSE("Waiting for a free cell");
      usleep(10e3); /* sleep 10 millis */
      tries++;
    }
    if (0 == found_cell) {
      RX_ERROR("After %d tries we couldn't find a free cell. Maybe increase the number of tries?", max_tries);
      return;
    }

    /* copy the image data into our buffer and keep track of it. */
    grid->lock();
    {
      Cell& cell = grid->cells.at(cdx);
      memcpy(cell.pixels, task->pixels, nbytes);
      grid->filled_cells++;
    }
    grid->unlock();
  }

  /* gets called when a file is added to the given path (see init) */
  static void on_dir_changed(std::string dir, std::string filename, void* user) {

    /* get the handle */
    Grid* grid = static_cast<Grid*>(user);
    if (NULL == grid) {
      RX_ERROR("Cannot get the Grid.");
      return;
    }

    Source source;
    source.filepath = dir + "/" +filename;
    source.mtime = rx_get_file_mtime(source.filepath);
    if (0 == source.mtime) {
      RX_ERROR("Cannot get tile mtime for %s", source.filepath.c_str());
      return;
    }
  
    /* add the new source. */
    grid->sources.push_back(source);
    std::sort(grid->sources.begin(), grid->sources.end(), source_sorter);

#if 0
    for (size_t i = 0; i < grid->sources.size(); ++i) {
      RX_VERBOSE(">> %llu == %s", grid->sources[i].mtime, rx_strip_dir(grid->sources[i].filepath).c_str());
    }
#endif
  }

  static bool source_sorter(const Source& a, const Source& b) {
    return a.mtime > b.mtime; 
  }

  /* ---------------------------------------------------------------------------------- */

} /* namespace grid */
