/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "common/formats/winexe_ne.h"
#include "common/formats/winexe_pe.h"
#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/global.h"
#include "gob/hotspots.h"
#include "gob/inter.h"
#include "gob/resources.h"
#include "gob/scenery.h"
#include "gob/script.h"

namespace Gob {

Draw_v7::Draw_v7(GobEngine *vm) : Draw_Playtoons(vm), _cursors(nullptr)  {
}

Draw_v7::~Draw_v7() {
	delete _cursors;
}

bool Draw_v7::loadCursorFile() {
	if (_cursors)
		return true;

	if (_vm->_dataIO->hasFile("cursor32.dll")) {
		_cursors = new Common::PEResources();
		if (_cursors->loadFromEXE("cursor32.dll"))
			return true;
	} else if (_vm->_dataIO->hasFile("cursor.dll")) {
		_cursors = new Common::NEResources();
		if (_cursors->loadFromEXE("cursor.dll"))
			return true;
	}

	delete _cursors;
	_cursors = nullptr;

	return false;
}

bool Draw_v7::loadCursorFromFile(Common::String cursorName) {
	Graphics::WinCursorGroup *cursorGroup = nullptr;
	Graphics::Cursor *defaultCursor = nullptr;

	// Load the cursor file and cursor group
	if (loadCursorFile())
		cursorGroup = Graphics::WinCursorGroup::createCursorGroup(_cursors, Common::WinResourceID(cursorName));

	// If the requested cursor does not exist, create a default one
	const Graphics::Cursor *cursor = nullptr;
	if (!cursorGroup || cursorGroup->cursors.empty() || !cursorGroup->cursors[0].cursor) {
		defaultCursor = Graphics::makeDefaultWinCursor();

		cursor = defaultCursor;
	} else
		cursor = cursorGroup->cursors[0].cursor;

	// Make sure the cursor sprite is big enough
	if (_scummvmCursor->getWidth() != cursor->getWidth() || _scummvmCursor->getHeight() != cursor->getHeight()) {
		_vm->_draw->_scummvmCursor.reset();
		_vm->_draw->_scummvmCursor = _vm->_video->initSurfDesc(cursor->getWidth(), cursor->getHeight(), SCUMMVM_CURSOR);
	}

	_scummvmCursor->clear();

	Surface cursorSurf(cursor->getWidth(), cursor->getHeight(), 1, cursor->getSurface());
	_scummvmCursor->blit(cursorSurf, 0, 0);

	CursorMan.replaceCursor(_scummvmCursor->getData(),
							cursor->getWidth(),
							cursor->getHeight(),
							cursor->getHotspotX(),
							cursor->getHotspotY(),
							cursor->getKeyColor(),
							false,
							&_vm->getPixelFormat(),
							cursor->getMask());
	CursorMan.replaceCursorPalette(cursor->getPalette(),
								   cursor->getPaletteStartIndex(),
								   cursor->getPaletteCount());
	CursorMan.disableCursorPalette(false);

	delete cursorGroup;
	delete defaultCursor;
	return true;
}

void Draw_v7::initScreen()
{
	_vm->_game->_preventScroll = false;

	_scrollOffsetX = 0;
	_scrollOffsetY = 0;

	if (!_spritesArray[kBackSurface] || _vm->_global->_videoMode != 0x18) {
		initSpriteSurf(kBackSurface, _vm->_video->_surfWidth, _vm->_video->_surfHeight, 0);
		_backSurface = _spritesArray[kBackSurface];
		_backSurface->clear();
	}

	if (!_spritesArray[kCursorSurface]) {
		initSpriteSurf(kCursorSurface, 32, 16, 2);
		_cursorSpritesBack = _spritesArray[kCursorSurface];
		_cursorSprites = _cursorSpritesBack;
		_scummvmCursor = _vm->_video->initSurfDesc(16, 16, SCUMMVM_CURSOR);
	}

	_spritesArray[kFrontSurface] = _frontSurface;
	_spritesArray[kBackSurface ] = _backSurface;

	_vm->_video->dirtyRectsAll();
}

void Draw_v7::animateCursor(int16 cursor) {
	if (!_cursorSprites)
		return;

	int16 cursorIndex = cursor;
	int16 newX = 0, newY = 0;
	uint16 hotspotX, hotspotY;

	_showCursor |= 1;

	// .-- _draw_animateCursorSUB1 ---
	if (cursorIndex == -1) {
		cursorIndex =
			_vm->_game->_hotspots->findCursor(_vm->_global->_inter_mouseX,
											  _vm->_global->_inter_mouseY);

		if (_cursorAnimLow[cursorIndex] == -1)
			cursorIndex = 1;
	}
	// '------

	if (_cursorAnimLow[cursorIndex] != -1) {
		// --- Advance cursor animation
		// TODO: Not sure if this is still valid in Adibou2/Adi4
		if (cursorIndex == _cursorIndex) {
			if ((_cursorAnimDelays[_cursorIndex] != 0) &&
				((_cursorTimeKey + (_cursorAnimDelays[_cursorIndex] * 10)) <=
				 _vm->_util->getTimeKey())) {
				_cursorAnim++;
				if ((_cursorAnimHigh[_cursorIndex] < _cursorAnim) ||
					(_cursorAnimLow[_cursorIndex] > _cursorAnim))
					_cursorAnim = _cursorAnimLow[_cursorIndex];
				_cursorTimeKey = _vm->_util->getTimeKey();
			} /* else { // Not found in Adibou 2 ASM
				if (_noInvalidated && (_vm->_global->_inter_mouseX == _cursorX) &&
					(_vm->_global->_inter_mouseY == _cursorY)) {
					_vm->_video->waitRetrace();
					return;
				}
			}*/
		} else {
			_cursorIndex = cursorIndex;
			if (_cursorAnimDelays[cursorIndex] != 0) {
				_cursorAnim = _cursorAnimLow[cursorIndex];
				_cursorTimeKey = _vm->_util->getTimeKey();
			}
		}

		if (_cursorAnimDelays[_cursorIndex] != 0) {
			if ((_cursorAnimHigh[_cursorIndex] < _cursorAnim) ||
				(_cursorAnimLow[_cursorIndex] > _cursorAnim))
				_cursorAnim = _cursorAnimLow[_cursorIndex];

			cursorIndex = _cursorAnim;
		}
		// '------

		if (cursorIndex == -1)
			return;

		bool cursorChanged = _cursorNames[cursorIndex] != _cursorName;
		if ((!_cursorDrawnFromScripts || !_cursorNames[cursorIndex].empty()) && cursorChanged) {
			_cursorName = _cursorNames[cursorIndex];
			// If the cursor name is empty, that cursor will be drawn by the scripts
			if (_cursorNames[cursorIndex].empty() || _cursorNames[cursorIndex] == "VIDE") { // "VIDE" is "empty" in french
				for (int i = 0; i < 40; i++) {
					_cursorNames[i].clear();
				}

				_cursorDrawnFromScripts = true;

				_cursorX =  _vm->_global->_inter_mouseX;
				_cursorY =  _vm->_global->_inter_mouseY;
				_showCursor &= ~1;
				return;
			} else {
				_cursorDrawnFromScripts = false;
				loadCursorFromFile(_cursorName);
				_cursorX =  _vm->_global->_inter_mouseX;
				_cursorY =  _vm->_global->_inter_mouseY;
			}
		}

		if (_cursorDrawnFromScripts) {
			hotspotX = 0;
			hotspotY = 0;

			if (_cursorHotspotXVar != -1) {
				hotspotX = (uint16)VAR(_cursorIndex + _cursorHotspotXVar);
				hotspotY = (uint16)VAR(_cursorIndex + _cursorHotspotYVar);
			} else if (_cursorHotspotX != -1) {
				hotspotX = _cursorHotspotX;
				hotspotY = _cursorHotspotY;
			}

			newX = _vm->_global->_inter_mouseX - hotspotX;
			newY = _vm->_global->_inter_mouseY - hotspotY;

			if (_scummvmCursor->getWidth() != _cursorWidth || _scummvmCursor->getHeight() != _cursorHeight) {
				_vm->_draw->_scummvmCursor.reset();
				_vm->_draw->_scummvmCursor = _vm->_video->initSurfDesc(_cursorWidth, _cursorHeight, SCUMMVM_CURSOR);
			}

			_scummvmCursor->clear();
			_scummvmCursor->blit(*_cursorSprites,
								 cursorIndex * _cursorWidth, 0,
								 (cursorIndex + 1) * _cursorWidth - 1,
								 _cursorHeight - 1, 0, 0);

			CursorMan.replaceCursor(_scummvmCursor->getData(),
									_cursorWidth, _cursorHeight, hotspotX, hotspotY, 0, false, &_vm->getPixelFormat());
			CursorMan.disableCursorPalette(true);
		}

		if (_frontSurface != _backSurface) {
			if (!_noInvalidated) {
				int16 tmp = _cursorIndex;
				_cursorIndex = -1;
				blitInvalidated();
				_cursorIndex = tmp;
			} else {
				_showCursor = 3;
				_vm->_video->waitRetrace();
				if (MIN(newY, _cursorY) < 50)
					_vm->_util->delay(5);
			}
		}

		if (!cursorChanged && !_cursorDrawnFromScripts)
			return;

	} else {
		blitCursor();
		_cursorX = newX;
		_cursorY = newY;
	}

	_showCursor &= ~1;
}

} // End of namespace Gob
