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
#include "dragons/credits.h"

#include "common/system.h"
#include "dragons/bigfile.h"
#include "dragons/dragons.h"
#include "dragons/screen.h"
#include "dragons/font.h"

namespace Dragons {

void creditsUpdateFunction() {
	getEngine()->_credits->update();
}

Credits::Credits(DragonsEngine *vm, FontManager *fontManager, BigfileArchive *bigfileArchive) : _vm(vm),
				_fontManager(fontManager), _bigfileArchive(bigfileArchive), _surface(nullptr), _curPtr(nullptr) {
	_running = false;
	_updateCounter = 0;
	_yOffset = 0;
	_linesRemaining = 0x1a;

	_creditsData = nullptr;
	_dataLength = 0;
	_curPosition = 0;
}

void Credits::start() {
	_surface = new Graphics::Surface();
	_surface->create(320, 208, Graphics::PixelFormat::createFormatCLUT8());
	_updateCounter = 0x78;
	_curPosition = 0;
	_creditsData = _bigfileArchive->load("credits.txt", _dataLength);
	_curPtr = (char *)_creditsData;
	assert(_creditsData);
	_vm->setVsyncUpdateFunction(creditsUpdateFunction);
	_running = true;
}

bool Credits::isRunning() {
	return _running;
}

void Credits::draw() {
	if (_running) {
		_vm->_screen->copyRectToSurface8bppWrappedY(*_surface, _vm->_screen->getPalette(2), _yOffset);
	}
}

void Credits::cleanup() {
	_vm->setVsyncUpdateFunction(nullptr);
	_surface->free();
	delete _surface;
}

void Credits::update() {
	uint16 line[41];
	if (_updateCounter == 0) {
		_updateCounter = 2;
		_yOffset = (_yOffset + 1) % 208;
		if (_yOffset % 8 == 0) {
			if (_curPosition < _dataLength) {
				uint32 length = strlen(_curPtr);
				debug(3, "Credit line: %s", _curPtr);
				convertToWideChar(line, (byte *)_curPtr, 40);
				_curPtr += length + 1;
				_curPosition += length + 1;
			} else {
				if (_linesRemaining) {
					_linesRemaining--;
				}
				convertToWideChar(line, (const byte *)" ", 40);
			}
			_fontManager->_fonts[0]->renderToSurface(_surface, 0, (_yOffset + 200) % 208, line, 40);

		}

	} else {
		_updateCounter--;
	}

	if (_linesRemaining == 0) {
		_running = false;
		cleanup();
	}

}

void Credits::convertToWideChar(uint16 *destBuf, const byte *text, uint16 maxLength) {
	bool finished = false;
	for (int i = 0; i < maxLength; i++) {
		if (text[i] == 0) {
			finished = true;
		}
		destBuf[i] = !finished ? text[i] : ' ';
	}
}

} // End of namespace Dragons
