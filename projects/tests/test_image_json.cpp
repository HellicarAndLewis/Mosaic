#include <stdio.h>
#include <stdlib.h>
#include <topshop/ImageJSON.h>

#define ROXLU_USE_LOG
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

int main() {

  printf("\n\ntest_image_json\n\n");
  rx_log_init();

  top::ImageJSON img_json;
  top::ImageInfo img_info;
  std::string filepath;

  double now = rx_hrtime();
  filepath = rx_get_exe_path() +"/../data/test/image.json";
  if (0 != img_json.parse(filepath, img_info)) {
    printf("Error: cannot parse the json.\n");
    exit(EXIT_FAILURE);
  }

  printf("\n-----------------------------------------------\n");
  printf("+ username: %s\n", img_info.username.c_str());
  printf("-----------------------------------------------\n");
  printf("+ took: %f ms\n", (double(rx_hrtime()) - now)/10e6);
  printf("-----------------------------------------------\n");

  printf("\n");
  return 0;
}
