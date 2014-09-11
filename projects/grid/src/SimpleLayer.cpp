#include <grid/SimpleLayer.h>

namespace grid {

  /* ------------------------------------------------------------------------ */

  SimpleCell::SimpleCell() 
    :row(0)
    ,col(0)
    ,x(0)
    ,y(0)
    ,pixels(NULL)
    ,img_width(0)
    ,img_height(0)
    ,allocated(0)
    ,nbytes(0)
    ,channels(0)
    ,layer(0)
    ,in_use(false)
  {

  }

  SimpleCell::~SimpleCell() {

    if (pixels) {
      delete[] pixels;
      pixels = NULL;
    }
    
    row = 0;
    col = 0;
    img_width = 0;
    img_height = 0;
    x = 0;
    y = 0;
    allocated = 0;
    nbytes = 0;
    channels = 0;
    layer = 0;
    in_use = false;
  }
  
  /* ------------------------------------------------------------------------ */

  SimpleLayer::SimpleLayer() 
    :cell_dx(0)
    ,nimages(0)
    ,tex(0)
    ,timeout(0)
    ,state(SIMPLE_STATE_NONE)
    ,on_event(NULL)
    ,user(NULL)
  {
  }

  SimpleLayer::~SimpleLayer() {
    on_event = NULL;
    user = NULL;
    if (0 != cells.size()) {
      shutdown();
    }
  }

  int SimpleLayer::init(SimpleSettings cfg) {

    size_t layer = 0;

    if (false == cfg.validate()) {
      return -1;
    }

    settings = cfg;

    /* initialize the cells */
    cells.resize(settings.rows * settings.cols);
    for (size_t i = 0; i < settings.cols; ++i) {
      for (size_t j = 0; j < settings.rows; ++j) {
        size_t dx = j * settings.cols + i;
        cells[dx].layer = layer++;
        cells[dx].row = j;
        cells[dx].col = i;
        cells[dx].x = settings.offset_x + settings.img_width * 0.5 + i * settings.img_width + i * settings.padding_x;
        cells[dx].y = settings.offset_y + settings.img_height * 0.5 + j * settings.img_height + j * settings.padding_y;
      }
    }

    /* create texture */
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, settings.img_width, settings.img_height, cells.size(), 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    return 0;
  }

  /* we assume you have double checked the filepath and that the image is a PNG! */
  int SimpleLayer::addImage(SimpleImage img) {

    if (cell_dx >= cells.size()) {
      RX_ERROR("Invalid cell index. Not supposed to happen.");
      return -1;
    }

    if (nimages >= cells.size()) {
      RX_ERROR("We cannot add a new image because we're full. You need to flip!");
      return -2;
    }

    /* laod the image in the next free cell. */
    SimpleCell& cell = cells[cell_dx];
    rx_load_png(img.filepath, 
                &cell.pixels, 
                cell.img_width, 
                cell.img_height, 
                cell.channels, 
                &cell.allocated, 
                RX_FLAG_LOAD_AS_RGBA);
    
    if (cell.img_width != settings.img_width) { 
      RX_ERROR("Loaded file has invalid image width: %d, we expect: %d", cell.img_width, settings.img_width);
      return -3;
    }
    if (cell.img_height != settings.img_height) {
      RX_ERROR("Loaded file has invalid image height: %d, we expect: %d", cell.img_height, settings.img_height);
      return -4;
    }

    /* update texture. */
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, cell.layer, settings.img_width, settings.img_height, 1, GL_BGRA, GL_UNSIGNED_BYTE, cell.pixels);
    
    cell.in_use = true;
    ++cell_dx %= cells.size();
    nimages++;

    if (nimages >= cells.size() && NULL != on_event) {
      on_event(this, SIMPLE_EVENT_FULL);
    }

    return 0;
  }

  int SimpleLayer::shutdown() {
    
    if (0 == cells.size()) {
      RX_WARNING("Not initialized, ignoring shutdown request.");
      return 0;
    }

    if (0 != tex) {
      glDeleteTextures(1, &tex);
      tex = 0;
    }

    cells.clear();

    state = SIMPLE_STATE_NONE;
    cell_dx = 0;
    nimages = 0;
    timeout = 0;

    return 0;
  }

  /* update physics */
  void SimpleLayer::updatePhysics(float dt) {

    if (SIMPLE_STATE_NONE == state) {
      return;
    }

    /* when showing or visible update physics*/
    if (SIMPLE_STATE_SHOWING == state || SIMPLE_STATE_VISIBLE == state) {
      float now = rx_millis();
      for (size_t i = 0; i < cells.size(); ++i) {
        SimpleCell& cell = cells[i];
        if (false == cell.in_use) {
          continue;
        }
        cell.tween_size.update(now);
      }
    }
    else if (SIMPLE_STATE_HIDING == state) {
      float now = rx_millis();
      for (size_t i = 0; i < cells.size(); ++i) {
        SimpleCell& cell = cells[i];
        if (false == cell.in_use) {
          continue;
        }
        cell.tween_size.update(now);
        cell.tween_x.update(now);
      }
    }
  }

  /* handles state changes. */
  void SimpleLayer::update() {

    /* layer is not initialized yet. */
    if (SIMPLE_STATE_NONE == state) {
      return;
    }

    /* check if we need to change state .*/
    uint64_t now = rx_hrtime();
    if (SIMPLE_STATE_SHOWING == state) {
      if (now > timeout) {
        state = SIMPLE_STATE_VISIBLE;
        if (on_event) {
          on_event(this, SIMPLE_EVENT_SHOWN);
        }
      }
      return;
    }
    else if (SIMPLE_STATE_HIDING == state) {
      /* reset when we're hidden, so we can be reused again. */
      if (now > timeout) {
        state = SIMPLE_STATE_HIDDEN;
        reset(); 
        if (on_event) {
          on_event(this, SIMPLE_EVENT_HIDDEN);
        }
      }
    }
  }

  /* prepare before the cell is shown */
  int SimpleLayer::prepareShow() {

    if (0 == cells.size()) {
      RX_ERROR("Not initialized");
      return -1;
    }

    if (SIMPLE_GRID_DIRECTION_RIGHT == settings.direction) {
      for (size_t i = 0; i < cells.size(); ++i) {
        SimpleCell& cell = cells[i];
        if (false == cell.in_use) {
          continue;
        }
        cell.tween_size.value.set(0.0f, 0.0f);
        cell.tween_size.set(1.5f, rx_random(0.0f, 2.0f), vec2(0.0f, 0.0f), vec2(settings.img_width, settings.img_height));
        cell.tween_x.value = cell.x;
      }
    }
    else if (SIMPLE_GRID_DIRECTION_LEFT == settings.direction) {
      for (size_t i = 0; i < cells.size(); ++i) {
        SimpleCell& cell = cells[i];
        if (false == cell.in_use) {
          continue;
        }
        cell.tween_size.value.set(0.0f, 0.0f);
        cell.tween_size.set(1.5f, rx_random(0.0f, 2.0f), vec2(0.0f, 0.0f), vec2(settings.img_width, settings.img_height));
        cell.tween_x.value = cell.x;
      }
    }
    else {
      RX_ERROR("unhandled grid direction: %d", settings.direction);
    }

    return 0;
  }

  /* prepare before the cell is hidden */
  int SimpleLayer::prepareHide() {


    if (0 == cells.size()) {
      RX_ERROR("Not initialized");
      return -1;
    }

    if (SIMPLE_GRID_DIRECTION_RIGHT == settings.direction) {
      for (size_t i = 0; i < cells.size(); ++i) {
        SimpleCell& cell = cells[i];
        if (false == cell.in_use) {
          continue;
        }

        float delay = ((settings.cols - cell.col) - 1) * 0.2 + rx_random(0.0, 0.7f);
        cell.tween_size.set(2.5f, delay, vec2(settings.img_width, settings.img_height), vec2(-settings.img_width, -settings.img_height));
        cell.tween_x.set(2.5f, delay, cell.tween_x.value, 1920 + settings.img_width + rx_random(500,1000));
      }
    }
    else if (SIMPLE_GRID_DIRECTION_LEFT == settings.direction) {
      for (size_t i = 0; i < cells.size(); ++i) {
        SimpleCell& cell = cells[i];
        if (false == cell.in_use) {
          continue;
        }

        float delay = (cell.col - 1) * 0.2 + rx_random(0.0, 0.7f);
        cell.tween_size.set(2.5f, delay, vec2(settings.img_width, settings.img_height), vec2(-settings.img_width, -settings.img_height));
        cell.tween_x.set(2.5f, delay, cell.tween_x.value, -(1920 + settings.img_width + rx_random(500,1000)));
      }
    }
    else {
      RX_ERROR("unhandled grid direction: %d", settings.direction);
    }

    return 0;
  }

  void SimpleLayer::reset() {
    for (size_t i = 0; i < cells.size(); ++i) {
      cells[i].in_use = false;
    }

    nimages = 0;
  }

  void SimpleLayer::hide() {

    /* already hidden ? */
    if (SIMPLE_STATE_HIDDEN == state) {
      RX_ERROR("Trying to hide a layer which is already hidden.");
      return;
    }
    
    /* already trying to hide */
    if (SIMPLE_STATE_HIDING == state) {
      RX_ERROR("We're now busy with hiding.");
      return;
    }

    /* no state yet. */
    if (SIMPLE_STATE_NONE == state) {
      state = SIMPLE_STATE_HIDING;
      timeout = rx_hrtime();
      prepareHide();
      return;
    }

    /* start hiding. */
    timeout = rx_hrtime() + 3e9; 
    state = SIMPLE_STATE_HIDING;
    prepareHide();
  }

  void SimpleLayer::show() {
    /* already shown. */
    if (SIMPLE_STATE_VISIBLE == state) {
      RX_ERROR("Trying to show a layer which is already visible.");
      return;
    }

    /* in progress. */
    if (SIMPLE_STATE_SHOWING == state) {
      RX_ERROR("We're already in progress to show the layer");
      return;
    }

    /* when we have no state yet, we can directly show the layer. */
    if (SIMPLE_STATE_NONE == state) {
      state = SIMPLE_STATE_SHOWING;
      timeout = rx_hrtime();
      prepareShow();
      return;
    }

    /* start showing */
    timeout = rx_hrtime() + 3e9;
    state = SIMPLE_STATE_SHOWING;
    prepareShow();
  }

} /* namespace grid */
