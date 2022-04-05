#ifndef GLK_SCOTT_SAGADRAW
#define GLK_SCOTT_SAGADRAW

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

typedef uint8_t RGB[3];
typedef RGB PALETTE[16];

uint8_t *drawSagaPictureFromData(uint8_t *dataptr, int xSize, int ySize, int xOff, int yOff);
void drawSagaPictureNumber(int pictureNumber);

void sagaSetup(size_t imgOffset);

void putPixel(glsi32 x, glsi32 y, int32_t color);
void rectFill(int32_t x, int32_t y, int32_t width, int32_t height, int32_t color);
void definePalette();

int32_t remap(int32_t color);

} // End of namespace Scott
} // End of namespace Glk

#endif
