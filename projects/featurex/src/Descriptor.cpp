#include <sstream>
#include <fstream>
#include <featurex/Descriptor.h>

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace fex {


  /* ---------------------------------------------------------------------------------- */

  Descriptor::Descriptor() {
    reset();
  }

  Descriptor::~Descriptor() {
    reset();
  }

  void Descriptor::reset() {
    average_color[0] = 0;
    average_color[1] = 0;
    average_color[2] = 0;
  }

  /* ---------------------------------------------------------------------------------- */

  int save_descriptors(std::string filename, std::vector<Descriptor>& descriptors) {

    /* open file. */
    std::ofstream ofs(filename.c_str());
    if (!ofs.is_open()) {
      RX_ERROR("Cannot save the descriptors; cannot open file");
      return -1;
    }
    
    for (size_t i = 0; i < descriptors.size(); ++i) {
      Descriptor& desc = descriptors[i];
      ofs << desc.filename 
          << " " << desc.average_color[0]
          << " " << desc.average_color[1]
          << " " << desc.average_color[2]
          << "\n";
    }

    ofs.close();

    return 0;
  }

  int load_descriptors(std::string filename, std::vector<Descriptor>& descriptors) {

    /* open the file. */
    std::ifstream ifs(filename.c_str());
    if (!ifs.is_open()) {
      RX_ERROR("Cannot load the descriptors; cannot open file %s", filename.c_str());
      return -1;
    }

    Descriptor desc;
    std::string line;

    while(std::getline(ifs, line)) {
      printf(">> %s\n", line.c_str());
      std::stringstream ss(line);
      ss >> desc.filename 
         >> desc.average_color[0]
         >> desc.average_color[1]
         >> desc.average_color[2];

      descriptors.push_back(desc);
    }

    RX_VERBOSE("Loaded %lu descriptors.", descriptors.size());

    ifs.close();

    return 0;
  }

} /* namespace fex */
