#include "driver_vga.h"

#define STUB_FUNC	printf("STUB: %s\n", __PRETTY_FUNCTION__)

namespace Gob {

void VGAVideoDriver::drawSprite(SurfaceDesc *source, SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) {
	if (x >= 0 && x < dest->width && y >= 0 && y < dest->height) {
		int16 width = (right - left) + 1;
		int16 height = (bottom - top) + 1;
	
		byte *srcPos = source->vidPtr + (top * source->width) + left;
		byte *destPos = dest->vidPtr + (y * dest->width) + x;
		while (height--) {
			for (int16 i = 0; i < width; ++i) {
				if (srcPos[i])
					destPos[i] = srcPos[i];
			}

			srcPos += source->width; //width ?
			destPos += dest->width;
		}
	}
}

void VGAVideoDriver::fillRect(SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, byte color) {
	if (left < dest->width && right < dest->width && top < dest->height && bottom < dest->height) {
		byte *pos = dest->vidPtr + (top * dest->width) + left;
		int16 width = (right - left) + 1;
		int16 height = (bottom - top) + 1;
		while (height--) {
			for (int16 i = 0; i < width; ++i) {
				pos[i] = color;
			}

			pos += dest->width;
		}
	}
}

void VGAVideoDriver::putPixel(int16 x, int16 y, byte color, SurfaceDesc *dest) {
	if (x >= 0 && x < dest->width && y >= 0 && y < dest->height)
		dest->vidPtr[(y * dest->width) + x] = color;
}

void VGAVideoDriver::drawLetter(char item, int16 x, int16 y, FontDesc *fontDesc, byte color1, byte color2, byte transp, SurfaceDesc *dest) {
	STUB_FUNC;
}

void VGAVideoDriver::drawLine(SurfaceDesc *dest, int16 x0, int16 y0, int16 x1, int16 y1, byte color) {
	STUB_FUNC;
}

void VGAVideoDriver::drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y, byte transp, SurfaceDesc *dest) {
	STUB_FUNC;
}

}

