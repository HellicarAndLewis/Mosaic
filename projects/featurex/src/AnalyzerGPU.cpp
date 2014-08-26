#include <featurex/AnalyzerGPU.h>
#include <featurex/Config.h>

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

#if !defined(NDEBUG)
    if (0 == fex::config.cols) {
      RX_ERROR("Cannot analyze because fex::config.cols hasn't been set");
      return -1;
    }
    if (0 == fex::config.rows) {
      RX_ERROR("Cannot analyze because fex::config.rows hasn't been set");
      return -2;
    }
#endif

    if (0 == input_tex) {
      RX_ERROR("Called analyze(), but input_tex is not set.");
      return -3;
    }
    if (GL_BGRA != average_color.async_download.read_format) {
      RX_ERROR("The read format of the average color download is not GL_BGRA!");
      return -6;
    }

    /* make sure that our container is big enough */
    size_t needed_descriptors = fex::config.cols * fex::config.rows;
    if (descriptors.size() != needed_descriptors) {
      descriptors.resize(needed_descriptors);
    }

    /* calculate the average colors.*/
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input_tex);
    average_color.calculate();

    /* get the average colors and put them in a descriptor. */
    unsigned char* colors = average_color.getAverageColorPtr();
    if (NULL == colors) {
      RX_ERROR("The average color buffer seems to be NULL.");
      return -4;
    }

    /* create descriptors + match. */
    int needed_bytes = fex::config.cols * fex::config.rows * 4;
    if (average_color.async_download.nbytes != needed_bytes) {
      RX_ERROR("The buffer size of the async download doesn't match up with what we expect it to be.");
      return -5;
    }

#if 1 
    /* this loop has been timed at ~0.000004 seconds */
    uint8_t r,g,b;
    for (int i = 0; i < fex::config.cols; ++i) {
      for (int j = 0; j < fex::config.rows; ++j) {
        int dx = (j * fex::config.cols * 4) + (i * 4); /* format is BGRA */
        b = colors[dx + 0];
        g = colors[dx + 1];
        r = colors[dx + 2];

        Descriptor& desc = descriptors[i * j];
        desc.average_color[0] = r;
        desc.average_color[1] = g;
        desc.average_color[2] = b;
        //printf("(%d,%d,%d,%d),  ", colors[dx + 0], colors[dx + 1], colors[dx + 2], colors[dx + 3]);
      }
      //printf("\n");
    }

    //printf("-\n");
#endif

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
