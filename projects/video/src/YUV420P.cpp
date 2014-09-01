#include <video/YUV420P.h>

namespace vid {


  static GLuint create_texture(int w, int h);

  YUV420P::YUV420P() 
    :vao(0)
    ,prog(0)
    ,frag(0)
    ,vert(0)
    ,tex_y(0)
    ,tex_u(0)
    ,tex_v(0)
    ,w(0)
    ,h(0)
    ,hw(0)
    ,hh(0)
  {
  }

  YUV420P::~YUV420P() {
    shutdown();
  }

  int YUV420P::init(int width, int height) {
    w = width;
    h = height; 
    hw = w * 0.5;
    hh = h * 0.5;

    /* validate */
    if (0 != prog) { 
      RX_ERROR("Looks like we're already initialized");
      return -1;
    }
    if (0 >= w) {
      RX_ERROR("Invalid width: %d", w);
      return -2;
    }
    if (0 >= h) {
      RX_ERROR("Invalid height: %d", h);
      return -3;
    }

    /* create textures. */
    tex_y = create_texture(w, h);
    tex_u = create_texture(hw, hh);
    tex_v = create_texture(hw, hh);

    /* create shaders */
    vert = rx_create_shader(GL_VERTEX_SHADER, YUV420P_VS);
    frag = rx_create_shader(GL_FRAGMENT_SHADER, YUV420P_FS);
    prog = rx_create_program(vert, frag, true);

    /* set the texture binding points. */
    glUseProgram(prog);
    rx_uniform_1i(prog, "y_tex", 0);
    rx_uniform_1i(prog, "u_tex", 1);
    rx_uniform_1i(prog, "v_tex", 2);

    /* create vao */
    glGenVertexArrays(1, &vao);
    return 0;
  }

  int YUV420P::shutdown() {

    /* delete the textures */
    if (0 != tex_y) {   glDeleteTextures(1, &tex_y);   }
    if (0 != tex_u) {   glDeleteTextures(1, &tex_u);   }
    if (0 != tex_v) {   glDeleteTextures(1, &tex_v);   }
    
    if (0 != vao) {
      glDeleteVertexArrays(1, &vao);
    }
    
    tex_y = 0;
    tex_u = 0;
    tex_v = 0;
    vao = 0;

    return 0;
  }

  int YUV420P::update(uint8_t* y, int ys, uint8_t* u, int us, uint8_t* v, int vs) {
    /* yep we're safe ^.^ */
    if (NULL == y) { return -1; } 
    if (NULL == u) { return -2; } 
    if (NULL == v) { return -3; } 
    if (0 == ys) { return -4; } 
    if (0 == us) { return -5; } 
    if (0 == vs) { return -6; }

    glBindTexture(GL_TEXTURE_2D, tex_y);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, ys);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, y);

    glBindTexture(GL_TEXTURE_2D, tex_u);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, us);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, hw, hh, GL_RED, GL_UNSIGNED_BYTE, u);

    glBindTexture(GL_TEXTURE_2D, tex_v);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, vs);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, hw, hh, GL_RED, GL_UNSIGNED_BYTE, v);

    /* reset */
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    return 0;
  }

  void YUV420P::draw() {
    if (0 == tex_y) { return; } 
    if (0 == prog) { return; } 

    glUseProgram(prog);
    glBindVertexArray(vao);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, tex_y);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, tex_u);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, tex_v);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  /* ---------------------------------------------------------------------------------- */

  static GLuint create_texture(int w, int h) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
  }

} /* namespace vid */
