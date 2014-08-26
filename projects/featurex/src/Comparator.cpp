#include <featurex/Comparator.h>

namespace fex {
  
  Comparator::Comparator()  {
  }

  Comparator::~Comparator() {
  }

  ssize_t Comparator::match(Descriptor& input, std::vector<Descriptor>& database) {
    
    int64_t dr, dg, db;
    uint64_t dist;
    uint64_t min_dist = UINT64_MAX;
    ssize_t dx = -1;

    /* basic distance check for now */
    for (size_t i = 0; i < database.size(); ++i) {
      Descriptor& other = database[i];
      dr = input.average_color[0] - other.average_color[0];
      dg = input.average_color[1] - other.average_color[1];
      db = input.average_color[2] - other.average_color[2];
      dr = dr * dr;
      dg = dg * dg;
      db = db * db;
      dist = dr + dg + db;
      if (dist < min_dist) {
        min_dist = dist;
        dx = i;
      }
    }

    if (-1 == dx) {
      RX_ERROR("Not supposed to happen; but didn't find a match.");
    }

    return dx;
  }

} /* namespace fex */
