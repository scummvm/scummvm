
#ifndef GRAPHICS_SURFACE_KEYCOLORED_H
#define GRAPHICS_SURFACE_KEYCOLORED_H

#include "graphics/surface.h"

namespace Graphics {

struct SurfaceKeyColored : Surface {

	void blit(Surface *surf_src, int16 x, int16 y, OverlayColor trans);
};


} // end of namespace Graphics

#endif