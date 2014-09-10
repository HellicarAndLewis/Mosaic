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
#include <topshop/GridAppSimple.h>
#include <gfx/Image.h>

void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

static int sort_monitors(GLFWmonitor* a, GLFWmonitor* b);

int main() {

#if defined(APP_GRID_LEFT)  
  rx_log_init(rx_to_data_path("log_left"));
#elif defined(APP_GRID_RIGHT)
  rx_log_init(rx_to_data_path("log_right"));
#else
  #error "No APP_GRID_LEFT or APP_GRID_RIGHT defined in CFLAGS"
#endif
 
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

  //  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  GLFWwindow* win = NULL;
  int w = top::config.grid_win_width;
  int h = top::config.grid_win_height;
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
    RX_ERROR("The used window size is not the same as you asked for (%d x %d), we're using %d x %d. This is probably because you're not running fullscreen on Mac", w, h, used_w, used_h);
    exit(EXIT_FAILURE);
  }

  int r = 0;
  std::string watch_dir;

#if defined(APP_GRID_LEFT)   /* left screen, scrolls to the right */

  grid::SimpleSettings cfg;
  cfg.img_width = top::config.grid_file_width;
  cfg.img_height = top::config.grid_file_height;
  cfg.rows = top::config.grid_rows;
  cfg.cols = top::config.grid_cols;
  cfg.padding_x = top::config.grid_padding_x;
  cfg.padding_y = top::config.grid_padding_y;
  cfg.offset_x = top::config.left_grid_x;
  cfg.offset_y = top::config.left_grid_y;
  cfg.direction = grid::SIMPLE_GRID_DIRECTION_RIGHT;
  cfg.watch_dir = top::config.raw_left_grid_filepath;
  cfg.image_dir = top::config.left_grid_filepath;
  
  RX_VERBOSE("%d %d", cfg.img_width, cfg.img_height);
  RX_VERBOSE("cols: %d, %d", cfg.cols, cfg.rows);
  top::GridAppSimple app;

#elif defined(APP_GRID_RIGHT)

#endif

  r = app.init(cfg);
  if (0 != r) {
    RX_ERROR("Cannot start the grid app");
    exit(EXIT_FAILURE);
  }

  /* load the header image */
  gfx::Image img_header;
  r = img_header.load(rx_to_data_path("assets/instagram_header.png"));
  if (0 != r) {
    RX_ERROR("Cannot find data/assets/instagram_header.png");
    exit(EXIT_FAILURE);
  }
  if (0 != img_header.createTexture()) {
    RX_ERROR("Cannot create texture for instagram header");
    exit(EXIT_FAILURE);
  }
  if (0 != img_header.updateTexture()) {
    RX_ERROR("Cannot update the instagram header texture");
    exit(EXIT_FAILURE);
  }

  Painter painter;

  while(!glfwWindowShouldClose(win)) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* draw header. */
    painter.clear();
    painter.texture(img_header.texid, 0, 0, img_header.width, img_header.height);
    painter.draw();
    
    app.update();
    app.draw();

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  app.shutdown();

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
