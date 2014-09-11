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
#define ROXLU_USE_CURL
#include <tinylib.h>

#define VIDEO_CAPTURE_USE_APPLE_RGB_422
#define VIDEO_CAPTURE_IMPLEMENTATION
#include <videocapture/CaptureGL.h>

#define RXP_PLAYER_GL_IMPLEMENTATION
#include <rxp_player/PlayerGL.h>
 
#include <topshop/Config.h>
#include <topshop/TopShop.h>

void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

static int sort_monitors(GLFWmonitor* a, GLFWmonitor* b);

int main() {

  rx_log_init(rx_to_data_path("log_mosaic"));
 
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

  /* --------------------------------------------------------------------- */

  /* Find monitors */
  std::vector<GLFWmonitor*> monitor_list;
  int count = 0;
  GLFWmonitor** monitors = glfwGetMonitors(&count);
  int monitor_x;
  int monitor_y;

  if (NULL == monitors) {
    RX_ERROR("Monitors == NULL.");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < count; ++i) {
    if (NULL == monitors[i]) {
      RX_ERROR("The returned monitor list is invalid.");
      exit(EXIT_FAILURE);
    }
    monitor_list.push_back(monitors[i]);
  }

  std::sort(monitor_list.begin(), monitor_list.end(), sort_monitors);
  if (0 == monitor_list.size()) {
    RX_ERROR("monitor list is invalid. not supposed to happen.");
    exit(EXIT_FAILURE);
  }

  GLFWmonitor* monitor = NULL;
  if (top::config.mosaic_monitor >= monitor_list.size()) {
    RX_ERROR("Invalid monitor number; we only have: %lu monitors.", monitor_list.size());
    exit(EXIT_FAILURE);
  }
  
  monitor = monitor_list[top::config.mosaic_monitor];
  if (NULL == monitor) {
    RX_ERROR("No monitor selected");
    exit(EXIT_FAILURE);
  }

  glfwGetMonitorPos(monitor, &monitor_x, &monitor_y);

  /* --------------------------------------------------------------------- */
 
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  GLFWwindow* win = NULL;
  int w = top::config.mosaic_win_width;
  int h = top::config.mosaic_win_height;
  int used_w = 0;
  int used_h = 0;
 
  if (1 == top::config.is_fullscreen) {
   win = glfwCreateWindow(w, h, "TopShop", monitor, NULL);
  }
  else {
    win = glfwCreateWindow(w, h, "TopShop", NULL, NULL);
    glfwSetWindowPos(win, monitor_x, 0);
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

  if (!GL_APPLE_rgb_422) {
    RX_ERROR("It seems that the APPLE_RGB_422 extension is not loaded. Did you enable it with GLAD?");
    exit(1);
  }

  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------

  glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

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
    case GLFW_KEY_D: {
      top::config.is_debug_draw = (1 == top::config.is_debug_draw) ? 0 : 1;
      break;
    }
    case GLFW_KEY_M: {
      static bool show = false;
      glfwSetInputMode(win, GLFW_CURSOR, (show) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
      show = !show;
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

static int sort_monitors(GLFWmonitor* a, GLFWmonitor* b) {

  if (NULL == a || NULL == b) {
    RX_ERROR("sort monitors received an invalid monitor pointer.");
    return 0;
  }

  int xa = 0, xb = 0;
  int ya = 0, yb = 0;

  glfwGetMonitorPos(a, &xa, &ya);
  glfwGetMonitorPos(b, &xb, &yb);

  return xa < xb;
}
