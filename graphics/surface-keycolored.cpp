#include "graphics/surface-keycolored.h"

namespace Graphics {

void SurfaceKeyColored::blit(Surface *surf_src, int16 x, int16 y, OverlayColor transparent) {
	
	if (bytesPerPixel != sizeof(OverlayColor) || surf_src->bytesPerPixel != sizeof(OverlayColor)) return ;

	const OverlayColor *src = (const OverlayColor*)surf_src->pixels;
	int blitW = surf_src->w;
	int blitH = surf_src->h;

	// clip co-ordinates
	if (x < 0) {
		blitW += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		blitH += y;
		src -= y * surf_src->w;
		y = 0;
	}
	if (blitW > w - x) blitW = w - x;
	if (blitH > h - y) blitH = h - y;
	if (blitW <= 0 || blitH <= 0)
		return;

	OverlayColor *dst = (OverlayColor*) getBasePtr(x, y);
	int dstAdd = w - blitW;
	int srcAdd = surf_src->w - blitW;

	for (int i = 0; i < blitH; ++i) { 
		for (int j = 0; j < blitW; ++j, ++dst, ++src) { 
			OverlayColor col = *src;
			if (col != transparent)
				*dst = col;
		}
		dst += dstAdd;
		src += srcAdd; 
	}
}

} // end of namespace Graphics
