#include <featurex/Config.h>
#include <featurex/AverageColor.h>

AverageColor::AverageColor() 
  :frag(0)
  ,vert(0)
  ,prog(0)
  ,vao(0)
  ,input_texid(0)
  ,fbo(0)
  ,output_texid(0)
{
}

AverageColor::~AverageColor() {
}

int AverageColor::init() {

  if (0 != frag) {
    RX_ERROR("Already create the average color shader.");
    return -1;
  }

  /* create texture shader. */
  vert = rx_create_shader(GL_VERTEX_SHADER, AVG_COL_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, AVG_COL_FS);
  prog = rx_create_program(vert, frag, true);

  /* set the input texture ID */
  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "input_texid"), 0);
  glUniform1i(glGetUniformLocation(prog, "u_cols"), fex::config.cols);
  glUniform1i(glGetUniformLocation(prog, "u_rows"), fex::config.rows);

  /* create the vao; necessary :< */
  glGenVertexArrays(1, &vao);

  /* create the FBO + texture */
  if (0 != reinit()) {
    return -2;
  }

  return 0;
}

int AverageColor::reinit() {

  /* when already created, delete first */
  if (0 != fbo) {
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &output_texid);
    fbo = 0;
    output_texid = 0;
  }

  /* create the output buffer that will contains the average colors */
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  /* create the texture that holds the 'general color' */
  glGenTextures(1, &output_texid);
  glBindTexture(GL_TEXTURE_2D, output_texid);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, fex::config.cols, fex::config.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_texid, 0);
  
  /* make sure the fbo is valid. */
  if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
    RX_ERROR("Framebuffer not complete in AverageColor");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return -2;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "u_cols"), fex::config.cols);
  glUniform1i(glGetUniformLocation(prog, "u_rows"), fex::config.rows);
  glUniform1i(glGetUniformLocation(prog, "u_image_w"), fex::config.input_image_width);
  glUniform1i(glGetUniformLocation(prog, "u_image_h"), fex::config.input_image_height);
  glUniform1i(glGetUniformLocation(prog, "u_tile_size"), fex::config.tile_size);
  
  return 0;
}

void AverageColor::calculate() {

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glClear(GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, fex::config.cols, fex::config.rows);
  glBindVertexArray(vao);
  glUseProgram(prog);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, input_texid);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glViewport(0, 0, 1280, 720);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
