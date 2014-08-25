#include <tinylib.h>
#include <gfx/Image.h>

namespace gfx {

  Image::Image()
    :type(IMAGE_NONE)
    ,width(0)
    ,height(0)
    ,channels(0)
    ,capacity(0)
    ,pixels(NULL)
    ,texid(0)
  {
  }

  Image::~Image() {

    if (pixels) {
      delete[] pixels;
    }

    pixels = NULL;
    width = 0;
    height = 0;
    channels = 0;
  }


  int Image::load(std::string filepath) {

    /* validate */
    if (0 == filepath.size()) { 
      RX_ERROR("Error: empty filepath.\n");
      return -1;
    }

    if (false == rx_file_exists(filepath)) { 
      RX_ERROR("File doesn't exist.\n");
      return -2;
    }

    std::string ext = rx_get_file_ext(filepath);
    if (ext == "jpg") {
      type = IMAGE_JPEG;
    }
    else if(ext == "png") {
      type = IMAGE_PNG;
    }
    else {
      RX_ERROR("Unknown extension: %s\n", ext.c_str());
      return -3;
    }

    if (type == IMAGE_PNG) {
      if (!rx_load_png(filepath, &pixels, width, height, channels, &capacity)) {
        RX_ERROR("Cannot load png: %s", filepath.c_str());
        return -4;
      }
    }
    else if(type == IMAGE_JPEG) {
      if (!rx_load_jpg(filepath, &pixels, width, height, channels, &capacity)) {
        RX_ERROR("Cannot load jpg: %s", filepath.c_str());
        return -5;
      }
    }
    else {
      RX_ERROR("Invald image type (shouldn't happen).");
      return -6;
    }

    RX_VERBOSE("Loaded: %s, allocated: %d bytes", filepath.c_str(), capacity);
    return 0;
  }

  /* Creates a texture for the loaded image. */
  int Image::createTexture() {

    if (0 != texid) {
      RX_ERROR("Already created a texture for this image.");
      return -1;
    }

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    RX_VERBOSE("Created texture: %d", texid);

    return 0;
  }

  int Image::updateTexture() {

    /* validate */
    if (NULL == pixels) {
      RX_ERROR("Cannot create texture because no pixels are loaded");
      return -1;
    }

    if (0 == width) {
      RX_ERROR("Cannot create texture; width == 0");
      return -2;
    }

    if (0 == height) {
      RX_ERROR("Cannot create texture; height == 0;");
      return -3;
    }
    
    if (0 == texid) {
      RX_ERROR("Cannot update texture; not texture created yet. Call createTexture().");
      return -4;
    }

    GLenum format;
    if (3 == channels) { 
      format = GL_RGB;
    }
    else if (4 == channels) {
      format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, texid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

    return 0;
  }

} /* namespace gfx */
