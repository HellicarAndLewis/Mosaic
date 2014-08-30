/*

  Load a RGB, 24bit PNG as 32bit. 

 */
#include <stdio.h>
#include <stdlib.h>

#define ROXLU_IMPLEMENTATION
#define ROXLU_USE_LOG
#define ROXLU_USE_JPG
#define ROXLU_USE_PNG
#include "tinylib.h"

int main() {

  rx_log_init();

  RX_VERBOSE("test_png_rgba");

  int width, height, channels;
  unsigned char* pixels = NULL;
  int nbytes;

  /* load an rgb png, and convert it to rgba when loading. */
  nbytes = rx_load_png(rx_to_data_path("test/rgb.png"), &pixels, width, height, channels, NULL, RX_FLAG_LOAD_AS_RGBA);
  if (0 > nbytes) {
    RX_ERROR("Cannot load test/rgb.png");
    exit(EXIT_FAILURE);
  }

  RX_VERBOSE("swidth: %d, height: %d, channels: %d, snbytes: %d", 
             width, height, channels, nbytes);

  rx_save_png(rx_to_data_path("test/rgb_to_rgba.png"), pixels, width, height, channels, false);
  return 0;
}
