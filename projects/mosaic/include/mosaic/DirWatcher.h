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

  DirWatcher
  ----------

  Calls a function when a file is added to a directory. 
  
*/
#ifndef MOSAIC_DIR_WATCHER_H
#define MOSAIC_DIR_WATCHER_H

extern "C" {
#  include <uv.h>
}

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace mos {

  typedef void (*dirwatch_on_rename_callback)(std::string dir, std::string filename, void* user);

  class DirWatcher {
  public:
    DirWatcher();
    ~DirWatcher();
    int init(std::string dir, dirwatch_on_rename_callback cb, void* user);   /* start listening the given dir */
    void update();                                                           /* call this often to process events. */
    int shutdown();                                                          /* shutdown and stop listening. */

  public:
    /* watch the directories */
    std::string directory;
    uv_loop_t* loop;
    uv_fs_event_t fs_event;

    /* callback */
    dirwatch_on_rename_callback on_rename;
    void* user;
  };

} /* namespace mos */

#endif
