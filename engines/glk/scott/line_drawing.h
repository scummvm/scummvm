#ifndef GLK_SCOTT_LINEDRAWING
#define GLK_SCOTT_LINEDRAWING

namespace Glk {
namespace Scott {

typedef unsigned char uint8_t;

struct LineImage {
	uint8_t *_data;
	int _bgColour;
	size_t _size;
};

struct PixelToDraw {
	uint8_t _x;
	uint8_t _y;
	uint8_t _colour;
};

//void drawVectorPicture(int image);
void drawSomeVectorPixels(int fromStart);
int drawingVector();

enum VectorStateType {
	NO_VECTOR_IMAGE,
	DRAWING_VECTOR_IMAGE,
	SHOWING_VECTOR_IMAGE
};

} // End of namespace Scott
} // End of namespace Glk

#endif
