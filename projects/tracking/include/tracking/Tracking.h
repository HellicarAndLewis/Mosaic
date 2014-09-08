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

  Tracking
  ------

  Interactive layer for the Mosaic. 
    
*/
#ifndef ROXLU_TOPSHOP_TRACKING_H
#define ROXLU_TOPSHOP_TRACKING_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#include <tinylib.h>
#include <videocapture/CaptureGL.h>
#include <tracker/Tracker.h>
#include <tracking/Tiles.h>
#include <tracking/InteractiveGrid.h>

#define USE_TRACKER 1
#define USE_TILES 1

namespace track {

  /* ------------------------------------------------------------------ */

  class TrackingSettings {
  public:
    TrackingSettings();
    ~TrackingSettings();
    bool validate();

  public:
    int webcam_device;      /* the webcam number/id */
    int webcam_width;       /* width of the webcam input */
    int webcam_height;      /* height of the webcam input */
    int tile_width;         /* the width o fthe tile that we show. */
    int tile_height;        /* the height of the tile that we show, this can be the polaroid or the bigger version of the mosaic */
    int tile_nlayers;       /* how many texture layer elements do we need to create */
  };

  /* ------------------------------------------------------------------ */

  class Tracking {
  public:
    Tracking();
    ~Tracking();
    int init(TrackingSettings cfg);
    int shutdown();
    void update();
    void draw();

    /* wrappers */
    int hasFreeLayer();
    int load(ImageOptions& opt);

  public:
    bool needs_update;
    bool is_init;

    TrackingSettings settings;
    ca::CaptureGL capture;

#if USE_TRACKER
    Tracker* tracker;
#endif
#if USE_TILES
    Tiles tiles;
#endif
    InteractiveGrid interactive_grid;
    
    /* wrapper around InteractiveGrid::on_activate */
    activate_cell_callback on_activate;                        /* is called when the InteractiveGrid tells us to "activate" a cell which meens that we need to show the bigger version of a cell image. */
    void* user;                                                /* gets passed into the on_activate function, see InteractiveGrid for the definition */ 
  };

  inline int Tracking::hasFreeLayer() {
#if USE_TILES
    return tiles.hasFreeLayer();
#else 
    return -1;
#endif    
  }

  inline int Tracking::load(ImageOptions& opt) {
#if USE_TILES
    return tiles.load(opt);
#else
    return 0;
#endif    
  }

} /* namespace track */

#endif
