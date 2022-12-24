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

bool Draw_v7::loadCursorFromFile(int cursorIndex) {
	if (cursorIndex < 0 || cursorIndex >= 40)
		return false;

	Common::String cursorName = _cursorNames[cursorIndex];

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

	// Make sure the cursors sprite it big enough
	resizeCursors(cursor->getWidth(), cursor->getHeight(), cursorIndex + 1, true);

	Surface cursorSurf(cursor->getWidth(), cursor->getHeight(), 1, cursor->getSurface());

	_vm->_draw->_cursorSprites->blit(cursorSurf, cursorIndex * _vm->_draw->_cursorWidth, 0);

	memcpy(_vm->_draw->_cursorPalettes + cursorIndex * 256 * 3, cursor->getPalette(), cursor->getPaletteCount() * 3);

	_vm->_draw->_doCursorPalettes   [cursorIndex] = true;
	_vm->_draw->_cursorKeyColors    [cursorIndex] = cursor->getKeyColor();
	_vm->_draw->_cursorPaletteStarts[cursorIndex] = cursor->getPaletteStartIndex();
	_vm->_draw->_cursorPaletteCounts[cursorIndex] = cursor->getPaletteCount();
	_vm->_draw->_cursorHotspotsX    [cursorIndex] = cursor->getHotspotX();
	_vm->_draw->_cursorHotspotsY    [cursorIndex] = cursor->getHotspotY();

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
		// .-- _draw_animateCursorSUB2 ---
		if (cursorIndex == _cursorIndex) {
			if ((_cursorAnimDelays[_cursorIndex] != 0) &&
				((_cursorTimeKey + (_cursorAnimDelays[_cursorIndex] * 10)) <=
				 _vm->_util->getTimeKey())) {
				_cursorAnim++;
				if ((_cursorAnimHigh[_cursorIndex] < _cursorAnim) ||
					(_cursorAnimLow[_cursorIndex] > _cursorAnim))
					_cursorAnim = _cursorAnimLow[_cursorIndex];
				_cursorTimeKey = _vm->_util->getTimeKey();
			} else {
				if (_noInvalidated && (_vm->_global->_inter_mouseX == _cursorX) &&
					(_vm->_global->_inter_mouseY == _cursorY)) {
					_vm->_video->waitRetrace();
					return;
				}
			}
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

		bool cursorChanged = _cursorNames[cursorIndex] != _cursorName;

		if (((!_cursorNames[cursorIndex].empty() || _isCursorFromExe)) && cursorChanged) {
			_cursorName = _cursorNames[cursorIndex];

			// If the cursor name is empty, that cursor will be drawn by the scripts
			if (_cursorNames[cursorIndex].empty() || _cursorNames[cursorIndex] == "VIDE") { // "VIDE" is "empty" in french
				for (int i = 0; i < 40; i++) {
					_cursorNames[i].clear();
				}

				_isCursorFromExe = false;

				// Make sure the cursors sprite is big enough and set to non-extern palette
				resizeCursors(-1, -1, cursorIndex + 1, true);
				for (int i = 0; i < _vm->_draw->_cursorCount; i++) {
					_vm->_draw->_doCursorPalettes[i] = false;
				}

				_cursorX =  _vm->_global->_inter_mouseX;
				_cursorY =  _vm->_global->_inter_mouseY;
				_showCursor &= ~1;
				return;
			} else {
				// Clear the cursor sprite at that index
				_vm->_draw->_cursorSprites->fillRect(cursorIndex * _vm->_draw->_cursorWidth, 0,
													 cursorIndex * _vm->_draw->_cursorWidth + _vm->_draw->_cursorWidth - 1,
													 _vm->_draw->_cursorHeight - 1, 0);

				if (!loadCursorFromFile(cursorIndex)) {
					_cursorX =  _vm->_global->_inter_mouseX;
					_cursorY =  _vm->_global->_inter_mouseY;
					return;
				}

				_isCursorFromExe = true;
			}
		}

		if (cursorChanged || !_isCursorFromExe) {
			hotspotX = 0;
			hotspotY = 0;

			if (_cursorHotspotXVar != -1) {
				hotspotX = (uint16)VAR(_cursorIndex + _cursorHotspotXVar);
				hotspotY = (uint16)VAR(_cursorIndex + _cursorHotspotYVar);
			} else if (_cursorHotspotX != -1) {
				hotspotX = _cursorHotspotX;
				hotspotY = _cursorHotspotY;
			} else if (_cursorHotspotsX != nullptr) {
				hotspotX = _cursorHotspotsX[_cursorIndex];
				hotspotY = _cursorHotspotsY[_cursorIndex];
			}

			newX = _vm->_global->_inter_mouseX - hotspotX;
			newY = _vm->_global->_inter_mouseY - hotspotY;

			_scummvmCursor->clear();
			_scummvmCursor->blit(*_cursorSprites,
								 cursorIndex * _cursorWidth, 0,
								 (cursorIndex + 1) * _cursorWidth - 1,
								 _cursorHeight - 1, 0, 0);

			uint32 keyColor = 0;
			if (_doCursorPalettes && _cursorKeyColors && _doCursorPalettes[cursorIndex])
				keyColor = _cursorKeyColors[cursorIndex];

			CursorMan.replaceCursor(_scummvmCursor->getData(),
									_cursorWidth, _cursorHeight, hotspotX, hotspotY, keyColor, false, &_vm->getPixelFormat());

			if (_doCursorPalettes && _doCursorPalettes[cursorIndex]) {
				CursorMan.replaceCursorPalette(_cursorPalettes + (cursorIndex * 256 * 3),
											   _cursorPaletteStarts[cursorIndex], _cursorPaletteCounts[cursorIndex]);
				CursorMan.disableCursorPalette(false);
			} else
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

			if (!cursorChanged)
				return;
		}
	} else {
		blitCursor();
		_cursorX = newX;
		_cursorY = newY;
	}

	_showCursor &= ~1;
}

} // End of namespace Gob
