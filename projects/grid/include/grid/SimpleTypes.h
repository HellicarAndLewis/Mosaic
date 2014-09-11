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
#ifndef ROXLU_SIMPLE_TYPES_H
#define ROXLU_SIMPLE_TYPES_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#include <tinylib.h>

namespace grid {

  /* ---------------------------------------------------------------- */
  template<class T>
  class SimpleTween {
  public:
    void reset();
    void update(float now);
    void set(float duration, float delay, T start, T change);

    float delay;
    float start_time;
    float t;                                /* current time [0-1] */  
    float d;                                /* duration */
    T b;                                    /* start value */
    T c;                                    /* change */
    T value;                                /* last updated value */
  };

  /* ---------------------------------------------------------------- */

  template<class T>
    void SimpleTween<T>::set(float duration, float delay, T start, T change) {
    start_time = rx_millis() + delay;
    d = duration;
    b = start;
    c = change;
  }

  template<class T>
  void SimpleTween<T>::update(float now) {
    float tt;
    float tc;
    float ts;

    if (now < start_time) {
      return;
    }

    tt = now - start_time;
    t = tt / d;
    if (t > 1.0f) {
      t = 1.0f;
    }

    ts = t * t;
    tc = ts * t;
    
    value = b + c * (tc * ts + -5.0 * ts * ts + 10 * tc + -10 * ts + 5 * t); /* out quintic */
  }

  /* ---------------------------------------------------------------- */

  class SimpleImage {
  public:
       std::string filepath;
  };

  /* ---------------------------------------------------------------- */

  struct SimpleVertex {
    vec2 position;
    vec2 size;
    int layer;
  };
  
} /* namespace grid */

#endif
