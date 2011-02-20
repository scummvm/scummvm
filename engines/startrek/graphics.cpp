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

Graphics::Graphics(StarTrekEngine *vm) : _vm(vm), _egaMode(false) {
	_font = 0;
	_egaData = 0;

	if (ConfMan.hasKey("render_mode"))
		_egaMode = (Common::parseRenderMode(ConfMan.get("render_mode").c_str()) == Common::kRenderEGA) && (_vm->getGameType() != GType_STJR) && !(_vm->getFeatures() & GF_DEMO);

	if (_vm->getGameType() == GType_ST25 && _vm->getPlatform() == Common::kPlatformDOS)
		_font = new Font(_vm);
}

Graphics::~Graphics() {
	if (_egaData)
		free(_egaData);

	delete _font;
}

void Graphics::setPalette(const char *paletteFile) {
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

void Graphics::drawImage(const char *filename) {
	// Draw a regular bitmap

	Common::SeekableReadStream *imageStream = _vm->openFile(filename);
	uint16 xoffset = (_vm->getPlatform() == Common::kPlatformAmiga) ? imageStream->readUint16BE() : imageStream->readUint16LE();
	uint16 yoffset = (_vm->getPlatform() == Common::kPlatformAmiga) ? imageStream->readUint16BE() : imageStream->readUint16LE();
	uint16 width = (_vm->getPlatform() == Common::kPlatformAmiga) ? imageStream->readUint16BE() : imageStream->readUint16LE();
	uint16 height = (_vm->getPlatform() == Common::kPlatformAmiga) ? imageStream->readUint16BE() : imageStream->readUint16LE();	

	byte *pixels = (byte *)malloc(width * height);

	if (_egaMode && _egaData) {
		// FIXME: This doesn't work right
		for (uint32 i = 0; i < (uint32)(width * height); i++)
			pixels[i] = _egaData[imageStream->readByte()];
	} else {
		imageStream->read(pixels, width * height);
	}

	_vm->_system->copyRectToScreen(pixels, width, xoffset, yoffset, width, height);
	_vm->_system->updateScreen();
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

}
