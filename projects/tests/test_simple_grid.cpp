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

#include <grid/SimpleGrid.h>
#include <topshop/ImageCollector.h>
 
void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

static void on_new_file(top::ImageCollector* col, top::CollectedFile& file);
 
grid::SimpleGrid* grid_ptr = NULL;
top::ImageCollector img_collector;
std::vector<std::string> images;

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
  int w = 1920;
  int h = 1080;
 
  win = glfwCreateWindow(w, h, "++ --- SimpleGrid -- ++ ", NULL, NULL);
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

  grid::SimpleSettings cfg;
  cfg.img_width = 200;
  cfg.img_height = 200;
  cfg.cols = 10;
  cfg.rows = 5;
  cfg.padding_x = 10;
  cfg.padding_y = 10;

  grid::SimpleGrid gr;
  grid_ptr = &gr;

  if (0 != gr.init(cfg)) {
    RX_ERROR("Cannot init simple grid.");
    exit(1);
  }

  std::string path = rx_get_exe_path() +"../data/input_grid_left/";
  RX_VERBOSE("Watching: %s", path.c_str());
  if (0 != img_collector.init(path)) {
    RX_ERROR("Cannot init collector");
    exit(1);
  }

  img_collector.on_file = on_new_file;
  img_collector.user = &gr;

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    img_collector.update();
    gr.update(0.16);
    gr.draw();

    glfwSwapBuffers(win);
    glfwPollEvents();
  }
  
  img_collector.shutdown();
  gr.shutdown();
 
  glfwTerminate();
 
  return EXIT_SUCCESS;
}

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
  
  if(action != GLFW_PRESS) {
    return;
  }

  if (NULL == grid_ptr) {
    return;
  }

  switch(key) {
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
  };
}

void char_callback(GLFWwindow* win, unsigned int key) {}
void resize_callback(GLFWwindow* window, int width, int height) {}
void cursor_callback(GLFWwindow* win, double x, double y) {}
void button_callback(GLFWwindow* win, int bt, int action, int mods) {}
void error_callback(int err, const char* desc) {  printf("GLFW error: %s (%d)\n", desc, err); }

/* ------------------------------------------------------------------------- */

static void on_new_file(top::ImageCollector* col, top::CollectedFile& file) {

  if (NULL == grid_ptr) {
    RX_ERROR("new file but grid pointer not set");
    return ;
  }

  std::string filepath = file.dir +"/" +file.filename;

#if 0
  images.push_back(filepath);

  RX_VERBOSE("We've got: %lu images now.", images.size());

  if (images.size() == 10) {
    for (size_t i = 0; i < images.size(); ++i) {
      grid::SimpleImage img;
      img.filepath = images[i];
      grid_ptr->addImage(img);
    }
    grid_ptr->prepare();
    images.clear();
  }
#else 
  grid::SimpleImage img;
  img.filepath = filepath;
  grid_ptr->addImage(img);
  if (grid_ptr->isFull()) {
    grid_ptr->flip();
    grid_ptr->prepare();
  }
#endif
}
