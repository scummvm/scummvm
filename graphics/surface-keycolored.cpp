#include "graphics/surface-keycolored.h"

namespace Graphics {

void SurfaceKeyColored::blit(Surface *surf_src, int16 x, int16 y, OverlayColor trans) {
	
	if (bytesPerPixel != sizeof(OverlayColor) || surf_src->bytesPerPixel != sizeof(OverlayColor)) return ;

	OverlayColor *dst = (OverlayColor*) getBasePtr(x, y);
	const OverlayColor *src = (const OverlayColor*)surf_src->pixels;

	int blitW = (surf_src->w + x > w) ? w - x : surf_src->w;
	int blitH = (surf_src->h + y > h) ? h - y : surf_src->h;
	int dstAdd = w - blitW;
	int srcAdd = surf_src->w - blitW;

	for (int i = 0; i < blitH; ++i) { 
		for (int j = 0; j < blitW; ++j, ++dst, ++src) { 
			OverlayColor col = *src;
			if (col != trans)
				*dst = col;
		}
		dst += dstAdd;
		src += srcAdd; 
	}
}

} // end of namespace Graphics