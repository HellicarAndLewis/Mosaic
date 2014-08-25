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
#ifndef ROXLU_FEATUREX_H
#define ROXLU_FEATUREX_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_FONT
#include <tinylib.h>

#include <string>
#include <featurex/AverageColor.h>
#include <featurex/AnalyzerCPU.h>
#include <gfx/BlurFBO.h>
#include <gfx/Image.h>
#include <gfx/Timer.h>

namespace fex {

  class Featurex {

  public:
    Featurex();
    ~Featurex();

    int init();
    int reinit();
    void update();
    void draw();

    int analyzeImageFile(std::string filepath);

    int loadInputImage(std::string filepath);

  public:
    Painter painter_bg;
    Painter painter_fg;
    AnalyzerCPU cpu_analyzer;
    AverageColor avg_color;
    gfx::Image input_image;
    gfx::BlurFBO blur;
    gfx::Timer timer;

    bool has_new_analyzed_image;
    gfx::Image analyzed_image;
    Descriptor analyzed_desc;
  };

} /* namespace fex */

#endif
