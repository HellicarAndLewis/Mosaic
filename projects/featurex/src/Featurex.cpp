#include <featurex/Descriptor.h>
#include <featurex/Config.h>
#include <featurex/Featurex.h>

namespace fex {


  /* ---------------------------------------------------------------------------------- */

  static void on_analyzed(Descriptor& desc, void* user);

  /* ---------------------------------------------------------------------------------- */

  Featurex::Featurex() 
  //    :has_new_analyzed_image(false)
  {
#if 0
    /* create texture for the input image. */
    input_image.createTexture();
#endif

  }

  Featurex::~Featurex() {
  }

  int Featurex::init() {
#if 0
    if (0 != avg_color.init()) {
      return -1;
    }

    if (0 != blur.init(385, 384, 15.0)) {
      return -2;
    }

    if (0 != analyzed_image.createTexture()) {
      RX_ERROR("Cannot create the analyzed image texture.");
      return -3;
    }

    if (0 != cpu_analyzer.init()) {
      return -4;
    }

    cpu_analyzer.on_analyzed = on_analyzed;
    cpu_analyzer.user = this;
#endif
    return 0;
  }

  int Featurex::analyzeImageFile(std::string filepath) {
    return 0;
    //return cpu_analyzer.analyze(filepath);
  }

  int Featurex::reinit() {
    return 0;
    //    return avg_color.reinit();
  }

  void Featurex::update() {
#if 0
    /* Update the analyzed texture data */
    if (has_new_analyzed_image) {
      if (0 != analyzed_image.updateTexture()) {
        RX_ERROR("Cannot update the analyzed texture data");
      }
      has_new_analyzed_image = false;
    }
#endif
  }

  void Featurex::draw() {
#if 0
    painter_bg.clear();
    painter_bg.nofill();
    painter_bg.texture(input_image.texid, 10, 10, input_image.width, input_image.height);


    timer.start("Average color");
    avg_color.input_texid = input_image.texid;
    avg_color.calculate();
    timer.stop();

    painter_bg.texture(avg_color.output_texid, 404, 10, input_image.width, input_image.height);

    painter_bg.draw();

    painter_fg.clear();
    painter_fg.color(0,0,0,1);
    painter_fg.rect(10,10,input_image.width, input_image.height);    
    painter_fg.rect(404,10,input_image.width, input_image.height);    

    if (NULL != analyzed_image.pixels) {
      painter_fg.texture(analyzed_image.texid, 10, 10, analyzed_image.width, analyzed_image.height);
      painter_fg.fill();
      painter_fg.color(analyzed_desc.average_color[0]/255.0f,
                       analyzed_desc.average_color[1]/255.0f,
                       analyzed_desc.average_color[2]/255.0f);
      painter_fg.rect(10 + analyzed_image.width, 10, analyzed_image.width, analyzed_image.height);
      painter_fg.nofill();
    }

    painter_fg.draw();

    if (fex::config.show_timer) {
      timer.draw();
    }
    else {
      timer.reset();
    }

#if 0
    {
      /* Blur the input image a couple of times. */
      timer.start("blur");
      blur.blur(avg_color.input_texid);
      blur.blur(blur.tex());
      //      blur.blur(blur.tex());
      painter_bg.texture(blur.tex(), 10, 384 + 20, input_image.width, input_image.height);
      timer.stop();
    }

    timer.start("draw");
    painter_bg.draw();    
#endif

    glViewport(0, 0, 1280, 720);
#endif
  }

  int Featurex::loadInputImage(std::string filepath) {
#if 0
    /* Load the image */
    int r = input_image.load(filepath);
    if (r != 0) {
      return 0;
    }

    input_image.updateTexture();
#endif
    return 0;
  }

  static void on_analyzed(Descriptor& desc, void* user) {
#if 0
    /* get our featurex instance */
    Featurex* feat = static_cast<Featurex*>(user);
    if (NULL == feat) {
      RX_ERROR("The user pointer is invalid. This is not supposed to happen");
      return;
    }

    /* load the image from disk */
    if (0 != feat->analyzed_image.load(fex::config.resized_filepath +desc.filename)) {
      RX_ERROR("Cannot load: %s", desc.filename.c_str());
      return;
    }

    feat->has_new_analyzed_image = true;
    feat->analyzed_desc = desc;
#endif
  }

  

} /* namespace fex */
