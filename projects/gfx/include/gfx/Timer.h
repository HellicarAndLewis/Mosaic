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

  References:
  ----------

  - https://gist.github.com/roxlu/69dfc2526ed1a166c25e

 */
#ifndef GFX_TIMER_H
#define GFX_TIMER_H

#include <glad/glad.h>
#include <vector>
#include <deque>
#include <map>
#include <string>

#define ROXLU_USE_FONT
#define ROXLU_USE_LOG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

namespace gfx {

  /* -------------------------------------------------------------------------------- */

  class TimerQuery {
  public:
    TimerQuery();
    ~TimerQuery();
    void reset();

  public:
    bool is_free;
    std::string name;
    GLuint id;
    GLuint64 timestamp;
  };

  /* -------------------------------------------------------------------------------- */

  class Timer {
  public:
    Timer();
    ~Timer();
    void start(std::string name);
    void stop();
    void tag(std::string name);
    void draw();
    void reset(); /* only call this when you don't call draw() - this will reset the timer queries every loop */
  private:
    void update();
    TimerQuery* getFreeQueryObject();
    void resetQueries();

  public:
    Painter painter_bg;
    Painter painter_fg;
    PixelFont font;
    int max_bars;
    bool is_started;
    GLint viewport[4];
    std::vector<TimerQuery*> queries;
    std::map<std::string, std::deque<double> > history;
    std::map<std::string, double> max_vals;
    double max_val;
  };

} /* namespace gfx */

#endif
