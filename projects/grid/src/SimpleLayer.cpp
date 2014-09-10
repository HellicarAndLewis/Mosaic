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
    ,tex(0)
  {
  }

  SimpleLayer::~SimpleLayer() {
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
        cells[dx].x = settings.img_width * 0.5 + i * settings.img_width + i * settings.padding_x;
        cells[dx].y = settings.img_height * 0.5 + j * settings.img_height + j * settings.padding_y;
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
      return -2;
    }
    if (cell.img_height != settings.img_height) {
      RX_ERROR("Loaded file has invalid image height: %d, we expect: %d", cell.img_height, settings.img_height);
      return -3;
    }

    /* update texture. */
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, cell.layer, settings.img_width, settings.img_height, 1, GL_BGRA, GL_UNSIGNED_BYTE, cell.pixels);
    
    cell.in_use = true;

    ++cell_dx %= cells.size();

    return 0;
  }

  int SimpleLayer::shutdown() {
    return 0;
  }

  void SimpleLayer::update(float dt) {
    float now = rx_millis();

    for (size_t i = 0; i < cells.size(); ++i) {
      SimpleCell& cell = cells[i];
      if (false == cell.in_use) {
        continue;
      }
      cell.tween_size.update(now);
    }

  }

  void SimpleLayer::draw() {
  }

  int SimpleLayer::prepare() {
    if (0 == cells.size()) {
      RX_ERROR("Not initialized");
      return -1;
    }

    for (size_t i = 0; i < cells.size(); ++i) {

      SimpleCell& cell = cells[i];
      if (false == cell.in_use) {
        continue;
      }

      /* prepare before the cell is shown */
      cell.tween_size.value.set(0.0, 0.0f);
      cell.tween_size.set(1.5f, rx_random(0.0, 2.0f), vec2(0.0, 0.0), vec2(settings.img_width, settings.img_height));
    }

    return 0;
  }

  bool SimpleLayer::isFull() {
    /* @todo we can use a counter in addImage, but that's not really necessary (may be a bit faster) .*/
    size_t num_full = 0;
    for (size_t i = 0; i < cells.size(); ++i) {
      if (cells[i].in_use) {
        num_full++;
      }
    }
    return num_full == cells.size();
  }

  void SimpleLayer::reset() {
    for (size_t i = 0; i < cells.size(); ++i) {
      cells[i].in_use = false;
    }
  }

} /* namespace grid */
