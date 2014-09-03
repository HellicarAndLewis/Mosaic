/*
 
  BASIC GLFW + GLXW WINDOW AND OPENGL SETUP 
  ------------------------------------------
  See https://gist.github.com/roxlu/6698180 for the latest version of the example.
   
  We make use of the GLAD library for GL loading, see: https://github.com/Dav1dde/glad/
 
*/
#include <stdlib.h>
#include <stdio.h>
#include <glad/glad.h>
#include <signal.h>
#include <GLFW/glfw3.h>

#define ROXLU_IMPLEMENTATION
#define ROXLU_USE_LOG
#define ROXLU_USE_JPG
#define ROXLU_USE_PNG
#define ROXLU_USE_MATH
#define ROXLU_USE_FONT
#define ROXLU_USE_OPENGL
#include <tinylib.h>

#define VIDEO_CAPTURE_IMPLEMENTATION
#include <videocapture/CaptureGL.h>

#define RXP_PLAYER_GL_IMPLEMENTATION
#include <rxp_player/PlayerGL.h>
 

#include <Topshop/Config.h>
#include <TopShop/TopShop.h>

void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

bool tmp_shutdown = false;
bool tmp_start = false;

int main() {

  rx_log_init();
 
  /* load all the configuration */
  if (0 != top::load_config()) {
    RX_ERROR("Cannot load config. stopping.");
    return -99;
  }

  rx_log_set_level(top::config.log_level);

  if (0 != top::config.validate()) {
    return -100;
  }



  /* --------------------------------------------------------------------- */

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
  int w = top::config.window_width;
  int h = top::config.window_height;
  int used_w = 0;
  int used_h = 0;
 
  if (1 == top::config.is_fullscreen) {
   win = glfwCreateWindow(w, h, "TopShop", glfwGetPrimaryMonitor(), NULL);
  }
  else {
    win = glfwCreateWindow(w, h, "TopShop", NULL, NULL);
  }
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

  /* make sure the created window has the resolution we want it to be */
  glfwGetWindowSize(win, &used_w, &used_h);
  if (used_w != w || used_h != h) {
    RX_VERBOSE("The used window size is not the same as you asked for (%d x %d), we're using %d x %d. This is probably because you're not running fullscreen on Mac", w, h, used_w, used_h);
    exit(EXIT_FAILURE);
  }

  mos::config.window_width = w;
  mos::config.window_height = h;
  top::TopShop topshop;

  if (0 != topshop.init()) {
    RX_ERROR("Cannot start the topshop application, check error messages");
    exit(EXIT_FAILURE);
  }

  while(!glfwWindowShouldClose(win)) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (tmp_shutdown) {
      tmp_shutdown = false;
      topshop.mosaic.video_input.backup_player.shutdown();
    }
    if (tmp_start) {
      topshop.mosaic.video_input.backup_player.init(rx_to_data_path("video/backup.ogg"));
      topshop.mosaic.video_input.backup_player.play();
      RX_VERBOSE("Restarted");
      tmp_start = false;
    }

    double n = rx_hrtime();
    topshop.update();
    topshop.draw();
    double dt = double(rx_hrtime() - n) / 1e9;
    //RX_VERBOSE("FRAME: %f", dt);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  topshop.shutdown();

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
     case GLFW_KEY_1: {
       //tmp_shutdown = true;
       break;
     }
    case GLFW_KEY_2: {
      // tmp_start = true;
      break;
    }
    case GLFW_KEY_D: {
      top::config.is_debug_draw = (1 == top::config.is_debug_draw) ? 0 : 1;
      break;
    }
    case GLFW_KEY_L: {
      break;
    }
    case GLFW_KEY_T: {
      break;
    }

    case GLFW_KEY_3: {
      break;
    }
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
  };
}
 
void resize_callback(GLFWwindow* window, int width, int height) {
}
 
void cursor_callback(GLFWwindow* win, double x, double y) {
}
 
void button_callback(GLFWwindow* win, int bt, int action, int mods) {
}
 
void error_callback(int err, const char* desc) {
  printf("GLFW error: %s (%d)\n", desc, err);
}

