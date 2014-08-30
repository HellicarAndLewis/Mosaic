#include <featurex/Descriptor.h>
#include <featurex/Config.h>
#include <featurex/Featurex.h>

namespace fex {

  Featurex::Featurex() 
    :mosaic_pixels(NULL)
    ,user(NULL) /* testing */
    ,on_pixdata(NULL) /* testing */
  {

  }

  Featurex::~Featurex() {
    shutdown();
  }

  int Featurex::init(GLuint inputTex) {
    int r;

    if (NULL != mosaic_pixels) {
      RX_ERROR("Mosaic pixel buffer already allocated; didn't you call shutdown?");
      return -1;
    }

    /* start the cpu analyzer. */
    r = analyzer_cpu.init();
    if (r != 0) {
      RX_ERROR("Cannot initialize the cpu analyzer.");
      return r;
    }

    /* load the previously calculated descriptors when starting. */
    r = analyzer_cpu.loadDescriptors();
    if (r != 0) {
      RX_ERROR("Cannot load the descriptors.");
      return r;
    }

    /* init gpu */
    r = analyzer_gpu.init(inputTex);
    if (0 != r) {
      RX_ERROR("Cannot init the gpu analyzer.");
      analyzer_cpu.shutdown();
      return r;
    }

    /* init the tiles pool */
    r = tiles.init();
    if (0 != r) {
      RX_ERROR("Cannot initialize the tiles pool.");
      analyzer_cpu.shutdown();
      analyzer_gpu.shutdown();
      return r;
    }

    /* BEGIN TESTING - LOAD FILES */                                
    /* -------------------------------------------------------------------------- */
    for (size_t i = 0; i < analyzer_cpu.descriptors.size(); ++i) {
      tiles.loadDescriptorTile(analyzer_cpu.descriptors[i]);
    }
    /* -------------------------------------------------------------------------- */
    /* END TESTING - LOAD FILES */

    
    /* create the surface that will hold the mosaic pixels. */
    int nbytes = (fex::config.getMosaicImageWidth() * fex::config.getMosaicImageHeight()) * 4; 
    mosaic_pixels = (unsigned char*)malloc(nbytes);
    if (NULL == mosaic_pixels) {
      RX_ERROR("Cannot allocate the pixel buffer");
      return -2;
    }

    return 0;
  }

  int Featurex::shutdown() {
    int r = 0;

    /* first make sure to save the current descriptors. */
    r = analyzer_cpu.saveDescriptors();
    if (0 != r) {
      RX_ERROR("Cannot save the descriptors.");
    }

    /* shutdown cpu analyzer */
    r = analyzer_cpu.shutdown();
    if (0 != r) {
      RX_ERROR("Cannot shutdown the cpu analyzer");
    }

    r = analyzer_gpu.shutdown();
    if (0 != r) {
      RX_ERROR("Cannot shutdown the gpu analyzer");
    }

    r = tiles.shutdown();
    if (0 != r) {
      RX_ERROR("Cannot shutdown the tiles pool");
    }

    /* free the mosaic pixel buffer. */
    if (NULL != mosaic_pixels) {
      free(mosaic_pixels);
    }
    mosaic_pixels = NULL;

    return r;
  }

  void Featurex::draw() {
    analyzer_gpu.draw();
  }

  int Featurex::analyzeCPU(std::string filepath) {
    return analyzer_cpu.analyze(filepath);
  }

  int Featurex::analyzeGPU() {
    return analyzer_gpu.analyze();
  }

  void Featurex::match() {

    if (0 == analyzer_cpu.descriptors.size()) {
      RX_WARNING("No descriptors found in the cpu analyzer, cannot match.");
      return;
    }
    if (0 == analyzer_gpu.descriptors.size()) {
      RX_WARNING("No descriptors found in the gpu analyzer. cannot match.");
      return;
    }
    if (NULL == mosaic_pixels) {
      RX_ERROR("Trying to match descriptors + create mosaic, but the pixel buffer is NULL. Forgot to cal init()?");
      return;
    }

    uint64_t n = rx_hrtime();

#if 0
    RX_VERBOSE("CPU descriptors: %lu, GPU descriptors: %lu", 
               analyzer_cpu.descriptors.size(), 
               analyzer_gpu.descriptors.size());
#endif

    for (size_t i = 0; i < analyzer_gpu.descriptors.size(); ++i) {

      ssize_t dx = comp.match(analyzer_gpu.descriptors[i], analyzer_cpu.descriptors);
      if (-1 == dx || dx >= analyzer_cpu.descriptors.size()) {
        RX_ERROR("Invalid dx: %lu", dx);
        continue;
      }

      Descriptor& gdesc = analyzer_gpu.descriptors[i];
      Descriptor& cdesc = analyzer_cpu.descriptors[dx];

      /* when it's the same match, there is no need to copy */
      if (gdesc.matched_id == cdesc.id) {
        continue;
      }

      gdesc.matched_id = cdesc.id;

      Tile* tile = tiles.getTileForDescriptorID(cdesc.id);
      if (NULL == tile) {
        //RX_VERBOSE("Cannot file a tile for the descriptor: %u", cdesc.id);
        continue;
      }

      if (4 != tile->nchannels) {
        RX_ERROR("We have optimized the tilepool for 4 channel images");
        continue;
      }

#if 1
      /* construct the mosaic. */
      int src_stride = tile->nchannels * fex::config.file_tile_width;
      int dest_stride = fex::config.getMosaicImageWidth() * 4; /* @todo - make dynamic */
      int y0 = (gdesc.row * fex::config.file_tile_height) * dest_stride;
      int x0 = (gdesc.col * fex::config.file_tile_width * 4);

      for (int k = 0; k < fex::config.file_tile_height; ++k) {
        int src_dx = (fex::config.file_tile_height - k) * src_stride;
        int dest_dx = y0 + (k * dest_stride) + x0;
        memcpy(mosaic_pixels + dest_dx, tile->pixels + src_dx, src_stride);
      }
#else
      if (NULL != on_pixdata) {
        int x = (gdesc.col * fex::config.file_tile_width);
        int y = (gdesc.row * fex::config.file_tile_height);
        on_pixdata(x, y, fex::config.file_tile_width, fex::config.file_tile_height, tile->pixels, user);
      }
#endif

#if 0
      RX_VERBOSE("Matched: (%d,%d,%d) <> (%d,%d,%d)",
                 gdesc.average_color[0],
                 gdesc.average_color[1],
                 gdesc.average_color[2],
                 cdesc.average_color[0],
                 cdesc.average_color[1],
                 cdesc.average_color[2]);
#endif

    }

    //double d = double(rx_hrtime() - n) / (1000.0 * 1000.0 * 1000.0);
    //RX_VERBOSE("Comparing + constructing the image took: ~%f", d);

#if 0
    static int frame = 1;
    if (frame % 30 == 0) {
      std::string fname = "mosaic_" +rx_get_time_string() +".png";
      rx_save_png(fname, mosaic_pixels, fex::config.getMosaicImageWidth(), fex::config.getMosaicImageHeight(), 4, false);
    }
    frame++;
#endif

    //    RX_VERBOSE("-");
  }

} /* namespace fex */
