#ifndef SAGADRAW_H
#define SAGADRAW_H

#include "glk/glk_types.h"

namespace Glk {
namespace Scott {

struct Image {
	uint8_t *_imageData;
	uint8_t _xOff;
	uint8_t _yOff;
	uint8_t _width;
	uint8_t _height;
};

uint8_t *drawSagaPictureFromData(uint8_t *dataptr, int xSize, int ySize, int xOff, int yOff);
void drawSagaPictureNumber(int pictureNumber);

void putPixel(glsi32 x, glsi32 y, int32_t color);
void rectFill(int32_t x, int32_t y, int32_t width, int32_t height, int32_t color);

int32_t remap(int32_t color);

} // End of namespace Scott
} // End of namespace Glk

#endif
