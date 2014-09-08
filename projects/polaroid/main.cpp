/*

  Used to create the polaroid images for the girds and interactive layer. 

  -c = the background 'canvas', must be png (this is the instagram image)
  -f = the foreground overlay, must be png. 
  -x = the x-offset of the background canvas, so you can move it into place of the overlay
  -y = the y-offset of the background canvas, so you can move it into place of the overlay
  -s = the x-offset of the name
  -t = the y-offset of the name
  -n = the name
  -w = font size for name
  -r,-g, -b = color for then name
  -a = the size in pixels of the squale that is visible
  -h = hashtag
  -o = output path

  Create the big version of the polaroid:
  ----------------------------------------

    ./AppPolaroid -x 55 -y 62 -f ./background.png -r 0.0 -g 0.0 -b 0.0 -n "roxlu" -s 60 -t 578 -w 28 -a 458 -c ./polaroid_overlay_big.png -o out_big.png

    
  Create the small version of the polaroid
  ----------------------------------------

    ./AppPolaroid -x 10 -y 10 -f ./background.png -r 0.0 -g 0.0 -b 0.0 -n "roxlu" -s 10 -t 187 -w 10 -a 180 -c ./polaroid_overlay_small.png -o out_small.png


 */
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cairo/cairo.h>
#include <sstream>

#define ROXLU_USE_JPG
#define ROXLU_USE_PNG
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

class Image {
public:
  Image();
  ~Image();
public:
  int width;
  int height;
  int channels;
  unsigned char* pixels; 
  int capacity;
  int nbytes;
};

Image::Image() 
  :width(0)
  ,height(0)
  ,channels(0)
  ,pixels(NULL)
  ,capacity(0)
  ,nbytes(0)
{
}

Image::~Image() {
  if (NULL != pixels) {
    delete[] pixels;
    pixels = NULL;
  }
  width = 0;
  height = 0;
  channels = 0;
  capacity = 0;
  nbytes = 0;
}

struct Options {
  Options();
  bool validate();
  void print();

  std::string foreground_file; /* -c */
  std::string background_file; /* -f */
  std::string output_file; /* -o */
  int background_x; /* -x */
  int background_y; /* -y */
  int name_x; /* -s */
  int name_y; /* -t */
  float name_r; /* -r */
  float name_g; /* -g */
  float name_b; /* -b */
  float name_font_size; /* -w */
  std::string name; /* -n */
  float visible_size; /* -a, the visible WxH of the area that is visible; must be a square */
  std::string hashtag; /* -h */
  int hashtag_x; /* -i */
  int hashtag_y; /* -j */
};

Options::Options() {
  background_x = -1;
  background_y = -1;
  name_x = -1;
  name_y = -1;
  name_font_size = -1.0f;
  name_r = 0.0f;
  name_g = 0.0f;
  name_b = 0.0f;
  visible_size = -1.0f;
  hashtag_x = 0;
  hashtag_y = 0;
}

void Options::print() {
  printf("\n\n-------------------------------\n");
  printf("+ background_file (-k): %s\n", background_file.c_str());
  printf("+ foreground_file (-c): %s\n", foreground_file.c_str());
  printf("+ output_file (-o): %s\n", output_file.c_str());
  printf("+ background_x (-x): %d\n", background_x);
  printf("+ background_y (-y): %d\n", background_y);
  printf("+ name_x (-s): %d\n", name_x);
  printf("+ name_y (-t): %d\n", name_y);
  printf("+ name (-n): %s\n", name.c_str());
  printf("+ name font size (-w): %f\n", name_font_size);
  printf("+ name rgb (-r, -g, -b): %f, %f, %f\n", name_r, name_g, name_b);
  printf("+ visible_size (-a): %f\n", visible_size);
  printf("+ hashtag (-h): %s\n", hashtag.c_str());
  printf("+ hashtag_x (-i): %d\n", hashtag_x);
  printf("+ hashtag_y (-j): %d\n", hashtag_y);
  printf("-------------------------------\n\n");
}

bool Options::validate() {
  if (0 == name.size()) { printf("+ error: name not set.\n"); return false; } 
  if (0 > background_x) { printf("+ error: background_x not set (-x).\n"); return false; } 
  if (0 > background_y) { printf("+ error: background_y not set (-y).\n"); return false; } 
  if (0 > name_x) { printf("+ error: name_x not set (-s).\n"); return false; } 
  if (0 > name_y) { printf("+ error: name_y not set (-t).\n"); return false; } 
  if (0 > visible_size) { printf("+ error: visible_size not set (-a).\n"); return false; } 
  if (0 == background_file.size()) { printf("+ error: background_file not set (-f)\n"); return false; };
  if (0 == foreground_file.size()) { printf("+ error: foreground_file not set (-c)\n"); return false; }
  if (0 == output_file.size()) { printf("+ error: output file not set (-o)\n"); return false; }
  return true;
}

template<class T> static T convert_type(char* input);

/* -------------------------------------------------------- */

int main(int argc, char** argv) {

  int c;
  float col_r, col_g, col_b = -1.0f;
  Options opt;

  while ((c = getopt(argc, argv, "a:x:y:f:s:t:n:w:r:g:b:c:h:i:j:o:")) != -1) {
    switch(c) {

      /* color */
      /* ------------------------------------------------------- */
      case 'r': {
        col_r = convert_type<float>(optarg);
        break;
      }
      case 'g': {
        col_g = convert_type<float>(optarg);
        break;
      }
      case 'b': {
        col_b = convert_type<float>(optarg);
        break;
      }

      /* ------------------------------------------------------- */
      /* hashtag */
      case 'h': {
        opt.hashtag = convert_type<std::string>(optarg);
        break;
      }
      case 'i': {
        opt.hashtag_x = convert_type<int>(optarg);
        break;
      }
      case 'j': {
        opt.hashtag_y = convert_type<int>(optarg);
        break;
      }
      
      /* ------------------------------------------------------- */
      /* foreground */
      case 'c': {
        opt.foreground_file = convert_type<std::string>(optarg);
        break;
      }
      /* ------------------------------------------------------- */
      /* background */
      case 'x': {
        opt.background_x = convert_type<int>(optarg);
        break;
      }
      case 'y': {
        opt.background_y = convert_type<int>(optarg);
        break;
      }
      case 'f': {
        opt.background_file = convert_type<std::string>(optarg);
        break;
      }

      /* ------------------------------------------------------- */
      /* visible size */
      case 'a': {
        opt.visible_size = convert_type<float>(optarg);
        break;
      }

      /* ------------------------------------------------------- */
      /* output */
      case 'o': {
        opt.output_file = convert_type<std::string>(optarg);
        break;
      }

      /* ------------------------------------------------------- */
      /* name */
      case 'n': {
        opt.name = convert_type<std::string>(optarg);

        /* we expect that r,g,b has been set */
        if (0 > col_r) {
          printf("Error: you haven't set -r -g -b for the name.\n");
          exit(EXIT_FAILURE);
        }
        opt.name_r = col_r;
        opt.name_g = col_g;
        opt.name_b = col_b;
        break;
      }
      /* name x position */
      case 's': {
        opt.name_x = convert_type<int>(optarg);
        break;
      }
      /* name y position */
      case 't': {
        opt.name_y = convert_type<int>(optarg);
        break;
      }
      /* name font size */
      case 'w': {
        opt.name_font_size = convert_type<float>(optarg);
        break;
      }
      default: {
        printf("Unkown option\n");
        break;
      }
    }
  }
  
  if (false == opt.validate()) {
    printf("+ error: cannot validate the given options.\n");
    exit(EXIT_FAILURE);
  }

  opt.print();

  /* ------------------------------------------------------------------------------------ */

  Image img;
  std::string path;
  std::string ext;
  cairo_surface_t* surf_bg = NULL;
  cairo_format_t img_format = CAIRO_FORMAT_INVALID;

  path = opt.foreground_file;
  if (false == rx_file_exists(path)) {
    printf("+ error: cannot find the file: %s\n", path.c_str());
    exit(EXIT_FAILURE);
  }

  cairo_surface_t* surf_overlay = cairo_image_surface_create_from_png(path.c_str());
  if (NULL == surf_overlay) {
    printf("Error: cannot create s1\n");
    exit(EXIT_FAILURE);
  }

  path = opt.background_file;
  if (false == rx_file_exists(path)) {
    printf("Error: file doesn't exist: %s\n", path.c_str());
    exit(EXIT_FAILURE);
  }
  
  /* check what file type was given. */
  ext = rx_get_file_ext(path);
  if (ext == "jpg") {

    printf("+ warning: jpg as input doesn't seem to work\n");

    /* cairo doesn't have support for PNG? */
    if (0 > rx_load_jpg(path, &img.pixels, img.width, img.height, img.channels)) {
      printf("Error: failed to load: %s\n", path.c_str());
      exit(EXIT_FAILURE);
    }
    if (0 == img.width || 0 == img.height || 0 == img.channels) {
      printf("Error: image has invalid flags: %d x %d, channels: %d\n", img.width, img.height, img.channels);
      exit(EXIT_FAILURE);
    }

    if (3 == img.channels) {
      img_format = CAIRO_FORMAT_RGB24;
      printf("+ Using RGB24\n");
    }  
    else if(4 == img.channels) {
      img_format = CAIRO_FORMAT_ARGB32;
      printf("+ Using ARGB32\n");
    }
    else {
      printf("Error: unsupported number of channels: %d.\n", img.channels);
      exit(EXIT_FAILURE);
    }

    if (NULL != img.pixels && NULL == surf_bg) {

        printf("Stride: %d\n", cairo_format_stride_for_width(img_format, img.width));
        printf("Info: creating %d x %d, channels: %d\n", img.width, img.height, img.channels);

        surf_bg = cairo_image_surface_create_for_data(img.pixels, 
                                                      img_format, 
                                                      img.width, 
                                                      img.height, 
                                                      cairo_format_stride_for_width(img_format, img.width));

#if 0
      /* TESTING */
      cairo_t* cr = cairo_create(surf_bg);
      if (NULL == cr) { 
        printf("Error: cannot create the cairo");
        exit(EXIT_FAILURE);
      }
      path = rx_get_exe_path() +"/generated_polaroid.png";
      cairo_surface_write_to_png(surf_bg, path.c_str());
      printf("Created\n");
      exit(0);
      /* END TESTING */
#endif
    }
  }
  else if (ext == "png") {

    /* use cairo png load feature. */
    surf_bg = cairo_image_surface_create_from_png(path.c_str());
    if (NULL == surf_bg) {
      printf("Error: cannot create s2\n");
      exit(EXIT_FAILURE);
    }

  }
  else {
    printf("Error: unsupported file format: %s\n", ext.c_str());
    exit(EXIT_FAILURE);
  }


  /* make sure the background is loaded correctly (aka the photo) */
  if (NULL == surf_bg) {
    printf("Error: cannot create background surface.\n");
    exit(EXIT_FAILURE);
  }

  if (CAIRO_STATUS_SUCCESS != cairo_surface_status(surf_bg)) {
    printf("Error: something went wrong: %d\n", cairo_surface_status(surf_bg));
    exit(EXIT_FAILURE);
  }

  float source_width = cairo_image_surface_get_width(surf_bg);
  float source_height = cairo_image_surface_get_height(surf_bg);

  /* create output */
  int dest_width = cairo_image_surface_get_width(surf_overlay);
  int dest_height = cairo_image_surface_get_height(surf_overlay);

  printf("+ Output size: %d x %d\n", dest_width, dest_height);

  cairo_surface_t* surf_out = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dest_width, dest_height);
  if (NULL == surf_out) {
    printf("Error: cannot create cairo_surface_t\n");
    exit(EXIT_FAILURE);
  }

  printf("+ Info: creating output surface: %d x %d\n", dest_width, dest_height);

  cairo_t* cr = cairo_create(surf_out);
  if (NULL == cr) { 
    printf("Error: cannot create the cairo");
    exit(EXIT_FAILURE);
  }


  /* fill background. */
  /*
  cairo_set_source_rgba(cr, 1, 1, 1, 1);
  cairo_paint(cr);
  */

  float scale_factor = opt.visible_size / source_width;
  printf("+ Scale factor: %f\n", scale_factor);

  /* paint background */  
  cairo_save(cr);
  cairo_scale(cr, scale_factor, scale_factor);
  cairo_set_source_surface(cr, surf_bg, opt.background_x, opt.background_y);
  cairo_rectangle(cr, 0, 0, img.width, img.height);
  cairo_paint(cr);
  cairo_restore(cr);

  /* paint overlay */
  cairo_set_source_surface(cr, surf_overlay, 0, 0);
  cairo_paint(cr);
  cairo_surface_flush(surf_out);

  /* font settings. */
  cairo_select_font_face(cr, "Open Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_source_rgba(cr, opt.name_r, opt.name_g, opt.name_b, 1.0); 

  /* name */
  if (0 != opt.name.size()) {
    cairo_move_to(cr, opt.name_x, opt.name_y);
    cairo_set_font_size(cr, opt.name_font_size);
    cairo_show_text(cr, opt.name.c_str());

    cairo_stroke(cr);
    cairo_fill(cr);
  }

  /* hashtag */
  if (0 != opt.hashtag.size()) {
    cairo_move_to(cr, opt.hashtag_x, opt.hashtag_y);
    cairo_set_font_size(cr, opt.name_font_size);
    cairo_show_text(cr, opt.hashtag.c_str());

    cairo_stroke(cr);
    cairo_fill(cr);
  }

  cairo_surface_flush(surf_out);

  /* write out */
  path = opt.output_file;
  cairo_surface_write_to_png(surf_out, path.c_str());

  /* cleanup */
  cairo_surface_destroy(surf_out);
  cairo_surface_destroy(surf_bg);
  cairo_surface_destroy(surf_overlay);
  cairo_destroy(cr);

  printf("\n");
  return 0;
}

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
