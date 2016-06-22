#include "text.h"


namespace DM {

TextMan::TextMan(DMEngine* vm) : _vm(vm) {}

#define kLetterWidth 5
#define kLetterHeight 6

void TextMan::printTextToBitmap(byte* destBitmap, uint16 destPixelWidth, uint16 destX, uint16 destY, Color textColor, Color bgColor, char* text, uint16 destHeight, Viewport &viewport) {
	uint16 textLength = strlen(text);
	uint16 nextX = destX;
	uint16 nextY = destY;
	byte *srcBitmap = _vm->_displayMan->getBitmap(kFontGraphicIndice);

	byte *tmp = _vm->_displayMan->_tmpBitmap;
	for (uint16 i = 0; i < (kLetterWidth + 1) * (kLetterHeight + 1) * 128; ++i) {
		tmp[i] = srcBitmap[i] ? textColor : bgColor;
	}
	srcBitmap = tmp;


	for (char *begin = text, *end = text + textLength; begin != end; ++begin) {
		// Note: this does no wraps in the middle of words
		if (nextX + kLetterWidth + 1 > destPixelWidth || (*begin == '\n')) {
			nextX = destX;
			nextY += kLetterHeight + 1;
		}
		if (nextY + kLetterHeight + 1 > destHeight)
			break;
		uint16 srcX = (1 + 5) * toupper(*begin); // 1 + 5 is not the letter width, arbitrary choice of the unpacking code
		_vm->_displayMan->blitToBitmap(srcBitmap, 6 * 128, srcX, 0, destBitmap, destPixelWidth,
									   nextX, nextX + kLetterWidth + 1, nextY, nextY + kLetterHeight + 1, kColorNoTransparency, viewport);
		nextX += kLetterWidth + 1;
	}
}

void TextMan::printTextToScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, char* text, Viewport &viewport) {
	printTextToBitmap(_vm->_displayMan->_vgaBuffer, _vm->_displayMan->_screenWidth, destX, destY, textColor, bgColor, text, _vm->_displayMan->_screenHeight, viewport);
}

}
