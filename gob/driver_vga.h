#ifndef DRIVER_VGA
#define DRIVER_VGA

#include "video.h"

namespace Gob {

class VGAVideoDriver : public VideoDriver {
public:
	VGAVideoDriver() {}
        virtual ~VGAVideoDriver() {}
        void drawSprite(SurfaceDesc *source, SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp);
        void fillRect(SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, byte color);
        void putPixel(int16 x, int16 y, byte color, SurfaceDesc *dest);
        void drawLetter(char item, int16 x, int16 y, FontDesc *fontDesc, byte color1, byte color2, byte transp, SurfaceDesc *dest);
	void drawLine(SurfaceDesc *dest, int16 x0, int16 y0, int16 x1, int16 y1, byte color);
	void drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y, byte transp, SurfaceDesc *dest);
};

}

#endif
