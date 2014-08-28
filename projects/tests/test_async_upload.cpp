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

#define ROXLU_IMPLEMENTATION
#define ROXLU_USE_LOG
#define ROXLU_USE_JPG
#define ROXLU_USE_PNG
#define ROXLU_USE_MATH
#define ROXLU_USE_FONT
#define ROXLU_USE_OPENGL
#include <tinylib.h>

#include <gfx/AsyncUpload.h>
 
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
  int w = 1280;
  int h = 720;
 
  win = glfwCreateWindow(w, h, "<< Async Buffer Transfers >>", NULL, NULL);
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

  /* does the test image exist. */
  std::string test_image = rx_to_data_path("test/test_async_upload.png");
  if (false == rx_file_exists(test_image)) {
    RX_ERROR("Cannot find: %s that we want to use for the async test.", test_image.c_str());
    exit(0);
  }

  /* load the test image. */
  int width, height, channels, allocated = 0;
  unsigned char* pixels = NULL;
  int len = rx_load_png(test_image, &pixels, width, height, channels, &allocated);
  if (0 > len) {
    RX_ERROR("Cannot load %s, invalid image?", test_image.c_str());
  }
  if (0 == width || 0 == height || 0 == channels) {
    RX_ERROR("Width, height or channels is 0.");
    exit(EXIT_FAILURE);
  }

  RX_VERBOSE("Loaded: %s, width: %d, height: %d, channels: %d, bytes: %d", rx_strip_dir(test_image).c_str(), width, height, channels, allocated);

  /* determine the pixel format. */
  GLenum format = GL_RGBA8;
  if (3 == channels) {
    format = GL_RGB;
  }
  else if (4 == channels) {
    format = GL_RGBA;
  }
  else {
    RX_ERROR("Unsupported number of channels: %d", channels);
    exit(EXIT_FAILURE);
  }

  /* create texture into which we upload */
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  gfx::AsyncUpload async_upload;
  if (0 != async_upload.init(width, height, format)) {
    RX_ERROR("Cannot upload the async uploader");
    exit(EXIT_FAILURE);
  }

  Painter painter;

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double n = rx_hrtime();
    glBindTexture(GL_TEXTURE_2D, tex);
    async_upload.upload(pixels);
    double dt = ((double)(rx_hrtime()) - n) / (1000.0 * 1000.0 * 1000.0);
    RX_VERBOSE("Took: %f", dt);

    painter.clear();
    painter.texture(tex, 0, 0, w, h);
    painter.draw();
 
    glfwSwapBuffers(win);
    glfwPollEvents();
  }
 
  glfwTerminate();

  /* cleanup */
  free(pixels);
  pixels = NULL;
  width = 0;
  height = 0;
  channels = 0;
  allocated = 0;
 
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
