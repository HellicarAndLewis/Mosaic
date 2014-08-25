/*
 
  BASIC GLFW + GLXW WINDOW AND OPENGL SETUP 
  ------------------------------------------
  See https://gist.github.com/roxlu/6698180 for the latest version of the example.
   
  We make use of the GLAD library for GL loading, see: https://github.com/Dav1dde/glad/

  ----------

  This test connects to an rtmp stream and starts decoding the frames in a separate
  thread. Once a frame is decoded it will be upload to the GPU and played back 
  using the YUV420P class. 

  ----------

  You can use libav/avconv to create a RTMP stream using:
  -
      ./avconv -re -i yourvideo.mov -an -v debug -f flv -listen 1 rtmp://localhost/
  -

  
 
*/
#include <stdlib.h>
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <video/Player.h>
#include <video/Stream.h>
#include <video/YUV420P.h>
#include <gfx/Timer.h>

#define ROXLU_IMPLEMENTATION
#define ROXLU_USE_LOG
#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#include <tinylib.h>


#define USE_TIMER 0
 
void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

static void on_video_frame(AVFrame* frame, void* user);

vid::Player* player_ptr = NULL;

#if USE_TIMER
  bool draw_timer = false;
  gfx::Timer* timer_ptr = NULL;
#endif
 
int main() {
 
  glfwSetErrorCallback(error_callback);
 
  if(!glfwInit()) {
    printf("Error: cannot setup glfw.\n");
    exit(EXIT_FAILURE);
  }
 
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  GLFWwindow* win = NULL;
  int w = 1280;
  int h = 720;
 
  win = glfwCreateWindow(w, h, "Video Stream Player Test - Using LibAV", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
 
  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwSetCharCallback(win, char_callback);
  glfwSetCursorPosCallback(win, cursor_callback);
  glfwSetMouseButtonCallback(win, button_callback);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  if (!gladLoadGL()) {
    printf("Cannot load GL.\n");
    exit(1);
  }

  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------
  rx_log_init();

  vid::Player player;
  player_ptr = &player;
  player.on_frame = on_video_frame;
  if (0 != player.init("rtmp://edge01.fms.dutchview.nl/botr/bunny.flv")) {
  //if (0 != player.init("rtmp://localhost")) {
    exit(0);
  }

#if USE_TIMER 
  gfx::Timer timer;
  timer_ptr = &timer;
#endif

  vid::YUV420P yuv;
  player.user = (void*)&yuv;

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (0 != player.update()) {
      RX_VERBOSE("player.update() failed.");
    }

#if USE_TIMER
    timer.start("draw");
#endif

    yuv.draw();

#if USE_TIMER
    timer.stop();
    if (draw_timer) {
      timer.draw();
    }
#endif

    glfwSwapBuffers(win);
    glfwPollEvents();
  }
 
  RX_VERBOSE("Stopping");
  player.shutdown();

  glfwTerminate();
 
  return EXIT_SUCCESS;
}

void char_callback(GLFWwindow* win, unsigned int key) {
}
 
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
  
  if(action != GLFW_PRESS) {
    return;
  }
 
  switch(key) {
    case GLFW_KEY_SPACE: {
#if USE_TIMER
      draw_timer = !draw_timer;
#endif
      break;
    }
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
    case GLFW_KEY_S: {
      if (NULL != player_ptr) {
        player_ptr->stop();
      }
      break;
    }
    case GLFW_KEY_P: {
      if (NULL != player_ptr) {
        player_ptr->play();
      }
      break;
    }
  };
}
 
void resize_callback(GLFWwindow* window, int width, int height) {
}
 
void cursor_callback(GLFWwindow* win, double x, double y) {
}
 
void button_callback(GLFWwindow* win, int bt, int action, int mods) {
  /*
    if(action == GLFW_PRESS) { 
    }
  */
}
 
void error_callback(int err, const char* desc) {
  printf("GLFW error: %s (%d)\n", desc, err);
}


/* ------------------------------------------------------------- */
static void on_video_frame(AVFrame* frame, void* user) {
  if (NULL == frame) { return; } 

  if (0 == frame->width) {
    RX_ERROR("Invalid frame width");
    return;
  }

  if (0 == frame->height) {
    RX_ERROR("Invalid frame height");
    return;
  }

  /* update yuv. */
  vid::YUV420P* yuv = static_cast<vid::YUV420P*>(user);
  if (NULL == yuv) {
    RX_ERROR("Invalid yuv420p pointer!");
    return ;
  }

  /* initialize yuv when not done yet. */
  if (0 == yuv->w) {
    RX_VERBOSE("Initialize the YUV420P shader.");
    if (0 != yuv->init(frame->width, frame->height)) {
      RX_ERROR("Cannot initialize the yuv decoder");
      exit(1);
    }
  }

#if USE_TIMER
  timer_ptr->start("upload");
#endif

  yuv->update(frame->data[0], frame->linesize[0],
              frame->data[1], frame->linesize[1],
              frame->data[2], frame->linesize[2]);

#if USE_TIMER
  timer_ptr->stop();
#endif
}
