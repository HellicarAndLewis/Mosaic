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

#ifndef TOPSHOP_H
#define TOPSHOP_H

#include <mosaic/Mosaic.h>
#include <grid/Grid.h>
#include <topshop/ImageCollector.h>
#include <topshop/ImageProcessor.h>
#include <topshop/RemoteState.h>
#include <tracking/Tracking.h>

namespace top {
  
  class TopShop {
  public:
    TopShop();
    ~TopShop();
    int init();
    int shutdown();
    void update();
    void draw();

  public:
    mos::Mosaic mosaic;
    ImageCollector img_collector;                       /* watches a directory for new files */
    track::Tracking tracking;                           /* responsible for the interactive/tracking part of the mosaic. */
    RemoteState remote_state;                           /* polls the server and checks if we need to change something in the application */
  };

} /* namespace top */

#endif
