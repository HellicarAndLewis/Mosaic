/*

  test_offline_analyzer
  --------------------

  This files uses the CPU based analyzer to read image files from the 
  input_raw directory, then uses ImageMagick to resize + blur them. The 
  resized and blurred files are stored in two sub dirs of the data dir:
  input_blurred and input_resized. 

  Once ready we save all the analyzed information into a 'descriptors.txt'
  file in the data dir. This descriptors file can use used to match descriptors
  with each other. 

 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <vector>

#define ROXLU_IMPLEMENTATION
#define ROXLU_USE_LOG
#define ROXLU_USE_JPG
#define ROXLU_USE_PNG
#include <tinylib.h>
#include <featurex/AnalyzerCPU.h>
#include <featurex/Config.h>

fex::AnalyzerCPU analyzer;

static void sigh(int s);

int main() {

  rx_log_init();

  RX_VERBOSE("\n\ntest_offline_analyzer\n\n");

  signal(SIGINT, sigh);

  /* load images */
  std::vector<std::string> input_files = rx_get_files(rx_to_data_path("input_raw"));
  if (0 == input_files.size()) {
    RX_ERROR("No files found in input_raw data dir");
    exit(1);
  }

  RX_VERBOSE("Loaded %lu files.", input_files.size());
  fex::config.file_tile_width = 64;
  fex::config.file_tile_height = 64;
  fex::config.resized_filepath = rx_to_data_path("input_resized") +"/";
  
  /* analyze the images. */
  if (0 != analyzer.init()){
    RX_ERROR("Cannot initialize the analyzer");
    exit(1);
  }

  for (size_t i = 0; i < input_files.size(); ++i) {
    if (0 != analyzer.analyze(input_files[i])) {
      RX_ERROR("Failed to analyze: %s", input_files[i].c_str());
    }
  }

  if (0 != analyzer.join()) {
    RX_ERROR("Failed to join the thread");
    exit(1);
  }

  if (0 != analyzer.saveDescriptors()) {
    RX_ERROR("Cannot save the descriptors.");
    exit(1);
  }

  RX_VERBOSE("Ready");
  return 0;
}

static void sigh(int s) {
  RX_VERBOSE("Stopping the analyzer");
  analyzer.shutdown();
}
