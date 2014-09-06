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

  InteractiveGrid
  ---------------

  The interactive grid uses the results of the Tracker (the blobs), to show 
  bigger versions of the mosaic tiles where there is motion. When there
  is motion detected, the position is interpolated to the resolution of the
  mosaic output and then a bigger version is loaded and shown. 

*/

#ifndef ROXLU_TOPSHOP_INTERACTIVE_GRID_H
#define ROXLU_TOPSHOP_INTERACTIVE_GRID_H

#include <tracker/Tracker.h>
#include <tracking/Tiles.h>

namespace track {

  /* ---------------------------------------------------------------------------------- */

  class InteractiveCell {
  public:
    InteractiveCell();
    ~InteractiveCell();

  public:
    int col;
    int row;
    uint64_t timeout;
  };

  /* ---------------------------------------------------------------------------------- */
  
  typedef void(*activate_cell_callback)(int i, int j, void* user);                      /* is called when the user needs to activate the given col/row */

  /* ---------------------------------------------------------------------------------- */

  class InteractiveGrid {
  public:
    InteractiveGrid();
    ~InteractiveGrid();
    int init(Tracker* tracker, Tiles* tiles);
    void update();
    int shutdown();

  public:
    Tracker* tracker;
    Tiles* tiles;
    std::vector<InteractiveCell> cells;
    activate_cell_callback on_activate;       /* when set this is called when a cells needs to be activated. */
    void* user;                               /* is passed into the callback */
  };

} /* namespace track */ 

#endif
