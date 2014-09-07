/*
 
  BASIC GLFW + GLXW WINDOW AND OPENGL SETUP 
  ------------------------------------------
  See https://gist.github.com/roxlu/6698180 for the latest version of the example.
   
  We make use of the GLAD library for GL loading, see: https://github.com/Dav1dde/glad/
 
*/
#include <stdlib.h>
#include <stdio.h>
 
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define USE_POLAROID 1

#define ROXLU_IMPLEMENTATION
#define ROXLU_USE_LOG
#define ROXLU_USE_JPG
#define ROXLU_USE_PNG
#define ROXLU_USE_MATH
#define ROXLU_USE_FONT
#define ROXLU_USE_OPENGL
#include <tinylib.h>

#define VIDEO_CAPTURE_USE_APPLE_RGB_422 
#define VIDEO_CAPTURE_IMPLEMENTATION
#include <videocapture/CaptureGL.h>  

#include <featurex/Config.h>
#include <mosaic/Config.h>
#include <topshop/Config.h>
#include <tracking/Tracking.h>

void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

track::Tracking* tracking_ptr = NULL;
std::vector<std::string> image_files;
size_t image_index = 0;
int win_w;
int win_h;

int main() {

  rx_log_init();
 
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
  win_w = w;
  win_h = h;
 
  win = glfwCreateWindow(w, h, "//_ - tracker test - //__ ", NULL, NULL);
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

  std::string files_path = rx_get_exe_path() +"/../data/input_mosaic/";
  image_files = rx_get_files(files_path, "png");
  if (0 == image_files.size()) {
    RX_ERROR("Cannot find the image files: %lu", image_files.size());
    ::exit(1);
  }

  mos::config.webcam_width = 320;
  mos::config.webcam_height = 240;
  fex::config.rows = 4;
  fex::config.cols = 10;

  track::Tracking tracking;
  tracking_ptr = &tracking;

  if (0 != tracking.init(0, mos::config.webcam_width, mos::config.webcam_height)) {
    exit(EXIT_FAILURE);
  }

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    tracking.update();
    tracking.draw();

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  tracking.shutdown();
  
  glfwTerminate();
 
  return EXIT_SUCCESS;
}

void char_callback(GLFWwindow* win, unsigned int key) {
}
 
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
  
  if(action != GLFW_PRESS) {
    return;
  }

  if (NULL == tracking_ptr) {
    RX_ERROR("Ignoring key callback because the tracking ptr is invalid.");
    return;
  }

  switch(key) {
    case GLFW_KEY_SPACE: {

#if USE_POLAROID
      std::string filepath = rx_get_exe_path() +"/../data/test/polaroid.png";
      //  std::string filepath = rx_get_exe_path() +"/../data/test/polaroid_300x256.png";
      if (false == rx_file_exists(filepath)) {
        RX_ERROR("Cannot find polaroid file: %s", filepath.c_str());
        return;
      }

      track::ImageOptions opt;
      opt.filepath = filepath;
      opt.x = rx_random(0, 400);
      opt.y = rx_random(0, 400);
      opt.mode = track::IMAGE_MODE_FLY;
      opt.tween_x.set(1.0f, win_w * 0.5, rx_random(-600, 600));
      opt.tween_y.set(1.0f, win_h, rx_random(-300, -600));
      opt.tween_angle.set(1.0f, 0.0, rx_random(-HALF_PI, HALF_PI));
      opt.tween_size.set(1.0f, 0.0f, 200.0f);

      tracking_ptr->tiles.load(opt);

#else
      if (image_index >= image_files.size()) {
        RX_ERROR("invalid image index.");
        return;
      }
      track::ImageOptions opt;
      opt.filepath = image_files[image_index];
      opt.x = rx_random(0, 400);
      opt.y = rx_random(0, 400);
      opt.mode = track::IMAGE_MODE_BOINK;
      opt.mode = track::IMAGE_MODE_FLY;
      opt.tween_x.set(1.0f, 0.0f, rx_random(0, 600));
      opt.tween_y.set(1.0f, 0.0f, rx_random(0, 600));
      opt.tween_angle.set(1.0f, 0.0, rx_random(0, HALF_PI));
      opt.tween_size.set(1.0f, 0.0f, 100.0f);

      tracking_ptr->tiles.load(opt);
      ++image_index %= image_files.size();
#endif
      break;
      
    }
    case GLFW_KEY_P: {

      
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

