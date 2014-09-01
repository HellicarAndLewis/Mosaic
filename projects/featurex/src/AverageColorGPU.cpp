#include <featurex/Config.h>
#include <featurex/AverageColorGPU.h>

AverageColorGPU::AverageColorGPU() 
  :frag(0)
  ,vert(0)
  ,prog(0)
  ,vao(0)
  ,input_tex(0)
  ,fbo(0)
  ,output_tex(0)
{
  viewport[0] = viewport[1] = viewport[2] = viewport[3] = 0;
}

AverageColorGPU::~AverageColorGPU() {
  viewport[0] = viewport[1] = viewport[2] = viewport[3] = 0;
}

int AverageColorGPU::init(GLuint inputTex) {

  /* validate state */
  if (0 != frag) {
    RX_ERROR("Already create the average color shader.");
    return -1;
  }

  if (false == fex::config.validateTileSettings()) {
    return -2;
  }

  if (0 == inputTex) {
    RX_ERROR("Invalid input texture id");
    return -3;
  }

  input_tex = inputTex;

  /* create texture shader. */
  vert = rx_create_shader(GL_VERTEX_SHADER, AVG_COL_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, AVG_COL_FS);
  prog = rx_create_program(vert, frag, true);

  /* set the input texture ID */
  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "u_tex"), 0);
  glUniform1i(glGetUniformLocation(prog, "u_cols"), fex::config.cols);
  glUniform1i(glGetUniformLocation(prog, "u_rows"), fex::config.rows);

  /* create the vao; necessary :< */
  glGenVertexArrays(1, &vao);

  /* create the FBO + texture */
  if (0 != reinit()) {
    return -4;
  }

  glGetIntegerv(GL_VIEWPORT, viewport);

  /* initialize our helper that retrieves the data from the gpu. */
  if (0 != async_download.init(fex::config.cols, fex::config.rows, GL_BGRA)) {
    return -5;
  }

  return 0;
}

int AverageColorGPU::reinit() {

  /* shutdown will destroy any initialized members/GL objects. */
  shutdown();

  /* create the output buffer that will contains the average colors */
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  /* create the texture that holds the 'general color' */
  glGenTextures(1, &output_tex);
  glBindTexture(GL_TEXTURE_2D, output_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fex::config.cols, fex::config.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_tex, 0);
  
  /* make sure the fbo is valid. */
  if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
    RX_ERROR("Framebuffer not complete in AverageColorGPU");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return -2;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "u_cols"), fex::config.cols);
  glUniform1i(glGetUniformLocation(prog, "u_rows"), fex::config.rows);
  glUniform1i(glGetUniformLocation(prog, "u_image_w"), fex::config.input_image_width);
  glUniform1i(glGetUniformLocation(prog, "u_image_h"), fex::config.input_image_height);
  glUniform1i(glGetUniformLocation(prog, "u_tile_size"), fex::config.input_tile_size);
  
  return 0;
}

int AverageColorGPU::shutdown() {

  if (0 != fbo) {
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &output_tex);
    fbo = 0;
    output_tex = 0;
  }

  return 0;
}

void AverageColorGPU::calculate() {
  if (0 == viewport[2] || 0 == viewport[3]) {
    RX_ERROR("Invalid viewport values");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glClear(GL_COLOR_BUFFER_BIT);

  /* calculate the average colors */
  glViewport(0, 0, fex::config.cols, fex::config.rows);
  glBindVertexArray(vao);
  glUseProgram(prog);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, input_tex);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  /* download the results */
  async_download.download();

#if 0
  /* store the result in a png file */
  static double next_sec = 0;
  double now = (double)(rx_hrtime()) / (1000.0 * 1000.0 * 1000.0);
  if (now >= next_sec) {
    std::string filename = rx_to_data_path(rx_get_time_string() +".png");
    rx_save_png(filename, async_download.buffer, async_download.width, async_download.height, 4, false);
    next_sec = now + 1;
  }
#endif

  /* reset fbo. */
  glViewport(0, 0, viewport[2], viewport[3]);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
