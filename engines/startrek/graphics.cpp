/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "startrek/graphics.h"

#include "common/config-manager.h"
#include "common/rendermode.h"
#include "graphics/palette.h"

namespace StarTrek {

Graphics::Graphics(StarTrekEngine *vm) : _vm(vm), _egaMode(false) {
	_font = nullptr;

	_egaData = nullptr;
	_priData = nullptr;
	_lutData = nullptr;

	_screenRect = Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT);

	if (ConfMan.hasKey("render_mode"))
		_egaMode = (Common::parseRenderMode(ConfMan.get("render_mode").c_str()) == Common::kRenderEGA) && (_vm->getGameType() != GType_STJR) && !(_vm->getFeatures() & GF_DEMO);

	if (_vm->getGameType() == GType_ST25 && _vm->getPlatform() == Common::kPlatformDOS)
		_font = new Font(_vm);

	_backgroundImage = new Bitmap(_vm->openFile("DEMON0.BMP"));
	_canvas = new Bitmap(SCREEN_WIDTH, SCREEN_HEIGHT);

	_numSprites = 0;
}

Graphics::~Graphics() {
	delete[] _egaData;
	delete[] _priData;
	delete[] _lutData;

	delete _font;
	delete _backgroundImage;
	delete _canvas;
}


void Graphics::loadEGAData(const char *filename) {
	// Load EGA palette data
	if (!_egaMode)
		return;

	if (!_egaData)
		_egaData = new byte[256];

	Common::SeekableReadStream *egaStream = _vm->openFile(filename);
	egaStream->read(_egaData, 256);
	delete egaStream;
}

void Graphics::drawBackgroundImage(const char *filename) {
	// Draw an stjr BGD image (palette built-in)

	Common::SeekableReadStream *imageStream = _vm->openFile(filename);
	byte *palette = new byte[256 * 3];
	imageStream->read(palette, 256 * 3);

	// Expand color components
	for (uint16 i = 0; i < 256 * 3; i++)
		palette[i] <<= 2;

	uint16 xoffset = imageStream->readUint16LE();
	uint16 yoffset = imageStream->readUint16LE();
	uint16 width = imageStream->readUint16LE();
	uint16 height = imageStream->readUint16LE();

	byte *pixels = new byte[width * height];
	imageStream->read(pixels, width * height);

	_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
	_vm->_system->copyRectToScreen(pixels, width, xoffset, yoffset, width, height);
	_vm->_system->updateScreen();

	delete[] palette;
	delete imageStream;
}


void Graphics::loadPalette(const Common::String &paletteName) {
	// Set the palette from a PAL file
	Common::String palFile = paletteName + ".PAL";
	Common::String lutFile = paletteName + ".LUT";

	Common::SeekableReadStream *palStream = _vm->openFile(palFile.c_str());
	byte *palette = new byte[256 * 3];
	palStream->read(palette, 256 * 3);

	// Expand color components
	if (_vm->getPlatform() == Common::kPlatformDOS || _vm->getPlatform() == Common::kPlatformMacintosh)
		for (uint16 i = 0; i < 256 * 3; i++)
			palette[i] <<= 2;

	_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);

	delete[] palette;
	delete palStream;

	// Load LUT file
	Common::SeekableReadStream *lutStream = _vm->openFile(lutFile.c_str());

	delete[] _lutData;
	_lutData = new byte[256];
	lutStream->read(_lutData, 256);

	delete lutStream;
}

void Graphics::loadPri(const char *priFile) {
	Common::SeekableReadStream *priStream = _vm->openFile(priFile);

	delete[] _priData;
	_priData = new byte[SCREEN_WIDTH*SCREEN_HEIGHT/2];
	priStream->read(_priData, SCREEN_WIDTH*SCREEN_HEIGHT/2);
}

void Graphics::redrawScreen() {
	// TODO: get rid of _canvas for efficiency
	memcpy(_canvas->pixels, _backgroundImage->pixels, SCREEN_WIDTH*SCREEN_HEIGHT);

	drawAllSprites();

	drawBitmapToScreen(_canvas);
}

void Graphics::drawSprite(const Sprite &sprite) {
	drawSprite(sprite, Common::Rect(sprite.drawX,sprite.drawY,sprite.drawX+sprite.bitmap->width-1,sprite.drawY+sprite.bitmap->height-1));
}

// rect is the portion of the sprite to update. It must be entirely contained within the
// sprite's actual, full rectangle.
void Graphics::drawSprite(const Sprite &sprite, const Common::Rect &rect) {
	Common::Rect spriteRect = Common::Rect(sprite.drawX, sprite.drawY,
			sprite.drawX+sprite.bitmap->width-1, sprite.drawY+sprite.bitmap->height-1);

	assert(_screenRect.contains(rect));
	assert(spriteRect.contains(rect));

	byte *dest = _canvas->pixels + rect.top*SCREEN_WIDTH + rect.left;

	switch(sprite.drawMode) {
	case 0: { // Normal sprite
		byte *src = sprite.bitmap->pixels + (rect.left - sprite.drawX)
			+ (rect.top - sprite.drawY) * sprite.bitmap->width;

		int priOffset = rect.top*SCREEN_WIDTH + rect.left;

		for (int y=rect.top; y<rect.bottom; y++) {
			for (int x=rect.left; x<rect.right; x++) {
				byte priByte = _priData[priOffset/2];
				byte bgPriority;
				if ((priOffset%2) == 1)
					bgPriority = priByte&0xf;
				else
					bgPriority = priByte>>4;
				priOffset++;

				byte b = *src++;
				if (b == 0 || sprite.drawPriority < bgPriority) {
					dest++;
					continue;
				}
				*dest++ = b;
			}

			src       += sprite.bitmap->width - rect.width();
			dest      += SCREEN_WIDTH - rect.width();
			priOffset += SCREEN_WIDTH - rect.width();
		}
		break;
	}

	case 1: // Invisible
		break;

	case 2: { // Normal sprite with darkened background for "transparent" pixels (and no priority)
		byte *src = sprite.bitmap->pixels + (rect.left - sprite.drawX)
			+ (rect.top - sprite.drawY) * sprite.bitmap->width;

		for (int y=rect.top; y<rect.bottom; y++) {
			for (int x=rect.left; x<rect.right; x++) {
				byte b = *src;

				if (b == 0) // Transparent (darken the pixel)
					*dest = _lutData[*dest];
				else // Solid color
					*dest = b;

				src++;
				dest++;
			}

			src += sprite.bitmap->width - rect.width();
			dest += SCREEN_WIDTH - rect.width();
		}

		break;
	}

	case 3: { // Text
		// The sprite's "bitmap" is not actually a bitmap, but instead the list of
		// characters to display.

		Common::Rect rectangle1;

		rectangle1.left   = (rect.left   - sprite.drawX)/8;
		rectangle1.top    = (rect.top    - sprite.drawY)/8;
		rectangle1.right  = (rect.right  - sprite.drawX)/8;
		rectangle1.bottom = (rect.bottom - sprite.drawY)/8;

		int drawWidth = rectangle1.width() + 1;
		int drawHeight = rectangle1.height() + 1;

		dest =_canvas->pixels + sprite.drawY*SCREEN_WIDTH + sprite.drawX
			+ rectangle1.top*8*SCREEN_WIDTH + rectangle1.left*8;

		byte *src = sprite.bitmap->pixels + rectangle1.top*sprite.bitmap->width/8 + rectangle1.left;

		for (int y=0; y<drawHeight; y++) {
			for (int x=0; x<drawWidth; x++) {
				byte c = *src;

				int textColor;
				if (c >= 0x10 && c <= 0x1A) // Border characters
					textColor = 0xb3;
				else
					textColor = sprite.textColor;

				byte *fontData = _font->getCharData(c);

				for (int i=0;i<8;i++) {
					for (int j=0;j<8;j++) {
						byte b = *fontData;

						if (b == 0) // Transparent: use lookup table to darken this pixel
							*dest = _lutData[*dest];
						else if (b == 0x78) // Inner part of character
							*dest = textColor;
						else // Outline of character
							*dest = b;

						fontData++;
						dest++;
					}
					dest += SCREEN_WIDTH - 8;
				}

				dest -= (SCREEN_WIDTH*8 - 8);
				src++;
			}

			src += sprite.bitmap->width/8 - drawWidth;
			dest += SCREEN_WIDTH*8 - drawWidth*8;

		}

		break;
	}

	default:
		error("drawSprite: draw mode %d invalid", sprite.drawMode);
		break;
	}
}

void Graphics::drawAllSprites() {
	// TODO: implement properly
	for (int i=0; i<_numSprites; i++) {
		Sprite *spr = _sprites[i];
		drawSprite(*spr);
	}
}

void Graphics::addSprite(Sprite *sprite) {
	if (_numSprites >= MAX_SPRITES)
		error("addSprite: too many sprites");

	// Initialize some fields
	sprite->drawMode = 0;
	sprite->field8 = 0;
	sprite->field16 = 0;

	sprite->rectangle1.top = -1;
	sprite->rectangle1.left = -1;
	sprite->rectangle1.bottom = -2;
	sprite->rectangle1.right = -2;

	_sprites[_numSprites++] = sprite;
}

void Graphics::delSprite(Sprite *sprite) {
	for (int i=0; i<_numSprites; i++) {
		if (sprite != _sprites[i])
			continue;

		_numSprites--;
		_sprites[i] = _sprites[_numSprites];
		return;
	}

	error("delSprite: sprite not in list");
}


void Graphics::drawBitmapToScreen(Bitmap *bitmap) {
	int xoffset = bitmap->xoffset;
	int yoffset = bitmap->yoffset;
	if (xoffset >= 320)
		xoffset = 0;
	if (yoffset >= 200)
		yoffset = 0;

	_vm->_system->copyRectToScreen(bitmap->pixels, bitmap->width, xoffset, yoffset, bitmap->width, bitmap->height);
	_vm->_system->updateScreen();
}


}
