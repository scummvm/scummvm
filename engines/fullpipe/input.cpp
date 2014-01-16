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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/input.h"
#include "fullpipe/gfx.h"
#include "fullpipe/scene.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/statics.h"
#include "fullpipe/interaction.h"
#include "fullpipe/constants.h"

namespace Fullpipe {

InputController::InputController() {
	g_fp->_inputController = this;

	_flag = 0;
	_cursorHandle = 0;
	_hCursor = 0;
	_field_14 = 0;
	_cursorId = 0;
	_cursorIndex = -1;
	_inputFlags = 1;

	_cursorBounds.left = 0;
	_cursorBounds.top = 0;
	_cursorBounds.right = 0;
	_cursorBounds.bottom = 0;

	_cursorItemPicture = 0;
}

InputController::~InputController() {
	removeMessageHandler(126, -1);

	g_fp->_inputController = 0;
}

void InputController::setInputDisabled(bool state) {
	_flag = state;
	g_fp->_inputDisabled = state;
}

void setInputDisabled(bool state) {
	g_fp->_inputController->setInputDisabled(state);
}

void InputController::addCursor(CursorInfo *cursor) {
	CursorInfo *newc = new CursorInfo(cursor);
	Common::Point p;
	
	cursor->picture->getDimensions(&p);

	newc->width = p.x;
	newc->height = p.y;

	newc->picture->_x = -1;
	newc->picture->_y = -1;

	_cursorsArray.push_back(newc);
}

void InputController::setCursorMode(bool enabled) {
	if (enabled)
		_inputFlags |= 1;
	else
		_inputFlags &= ~1;
}

void InputController::drawCursor(int x, int y) {
	if (_cursorIndex == -1)
		return;

	_cursorBounds.left = g_fp->_sceneRect.left + x - _cursorsArray[_cursorIndex]->hotspotX;
	_cursorBounds.top = g_fp->_sceneRect.top + y - _cursorsArray[_cursorIndex]->hotspotY;
	_cursorBounds.right = _cursorBounds.left + _cursorsArray[_cursorIndex]->width;
	_cursorBounds.bottom = _cursorBounds.top + _cursorsArray[_cursorIndex]->height;

	_cursorsArray[_cursorIndex]->picture->draw(_cursorBounds.left, _cursorBounds.top, 0, 0);

	if (_cursorItemPicture)
		_cursorItemPicture->draw(_cursorBounds.left + _cursorsArray[_cursorIndex]->itemPictureOffsX,
								 _cursorBounds.top + _cursorsArray[_cursorIndex]->itemPictureOffsY, 0, 0);
}

void InputController::setCursor(int cursorId) {
	if (_cursorIndex == -1 || _cursorsArray[_cursorIndex]->pictureId != cursorId) {
		_cursorIndex = -1;

		for (uint i = 0; i < _cursorsArray.size(); i++) {
			if (_cursorsArray[i]->pictureId == cursorId) {
				_cursorIndex = i;
				break;
			}
		}
	}
}

CursorInfo::CursorInfo() {
	pictureId = 0;
	picture = 0;
	hotspotX = 0;
	hotspotY = 0;
	itemPictureOffsX = 0;
	itemPictureOffsY = 0;
	width = 0;
	height = 0;
}

CursorInfo::CursorInfo(CursorInfo *src) {
	pictureId = src->pictureId;
	picture = src->picture;
	hotspotX = src->hotspotX;
	hotspotY = src->hotspotY;
	itemPictureOffsX = src->itemPictureOffsX;
	itemPictureOffsY = src->itemPictureOffsY;
	width = src->width;
	height = src->height;
}

void FullpipeEngine::setCursor(int id) {
	if (_inputController)
		_inputController->setCursor(id);
}

const char *input_cheats[] = {
	"HELP",
	"STUFF",
	"FASTER",
	"OHWAIT",
	"MUSOFF",
	""
};

void FullpipeEngine::defHandleKeyDown(int key) {
	if (_currentCheat == -1) {
		for (int i = 0; input_cheats[i][0]; i++)
			if (toupper(key) == input_cheats[i][0]) {
				_currentCheat = i;
				_currentCheatPos = 1;
			}

		return;
	}

	if (toupper(key) != input_cheats[_currentCheat][_currentCheatPos]) {
		_currentCheat = -1;

		return;
	}

	_currentCheatPos++;

	if (!input_cheats[_currentCheat][_currentCheatPos]) {
		switch (_currentCheat) {
		case 0:                               // HELP
			winArcade();
			break;
		case 1:                               // STUFF
			getAllInventory();
			break;
		case 2:                               // FASTER
			_normalSpeed = !_normalSpeed;
			break;
		case 3:                               // OHWAIT
			_gamePaused = 1;
			_flgGameIsRunning = 0;
			break;
		case 4:                               // MUSOFF
			if (_musicAllowed & 2)
				setMusicAllowed(_musicAllowed & 0xFFFFFFFD);
			else
				setMusicAllowed(_musicAllowed | 2);
			break;
		default:
			break;
		}

		_currentCheatPos = 0;
		_currentCheat = -1;
	}
}

void FullpipeEngine::winArcade() {
	ExCommand *ex = new ExCommand(0, 17, MSG_CMN_WINARCADE, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 3;

	ex->postMessage();

}

void FullpipeEngine::updateCursorCommon() {
	GameObject *ani = _currentScene->getStaticANIObjectAtPos(_mouseVirtX, _mouseVirtY);

	GameObject *pic = _currentScene->getPictureObjectAtPos(_mouseVirtX, _mouseVirtY);
	if (!ani || (pic && pic->_priority < ani->_priority))
		ani = pic;

	int selId = getGameLoaderInventory()->getSelectedItemId();

	_objectAtCursor = ani;

	if (ani) {
		_objectIdAtCursor = ani->_id;

		if (!selId && ani->_id >= _minCursorId && ani->_id <= _maxCursorId) {
			selId = _objectIdCursors[ani->_id - _minCursorId];
			if (selId) {
				_cursorId = selId;
				return;
			}
		}
		if (canInteractAny(_aniMan, ani, selId)) {
			_cursorId = selId > 0 ? PIC_CSR_ITN_INV : PIC_CSR_ITN;
			return;
		}
		if (selId) {
			_cursorId = PIC_CSR_DEFAULT_INV;
			return;
		}
		if (_objectIdAtCursor == ANI_LIFTBUTTON && lift_getButtonIdP(((StaticANIObject *)ani)->_statics->_staticsId)) {
			_cursorId = PIC_CSR_LIFT;
			return;
		}
		if (_sceneRect.right - _mouseVirtX < 47 && _sceneRect.right < _sceneWidth - 1) {
			_cursorId = PIC_CSR_GOFAR_R;
			return;
		}
		if (_mouseVirtX - _sceneRect.left < 47 && _sceneRect.left > 0) {
			_cursorId = PIC_CSR_GOFAR_L;
			return;
		}
		_cursorId = PIC_CSR_DEFAULT;
		return;
	} else {
		_objectIdAtCursor = 0;

		if (selId) {
			_cursorId = PIC_CSR_DEFAULT_INV;
			return;
		}
		if (_sceneRect.right - _mouseVirtX < 47 && _sceneRect.right < _sceneWidth - 1) {
			_cursorId = PIC_CSR_GOFAR_R;
			return;
		}
		if (_mouseVirtX - _sceneRect.left < 47 && _sceneRect.left > 0) {
			_cursorId = PIC_CSR_GOFAR_L;
			return;
		}
	}

	_cursorId = PIC_CSR_DEFAULT;
}

void FullpipeEngine::initArcadeKeys(const char *varname) {
	GameVar *var = getGameLoaderGameVar()->getSubVarByName(varname)->getSubVarByName("KEYPOS");

	if (!var)
		return;

	int cnt = var->getSubVarsCount();

	for (int i = 0; i < cnt; i++) {
		Common::Point *point = new Common::Point;

		GameVar *sub = var->getSubVarByIndex(i);

		point->x = sub->getSubVarAsInt("X");
		point->y = sub->getSubVarAsInt("Y");

		_arcadeKeys.push_back(point);
	}
}

void FullpipeEngine::setArcadeOverlay(int picId) {
	Common::Point point;
	Common::Point point2;

	_arcadeOverlayX = 800;
	_arcadeOverlayY = 545;

	_arcadeOverlayHelper = accessScene(SC_INV)->getPictureObjectById(PIC_CSR_HELPERBGR, 0);
	_arcadeOverlay = accessScene(SC_INV)->getPictureObjectById(picId, 0);

	_arcadeOverlay->getDimensions(&point);
	_arcadeOverlayHelper->getDimensions(&point2);

	_arcadeOverlayMidX = (point2.x - point.x) / 2;
	_arcadeOverlayMidY = abs(point2.y - point.y) / 2;
}

int FullpipeEngine::drawArcadeOverlay(int adjust) {
	_arcadeOverlayHelper->drawAt(_sceneRect.left + _arcadeOverlayX, _sceneRect.top + _arcadeOverlayY);
	_arcadeOverlay->drawAt(_sceneRect.left + _arcadeOverlayX + _arcadeOverlayMidX, _sceneRect.top + _arcadeOverlayY + _arcadeOverlayMidY);

	if (adjust) {
		if (_arcadeOverlayX > 745) {
			_arcadeOverlayX -= 15;

			if (_arcadeOverlayX < 745)
				_arcadeOverlayX = 745;
		}

		return 1;
	}

	if (_arcadeOverlayX >= 800) {
		return 0;
	} else {
		_arcadeOverlayX += 15;

		if (_arcadeOverlayX <= 800)
			return 1;

		_arcadeOverlayX = 800;
	}

	return 1;
}

} // End of namespace Fullpipe
