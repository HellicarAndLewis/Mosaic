#include <topshop/ImageJSON.h>
#include <jansson.h>

namespace top {

  /* ---------------------------------------------------------- */

  static int read_element(json_t* el, int& result);
  static int read_element(json_t* el, std::string& result);

  template<class T> int read_type(json_t* el, std::string name, T& result);

  /* ---------------------------------------------------------- */
  
  ImageJSON::ImageJSON() {
  }

  ImageJSON::~ImageJSON() {
  }

  int ImageJSON::parse(std::string filepath, ImageInfo& result) {

    std::string json;
    json_t* root = NULL;
    json_error_t err; 
    json_t* el_user = NULL;
    json_t* el = NULL;
    
    if (0 == filepath.size()) {
      RX_ERROR("Invalid filepath (empty).");
      return -1;
    }

    if (false == rx_file_exists(filepath)) {
      RX_ERROR("Cannot find filepath: %s", filepath.c_str());
      return -2;
    }

    json = rx_read_file(filepath);
    if (0 == json.size()) {
      RX_ERROR("The json file is empty");
      return -3;
    }

    root = json_loads(json.c_str(), 0, &err);
    if (NULL == root) {
      RX_ERROR("Cannot parse json: %d, %s", err.line, err.text);
      return -4;
    }

    /* get user object */
    el_user = json_object_get(root, "user");
    if (NULL == el_user) {
      RX_ERROR("Cannot find the user object in json.");
      goto error;
    }

    /* username */
    if (0 != read_type<std::string>(el_user, "username", result.username)) {
      RX_ERROR("Cannot find username");
      goto error;
    } 

    /* and cleanup */
    json_decref(root);
    root = NULL;

    return 0;

  error:
    if (NULL != root) {
      json_decref(root);
      root = NULL;
    }
    el_user = NULL;
    el = NULL;
    return -1;
  }

  /* ---------------------------------------------------------- */

  static int read_element(json_t* field, int& result) {
    result = json_integer_value(field);
    return 0;
  }

  static int read_element(json_t* field, std::string& result) {
    result = json_string_value(field);
    return 0;
  }

  template<class T> int read_type(json_t* el, std::string name, T& result) {
    json_t* field = json_object_get(el, name.c_str());
    if (NULL == field) {
      RX_ERROR("Cannot find json field, %s", name.c_str());
      return -1;
    }

    return read_element(field, result);
  }

} /* namespace top */
