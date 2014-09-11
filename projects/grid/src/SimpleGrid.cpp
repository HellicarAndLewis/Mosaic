#include <grid/SimpleGrid.h>

namespace grid {

  /* --------------------------------------------------------------------------------- */

  static void on_layer_event(SimpleLayer* layer, int event);

  /* --------------------------------------------------------------------------------- */

  SimpleGrid::SimpleGrid()
    :front_layer(NULL)
    ,back_layer(NULL)
    ,vao(0)
    ,vbo(0)
    ,frag(0)
    ,vert(0)
    ,prog(0)
    ,vbo_allocated(0)
    ,num_cells(0)
    ,on_event(NULL)
    ,user(NULL)
  {
  }

  SimpleGrid::~SimpleGrid() {
    if (vao != 0) {
      shutdown();
    }
  }

  int SimpleGrid::init(SimpleSettings cfg) {

    if (false == cfg.validate()) {
      RX_ERROR("Invalid settings.");
      return -1;
    }

    if (0 != layer_a.init(cfg)) {
      return -2;
    }

    if (0 != layer_b.init(cfg)) {
      return -3;
    }

    back_layer = &layer_a;
    front_layer = &layer_b;

    layer_a.on_event = on_layer_event;
    layer_a.user = this;
    layer_b.on_event = on_layer_event;
    layer_b.user = this;

    settings = cfg;    

    /* init opengl */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid*) 0); /* pos */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid*) 8); /* size */
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(SimpleVertex), (GLvoid*) 16); /* layer */
    glEnableVertexAttribArray(0); /* pos */
    glEnableVertexAttribArray(1); /* size */
    glEnableVertexAttribArray(2); /* layer */
    glVertexAttribDivisor(0, 1); /* pos */
    glVertexAttribDivisor(1, 1); /* size */
    glVertexAttribDivisor(2, 1); /* layer */

    vert = rx_create_shader(GL_VERTEX_SHADER, SG_VS);
    frag = rx_create_shader(GL_FRAGMENT_SHADER, SG_FS);
    prog = rx_create_program(vert, frag, true);
    glUseProgram(prog);

    /* get viewport size */
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    if (0 == vp[2] || 0 == vp[3]) {
      RX_ERROR("Cannot get the viewport info");
      return -4;
    }

    /* set projection matrix. */
    mat4 pm;
    GLint u_pm = glGetUniformLocation(prog, "u_pm");
    if (-1 == u_pm) {
      RX_ERROR("Cannot get u_pm from shader! Not used?");
      return -5;
    }
    pm.ortho(0, vp[2], vp[3], 0, 0.0f, 100.0f);
    glUniformMatrix4fv(u_pm, 1, GL_FALSE, pm.ptr());

    /* set sampler */
    GLint u_tex = glGetUniformLocation(prog, "u_tex");
    if (-1 == u_tex) {
      RX_ERROR("Cannot get the u_tex from the shader. Not used?");
      return -6;
    }
    glUniform1i(u_tex, 0);

    /* just checking ... */
    if (settings.offset_x > (vp[2] / 2)) {
      RX_ERROR("It seems that x-offset of the SimpleGrid is a bit too much? Offset is: %d", settings.offset_x);
    }

    return 0;
  }

  int SimpleGrid::shutdown() {
    RX_ERROR("Cleanup simple grid!");
    layer_a.shutdown();
    layer_b.shutdown();
    return 0;
  }

  void SimpleGrid::updatePhysics(float dt) {
    if (NULL != front_layer) {
      front_layer->updatePhysics(dt);
    }
    if (NULL != back_layer) {
      back_layer->updatePhysics(dt);
    }
  }

  void SimpleGrid::update() {

    if (NULL != front_layer) {
      front_layer->update();
    }
    if (NULL != back_layer) {
      back_layer->update();
    }

    std::vector<SimpleVertex> vertices;
    updateVertices(front_layer, vertices);
    updateVertices(back_layer, vertices);

    size_t needed = vertices.size() * sizeof(SimpleVertex);
    if (needed == 0) {
      return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    if (needed > vbo_allocated) {
      while (vbo_allocated < needed) {
        vbo_allocated = std::max<size_t>(vbo_allocated * 2, 1024);
      }
      glBufferData(GL_ARRAY_BUFFER, vbo_allocated, vertices[0].position.ptr(), GL_STREAM_DRAW);
    }
    else {
      glBufferSubData(GL_ARRAY_BUFFER, 0, needed, vertices[0].position.ptr());
    }

    num_cells = vertices.size();
  }

  void SimpleGrid::updateVertices(SimpleLayer* from, std::vector<SimpleVertex>& vertices) {

    if (NULL == from) { 
      return;
    } 

    for (size_t i = 0; i < from->cells.size(); ++i) {
      SimpleCell& cell = from->cells[i];
      if (false == cell.in_use) {
        continue;
      }

      SimpleVertex v;
      v.position.x = cell.tween_x.value;
      v.position.y = cell.y;
      v.size = cell.tween_size.value;
      v.layer = cell.layer;

      vertices.push_back(v);
    }
  }

  void SimpleGrid::draw() {

    if (NULL == front_layer) { 
#if !defined(NDEBUG)
      RX_ERROR("Front layer not set"); 
#endif
      return; 
    } 

    if (0 == num_cells) {
      return;
    }

    // RX_VERBOSE("Drawing: %lu cells", num_cells);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, front_layer->tex);
    glBindVertexArray(vao);
    glUseProgram(prog);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, num_cells);
  }

  int SimpleGrid::addImage(SimpleImage img) {

    if (NULL == back_layer) {
      RX_ERROR("No back layer");
      return -1;
    }

    if (0 == img.filepath.size()) {
      RX_ERROR("Filepath is invalid");    
      return -2;
    }

    if (false == rx_file_exists(img.filepath)) {
      RX_ERROR("Cannot find file: %s", img.filepath.c_str());
      return -3;
    }

    std::string ext = rx_get_file_ext(img.filepath);
    if (ext != "png") {
      RX_ERROR("SimpleGrid can only use png.");
      return -4;
    }

    return back_layer->addImage(img);

  }

  /* --------------------------------------------------------------------------------- */

  static void on_layer_event(SimpleLayer* layer, int event) {

    if (NULL == layer) { 
      RX_ERROR("Layer is NULL"); 
      return; 
    }
    
    SimpleGrid* grid = static_cast<SimpleGrid*>(layer->user);
    if (NULL == grid) {
      RX_ERROR("Cannot get grid handle.");
      return;
    }

    if (SIMPLE_EVENT_FULL == event) {
      grid->front_layer->hide();
    }
    else if (SIMPLE_EVENT_SHOWN == event) {
      /* we don't need this one. */
    }
    else if (SIMPLE_EVENT_HIDDEN == event) {
      SimpleLayer* tmp = grid->back_layer;
      grid->back_layer = grid->front_layer;
      grid->front_layer = tmp;
      grid->front_layer->show();
    }
    else {
      RX_ERROR("Unhandled event");
    }

    /* pass along the event. */
    if (grid->on_event) {
      grid->on_event(grid, layer, event);
    }
  } 

  /* --------------------------------------------------------------------------------- */

} /* namespace grid */
