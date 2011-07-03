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
 */

#include "graphics/palette.h"
#include "neverhood/screen.h"

namespace Neverhood {

Screen::Screen(NeverhoodEngine *vm)
	: _vm(vm), _paletteData(NULL), _paletteChanged(false) {
	
	_ticks = _vm->_system->getMillis();
	
	_backScreen = new Graphics::Surface();
	_backScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	
}

Screen::~Screen() {
	delete _backScreen;
}

void Screen::wait() {
}

void Screen::setFps(int fps) {
	_frameDelay = 1000 / fps;
}

int Screen::getFps() {
	return 1000 / _frameDelay;
}

void Screen::setPaletteData(byte *paletteData) {
	_paletteChanged = true;
	_paletteData = paletteData;
}

void Screen::unsetPaletteData(byte *paletteData) {
	if (_paletteData == paletteData) {
		_paletteChanged = false;
		_paletteData = NULL;
	}
}

void Screen::testPalette(byte *paletteData) {
	if (_paletteData == paletteData)
		_paletteChanged = true;
}

void Screen::updatePalette() {
	if (_paletteChanged && _paletteData) {
		byte *tempPalette = new byte[768];
		for (int i = 0; i < 256; i++) {
			tempPalette[i * 3 + 0] = _paletteData[i * 4 + 0];
			tempPalette[i * 3 + 1] = _paletteData[i * 4 + 1];
			tempPalette[i * 3 + 2] = _paletteData[i * 4 + 2];
		}
		_vm->_system->getPaletteManager()->setPalette(tempPalette, 0, 256);
		delete[] tempPalette;
		_paletteChanged = false;
	}
}

void Screen::clear() {
	memset(_backScreen->pixels, 0, _backScreen->pitch * _backScreen->h);
}

} // End of namespace Neverhood
