#include "common/rect.h"

#include "graphics/primitives.h"

#include "prisoner/prisoner.h"
#include "prisoner/screen.h"

namespace Prisoner {

int *Screen::gfxPrimitivesPolyInts = NULL;
uint Screen::gfxPrimitivesPolyAllocated = 0;

Screen::Screen(PrisonerEngine *vm) : _vm(vm) {
	_workScreen = new Graphics::Surface();
	_workScreen->create(640, 480, 1);
	setClipRect(0, 0, 640, 480);

	for (int16 i = 0; i < 256; i++)
		_fontColorTable[i] = i;

	/* Palette transparency */
	_pttColorIncr = 0;

}

Screen::~Screen() {
	delete _workScreen;
}

void Screen::update() {

	_vm->_system->copyRectToScreen((byte*)_workScreen->pixels, _workScreen->pitch, 0, 0, _workScreen->w, _workScreen->h);
	_vm->_system->updateScreen();

}

void Screen::setPartialPalette(byte *palette, int start, int count) {
	byte colors[1024];
	for (int i = start; i < count; i++) {
		colors[i * 4 + 0] = palette[i * 3 + 0];
		colors[i * 4 + 1] = palette[i * 3 + 1];
		colors[i * 4 + 2] = palette[i * 3 + 2];
		colors[i * 4 + 3] = 0;
	}
	_vm->_system->setPalette(colors, start, count);
}

void Screen::setFullPalette(byte *palette) {
	setPartialPalette(palette, 0, 256);
}

void Screen::clear() {
	memset(_workScreen->pixels, 0, _workScreen->pitch * _workScreen->h);
}

void Screen::putPixel(int x, int y, byte color) {
#if 0
	if (x >= _clipX1 && x < _clipX2 && y >= _clipY1 && y < _clipY2)
		_workScreen[x + y * 320] = color;
#endif
}

void Screen::hLine(int x, int y, int x2, byte color) {
	_workScreen->hLine(x, y, x2, color);
}

void Screen::vLine(int x, int y, int y2, byte color) {
	_workScreen->vLine(x, y, y2, color);
}

void Screen::drawLine(int x1, int y1, int x2, int y2, byte color) {
	_workScreen->drawLine(x1, y1, x2, y2, color);
}

void Screen::fillRect(int x1, int y1, int x2, int y2, byte color) {
	_workScreen->fillRect(Common::Rect(x1, y1, x2, y2), color);
}

void Screen::frameRect(int x1, int y1, int x2, int y2, byte color) {
	_workScreen->frameRect(Common::Rect(x1, y1, x2, y2), color);
}

void Screen::drawAnimationCelSprite(AnimationCel &cel, int16 x, int16 y, uint16 flags) {

	byte *frameData = cel.data;

	int width = cel.width;
	int lineWidth = width;
	int height = cel.height;
	int skipX = 0;

	flags ^= (cel.flags >> 8);

	// TODO: More clipping
	y -= height;
	y++;

	if (x + width - 1 > _clipX2)
		width = _clipX2 - x + 1;

	if (y + height - 1 > _clipY2)
		height = _clipY2 - y + 1;

	if (y < _clipY1) {
		if (y + height - 1 < _clipY1)
			return;
		height -= _clipY1 - y;
		if (cel.flags & 0x4000) {
			while (y < _clipY1) {
				frameData += cel.width;
				y++;
			}
		} else {
			while (y < _clipY1) {
				// Skip the clipped RLE-compressed line
				byte chunks = *frameData++;
				while (chunks--)
					frameData += 3 + frameData[1] * 4 + frameData[2];
				frameData++;
				y++;
			}
		}
	}

	if (x < _clipX1) {
		if (x + width - 1 < _clipX1)
			return;
		skipX = _clipX1 - x;
		x = _clipX1;
	}

	if (x > _clipX2 || y > _clipY2)
		return;

	byte *screenDestPtr = (byte*)_workScreen->getBasePtr(x, y);
	byte *linePixels;
	byte lineBuffer[640];

	while (height--) {

		if (cel.flags & 0x4000) {
			linePixels = frameData;
			frameData += cel.width;
		} else {
			// TODO: Possibly merge decompression and drawing of pixels
			memset(lineBuffer, 0, lineWidth);
			// Decompress the current pixel row
			byte chunks = *frameData++;
			byte *lineBufferPtr = lineBuffer;
			while (chunks--) {
				byte skip = frameData[0];
				uint count = frameData[1] * 4 + frameData[2];
				frameData += 3;
				lineBufferPtr += skip;
				memcpy(lineBufferPtr, frameData, count);
				lineBufferPtr += count;
				frameData += count;
			}
			memset(lineBufferPtr, 0, *frameData++);
			linePixels = lineBuffer;
		}

		// Draw the decompressed pixels
		if (flags & 0x8000) {
			for (int xc = skipX; xc < width; xc++) {
				if (linePixels[lineWidth-xc-1] != 0)
					screenDestPtr[xc-skipX] = linePixels[lineWidth-xc-1];
			}
		} else {
			for (int xc = skipX; xc < width; xc++) {
				if (linePixels[xc] != 0)
					screenDestPtr[xc-skipX] = linePixels[xc];
			}
		}

		screenDestPtr += _workScreen->pitch;
	}

}

void Screen::drawAnimationElement(AnimationResource *animation, int16 elementIndex, int16 x, int16 y, uint16 parentFlags) {
	AnimationElement *element = animation->_elements[elementIndex];
	uint16 flags = element->flags | (parentFlags & 0xA0);
	for (Common::Array<AnimationCommand*>::iterator iter = element->commands.begin(); iter != element->commands.end(); iter++) {
		drawAnimationCommand(animation, (*iter), x, y, flags);
	}
}

void Screen::drawAnimationCommand(AnimationResource *animation, AnimationCommand *cmd, int16 x, int16 y, uint16 parentFlags) {

	//debug(8, "Screen::drawAnimationCommand() cmd = %d; points = %d", cmd->cmd, cmd->points.size());

	Common::Array<Common::Point> points;

	// The commands' points need to be adjusted according to the parentFlags and the x/y position
	points.reserve(cmd->points.size() + 1);
	for (uint i = 0; i < cmd->points.size(); i++) {
		int16 ax = cmd->points[i].x;
		int16 ay = cmd->points[i].y;
		// CHECKME: The flags?
		if (parentFlags & 0x8000)
			ax = -ax;
		if (parentFlags & 0x2000)
			ay = -ay;
		points.push_back(Common::Point(x + ax, y + ay));
	}

	switch (cmd->cmd) {

	case kActElement:
	{
		int16 subElementIndex = (cmd->arg2 << 8) | cmd->arg1;
		drawAnimationElement(animation, subElementIndex & 0x0FFF, points[0].x, points[0].y, parentFlags | (cmd->arg2 & 0xA0));
		break;
	}

	case kActCelSprite:
	{
		int16 celIndex = ((cmd->arg2 << 8) | cmd->arg1) & 0x0FFF;
		int16 celX = points[0].x, celY = points[0].y;
		if (parentFlags & 0x8000)
			celX -= animation->getCelWidth(celIndex);
		if (parentFlags & 0x2000)
			celY += animation->getCelHeight(celIndex);
		drawAnimationCelSprite(*animation->_cels[celIndex], celX, celY, parentFlags | (cmd->arg2 & 0xA0));
		break;
	}

	default:
		warning("Screen::drawAnimationCommand() Unknown command %d", cmd->cmd);

	}

}

int16 Screen::getTextWidth(FontResource *fontResource, const Common::String &text) {
	int16 width = 0;
	for (uint i = 0; i < text.size(); i++) {
		byte ch = text[i];
		if (fontResource->isValidChar(ch)) {
			width += fontResource->getFontChar(ch)->interletter; // TODO: + _interletter;
		}
	}
	return width;
}

void Screen::drawChar(const FontChar *fontChar, const byte *fontCharData, int16 x, int16 y) {

	byte *dest;
	int startX = 0;
	int startY = 0;
	int clipWidth = fontChar->width;
	int clipHeight = fontChar->height;

	if (fontChar->offset == 0)
		return;

	x -= fontChar->xoffs;
	y -= fontChar->yoffs;

	if (x < _clipX1) {
		startX = _clipX1 - x;
		clipWidth -= startX;
		x = _clipX1;
	}

	if (y < _clipY1) {
		startY = _clipY1 - y;
		clipHeight -= startY;
		y = _clipY1;
	}

	if (x + clipWidth > _clipX2) {
		clipWidth = _clipX2 - x;
	}

	if (y + clipHeight > _clipY2) {
		clipHeight = _clipY2 - y;
	}

	dest = (byte*)_workScreen->getBasePtr(x, y);
	fontCharData += startY * fontChar->width;

	for (int16 yc = 0; yc < clipHeight; yc++) {
		const byte *linePtr = fontCharData + startX;
		for (int16 xc = 0; xc < clipWidth; xc++) {
			byte color = _fontColorTable[*linePtr++];
			if (color)
				dest[xc] = color;
		}
		fontCharData += fontChar->width;
		dest += _workScreen->pitch;
	}

}

void Screen::drawText(FontResource *fontResource, const Common::String &text, int16 x, int16 y) {
	for (uint i = 0; i < text.size(); i++) {
		byte ch = text[i];
		if (fontResource->isValidChar(ch)) {
			const FontChar *fontChar = fontResource->getFontChar(ch);
			const byte *fontCharData = fontResource->getFontCharData(ch);
			drawChar(fontChar, fontCharData, x, y);
			x += fontChar->interletter; // TODO: + _interletter;
		}
	}
}

void Screen::setClipRect(int clipX1, int clipY1, int clipX2, int clipY2) {
	// The clipping rect is only used in drawAnimationCelSprite
	_clipX1 = clipX1;
	_clipY1 = clipY1;
	_clipX2 = clipX2;
	_clipY2 = clipY2;
}

void Screen::copyRectFrom(Graphics::Surface *source, int sourceX, int sourceY, int destX, int destY, int width, int height) {
	byte *src = (byte*)source->getBasePtr(sourceX, sourceY);
	byte *dst = (byte*)_workScreen->getBasePtr(destX, destY);
	while (height--) {
		memcpy(dst, src, width);
		src += source->pitch;
		dst += _workScreen->pitch;
	}
}

void Screen::setFontColorTable(byte index, byte color) {
	_fontColorTable[index] = color;
}

void Screen::initPaletteTransTable(byte colorIncr) {

	if (colorIncr == _pttColorIncr)
		return;

	_pttColorIncr = colorIncr;

	uint32 m = colorIncr;

	for (uint i = 0; i < 256; i++) {
		byte a = m / 100;
		_pttTable1[i] = a;
		_pttTable2[i] = i - a;
		m += colorIncr;
	}

}

void Screen::buildPaletteTransTable(byte *sourcePalette, byte color) {

	uint32 pttTable3[256], pttTable4[256], pttTable5[256];
	byte *src = sourcePalette;

	for (uint i = 0; i < 256; i++) {
		byte r = *src++;
		byte g = *src++;
		byte b = *src++;
		pttTable3[i] = (_pttTable1[r] << 16) | (_pttTable1[g] << 8) | _pttTable1[b];
		pttTable4[i] = (_pttTable2[r] << 16) | (_pttTable2[g] << 8) | _pttTable2[b];
		pttTable5[i] = (r << 16) | (g << 8) | b;
	}

	for (uint i = 0; i < 256; i++) {
		uint32 irgb = pttTable3[color] + pttTable4[i];
		uint32 minDistance = 0x7FFFFFFF;
		uint minColorIndex = 0;
		byte ir = (irgb >> 16) & 0xFF;
		byte ig = (irgb >> 8) & 0xFF;
		byte ib = irgb & 0xFF;
		for (uint j = 0; j < 256; j++) {
			byte jr = (pttTable5[j] >> 16) & 0xFF;
			byte jg = (pttTable5[j] >> 8) & 0xFF;
			byte jb = pttTable5[j] & 0xFF;
			uint32 distance = ABS(jr - ir) + ABS(jg - ig) + ABS(jb - ib);
			if (distance < minDistance) {
				minColorIndex = j;
				minDistance = distance;
			}
		}
		_paletteTransTable[i] = minColorIndex;
	}

}

void Screen::drawTransparentRect(int16 x1, int16 y1, int16 x2, int16 y2) {
	// TODO: Clipping
	int16 w = x2 - x1 + 1, h = y2 - y1 + 1;
	byte *pixels = (byte*)_workScreen->getBasePtr(x1, y1);
	while (h--) {
		for (int16 xc = 0; xc < w; xc++)
			pixels[xc] = _paletteTransTable[pixels[xc]];
		pixels += _workScreen->pitch;
	}
}

} // End of namespace Comet
