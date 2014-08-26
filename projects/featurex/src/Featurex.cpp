#include <featurex/Descriptor.h>
#include <featurex/Config.h>
#include <featurex/Featurex.h>

namespace fex {

  Featurex::Featurex() {
  }

  Featurex::~Featurex() {
  }

  int Featurex::init(GLuint inputTex) {
    int r;

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

    uint64_t n = rx_hrtime();

    for (size_t i = 0; i < analyzer_gpu.descriptors.size(); ++i) {

      ssize_t dx = comp.match(analyzer_gpu.descriptors[i], analyzer_cpu.descriptors);
      if (-1 == dx || dx >= analyzer_cpu.descriptors.size()) {
        RX_ERROR("Invalid dx: %lu", dx);
        continue;
      }

      Descriptor& gdesc = analyzer_gpu.descriptors[i];
      Descriptor& cdesc = analyzer_cpu.descriptors[dx];
      
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

    double d = double(rx_hrtime() - n) / (1000.0 * 1000.0 * 1000.0);
    RX_VERBOSE("Comparing took: %f", d);
    //    RX_VERBOSE("-");
  }

} /* namespace fex */
