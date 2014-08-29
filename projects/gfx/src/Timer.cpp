#include <gfx/Timer.h>

namespace gfx {
  
  /* -------------------------------------------------------------------------------- */
  
  TimerQuery::TimerQuery() 
    :id(0)
    ,timestamp(0)
    ,is_free(true)
  {
  }


  TimerQuery::~TimerQuery() {
  }

  void TimerQuery::reset() {

    /* makes this query object available again */
    is_free = true;
    timestamp = 0;
    name.clear();
  }

  /* -------------------------------------------------------------------------------- */

  Timer::Timer() 
    :is_started(false)
    ,max_bars(0)
    ,max_val(0.0)
  {
                                                
    if (false == GL_ARB_timer_query) {
      RX_ERROR("No timer query object. The Timer won't work.");
    }

    /* get the width / height of the viewport, used to draw the graphs */
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (0 == viewport[2] || 0 == viewport[3]) {
      RX_ERROR("Invalid viewport; can't draw the graph.");
    }

    max_bars = viewport[2] * 0.18;
    painter_fg.fill();
    painter_bg.fill();
  }

  Timer::~Timer() {

    for (size_t i = 0; i < queries.size(); ++i) {
      glDeleteQueries(1, &queries[i]->id);
      delete queries[i];
    }

    queries.clear();
    history.clear();
    max_vals.clear();
    is_started = false;
    max_bars = 0;
    max_val = 0;
  }

  TimerQuery* Timer::getFreeQueryObject() {
    for (size_t i = 0; i < queries.size(); ++i) {
      if (queries[i]->is_free) {
        return queries[i];
      }
    }
    return NULL;
  }

  void Timer::resetQueries() {
    for (size_t i = 0; i < queries.size(); ++i) {
      queries[i]->reset();
    }
  }

  void Timer::start(std::string name) {

     /* validate */
    if (false == GL_ARB_timer_query) {
      RX_ERROR("Timer query not supported.");
      return;
    }
    
    /* get an existing query object, or create a new one. */
    TimerQuery* tq = getFreeQueryObject();
    if (NULL == tq) {
      tq = new TimerQuery();
      glGenQueries(1, &tq->id);
      queries.push_back(tq);
    }

    tq->name = name;
    tq->is_free = false;
    is_started = true;

    glBeginQuery(GL_TIME_ELAPSED, tq->id);
  }

  /* Does a stop/start */
  void Timer::tag(std::string name) {          

    if (false == is_started) {
      start(name);
    }
    else {
      stop();
      start(name);
    }
  }

  void Timer::stop() {

    /* validate */
    if (false == GL_ARB_timer_query) {
      RX_ERROR("Timer query not supported.");
    }

    if (false == is_started) {
      RX_ERROR("Cannot stop because we didn't start timing yet.");
      return;
    }

    glEndQuery(GL_TIME_ELAPSED);

    is_started = false;
  }

  void Timer::draw() {

    /* validate */
    if (false == GL_ARB_timer_query) {
      RX_ERROR("Timer query not supported.");
    }

    /* update the current values. */
    update();

    painter_bg.clear();
    painter_fg.clear();
    font.clear();
    font.color(1,1,1,1);

    /* big background */
    painter_bg.color(0.0, 0.03, 0.1, 0.95);
    painter_bg.rect(0, 0, viewport[2], viewport[3]);
    
    /* used to position */
    float x = 0;
    float y = 0;
    float bar_w = float( (viewport[2] - max_bars) ) / max_bars;
    float max_bar_h = 50;
    float val = 0;
    char buf[128] = { 0 };

    /* And draw :) */
    std::map<std::string, std::deque<double> >::iterator it = history.begin();
    while (it != history.end()) {

      std::string name = it->first;
      std::deque<double>& vals = it->second;
      double max_tq_val = max_vals[name];

      /* background */
      painter_bg.color(0.0f, 0.0f, 0.1f, 0.95f);
      painter_bg.rect(0, y, viewport[2], max_bar_h + 50);
      y += 25;

      /* write max value */
      sprintf(buf, "%03.06f ms.", max_tq_val);
      font.color(1,1,1,1);
      font.write(5, y - 22, buf);

      /* lines */
      painter_fg.color(0.0f, 1.0f, 0.0f, 0.6f); /* top line */
      painter_fg.rect(0, y + max_bar_h - 1, viewport[2], 1); // y + max_bar_h);
      painter_fg.color(1.0f, 0.0f, 0.0f, 0.6f); /* bottom line */
      painter_fg.rect(0, y - 3, viewport[2], 3);
      painter_bg.color(0.3f, 0.1f, 0.8f, 0.8f);      
      

      /* bars */
      for (size_t i = 0; i < vals.size(); ++i) {

        /* make sure we never overshoot ^.^ */
        val = vals[i] / max_val; 
        if (val > 1.0) {
          val = 1.0;
        }

        painter_bg.rect(x + i * 1, y + max_bar_h, bar_w, - (val * max_bar_h) );
        x += bar_w;
      }

      /* name of the bar. */
      font.color(1.0f, 1.0f, 1.0f, 0.8f);
      font.write(5, y + max_bar_h + 3, name);
      y += max_bar_h + 50;
      x = 0;
      ++it;
    }

    painter_bg.draw();
    painter_fg.draw();
    font.draw();
  }

  void Timer::update() { 

    /* validate */
    if (false == GL_ARB_timer_query) {
      RX_ERROR("Timer query not supported.");
    }

    /* just make sure we're stopped */
    if (is_started) {
      stop();
    }

    GLint available = 0;
    for (size_t i = 0; i < queries.size(); ++i) {

      /* @todo - we could use a couple of query objects per 'tag' to make sure its ready. */
      /* wait until result becomes available - THIS IS VERY TIME CONSUMING - */
      TimerQuery* q = queries[i];
      while (!available) {
        glGetQueryObjectiv(q->id, GL_QUERY_RESULT_AVAILABLE, &available);
      }

      glGetQueryObjectui64v(q->id, GL_QUERY_RESULT, &q->timestamp);
      available = 0;

      /* store the results in our history. */
      double millis = double(q->timestamp)/1000000.0;
      history[q->name].push_back(millis);
      if (history[q->name].size() > max_bars) {
        history[q->name].erase(history[q->name].begin());
      }

      /* keep track of the max values per name */
      if (millis > max_val) {
        max_val = millis;
      }

      std::map<std::string, double>::iterator it = max_vals.find(q->name);
      if (it == max_vals.end()) {
        max_vals[q->name] = 0.0;
      }
      else {
        if (max_vals[q->name] < millis) {
          max_vals[q->name] = millis;
        }
      }
    }
    
    /* and reset the query objects again. */
    resetQueries();
  }

  void Timer::reset() {
    resetQueries();
  }

} /* namespace gfx */
