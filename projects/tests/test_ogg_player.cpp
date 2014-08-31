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

#define RXP_PLAYER_GL_IMPLEMENTATION
#include <rxp_player/PlayerGL.h>
 
void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);
 
rxp::PlayerGL player;
static void on_event(rxp::PlayerGL* p, int event);
static int setup_player(rxp::PlayerGL* p);  
bool restart = false;

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
 
  win = glfwCreateWindow(w, h, "++ --- .ogg player, create a video only file (no audio) in: bin/data/test/tiny.ogg -- //__ ", NULL, NULL);
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

  if (0 != setup_player(&player)) {
    exit(EXIT_FAILURE);
  }

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    

    if (true == restart) {
      if (0 != setup_player(&player)) {
        exit(EXIT_FAILURE);
      }
      restart = false;
    }

    player.update();
    player.draw();
    player.draw(10, 10, 640, 360);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }
  
  player.stop();
  player.shutdown();
 
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

/* gets called from a different thread, so we can't do any GL here! */
static void on_event(rxp::PlayerGL* p, int event) {

  if (NULL == p) {
    RX_ERROR("Error: playergl handle is invalid.");
    return;
  }

  if (RXP_PLAYER_EVENT_RESET == event) {
    restart = true;
  }
}


static int setup_player(rxp::PlayerGL* gl) {

  if (NULL == gl) {
    RX_ERROR("Error: invalid playergl pointer.");
    return -1;
  }

  RX_VERBOSE("+ We're going to (re)start the player.");

  std::string video = rx_to_data_path("test/tiny.ogg");
  if (false == rx_file_exists(video)) {
    RX_ERROR("Error: the ogg file doesnt exist: %s", video.c_str());
    return -1;
  }

  if (0 != gl->init(video)) {
    RX_ERROR("Error: canot open the video file.");
    ::exit(EXIT_FAILURE);
  }

  if (0 != gl->play()) {
    RX_ERROR("Error: cannot start playing.");
    ::exit(EXIT_FAILURE);
  }

  return 0;
}
