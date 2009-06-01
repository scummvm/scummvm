#ifndef ASYLUM_RESOURCE_H
#define ASYLUM_RESOURCE_H

#include "common/str.h"

namespace Asylum {

class Resource {
public:
    Resource();
    ~Resource();

    int load(String filename);
private:
    String _filename;

}; // end of class Resource

} // end of namespace Asylum

#endif
