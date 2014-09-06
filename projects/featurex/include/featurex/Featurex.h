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
#include <featurex/AnalyzerCPU.h>
#include <featurex/AnalyzerGPU.h>
#include <featurex/Comparator.h>
#include <featurex/TilesPool.h>

namespace fex {

  class Featurex {

  public:
    Featurex();
    ~Featurex();

    int init(GLuint inputTex);                               /* initialize the analyzers; the inputTex is passed to the gpu analyzer. */
    int shutdown(); 
    void draw();

    int analyzeCPU(std::string filepath);
    int analyzeGPU();
    void match();                                            /* when called, we use the current set of cpu/gpu descriptors to find the best matches. make sure to only call this when you actually updated the input. */
    int getDescriptorGPU(int i, int j, Descriptor& out);     /* will return the descriptor for the given col and row, sets `out` and returns 0 on success else -1. */

  public:
    AnalyzerCPU analyzer_cpu;
    AnalyzerGPU analyzer_gpu;
    Comparator comp;
    TilesPool tiles;                                          /* the tiles pool contains the images that make up the mosiac. */
    unsigned char* mosaic_pixels; 
  };

  /* ----------------------------------------------------------------------------------- */

  inline int Featurex::getDescriptorGPU(int i, int j, Descriptor& out) {
    return analyzer_gpu.getDescriptor(i, j, out);
  }

  /* ----------------------------------------------------------------------------------- */

} /* namespace fex */

#endif
