#include <rapidxml.hpp>
#include <fstream>
#include <sstream>
#include <mosaic/Config.h>
#include <featurex/Config.h>

using namespace rapidxml;

namespace mos {


  /* --------------------------------------------------------------------------------- */
  template<class T> T read_xml(xml_node<>* node, std::string name, T def);
  static std::string read_xml_str(xml_node<>* node, std::string name, std::string def);
  static int read_xml_int(xml_node<>* node, std::string name, int def);

  /* --------------------------------------------------------------------------------- */

  Config config;

  /* --------------------------------------------------------------------------------- */
  
  Config::Config() {
    reset();
  }

  Config::~Config() {
    reset();
  }

  
  int Config::validateWebcam() {
    if (0 == webcam_width) {
      RX_ERROR("Invalid width");
      return -1;
    }
    if (0 == webcam_height) {
      RX_ERROR("Invalid height");
      return -1;
    }
    return 0;
  }

  int Config::validateWindowSize() {
    if (0 == mos::config.window_width) {
      RX_ERROR("Invalid window width.");
      return -1;
    }
    if (0 == mos::config.window_height) {
      RX_ERROR("Invalid window height.");
      return -2;
    }
    return 0;
  }

  void Config::reset() {
    webcam_device = 0;
    webcam_width = 0;
    webcam_height = 0;
    window_width = 0;
    window_height = 0;
  }

  /* --------------------------------------------------------------------------------- */

  int load_config() {

    /* does the config file exist. */
    std::string config_file = rx_to_data_path("settings/settings.xml");
    if (false == rx_file_exists(config_file)) {
      RX_ERROR("Cannot load configuration file.");
      return -1;
    }

    std::ifstream ifs(config_file.c_str(), std::ios::in);
    if (false == ifs.is_open()) {
      RX_ERROR("Cannot open the configuration file.");
      return -2;
   
 }

    std::string xml_str;
    xml_str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
    if (0 == xml_str.size()) {
      RX_ERROR("The size of the xml settings file is 0.");
      return -3;
    }

    xml_document<> doc;
    try {
      doc.parse<0>((char*)xml_str.c_str());

      /* get settings. */
      xml_node<>* cfg = doc.first_node("settings");
      if (NULL == cfg) {
        RX_ERROR("Cannot find the settings element.");
        return -4;
      }

      /* mosaic */
      mos::config.webcam_device = read_xml_int(cfg, "webcam_device", 0);
      mos::config.webcam_width = read_xml_int(cfg, "webcam_width", 640);
      mos::config.webcam_height = read_xml_int(cfg, "webcam_height", 480);
      
      /* feature extractor and matcher */
      fex::config.raw_filepath = rx_to_data_path(read_xml_str(cfg, "raw_filepath", "input_raw/"));
      fex::config.resized_filepath = rx_to_data_path(read_xml_str(cfg, "resized_filepath", "input_resized/"));
      fex::config.blurred_filepath = rx_to_data_path(read_xml_str(cfg, "blurred_filepath", "input_blurred/"));
      fex::config.input_tile_size = read_xml_int(cfg, "input_tile_size", 16);
      fex::config.file_tile_width = read_xml_int(cfg, "file_tile_width", 64);
      fex::config.file_tile_height = read_xml_int(cfg, "file_tile_height", 64);
      fex::config.memory_pool_size = read_xml_int(cfg, "memory_pool_size", 1000);
      fex::config.input_image_width = mos::config.webcam_width;
      fex::config.input_image_height = mos::config.webcam_height;
      fex::config.cols = (fex::config.input_image_width / fex::config.input_tile_size);
      fex::config.rows = (fex::config.input_image_height / fex::config.input_tile_size);
    }
    catch (...) {
      RX_ERROR("Caught XML exception, check if the settings xml is valid.");
      return -5;
    }

    return 0;
  }

  int save_config() {
    return 0;
  }

  template<class T> T read_xml(xml_node<>* node, std::string name, T def) {

    if (NULL == node) {
      RX_ERROR("XML node is invalid.");
      return def;
    }

    xml_node<>* v = node->first_node(name.c_str());
    if (NULL == v) {
      RX_ERROR("Element: %s not found in xml", name.c_str());
      return def;
    }

    T result;    
    std::stringstream ss(v->value());
    ss >> result;
    return result;
  }

  static std::string read_xml_str(xml_node<>* node, std::string name, std::string def) {
    return read_xml<std::string>(node, name, def);
  }

  static int read_xml_int(xml_node<>* node, std::string name, int def) {
    return read_xml<int>(node, name, def);
  }
  
} /* namespace mos */
