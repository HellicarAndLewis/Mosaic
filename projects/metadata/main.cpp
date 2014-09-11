#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

template<class T> static T convert_type(char* input);
static void print_usage();

/* ----------------------------------------------------------------------------------- */

struct options {
  std::string username;
  std::string source_path;
  std::string dest_path;
};

/* ----------------------------------------------------------------------------------- */

int main(int argc, char** argv) {

  print_usage();

  options opt;
  int c;

  while ((c = getopt(argc, argv, "s:d:u:")) != -1) {
    switch(c) {
      case 's': {
        opt.source_path = convert_type<std::string>(optarg);
        break;
      }
      case 'd': {
        opt.dest_path = convert_type<std::string>(optarg);
        break;
      }
      case 'u': {
        opt.username = convert_type<std::string>(optarg);
        break;
      }
      default: {
        printf("+ Error: invalid option");
        break;
      }
    }
  }


  /* validate options. */
  if (0 == opt.username.size()) {
    printf("+ error: no username given, (-u)\n");
    exit(EXIT_FAILURE);
  }
  if (0 == opt.source_path.size()) {
    printf("+ error: no source_path, (-s)\n");
    exit(EXIT_FAILURE);
  }
  if (0 == opt.dest_path.size()) {
    printf("+ error: no dest_path, (-d)\n");
    exit(EXIT_FAILURE);
  }
  if (false == rx_is_dir(opt.source_path)) {
    printf("+ error: source path not found: %s\n", opt.source_path.c_str());
    exit(EXIT_FAILURE);
  }
  if (false == rx_is_dir(opt.dest_path)) {
    printf("+ error: dest path not found: %s\n", opt.dest_path.c_str());
    exit(EXIT_FAILURE);
  }

  /* print options */
  printf("\n\n---------------------------------------------------\n");
  printf("opt.username (-u): %s\n", opt.username.c_str());
  printf("opt.source_path (-s): %s\n", opt.source_path.c_str());
  printf("opt.dest_path (-d): %s\n", opt.dest_path.c_str());
  printf("---------------------------------------------------\n");

  /* find source files. */
  std::vector<std::string> source_files = rx_get_files(opt.source_path);
  if (0 == source_files.size()) {
    printf("+ error: not source files found in %s\n", opt.source_path.c_str());
    exit(EXIT_FAILURE);
  }

  /* generate the json */
  std::stringstream ss;
  ss << "{\n"
     << "  \"user\": {\n"
     << "    \"username\":\"" << opt.username << "\"\n"
     << "  }\n"
     << "}";

  std::string json = ss.str();

  /* generate the json for each source file. */
  for (size_t i = 0; i < source_files.size(); ++i) {

    /* get file. */
    std::string sf = source_files[i];
    if (false == rx_file_exists(sf)) {
      printf("+ error: someone remove %s\n", sf.c_str());
      continue;
    }

    /* check extension (only jpg) */
    std::string ext = rx_get_file_ext(sf);
    if (ext != "jpg") {
      printf("+ warning: only jpg supported for now, skipping: %s", rx_strip_dir(sf).c_str());
      continue;
    }

    std::string outfile = opt.dest_path +"/" +rx_strip_file_ext(rx_strip_dir(sf)) +".json";
    std::ofstream ofs(outfile.c_str());
    if (false == ofs.is_open()) {
      printf("+ error: cannot open file %s. no permission?\n", outfile.c_str());
      exit(EXIT_FAILURE);
    }


    ofs << json;
    ofs.close();

    printf("- generated: %s\n", (rx_strip_dir(outfile).c_str()));

    
  }
  
  return 0;
}

/* ----------------------------------------------------------------------------------- */

template<class T> static T convert_type(char* input) {
  T out;

  if (NULL == input) {
    printf("+ Error: invalid input\n");
    return out;
  }

  std::stringstream ss;
  ss << input;
  ss >> out;
  return out;
}

/* ----------------------------------------------------------------------------------- */

static void print_usage() {
  printf("\n\n---------------------------------------------------\n");
  printf("-u [string]     The username for the images.\n");
  printf("-s [string]     The source path with all the images (jpg).\n");
  printf("-d [string]     The output path where all json files are created.\n");
  printf("---------------------------------------------------\n\n");
}
