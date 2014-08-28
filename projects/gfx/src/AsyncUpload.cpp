#include <gfx/AsyncUpload.h>

namespace gfx {

  AsyncUpload::AsyncUpload()
    :width(0)
    ,height(0)
    ,dx(0)
    ,channels(0)
    ,n(0)
  {
  }

  AsyncUpload::~AsyncUpload() {
    shutdown();
  }

  int AsyncUpload::init(int w, int h, GLenum fmt) {
    if (0 == w) {
      RX_ERROR("Width is 0.");
      return -1;
    }

    if (0 == h) {
      RX_ERROR("Height is 0.");
      return -2;
    }

    if (0 != width) {
      RX_ERROR("The width is not 0, did you call shutdown?");
      return -3;
    }

    if (fmt == GL_RGBA || fmt == GL_RGBA8) {
      format = GL_RGBA;
      channels = 4;
    }
    else if (fmt == GL_RGB || fmt == GL_RGB8) {
      format = GL_RGB;
      channels = 3;
    }
    else {
      RX_ERROR("Format is not GL_RGBA, GL_RGBA8, GL_RGB, GL_RGB8; for now only GL_RGBA, GL_RGB are supported.");
      return -4;
    }
    
    width = w;
    height = h;
    format = fmt;
    dx = 0;
    nbytes = width * height * channels;

    glGenBuffers(ASYNC_UPLOAD_NUM_BUFFERS, pbo);
    for (int i = 0; i < ASYNC_UPLOAD_NUM_BUFFERS; ++i) {
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]);
      glBufferData(GL_PIXEL_UNPACK_BUFFER, nbytes, NULL, GL_STREAM_DRAW); 
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    RX_VERBOSE("Created %d pixel unpack buffers that can hold %d bytes, width: %d, height: %d, channels: %d", ASYNC_UPLOAD_NUM_BUFFERS, nbytes, width, height, channels);

    return 0;
  }

  int AsyncUpload::upload(unsigned char* pixels) {

    if (0 == width || 0 == height || 0 == channels) {
      RX_ERROR("Trying to upload pixels; but it looks like we're not yet initialized.");
      return -1;
    }

    if (NULL == pixels) { 
      RX_ERROR("Invalid pixels given; NULL");
      return -2;
    }

#if 1

    /* @todo - should we use GL_BGRA // GL_UNIGNED_INT_8_8_8_8_REV too? Probably! */

    /* fast upload */
    dx = n % ASYNC_UPLOAD_NUM_BUFFERS;

    if (n < ASYNC_UPLOAD_NUM_BUFFERS) {
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[dx]);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, NULL);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
    else {
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[dx]);
      GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
      if (NULL != ptr) {
        memcpy(ptr, pixels, nbytes); /* takes about 0.0039 seconds for 1920 x 1200 rgba buffer */
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, NULL);
      }
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    ++n;
#else 
    /* non-optimal upload */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, pixels);
#endif
    return 0;
  }

  int AsyncUpload::shutdown() {

    if (0 != width) {
      RX_ERROR("Cannot shutdown as width is 0. Did you init?");
      return -1;
    }

    glDeleteBuffers(ASYNC_UPLOAD_NUM_BUFFERS, pbo);

    width = 0;
    height = 0;
    dx = 0;
    channels = 0;
    n = 0;

    return 0;
  }

} /* namespace gfx */
