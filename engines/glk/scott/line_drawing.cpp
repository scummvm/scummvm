#include "line_drawing.h"
#include "globals.h"
#include "scott.h"
#include "saga_draw.h"

namespace Glk {
namespace Scott {

void drawSomeVectorPixels(int fromStart) {
	_G(_vectorState) = DRAWING_VECTOR_IMAGE;
	int i = _G(_currentDrawInstruction);
	if (fromStart)
		i = 0;
	if (i == 0)
		rectFill(0, 0, _G(_scottGraphicsWidth), _G(_scottGraphicsHeight), remap(_G(_bgColour)));
	for (; i < _G(_totalDrawInstructions) && (!_G(_gliSlowDraw) || i < _G(_currentDrawInstruction) + 50); i++) {
		PixelToDraw toDraw = *_G(_pixelsToDraw)[i];
		putPixel(toDraw._x, toDraw._y, remap(toDraw._colour));
	}
	_G(_currentDrawInstruction) = i;
	if (_G(_currentDrawInstruction) >= _G(_totalDrawInstructions)) {
		g_vm->glk_request_timer_events(0);
		_G(_vectorState) = SHOWING_VECTOR_IMAGE;
	}
}

int drawingVector() {
	return (_G(_totalDrawInstructions) > _G(_currentDrawInstruction));
}

} // End of namespace Scott
} // End of namespace Glk
