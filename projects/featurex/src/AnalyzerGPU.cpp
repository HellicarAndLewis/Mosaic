#include <featurex/AnalyzerGPU.h>

namespace fex {

  AnalyzerGPU::AnalyzerGPU() 
    :input_tex(0)
  {
  }

  AnalyzerGPU::~AnalyzerGPU() {
  }

  int AnalyzerGPU::init(GLuint inputTex) {

    int r = 0;
    input_tex = inputTex;

    /* make sure that we get a valid input text handle */
    if (0 == inputTex) {
      RX_ERROR("Inavlid input texture");
      return -1;
    }

    /* initialize the color analyzer */
    r = average_color.init(input_tex);
    if (0 != r) {
      return r;
    }

    return 0;
  }

  int AnalyzerGPU::analyze() {

    if (0 == input_tex) {
      RX_ERROR("Called analyze(), but input_tex is not set.");
      return -1;
    }

    /* calculate the average colors.*/
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input_tex);
    average_color.calculate();

    return 0;

  }

  void AnalyzerGPU::draw() {

    if (0 == input_tex) {
      RX_VERBOSE("No input_tex set; cannot draw anything.");
      return;
    }

    /* draw the average color tex. */
    painter_fg.clear();
    painter_fg.texture(average_color.output_tex, 0, average_color.viewport[3], average_color.viewport[2], -average_color.viewport[3]);
    painter_fg.draw();
  }

  int AnalyzerGPU::shutdown() {

    if (0 == input_tex) {
      RX_ERROR("Asking to shutdown, but input_tex == 0. Did you call init()?");
      return -1;
    }

    average_color.shutdown();

    return 0;
  }


} /* namespace fex */
