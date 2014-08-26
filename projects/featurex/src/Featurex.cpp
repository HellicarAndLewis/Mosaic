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

} /* namespace fex */
