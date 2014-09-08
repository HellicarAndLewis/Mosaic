#include <tracking/Tiles.h>

namespace track {

  /* ---------------------------------------------------------------------------------- */

  static void on_image_loaded(mos::ImageTask* task, void* user);                            /* images are loaded in a separate thread, this function is called when the image has been loaded. We only support 4 channel PNGs*/

  /* ---------------------------------------------------------------------------------- */

  Image::Image() {
    layer = 0;
    is_free = true;
    allocated = 0;
  }

  Image::~Image() {                             

    if (NULL != pixels) {
      delete[] pixels;
    }
    pixels = NULL;

    layer = 0;
    is_free = false;
    allocated = 0;
    x = 0;
    y = 0;
  }

  /* ---------------------------------------------------------------------------------- */
  
  void Tween::reset() {
    start_time = 0.0f;
    t = 0.0f;
    d = 0.0f;
    b = 0.0f;
    c = 0.0f;
    value = 0.0f;
  }

  void Tween::update(float now) {
    float tt;
    float tc;
    float ts;
    
    tt = now - start_time;
    t = tt / d;
    if (t > 1.0f) {
      t = 1.0f;
    }

    ts = t * t;
    tc = ts * t;
 
    value = b + c * (tc * ts + -5.0 * ts * ts + 10 * tc + -10 * ts + 5 * t); /* out quintic */
  }

  /* ---------------------------------------------------------------------------------- */

  Tiles::Tiles() 
    :is_init(false)
    ,tex_width(0)
    ,tex_height(0)
    ,must_update(false)
    ,frag(0)
    ,vert(0)
    ,prog(0)
    ,vao(0)
    ,vbo(0)
    ,tex(0)
  {
    int r = pthread_mutex_init(&mutex, NULL);
    if (0 != r) {
      RX_ERROR("Cannot initialize the mutex: %s", strerror(r));
      ::exit(EXIT_FAILURE);
    }
  }

  Tiles::~Tiles() {
    if (true == is_init) {
      shutdown();
    }

    int r = pthread_mutex_destroy(&mutex);
    if (0 != r) {
      RX_ERROR("Cannot destroy the mutex: %s", strerror(r));
    }

    is_init = false;
    must_update = false;
  }

  int Tiles::init(int texW, int texH, int texLayers) {

    tex_width = texW;
    tex_height = texH;
    tex_ntotal = texLayers;

    /* validate input */
    if (true == is_init) {
      RX_ERROR("Tiles already initialized, first shutdown");
      return -1;
    }
    if (0 == tex_ntotal) {
      RX_ERROR("Invalid number of texture layers");
      return -10;
    }
    if (0 == tex_width) {
      RX_ERROR("Invalid texture width: %d", tex_width);
      return -11;
    }
    if (0 == tex_height) {
      RX_ERROR("Invalid texture height: %d", tex_height);
      return -12;
    }

    /* check if we can store the requested number of textures */
    GLint max_tex_layers = 0;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_tex_layers);
    if (max_tex_layers < tex_ntotal) {
      RX_ERROR("The requested amount of textures for the Tiles can be stored in one texture array, max is: %d", max_tex_layers);
      return -2;
    }

    if (0 != img_loader.init()) {
      RX_ERROR("Cannot init the image loader of the Tiles");
      return -3;
    }

    img_loader.user = this;
    img_loader.on_loaded = on_image_loaded;

    /* init opengl */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * tex_ntotal, NULL, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 0); /* pos */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 8); /* size */
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (GLvoid*) 16); /* layer */
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 20); /* age */
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 24); /* angle */
    glEnableVertexAttribArray(0); /* pos */
    glEnableVertexAttribArray(1); /* size */
    glEnableVertexAttribArray(2); /* layer */
    glEnableVertexAttribArray(3); /* age */
    glEnableVertexAttribArray(4); /* angle */
    glVertexAttribDivisor(0, 1); /* pos */
    glVertexAttribDivisor(1, 1); /* size */
    glVertexAttribDivisor(2, 1); /* layer */
    glVertexAttribDivisor(3, 1); /* age */
    glVertexAttribDivisor(4, 1); /* angle */

    vert = rx_create_shader(GL_VERTEX_SHADER, TRACK_TILES_VS);
    frag = rx_create_shader(GL_FRAGMENT_SHADER, TRACK_TILES_FS);
    prog = rx_create_program(vert, frag, true);

    glUseProgram(prog);

    /* set ortho matrix. */
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    if (0 == vp[2] || 0 == vp[3]) {
      RX_ERROR("Cannot get the viewport info");
      return -4;
    }

    GLint u_pm = glGetUniformLocation(prog, "u_pm");
    if (-1 == u_pm) {
      RX_ERROR("Cannot get u_pm from shader! Not used?");
      return -5;
    }

    mat4 pm;
    pm.ortho(0, vp[2], vp[3], 0, 0.0f, 100.0f);
    glUniformMatrix4fv(u_pm, 1, GL_FALSE, pm.ptr());

    /* set sampler */
    GLint u_tex = glGetUniformLocation(prog, "u_tex");
    if (-1 == u_tex) {
      RX_ERROR("Cannot get the u_tex from the shader. Not used?");
      return -6;
    }
    glUniform1i(u_tex, 0);
    

    /* create the tex array */
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, tex_width, tex_height, tex_ntotal, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    is_init = true;

    RX_VERBOSE("Created array texture: %d", tex);

    for (size_t i = 0; i < tex_ntotal; ++i) {
      free_layers.push_back(i);
    }

    vertices.resize(tex_ntotal);

    return 0;
  }

  void Tiles::update() {

    /* Do we need to update the texture array */
    bool update_texture = false;
    lock();
      update_texture = must_update;
      must_update = false;
    unlock();

    if (true == update_texture) {
      Image* img = NULL;
      int free_layer = 0;

      glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

      lock();
      {
        for (size_t i = 0; i < images.size(); ++i) {
          if (true == images[i]->is_free) {
            continue; /* skip free images */
          }

          img = images[i];

          if (NULL == img->pixels || 0 == img->allocated) {
            RX_ERROR("Trying to upload an invalid image!");
            continue;
          }

          if (0 == free_layers.size()) {
            RX_VERBOSE("No free layers! The image will be shown as soon as we have a free layer");
            img->is_free = true;
            continue;
          }

          if (IMAGE_MODE_BOINK != img->mode && IMAGE_MODE_FLY != img->mode) {
            RX_ERROR("Invalid image mode: ", img->mode);
            img->is_free = true;
            continue;
          }

          img->is_free = true;

          Particle* p = new Particle();
          if (NULL == p) {
            RX_VERBOSE("Cannot allocate a particle; out of mem?");
            continue;
          }
          particles.push_back(p);

          free_layer = free_layers.front();
          free_layers.pop_front();


          /* update the texture layer */
          glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, free_layer, tex_width, tex_height, 1, GL_BGRA, GL_UNSIGNED_BYTE, img->pixels);

          /* initialize the particle    */
          /* -------------------------- */
          p->mode = img->mode;
          p->state = VERTEX_STATE_TWEEN_IN;
          p->layer = free_layer;
          p->age = 0.0f; 
          p->angle = 0.0f;
          p->tween_size.set(img->tween_size.d, img->tween_size.b, img->tween_size.c);

          if (IMAGE_MODE_BOINK == p->mode) {
            p->position.set(img->x, img->y);
            p->tween_angle.set(img->tween_angle.d, img->tween_angle.b, img->tween_angle.c);
          }

          if (IMAGE_MODE_FLY == p->mode) {
            p->tween_angle.set(img->tween_angle.d, img->tween_angle.b, img->tween_angle.c);
            p->tween_x.set(img->tween_x.d, img->tween_x.b, img->tween_x.c);
            p->tween_y.set(img->tween_y.d, img->tween_y.b, img->tween_y.c);
          }
          
          /* ------------------------- */
        }
      }
      unlock();
    }

    /* update the vertices, positions etc. */
    updateVertexState();

  }

  void Tiles::updateVertexState() {

    size_t dx = 0;
    float now = rx_millis();

    std::vector<Particle*>::iterator it = particles.begin();
    while (it != particles.end()) {

      Particle& p = *(*it);

      p.tween_size.update(now);
      
      if (IMAGE_MODE_BOINK == p.mode) {
        p.tween_angle.update(now);
        p.angle = p.tween_angle.value;
        p.size.set(p.tween_size.value, p.tween_size.value);
      }
      else if (IMAGE_MODE_FLY == p.mode) {

        p.tween_angle.update(now);
        p.tween_x.update(now);
        p.tween_y.update(now);

        p.size.set(p.tween_size.value, p.tween_size.value);
        p.position.set(p.tween_x.value, p.tween_y.value);
        p.angle = p.tween_angle.value;

      }
      else {
        RX_ERROR("Unhandled animation mode.");
      }

      /* remote the particle and free the layer. */
      if (VERTEX_STATE_TWEEN_OUT == p.state && p.tween_size.t >= 1.0f) {
        free_layers.push_back(p.layer);
        delete *it;
        it = particles.erase(it);
        continue;
      }

      /* update vertex info */
      Vertex v;
      vertices[dx].size = p.size;
      vertices[dx].position = p.position;
      vertices[dx].layer = p.layer;
      vertices[dx].angle = p.angle;

      dx++;
      ++it;
    }

    /* update vertices. */
    if (particles.size() > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * particles.size(), vertices[0].position.ptr());
    }

    /* update states and hide the image again*/
    it = particles.begin();
    while (it != particles.end()) {
      Particle& p = *(*it);
      float dt = now - (p.tween_size.start_time + p.tween_size.d);
      /* start the tween out with a `dt` sec delay */
      if (VERTEX_STATE_TWEEN_IN == p.state && dt > 0.4) {
        p.state = VERTEX_STATE_TWEEN_OUT;
        p.tween_size.set(0.5f, p.tween_size.b + p.tween_size.c, -100);
      }
      ++it;
    }
  }

  void Tiles::draw() {

    if (0 == particles.size()) {
      return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glBindVertexArray(vao);
    glUseProgram(prog);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particles.size());
  }

  int Tiles::shutdown() {

    RX_VERBOSE("Shutting down the Tiles");

    if (false == is_init) {
      RX_ERROR("Cannot shutdown because we're not initialized.");
      return -1;
    }

    if (0 != img_loader.shutdown()) {
      RX_ERROR("Failed to shutdown the image loader.");
    }

    /* delete our GL objects. */
    if (0 != vert) { glDeleteShader(vert);            vert = 0; }
    if (0 != frag) { glDeleteShader(frag);            frag = 0; }
    if (0 != prog) { glDeleteProgram(prog);           prog = 0; }
    if (0 != tex)  { glDeleteTextures(1, &tex);       tex = 0;  } 
    if (0 != vao)  { glDeleteVertexArrays(1, &vao);   vao = 0;  }
    if (0 != vbo)  { glDeleteBuffers(1, &vbo);        vbo = 0;  }

    /* delete our images. */
    for (size_t i = 0; i < images.size(); ++i) {
      delete images[i];
    }

    /* delete particles */
    for (size_t i = 0; i < particles.size(); ++i) {
      delete particles[i];
    }

    images.clear();
    vertices.clear();
    particles.clear();
    free_layers.clear();

    is_init = false;
    tex_width = 0;
    tex_height = 0;
    tex_ntotal = 0;
    must_update = false;
    return 0;
  }

  int Tiles::load(ImageOptions& opt) {// std::string filepath) {

    if (false == is_init) {
      RX_ERROR("Canot load image, we're not initialized");
      return -1;
    }

    if (0 == opt.filepath.size()) {
      RX_ERROR("Invald filepath, is empty.");
      return -2;
    }

    if (false == rx_file_exists(opt.filepath)) {
      RX_ERROR("The given filepath doesn't exist: %s", opt.filepath.c_str());
      return -3;
    }

    std::string ext = rx_get_file_ext(opt.filepath);
    if (ext != "png") {
      RX_ERROR("The tile images have to be PNGs with 4 channels.");
      return -4;
    }

    if (IMAGE_MODE_BOINK != opt.mode 
        && IMAGE_MODE_FLY != opt.mode) 
      {
        RX_ERROR("The image optoin has an invalid mode: %d", opt.mode);
        return -5;
      }

    /* 
       @todo we're not keeping track of created image options; 
       this means that when the image loader doesn't call our callback
       (on_image_loaded) we're leaking memory! */
       
    ImageOptions* io = new ImageOptions();
    if (NULL == io) {
      RX_ERROR("Cannot allocate an image option.");
      return -6;
    }

    io->x = opt.x;
    io->y = opt.y;
    io->mode = opt.mode;
    io->tween_angle = opt.tween_angle;
    io->tween_x = opt.tween_x;
    io->tween_y = opt.tween_y;
    io->tween_size = opt.tween_size;

    if (0 != img_loader.load(opt.filepath, io)) {
      RX_ERROR("The threaded image loader rejected the file: %s", opt.filepath.c_str());
      return -5;
    }

    return 0;
  }

  /*
  int Tiles::showTileAtPosition(std::string filename, int x, int y) {

  }
  */

  Image* Tiles::getFreeImage() {
    if (false == is_init) {
      RX_ERROR("Cannot get a free image because we're not initialized.");
      return NULL;
    }

    Image* img = NULL;
    int nbytes = tex_width * tex_height * 4; /* size per image. */

    /* find one which is free */
    lock();
    {
      for (size_t i = 0; i < images.size(); ++i) {
        if (images[i]->is_free) {
          images[i]->is_free = false;
          img = images[i];
          break;
        }
      }
    }
    unlock();

    /* allocate a new image. */
    if (NULL == img) {
      img = new Image();
      if (NULL == img) {
        RX_ERROR("Cannot allocate an image; out of memory?");
        return NULL;
      }
      
      img->pixels = new unsigned char[nbytes];
      if (NULL == img->pixels) {
        RX_ERROR("Cannot allocate the buffer for the image. out of memory?");
        delete img;
        img = NULL;
        return NULL;
      }

      img->allocated = nbytes;
      img->is_free = false;

      lock();
      {
        images.push_back(img);
        RX_VERBOSE("Allocated a new Tiles::Image, we have %lu allocated now.", images.size());
      }
      unlock();
    }

    return img;
  }

  /* ---------------------------------------------------------------------------------- */

  static void on_image_loaded(mos::ImageTask* task, void* user) {
   
    Image* img = NULL;
    ImageOptions* io = static_cast<ImageOptions*>(task->user);
    if (NULL == io) {
      RX_ERROR("The image options are invalid. Not supposed to happen!");
      return;
    }

    Tiles* tiles = static_cast<Tiles*>(user);
    if (NULL == tiles) {
      RX_ERROR("Cannot get the handle to Tiles");
      goto error;
    }

    if (task->width != tiles->tex_width) {
      RX_ERROR("We loaded an invalid image, wrong width. We expect: %d and got: %d", tiles->tex_width, task->width);
      goto error;
    }

    if (task->height != tiles->tex_height) {
      RX_ERROR("We loaded an invalid image, wrong height. We expect: %d and got: %d", tiles->tex_height, task->height);
      return;
    }

    img = tiles->getFreeImage();
    if (NULL == img) {
      RX_ERROR("Cannot get a free image for the track::Tiles");
      return;
    }

    if (img->allocated < task->nbytes) {
      RX_ERROR("The ImageTask contains more bytes then that we can store! Not supposed to happen!");
      tiles->lock();
      img->is_free = true;
      tiles->unlock();
      goto error;
    }

    /* Set custom image specs here! */               
    /* ---------------------- */
    memcpy(img->pixels, task->pixels, task->nbytes);
    img->x = io->x;
    img->y = io->y;
    img->mode = io->mode;
    img->tween_angle = io->tween_angle;
    img->tween_x = io->tween_x;
    img->tween_y = io->tween_y;
    img->tween_size = io->tween_size;
    /* ---------------------- */

    RX_VERBOSE("Copied pixels that we need to update!");

    delete io;
    io = NULL;

    /* make sure the main thread knows that we must update. */
    tiles->lock();
      tiles->must_update = true;
    tiles->unlock();

  error:
    if (NULL != io) {
      delete io;
      io = NULL;
    }
  }

} /* namespace track */
