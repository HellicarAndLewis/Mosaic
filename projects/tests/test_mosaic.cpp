/*
 
  BASIC GLFW + GLXW WINDOW AND OPENGL SETUP 
  ------------------------------------------
  See https://gist.github.com/roxlu/6698180 for the latest version of the example.
   
  We make use of the GLAD library for GL loading, see: https://github.com/Dav1dde/glad/
 
*/
#include <stdlib.h>
#include <stdio.h>
#include <featurex/Featurex.h>
#include <featurex/Config.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gfx/Timer.h>

#define ROXLU_IMPLEMENTATION
#define ROXLU_USE_LOG
#define ROXLU_USE_JPG
#define ROXLU_USE_PNG
#define ROXLU_USE_MATH
#define ROXLU_USE_FONT
#include <tinylib.h>
 
void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

fex::Featurex* feat_ptr = NULL;
std::vector<std::string> test_images; /* is filled with a collection of images that are analyzed on the cpu */
size_t test_image_dx = 0;

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
 
  win = glfwCreateWindow(w, h, ">>> Tha Feature-X-Tractor <<<", NULL, NULL);
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

  fex::config.raw_filepath = rx_to_data_path("input_raw/");
  fex::config.resized_filepath = rx_to_data_path("input_resized/");
  fex::config.blurred_filepath = rx_to_data_path("input_blurred/");
  fex::config.tile_size = 32;
  fex::config.input_image_width = 384;
  fex::config.input_image_height = 384;
  fex::config.cols = (fex::config.input_image_width / fex::config.tile_size);
  fex::config.rows = (fex::config.input_image_height / fex::config.tile_size);
  fex::config.show_timer = false;

  fex::Featurex feat;
  feat_ptr = &feat;

  if (0 != feat.init()) {
    exit(1);
  }

  test_images = rx_get_files(fex::config.raw_filepath);
  RX_VERBOSE("Loaded %lu test images", test_images.size());

  // feat.loadInputImage(rx_to_data_path("test_input0.png"));
  // feat.loadInputImage(rx_to_data_path("test_input1.jpg"));
  // feat.loadInputImage(rx_to_data_path("test_input2.png"));
  // feat.loadInputImage(rx_to_data_path("test_input4.png"));
  // feat.loadInputImage(rx_to_data_path("test_input5.png"));

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    feat.update();
    feat.draw();

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
    case GLFW_KEY_SPACE: {
      fex::config.show_timer = !fex::config.show_timer;
      break;
    }
    case GLFW_KEY_S: {
      /* @todo = cleanup 
      if (0 != feat_ptr->cpu_analyzer.saveDescriptors()) {
        RX_ERROR("Cannot save the descriptors.");
      }
      */
      break;
    }
    case GLFW_KEY_L: {
      /* @todo = cleanup 
      if (0 != feat_ptr->cpu_analyzer.loadDescriptors()) {
        RX_ERROR("Cannot load the descriptors.");
      }
      */
      break;
    }
    case GLFW_KEY_T: {
      if (0 == test_images.size()) {
        RX_ERROR("Trying to load test images, but none loaded!");
        return;
      }
      std::string new_test_file = test_images[test_image_dx];
      RX_VERBOSE("Loading next test image: %s", new_test_file.c_str());
      ++test_image_dx %= test_images.size();
      break;
    }
    case GLFW_KEY_1: {
      /*
      fex::config.tile_size = fex::config.input_image_width;
      fex::config.cols = (fex::config.input_image_width / fex::config.tile_size);
      fex::config.rows = (fex::config.input_image_height / fex::config.tile_size);
      */
      break;
    }
    case GLFW_KEY_2: {
      /*
      fex::config.tile_size = 32;
      fex::config.cols = (fex::config.input_image_width / fex::config.tile_size);
      fex::config.rows = (fex::config.input_image_height / fex::config.tile_size);
      */
      break;
    }
    case GLFW_KEY_3: {
      /*
      fex::config.tile_size = 12;
      fex::config.cols = (fex::config.input_image_width / fex::config.tile_size);
      fex::config.rows = (fex::config.input_image_height / fex::config.tile_size);
      */
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
