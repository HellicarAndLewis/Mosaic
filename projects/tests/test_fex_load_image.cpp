/* 

   test_fex_load_image
   -------------------

   Plain test which loads + reloads a couple of images and reallocating the 
   previously allocated buffer when needed. Just a tiny test to speed up
   the image loading process.

 */

#include <stdio.h>
#include <stdlib.h>

#define ROXLU_IMPLEMENTATION
#define ROXLU_USE_JPG
#define ROXLU_USE_PNG
#include <tinylib.h>

int main() {

  printf("\n\ntest_fex_load_image\n\n");

  int width = 0;
  int height = 0;
  int channels = 0;
  unsigned char* pix = NULL;
  int allocated = 0;
  int nbytes = 0;

  /* --------------------------------------------------------------------------------- */
  /*              T E S T I N G   R E A L L O C   F O R   J P G                        */
  /* --------------------------------------------------------------------------------- */
  /* make sure we have a file to test. */
  std::string small_file = rx_to_data_path("test_input1.jpg");
  if (false == rx_file_exists(small_file)) {
    printf("Error: cannot find the %s image.\n", small_file.c_str());
    exit(1);
  }

  std::string big_file = rx_to_data_path("test_input1_big.jpg");
  if (false == rx_file_exists(big_file)) {
    printf("Error: cannot find the %s image.\n", big_file.c_str());
    exit(1);
  }

  nbytes = rx_load_jpg(small_file, &pix, width, height, channels, &allocated);
  printf("Loaded, width: %d, height: %d, channels: %d, allocated: %d, size: %d\n", width, height, channels, allocated, nbytes);
  printf("-\n");

  for (int i = 0; i < 5; ++i) {
    nbytes = rx_load_jpg(big_file, &pix, width, height, channels, &allocated);
    printf("Loaded, width: %d, height: %d, channels: %d, allocated: %d, size: %d\n", width, height, channels, allocated, nbytes);
    printf("-\n");
  }

  nbytes = rx_load_jpg(small_file, &pix, width, height, channels, &allocated);
  printf("Loaded, width: %d, height: %d, channels: %d, allocated: %d, size: %d\n", width, height, channels, allocated, nbytes);
  printf("-\n");

  /* --------------------------------------------------------------------------------- */
  /*              T E S T I N G   R E A L L O C   F O R   P N G                        */
  /* --------------------------------------------------------------------------------- */

  printf("------------------------------------------------------------------------------\n");

  small_file = rx_to_data_path("test_trans.png");
  if (false == rx_file_exists(small_file)) {
    printf("Error: cannot find the %s image.\n", small_file.c_str());
    exit(1);
  }

  big_file = rx_to_data_path("test_input2_big.png");
  if (false == rx_file_exists(big_file)) {
    printf("Error: cannot find the %s image.\n", big_file.c_str());
    exit(1);
  }

  /* re-use mem that was allocated a couple of lines above */
  nbytes = rx_load_png(small_file, &pix, width, height, channels, &allocated);
  printf("Loaded small file, width: %d, height: %d, channels: %d, allocated: %d, size: %d\n", width, height, channels, allocated, nbytes);
  printf("-\n");

  /* try w/o the allocated param - will allocated again */
  nbytes = rx_load_png(small_file, &pix, width, height, channels);
  printf("Loaded, width: %d, height: %d, channels: %d, allocated: %d, size: %d\n", width, height, channels, allocated, nbytes);
  printf("-\n");

  allocated = 0;
  nbytes = rx_load_png(small_file, &pix, width, height, channels, &allocated);
  printf("Loaded, width: %d, height: %d, channels: %d, allocated: %d, size: %d\n", width, height, channels, allocated, nbytes);
  printf("-\n");

  return 0;  
}

