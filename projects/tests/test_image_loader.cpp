/*
 
  BASIC GLFW + GLXW WINDOW AND OPENGL SETUP 
  ------------------------------------------
  See https://gist.github.com/roxlu/6698180 for the latest version of the example.
   
  We make use of the GLAD library for GL loading, see: https://github.com/Dav1dde/glad/

  ------------------------------------------------------------------------------------


  test_image_loader
  ------------------

  Testing the Mosaic/ImageLoader class. This class will watch a directory and if it
  detects a new file it will load this in a separate thread. This example displays 
  the last image it found in the bin/data/test directory.
 
  ------------------------------------------------------------------------------------
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gfx/Timer.h>

#define ROXLU_IMPLEMENTATION
#define ROXLU_USE_LOG
#define ROXLU_USE_JPG
#define ROXLU_USE_PNG
#define ROXLU_USE_MATH
#define ROXLU_USE_FONT
#define ROXLU_USE_OPENGL
#include <tinylib.h>

#include <mosaic/DirWatcher.h>
#include <mosaic/ImageLoader.h>

void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

static void on_dir_change(std::string dir, std::string filename, void* user);
static void on_loaded(mos::ImageTask* img, void* user);

unsigned char* pixels = NULL;
int loaded_bytes = 0;
bool must_update = false;
bool must_recreate = false;
GLuint tex_width = 0;
GLuint tex_height = 0;
GLuint tex_channels = 0;
GLuint tex_id = 0;
 
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
 
  win = glfwCreateWindow(w, h, "~`` Image Loader - Copy Images Into data/test/ ``~", NULL, NULL);
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

  mos::ImageLoader loader;
  if (0 != loader.init()) {
    exit(EXIT_FAILURE);
  }
  loader.on_loaded = on_loaded;

  mos::DirWatcher watcher;
  if (0 != watcher.init(rx_to_data_path("test"), on_dir_change, &loader)) {
    exit(EXIT_FAILURE);
  }

  Painter painter;

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    watcher.update();

    GLenum fmt = GL_NONE;
    if (must_recreate || must_update) {
      if (1 == tex_channels) {
        fmt = GL_RED;
      }
      else if (2 == tex_channels) {
        fmt = GL_RG;
      }
      else if (3 == tex_channels) {
        fmt = GL_RGB;
      }
      else if (4 == tex_channels) {
        fmt = GL_RGBA;
      }
      else {
        RX_ERROR("Unsupported number of channels: %d", tex_channels);
      }
    }

    if (must_recreate) {
      RX_VERBOSE("Create texture, width: %d, height: %d, channels: %d", tex_width, tex_height, tex_channels);
      if (0 != tex_id) {
        glDeleteTextures(1, &tex_id);
      }

      if (GL_NONE != fmt) {
        glGenTextures(1, &tex_id);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, fmt, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      }
      must_recreate = false;
      must_update = false;
    }

    if (must_update && GL_NONE != fmt) {
      RX_VERBOSE("Updating pixels");
      glBindTexture(GL_TEXTURE_2D, tex_id);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, tex_width, tex_height, 0, fmt, GL_UNSIGNED_BYTE, pixels);
      must_update = false;
    }

    if (0 != tex_id) {
      int x = MAX(0, (w/2) - (tex_width/2));
      int y = 0;

      painter.clear();
      painter.texture(tex_id, x, y, tex_width, tex_height);
      painter.draw();
    }

    glfwSwapBuffers(win);
    glfwPollEvents();
  }
 
  loader.shutdown();

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

static void on_loaded(mos::ImageTask* img, void* user) {

  if (0 == img->nbytes) {
    RX_ERROR("The laoded image has no bytes?");
    return;
  }

  if (NULL == pixels) {
    pixels = (unsigned char*)malloc(img->nbytes);
    loaded_bytes = img->nbytes;
  }
  else {
    if (img->nbytes > loaded_bytes) {
      unsigned char* tmp = (unsigned char*)realloc(pixels, img->nbytes);
      if (tmp == NULL) {
        RX_ERROR("Cannot realloc");
        return;
      }

      pixels = tmp;
      loaded_bytes = img->nbytes;
    }
  }

  if (img->width != tex_width || img->height != tex_height) {
    must_recreate = true;
    tex_width = img->width;
    tex_height = img->height;
    tex_channels = img->channels;
  }

  memcpy(pixels, img->pixels, img->nbytes);
  must_update = true;
}

static void on_dir_change(std::string dir, std::string filename, void* user) {

  /* get a handle */
  mos::ImageLoader* loader = static_cast<mos::ImageLoader*>(user);
  if (NULL == loader) {
    RX_ERROR("Invalid user pointer.");
    return;
  }
  
  if (0 != loader->load(dir +"/" +filename)) {
    RX_ERROR("Failed to load file.");
  }
}
