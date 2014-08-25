#include <gfx/FBO.h>
namespace gfx {
  
  FBO::FBO() 
    :fbo(0)
    ,width(0)
    ,height(0)
  {
  }

  FBO::~FBO() {
    /* @todo - remove all textures + fbo */
  }

  int FBO::init(int w, int h) {

    /* validate */
    if (0 == w) { 
      RX_ERROR("Invalid width");
      return -1;
    }
    if (0 == h) {
      RX_ERROR("Invalid height");
      return -2;
    }
    if (0 != fbo) {
      RX_ERROR("Already created");
      return -3;
    }

    glGenFramebuffers(1, &fbo);

    return 0;
  }

  GLuint FBO::addTexture(GLenum format, int w, int h, GLenum internalFormat, GLenum type, GLenum attach) {

    /* create the texture */
    GLuint tex; 
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, internalFormat, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    textures.push_back(tex);

    /* attach it. */
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glFramebufferTexture2D(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return tex;
  }


  int FBO::isComplete() {

    /* validate */
    if (0 == fbo) {
      RX_ERROR("No fbo created yet. call init().");
      return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    /* make sure the fbo is valid. */
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
      RX_ERROR("Framebuffer not complete");
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;

  }

  void FBO::bind() {

    if (0 == fbo) {
      RX_ERROR("Trying to bind an FBO with ID = 0. Not initialized?");
      return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  }

  void FBO::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  
  void FBO::setDrawBuffer(GLenum attach) {

    if (0 == fbo) {
      RX_ERROR("Trying to set a drawbuffer but you didn't create the FBO yet.");
      return;
    }

    GLenum drawbufs[] = { attach };
    glDrawBuffers(1, drawbufs);
  }

} /* namespace gfx */
