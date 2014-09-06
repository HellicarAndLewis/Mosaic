/*
---------------------------------------------------------------------------------

                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'

                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu

---------------------------------------------------------------------------------

*/

#ifndef ROXLU_ANALYZER_GPU_H
#define ROXLU_ANALYZER_GPU_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

#include <featurex/AverageColorGPU.h>
#include <featurex/Descriptor.h>

namespace fex {

  class AnalyzerGPU {

  public:
    AnalyzerGPU();
    ~AnalyzerGPU();
    int init(GLuint inputTex);                              /* initialize; settings the texture that we need to use to analyze. */
    int shutdown();                                         /* cleanup // stop */
    int analyze();                                          /* analyze the current input texture; only call this when the input texture has changed. */
    void draw();                                            /* draw some info to show the current state of the analyzer. */
    int getDescriptor(int i, int j, Descriptor& out);       /* get the descriptor for the given column/row. Sets `out` and returns 0 on success else < 0 */

  public:
    GLuint input_tex;
    AverageColorGPU average_color;
    Painter painter_fg;
    std::vector<Descriptor> descriptors; /* after calling analyze this will hold all the descriptors. it will have (fex::config.cols * fex::config.rows) number of descriptors. */
  };

} /* namespace fex */

#endif
