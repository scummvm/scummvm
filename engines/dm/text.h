#ifndef DM_TEXT_H
#define DM_TEXT_H

#include "dm.h"
#include "gfx.h"

namespace DM {

class TextMan {
	DMEngine *_vm;
public:
	TextMan(DMEngine *vm);
	void printTextToBitmap(byte *destBitmap, uint16 destPixelWidth, uint16 destX, uint16 destY,
						   Color textColor, Color bgColor, char *text, uint16 destHeight, Viewport &viewport = gDefultViewPort); // @ F0040_TEXT_Print
	void printTextToScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, char *text, Viewport &viewport = gDefultViewPort); 
};

}

#endif
