
// Memory allocator for TinyGL

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

// modify these functions so that they suit your needs

void gl_free(void *p) {
    free(p);
}

void *gl_malloc(int size) {
    return malloc(size);
}

void *gl_zalloc(int size) {
    return calloc(1, size);
}

} // end of namespace TinyGL
