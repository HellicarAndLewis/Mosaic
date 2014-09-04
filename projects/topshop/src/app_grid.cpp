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

#include <mosaic/Config.h>
#include <topshop/Config.h>
#include <topshop/GridApp.h>
#include <grid/Grid.h>

void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

static int sort_monitors(GLFWmonitor* a, GLFWmonitor* b);

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

  for (int i = 0; i < monitor_list.size(); ++i) {
    int xa, ya;
    glfwGetMonitorPos(monitor_list[i], &xa, &ya);
    RX_VERBOSE("Monitor %d: %d x %d", i, xa, ya);
  }

#if defined(APP_GRID_LEFT)

  if (top::config.grid_left_monitor >= monitor_list.size()) {
    RX_ERROR("The left monitor setting is invalid.");
    exit(EXIT_FAILURE);
  }
  monitor = monitor_list[top::config.grid_left_monitor];

#elif defined(APP_GRID_RIGHT)

  if (top::config.grid_right_monitor >= monitor_list.size()) {
    RX_ERROR("The right monitor setting is invalid.");
    exit(EXIT_FAILURE);
  }
  monitor = monitor_list[top::config.grid_right_monitor];

  
#else
# error Unsupported direction
#endif

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
  int w = 1920;
  int h = 1080;
  int used_w = 0;
  int used_h = 0;
 
  if (1 == top::config.is_fullscreen) {
   win = glfwCreateWindow(w, h, "AppGrid", monitor, NULL);
  }
  else {
    win = glfwCreateWindow(w, h, "AppGrid", NULL, NULL);
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

  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------

  const GLubyte* renderer = glGetString(GL_RENDERER);
  if (NULL == renderer) {
    RX_ERROR("Cannot get the renderer type: %s", renderer);
  }
  else {
    RX_VERBOSE("Renderer: %s", renderer);
  }

  /* make sure the created window has the resolution we want it to be */
  glfwGetWindowSize(win, &used_w, &used_h);
  if (used_w != w || used_h != h) {
    RX_VERBOSE("The used window size is not the same as you asked for (%d x %d), we're using %d x %d. This is probably because you're not running fullscreen on Mac", w, h, used_w, used_h);
    exit(EXIT_FAILURE);
  }

  int r = 0;
  mos::config.window_width = w;
  mos::config.window_height = h;

#if defined(APP_GRID_LEFT)  

  top::GridAppSettings cfg;
  cfg.image_path = top::config.left_grid_filepath;
  cfg.watch_path = top::config.raw_left_grid_filepath;
  cfg.image_width = top::config.grid_file_width;
  cfg.image_height = top::config.grid_file_height;
  cfg.grid_rows = top::config.grid_rows;
  cfg.grid_cols = top::config.grid_cols;

  top::GridApp app(GRID_DIR_RIGHT);
  app.grid.offset.set(top::config.right_grid_x, top::config.right_grid_y);
  app.grid.padding.set(top::config.grid_padding_x, top::config.grid_padding_y);

#elif defined(APP_GRID_RIGHT)

  top::GridAppSettings cfg;
  cfg.image_path = top::config.right_grid_filepath;
  cfg.watch_path = top::config.raw_right_grid_filepath;
  cfg.image_width = top::config.grid_file_width;
  cfg.image_height = top::config.grid_file_height;
  cfg.grid_rows = top::config.grid_rows;
  cfg.grid_cols = top::config.grid_cols;

  top::GridApp app(GRID_DIR_LEFT); 
  app.grid.offset.set(top::config.left_grid_x, top::config.left_grid_y);
  app.grid.padding.set(top::config.grid_padding_x, top::config.grid_padding_y);

#endif

  r = app.init(cfg);
  if (r != 0) {
    RX_ERROR("Cannot start the grid app");
    exit(EXIT_FAILURE);
  }



  /*
  r = app_grid.init(top::config.left_grid_filepath, 
                     top::config.grid_file_width, 
                     top::config.grid_file_height, 
                     top::config.grid_rows, 
                     top::config.grid_cols);

  app_grid.offset.set(top::config.left_grid_x, top::config.left_grid_y);
  app_grid.padding.set(top::config.grid_padding_x, top::config.grid_padding_y);
  */

  while(!glfwWindowShouldClose(win)) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    app.update();
    app.draw();

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

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
