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
 
#include <featurex/Config.h>
#include <mosaic/Mosaic.h>
#include <mosaic/Config.h>

void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

static void sigh(int s);

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
 
  win = glfwCreateWindow(w, h, ">>> Mosaic Tester <<<", NULL, NULL);
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

  signal(SIGPIPE, sigh);

  rx_log_init();
  mos::config.window_width = w;
  mos::config.window_height = h;

#if 0
  /* mosaic settings. */
  mos::config.webcam_device = 0;
  mos::config.webcam_width = 640;
  mos::config.webcam_height = 480;

  /* feature extractor settings. */
  fex::config.raw_filepath = rx_to_data_path("input_raw/");
  fex::config.resized_filepath = rx_to_data_path("input_resized/");
  fex::config.blurred_filepath = rx_to_data_path("input_blurred/");
  fex::config.input_tile_size = 16;
  fex::config.input_image_width = mos::config.webcam_width;
  fex::config.input_image_height = mos::config.webcam_height;
  fex::config.cols = (fex::config.input_image_width / fex::config.input_tile_size);
  fex::config.rows = (fex::config.input_image_height / fex::config.input_tile_size);
  fex::config.show_timer = false;
  fex::config.file_tile_width = 64;
  fex::config.file_tile_height = 64;
  fex::config.memory_pool_size = 1000;
#endif

  mos::Mosaic mosaic;
  if (0 != mosaic.init()) {
    RX_ERROR("Cannot start the mosaic, check error messages");
    exit(EXIT_FAILURE);
  }

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double n = rx_hrtime();
    mosaic.update();
    mosaic.draw();
    double dt = double(rx_hrtime() - n) / 1e9;
    //RX_VERBOSE("FRAME: %f", dt);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  mosaic.shutdown();

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
      fex::config.show_timer = !fex::config.show_timer;
      break;
    }
    case GLFW_KEY_S: {
      break;
    }
    case GLFW_KEY_L: {
      break;
    }
    case GLFW_KEY_T: {
      break;
    }
    case GLFW_KEY_1: {
      break;
    }
    case GLFW_KEY_2: {
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
  /*
    if(action == GLFW_PRESS) { 
    }
  */
}
 
void error_callback(int err, const char* desc) {
  printf("GLFW error: %s (%d)\n", desc, err);
}

static void sigh(int s) {
  RX_VERBOSE("GOT SIGNAL!");
}
