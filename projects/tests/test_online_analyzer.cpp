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

#define ROXLU_USE_LOG
#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#define ROXLU_USE_FONT
#define ROXLU_USE_PNG
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

#define VIDEO_CAPTURE_IMPLEMENTATION
#include <videocapture/CaptureGL.h>
#include <gfx/FBO.h>
#include <gfx/AsyncDownload.h>
#include <gfx/Timer.h>
#include <featurex/AnalyzerGPU.h>
#include <featurex/Config.h>

#define USE_TIMER 0

void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);
 
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
  int w = 640;
  int h = 480;
 
  win = glfwCreateWindow(w, h, "Generating tile descriptors on GPU", NULL, NULL);
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

  ca::CaptureGL capture;
  std::vector<ca::Device> devices = capture.cap.getDevices();
  if (0 == devices.size()) {
    RX_ERROR("Cannot find any capture devices");
    exit(1);
  }

  /* find the logitech c920 device. */
  int c920_device = -1;
  for (size_t i = 0; i < devices.size(); ++i) {
    ca::Device& dev = devices[i];
    if (dev.name.find("C920") != std::string::npos) {
      c920_device = dev.index;
      break;
    }
  }
  if (-1 == c920_device) {
    RX_ERROR("No Logitech C920 found. This library is optimized for C920 specifically, so please get one.");
    exit(1);
  }
  
  /* open capture. */
  if (capture.open(c920_device, 640, 480) < 0) {
    RX_ERROR("Cannot open capture device.");
    exit(1);
  }

  if (capture.start() < 0) {
    RX_ERROR("Cannot start the capture device.");
    exit(1);
  }

  /* setup FBO to capture webcam */

  GLuint webcam_tex;
  gfx::FBO fbo;
  if (0 != fbo.init(w, h)) {
    RX_ERROR("Cannot initialize the FBO.");
    exit(1);
  }

  webcam_tex = fbo.addTexture(GL_RGBA8, w, h, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);
  if (0 != fbo.isComplete()) {
    RX_ERROR("FBO not complete");
    exit(1);
  }

  /* make sure to set the mosaic info before initializing the gpu analyzer */
  fex::config.cols = 20;
  fex::config.rows = 15;
  fex::config.input_image_width = capture.width;
  fex::config.input_image_height = capture.height;
  fex::config.tile_size = 32;

  /* create the GPU analyzer */
  fex::AnalyzerGPU analyzer;
  if (0 != analyzer.init(webcam_tex)) {
    RX_ERROR("Cannot initialize the GPU analyzer.");
    exit(1);
  }

  /* testing performance with async download */
#if 0
  gfx::AsyncDownload download;
  if (0 != download.init(64, 64, GL_BGRA)) {
    RX_ERROR("Cannot init async download.");
    exit(1);
  }
#endif

#if USE_TIMER
  gfx::Timer timer;
#endif

  double next_sec = 0;

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if USE_TIMER
    timer.start("analyze");
#endif

    capture.update();

    /* render the webcam into a texture that we feed into the GPU analyzer. */
    fbo.bind();
    {
      capture.draw();
    }
    fbo.unbind();
    
    if (0 != analyzer.analyze()) {
      RX_ERROR("Failed to analyze.");
      exit(1);
    }

    analyzer.draw();
    
    capture.draw(0, 0, w >> 2, h >> 2);

#if USE_TIMER
    timer.stop();
    timer.start("download");
#endif

#if 0
    uint64_t n = rx_hrtime();
    glReadBuffer(GL_BACK_LEFT);
    if (0 != download.download()) {
      RX_VERBOSE("Cannot download");
    }
    uint64_t d = rx_hrtime() - n;
    double td = (double)d/(1000.0 * 1000.0 * 1000.0);
    RX_VERBOSE("Took: %llu, %f ms.", d, td);
    
    double sec = (double)n/(1000.0 * 1000.0 * 1000.0);
    if (sec > next_sec) {
      printf("SCREENSHOT!\n");
      rx_save_png(rx_get_time_string() +".png", download.buffer, w, h, 4);
      next_sec = sec + 1;
    }
#endif

#if USE_TIMER
    timer.stop();
    timer.draw();
#endif

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
