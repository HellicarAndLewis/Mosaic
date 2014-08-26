#include <gfx/AsyncDownload.h>

namespace gfx {

  AsyncDownload::AsyncDownload()
    :dx(0)
    ,width(0)
    ,height(0)
    ,nbytes(0)
    ,buffer(NULL)
  {
    pbo[0] = pbo[1] = 0;
  }

  AsyncDownload::~AsyncDownload() {
    shutdown();
  }

  int AsyncDownload::init(int w, int h, GLenum fmt) {
    
    if (0 == w) { RX_ERROR("Invalid width, 0");  return -1;  }
    else if (0 == h) { RX_ERROR("Invalid height, 0"); return -2; }
    else if (0 != width) { RX_ERROR("Looks like you already initialized the AsyncDownlaod"); return -3; } 

    format = fmt;
    width = w;
    height = h;
    
    /* how many bytes do we need? */
    if (format == GL_BGRA) {  /* THE RECOMMENDED FORMAT: https://developer.apple.com/library/mac/documentation/graphicsimaging/conceptual/opengl-macprogguide/opengl_texturedata/opengl_texturedata.html  */
      nbytes = width * height * 4;
      read_format = GL_BGRA;
    }
    else if (format == GL_RGBA) {
      nbytes = width * height * 4;
      read_format = GL_RGBA;
    }
    else if (format == GL_RGB || format == GL_RGB8) {
      nbytes = width * height * 3;
      read_format = GL_RGB;
      RX_WARNING("You're using GL_RGB or GL_RGB8 for the AsyncDownload format; this is a non optimal format and padding may occur that slows down the downloads.");
    }
    else {
      RX_ERROR("Unsupported format: %d, not that we recommend you to use GL_RGBA8", fmt);
      return -4;
    }

    /* allocate the buffer into which we read back the data. */
    buffer = new unsigned char[nbytes];
    if (NULL == buffer) {
      RX_ERROR("Cannot allocate the buffer of %d bytes", nbytes);
      width = 0;
      height = 0;
      return -5;
    }

    glGenBuffers(2, pbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[0]);
    glBufferData(GL_PIXEL_PACK_BUFFER, nbytes, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[1]);
    glBufferData(GL_PIXEL_PACK_BUFFER, nbytes, NULL, GL_STREAM_DRAW);
    
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    return 0;
  }

  int AsyncDownload::shutdown() {

    if (0 != pbo[0]) {
      glDeleteBuffers(1, &pbo[0]);
    }

    if (0 != pbo[1]) {
      glDeleteBuffers(1, &pbo[1]);
    }

    dx = 0;
    pbo[0] = 0;
    pbo[1] = 0;
    width = 0;
    height = 0;

    if (NULL != buffer) {
      delete[] buffer;
    }

    buffer = NULL;
    return 0;
  }

  int AsyncDownload::download() {

    if (0 == width || 0 == height || 0 == pbo[0] || 0 == pbo[1]) { 
      RX_ERROR("Cannot download the buffer; async not initialized?");
      return -1;
    }

#if 1
    int curr_dx = dx;
    int next_dx = 1 - dx;
    
    /* read into first pbo */
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[curr_dx]);
    glReadPixels(0, 0, width, height, read_format, GL_UNSIGNED_BYTE, NULL);

    /* read back from the other pbo */
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[next_dx]);
    GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    if (NULL != ptr) {
      memcpy(buffer, ptr, nbytes);
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    dx = 1 - dx;
#else
    /* the non-optimal reads */
    glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
#endif

    return 0;
  }

} /* namespace gfx */
