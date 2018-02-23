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
 *
 * $URL: https://scummvm-startrek.googlecode.com/svn/trunk/graphics.cpp $
 * $Id: graphics.cpp 14 2010-05-26 15:44:12Z clone2727 $
 *
 */

#include "startrek/graphics.h"

#include "common/config-manager.h"
#include "common/rendermode.h"
#include "graphics/palette.h"

namespace StarTrek {

// Bitmap class

Bitmap::Bitmap(Common::ReadStreamEndian *stream) {
	xoffset = stream->readUint16();
	yoffset = stream->readUint16();
	width = stream->readUint16();
	height = stream->readUint16();

	pixels = (byte*)malloc(width*height);
	stream->read(pixels, width*height);
}

Bitmap::Bitmap(int w, int h) : width(w), height(h) {
	pixels = (byte*)malloc(width*height);
}

Bitmap::~Bitmap() {
	free(pixels);
}


// Graphics class

Graphics::Graphics(StarTrekEngine *vm) : _vm(vm), _egaMode(false) {
	_font = 0;
	_egaData = 0;
	_priData = 0;

	if (ConfMan.hasKey("render_mode"))
		_egaMode = (Common::parseRenderMode(ConfMan.get("render_mode").c_str()) == Common::kRenderEGA) && (_vm->getGameType() != GType_STJR) && !(_vm->getFeatures() & GF_DEMO);

	if (_vm->getGameType() == GType_ST25 && _vm->getPlatform() == Common::kPlatformDOS)
		_font = new Font(_vm);

	_backgroundImage = new Bitmap(_vm->openFile("DEMON0.BMP"));
	_canvas = new Bitmap(SCREEN_WIDTH, SCREEN_HEIGHT);
}

Graphics::~Graphics() {
	if (_egaData)
		free(_egaData);
	if (_priData)
		free(_priData);

	delete _font;
	delete _backgroundImage;
	delete _canvas;
}

void Graphics::loadPalette(const char *paletteFile) {
	// Set the palette from a PAL file

	Common::SeekableReadStream *palStream = _vm->openFile(paletteFile);
	byte *palette = new byte[256 * 3];
	palStream->read(palette, 256 * 3);

	// Expand color components
	if (_vm->getPlatform() == Common::kPlatformDOS || _vm->getPlatform() == Common::kPlatformMacintosh)
		for (uint16 i = 0; i < 256 * 3; i++)
			palette[i] <<= 2;

	_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);

	delete[] palette;
	delete palStream;
}

void Graphics::loadPri(const char *priFile) {
	Common::SeekableReadStream *priStream = _vm->openFile(priFile);

	free(_priData);
	_priData = (byte*)malloc(SCREEN_WIDTH*SCREEN_HEIGHT/2);
	priStream->read(_priData, SCREEN_WIDTH*SCREEN_HEIGHT/2);
}

void Graphics::redrawScreen() {
	// TODO: get rid of _canvas for efficiency
	memcpy(_canvas->pixels, _backgroundImage->pixels, SCREEN_WIDTH*SCREEN_HEIGHT);

	// drawSprite test
	Sprite spr;
	memset(&spr,0,sizeof(Sprite));
	spr.bitmap = new Bitmap(_vm->openFile("MWALKE00.BMP"));
	spr.drawPriority = 1;
	spr.drawX = 150;
	spr.drawY = 30;

	drawSprite(spr, Common::Rect(spr.drawX,spr.drawY,spr.drawX+spr.bitmap->width,spr.drawY+spr.bitmap->height));

	drawBitmapToScreen(_canvas);
}

// rect is the rectangle on-screen to draw. It should encompass the sprite itself, but if
// it doesn't, part of the sprite will be cut off.
void Graphics::drawSprite(const Sprite &sprite, const Common::Rect &rect) {
	int drawWidth = rect.right - rect.left;
	int drawHeight = rect.bottom - rect.top;

	if (drawWidth <= 0 || drawHeight <= 0)
		error("drawSprite: w/h <= 0");
	

	byte *dest = _canvas->pixels + rect.top*SCREEN_WIDTH + rect.left;
	int canvasOffsetToNextLine = SCREEN_WIDTH - drawWidth;

	switch(sprite.drawMode) {
	case 0: { // Normal sprite
		byte *src = sprite.bitmap->pixels + (rect.left - sprite.drawX)
			+ (rect.top - sprite.drawY) * sprite.bitmap->width;

		int spriteOffsetToNextLine = sprite.bitmap->width - drawWidth;
		int priOffset = rect.top*SCREEN_WIDTH + rect.left;

		for (int y=rect.top; y<rect.bottom; y++) {
			for (int x=rect.left; x<rect.right; x++) {
				byte priByte = _priData[priOffset/2];
				byte bgPri;
				if ((priOffset%2) == 1)
					bgPri = (priByte)&0xf;
				else
					bgPri = (priByte)>>4;
				priOffset++;

				byte b = *src++;
				if (b == 0 || sprite.drawPriority < bgPri) {
					dest++;
					continue;
				}
				*dest++ = b;
			}

			src += spriteOffsetToNextLine;
			dest += canvasOffsetToNextLine;
			priOffset += canvasOffsetToNextLine;
		}
		break;
	}

	case 1:
	case 2:
	case 3:
	default:
		error("drawSprite: draw mode %d not implemented", sprite.drawMode);
		break;
	}
}


void Graphics::loadEGAData(const char *filename) {
	// Load EGA palette data
	if (!_egaMode)
		return;

	if (!_egaData)
		_egaData = (byte *)malloc(256);

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

	byte *pixels = (byte *)malloc(width * height);
	imageStream->read(pixels, width * height);

	_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
	_vm->_system->copyRectToScreen(pixels, width, xoffset, yoffset, width, height);
	_vm->_system->updateScreen();

	delete[] palette;
	delete imageStream;
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
