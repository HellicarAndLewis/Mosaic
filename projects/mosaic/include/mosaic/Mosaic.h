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
#ifndef ROXLU_MOSAIC_H
#define ROXLU_MOSAIC_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_FONT
#include <tinylib.h>

#include <mosaic/Config.h>
#include <mosaic/VideoInput.h>
#include <featurex/Featurex.h>
#include <gfx/AsyncUpload.h>

namespace mos {

  class Mosaic {

  public:
    Mosaic();
    ~Mosaic();

    int init();                      /* initializes everything; starts threads, allocates GL objects (indirectly) etc.. */
    void update();                   /* call this often; will process any input/output data. */
    void draw();                     /* draw all visuals! */
    int shutdown();                  /* destory and free all allocated objects so init() could be called again. */

  public:
    fex::Featurex featurex;          /* the feature extractor library. */
    VideoInput video_input;          /* the webcam or rtmp input. */
    gfx::AsyncUpload async_upload;   /* used to upload the mosaic texture */
    GLuint mosaic_tex;               /* the mosaic texture. */
    Painter painter;                 /* @todo - we my replace this with a custom shader, but for testing the result of the mosaic we use this. */
  };

} /* namespace mos */

#endif
