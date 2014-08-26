/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------
*/

#ifndef ROXLU_COMPARATOR_H
#define ROXLU_COMPARATOR_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_FONT
#include <tinylib.h>

#include <featurex/Descriptor.h>

namespace fex {

  class Comparator {
  public:
    Comparator();      
    ~Comparator();
    ssize_t match(Descriptor& input, std::vector<Descriptor>& database);

  public:

  };

} /* namespace fex */

#endif
