#include <gfx/BlurFBO.h>

namespace gfx {
  
  BlurFBO::BlurFBO() 
    :is_init(0)
    ,width(0)
    ,height(0)
    ,win_width(0)
    ,win_height(0)
  {
  }

  BlurFBO::~BlurFBO() {
  }

  int BlurFBO::init(int w, int h, double amount) {

    width = w;
    height = h;

    if (0 != fbo.init(w, h)) {
      return -1; 
    }
    
    tex_pass0 = fbo.addTexture(GL_RGBA8, w, h, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);
    tex_pass1 = fbo.addTexture(GL_RGBA8, w, h, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT1);

    if (0 != fbo.isComplete()) {
      RX_ERROR("Cannot init blur fbo.\n");
      return -2;
    }

    if (0 != blur_prog.init(amount)) {
      RX_ERROR("Cannot init blur.\n");
      return -3;
    }

    /* get current viewport (used to reset after blurring) */
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    win_width = vp[2];
    win_height = vp[3];

    if (0 == win_width || 0 == win_height) {
      RX_ERROR("Cannot get the window sizes.");
      return -4;
    }

    is_init = 1;

    return 0;
  }

  void BlurFBO::blur(GLuint tex) {

    if (1 != is_init) {
      RX_ERROR("Not yet initialized.");
      return;
    }

    /* set fbo + viewport */
    glViewport(0, 0, width, height);
    fbo.bind();

    /* BLUR-X PASS */
    { 
      fbo.setDrawBuffer(GL_COLOR_ATTACHMENT0);
      glClear(GL_COLOR_BUFFER_BIT);

      /* set the source texture */
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex);
      
      /* blur X on source texture */
      blur_prog.blurX(width, height);
    }

    /* BLUR-Y PASS */
    {
      fbo.setDrawBuffer(GL_COLOR_ATTACHMENT1);
      glClear(GL_COLOR_BUFFER_BIT);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex_pass0);

      blur_prog.blurY(width, height);
    }


    /* and reset the fbo and viewport*/
    fbo.unbind();
    glViewport(0, 0, win_width, win_height);
  }

} /* namespace gfx */
