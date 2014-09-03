#include <fstream>
#include <sstream>
#include <rapidxml.hpp>
#include <mosaic/Config.h>
#include <topshop/Config.h>
#include <featurex/Config.h>

using namespace rapidxml;

namespace top {

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

  void Config::reset() {
    is_fullscreen = -1;
    window_width = -1;
    window_height = -1;
    mosaic_width = -1;
    mosaic_height = -1;
    mosaic_x = 0;
    mosaic_y = 0;
    grid_rows = -1;
    grid_cols = -1;
    grid_padding_x = -1;
    grid_padding_y = -1;
    grid_file_width = -1;
    grid_file_height = -1;
    left_grid_x = -1;
    left_grid_y = -1;
    right_grid_x = -1;
    right_grid_y = -1;
  }

  int Config::validate() {
    /* @todo, okay I'm aware this could have been done a bit more generic ^.^ */
    if (-1 == is_fullscreen) {
      RX_ERROR("Invalid fullscreen.");
      return -99;
    }
    if (-1 == window_width) {
      RX_ERROR("Invalid window width");
      return -100;
    }
    if (-1 == window_height) {
      RX_ERROR("Invalid window height.");
      return -101;
    }
    if (-1 == mosaic_width) {
      RX_ERROR("Invalid mosaic width");
      return -1;
    }
    if (-1 == mosaic_height) {
      RX_ERROR("Invalid mosac height");
      return -2;
    }
    if (0 == left_grid_filepath.size()) {
      RX_ERROR("Invalid left grid filepath.");
      return -3;
    }
    if (0 == right_grid_filepath.size()) {
      RX_ERROR("Invald right grid filepath.");
      return -4;
    }
    if (false == rx_is_dir(left_grid_filepath)) {
      RX_ERROR("%s is not a filepath.", left_grid_filepath.c_str());
      return -5;
    }
    if (false == rx_is_dir(right_grid_filepath)) {
      RX_ERROR("%s is not a filepath.", right_grid_filepath.c_str());
      return -6;
    }
    if (false == rx_is_dir(raw_left_grid_filepath)) {
      RX_ERROR("%s is not a filepath.", raw_left_grid_filepath.c_str());
      return -101;
    }
    if (false == rx_is_dir(raw_right_grid_filepath)) {
      RX_ERROR("%s is not a filepath.", raw_right_grid_filepath.c_str());
      return -101;
    }
    if (-1 == left_grid_x) {
      RX_ERROR("No left_grid_x setting found.");
      return -7;
    } 
    if (-1 == left_grid_y) {
      RX_ERROR("No left_grid_y setting found.");
      return -8;
    }
    if (-1 == right_grid_x) {
      RX_ERROR("No right_grid_x setting found.");
      return -9;
    }
    if (-1 == right_grid_y) {
      RX_ERROR("No right_grid_y setting found.");
      return -10;
    }
    if (-1 == grid_padding_x) {
      RX_ERROR("No grid_padding_x setting found.");
      return -11;
    }
    if (-1 == grid_padding_y) {
      RX_ERROR("No grid_padding_y setting found.");
      return -12;
    }
    if (-1 == grid_rows) {
      RX_ERROR("No grid_rows setting found.");
      return -13;
    }
    if (-1 == grid_cols) {
      RX_ERROR("No grid_cols setting found.");
      return -14;
    }
    if (-1 == grid_file_width) {
      RX_ERROR("No grid_file_width setting found.");
      return -15;
    }
    if (-1 == grid_file_height) {
      RX_ERROR("No grid_file_height setting found.");
      return -15;
    }

    return 0;
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
      xml_node<>* settings = doc.first_node("settings");
      if (NULL == settings) {
        RX_ERROR("Cannot find the settings element.");
        return -4;
      }

      /* what group to use */
      xml_attribute<>* att = settings->first_attribute("group");
      if (NULL == att) {
        RX_ERROR("No attribute `group` found in the settings.");
        return -5;
      }

      std::string group_name = att->value();
      if (0 == group_name.size()) {
        RX_ERROR("Invalid group name.");
        return -6;
      }

      xml_node<>* cfg = settings->first_node("group");
      if (NULL == cfg) {
        RX_ERROR("No group elements found.");
        return -7;
      }

      /* Find the settings group. */
      while(cfg) {
        xml_attribute<>* name = cfg->first_attribute("name");
        if (NULL == name) {
          RX_ERROR("No name attribute in group");
          cfg = NULL;
          break;
        }
        std::string w = name->value();
        if (0 == w.size()) {
          RX_ERROR("Invalid name attribute; empty");
          cfg = NULL;
          break;
        }

        if (group_name == w) {
          break;
        }

        cfg = cfg->next_sibling();
      }

      if (NULL == cfg) {
        RX_ERROR("Cannot find the settings group: %s", group_name.c_str());
        return -8;
      }

      /* @todo - we should rename filepath to dir as they are no paths. */

      /* topshop */
      top::config.is_fullscreen = read_xml_int(cfg, "fullscreen", -1);
      top::config.window_width = read_xml_int(cfg, "window_width", -1);
      top::config.window_height = read_xml_int(cfg, "window_height", -1);
      top::config.mosaic_width = read_xml_int(cfg, "mosaic_width", -1);
      top::config.mosaic_height = read_xml_int(cfg, "mosaic_height", -1);
      top::config.mosaic_x = read_xml_int(cfg, "mosaic_x", 0);
      top::config.mosaic_y = read_xml_int(cfg, "mosaic_y", 0);
      top::config.left_grid_filepath = read_xml_str(cfg, "left_grid_filepath", "");
      top::config.right_grid_filepath = read_xml_str(cfg, "right_grid_filepath", "");
      top::config.raw_right_grid_filepath = read_xml_str(cfg, "raw_right_grid_filepath", "");
      top::config.raw_left_grid_filepath = read_xml_str(cfg, "raw_left_grid_filepath", "");
      top::config.left_grid_x = read_xml_int(cfg, "left_grid_x", -1);
      top::config.left_grid_y = read_xml_int(cfg, "left_grid_y", -1);
      top::config.right_grid_x = read_xml_int(cfg, "right_grid_x", -1);
      top::config.right_grid_y = read_xml_int(cfg, "right_grid_y", -1);
      top::config.grid_padding_x = read_xml_int(cfg, "grid_padding_x", -1);
      top::config.grid_padding_y = read_xml_int(cfg, "grid_padding_y", -1);
      top::config.grid_rows = read_xml_int(cfg, "grid_rows", 10);
      top::config.grid_cols = read_xml_int(cfg, "grid_cols", 10);
      top::config.grid_file_width =read_xml_int(cfg, "grid_file_width", -1);
      top::config.grid_file_height =read_xml_int(cfg, "grid_file_height", -1);

      if (0 != top::config.raw_left_grid_filepath.size()) {
        top::config.raw_left_grid_filepath = rx_to_data_path(top::config.raw_left_grid_filepath);
      }
      if (0 != top::config.raw_right_grid_filepath.size()) {
        top::config.raw_right_grid_filepath = rx_to_data_path(top::config.raw_right_grid_filepath);
      }
      if (0 != top::config.left_grid_filepath.size()) {
        top::config.left_grid_filepath = rx_to_data_path(top::config.left_grid_filepath);
      }
      if (0 != top::config.right_grid_filepath.size()) {
        top::config.right_grid_filepath = rx_to_data_path(top::config.right_grid_filepath);
      }

      /* mosaic */
      mos::config.webcam_device = read_xml_int(cfg, "webcam_device", 0);
      mos::config.webcam_width = read_xml_int(cfg, "webcam_width", 640);
      mos::config.webcam_height = read_xml_int(cfg, "webcam_height", 480);
      mos::config.stream_url = read_xml_str(cfg, "stream_url", "");
      mos::config.stream_width = read_xml_int(cfg, "stream_width", 0);
      mos::config.stream_height = read_xml_int(cfg, "stream_height", 0);
      mos::config.analyzer_width =  read_xml_int(cfg, "analyzer_width", 0);
      mos::config.analyzer_height =  read_xml_int(cfg, "analyzer_height", 0);
            
      /* feature extractor and matcher */
      fex::config.raw_filepath = read_xml_str(cfg, "raw_filepath", "");
      fex::config.resized_filepath = read_xml_str(cfg, "resized_filepath", "");
      fex::config.blurred_filepath = read_xml_str(cfg, "blurred_filepath", "");
      fex::config.input_tile_size = read_xml_int(cfg, "input_tile_size", 16);
      fex::config.file_tile_width = read_xml_int(cfg, "file_tile_width", 64);
      fex::config.file_tile_height = read_xml_int(cfg, "file_tile_height", 64);
      fex::config.memory_pool_size = read_xml_int(cfg, "memory_pool_size", 1000);
      fex::config.input_image_width = mos::config.analyzer_width;
      fex::config.input_image_height = mos::config.analyzer_height;
      fex::config.cols = (fex::config.input_image_width / fex::config.input_tile_size);
      fex::config.rows = (fex::config.input_image_height / fex::config.input_tile_size);

      /* convert dirs to paths. */
      if (0 != fex::config.raw_filepath.size()) {
        fex::config.raw_filepath = rx_to_data_path(fex::config.raw_filepath);
      }
      if (0 != fex::config.resized_filepath.size()) {
        fex::config.resized_filepath = rx_to_data_path(fex::config.resized_filepath);
      }
      if (0 != fex::config.blurred_filepath.size()) {
        fex::config.blurred_filepath = rx_to_data_path(fex::config.blurred_filepath);
      }
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

} /* namespace top */
