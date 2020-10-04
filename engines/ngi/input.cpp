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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ngi/ngi.h"

#include "ngi/objects.h"
#include "ngi/input.h"
#include "ngi/gfx.h"
#include "ngi/scene.h"
#include "ngi/gameloader.h"
#include "ngi/statics.h"
#include "ngi/interaction.h"
#include "ngi/constants.h"

namespace NGI {

InputController::InputController() {
	g_nmi->_inputController = this;

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

	g_nmi->_inputController = 0;

	for (uint i = 0; i < _cursorsArray.size(); i++)
		delete _cursorsArray[i];
}

void InputController::setInputDisabled(bool state) {
	_flag = state;
	g_nmi->_inputDisabled = state;
}

void setInputDisabled(bool state) {
	g_nmi->_inputController->setInputDisabled(state);
}

void InputController::addCursor(CursorInfo *cursor) {
	CursorInfo *newc = new CursorInfo(*cursor);
	const Dims dims = cursor->picture->getDimensions();

	newc->width = dims.x;
	newc->height = dims.y;

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

	_cursorBounds.left = g_nmi->_sceneRect.left + x - _cursorsArray[_cursorIndex]->hotspotX;
	_cursorBounds.top = g_nmi->_sceneRect.top + y - _cursorsArray[_cursorIndex]->hotspotY;
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

void NGIEngine::setCursor(int id) {
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

void NGIEngine::defHandleKeyDown(int key) {
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
		case 0:								// HELP
			winArcade();
			break;
		case 1:								// STUFF
			getAllInventory();
			break;
		case 2:								// FASTER
			_normalSpeed = !_normalSpeed;
			break;
		case 3:								// OHWAIT
			_gamePaused = true;
			_flgGameIsRunning = false;
			break;
		case 4:								// MUSOFF
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

void NGIEngine::winArcade() {
	ExCommand *ex = new ExCommand(0, 17, MSG_CMN_WINARCADE, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 3;

	ex->postMessage();

}

void NGIEngine::updateCursorCommon() {
	GameObject *ani = _currentScene->getStaticANIObjectAtPos(_mouseVirtX, _mouseVirtY);

	PictureObject *pic = _currentScene->getPictureObjectAtPos(_mouseVirtX, _mouseVirtY);
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
		if (_objectIdAtCursor == ANI_LIFTBUTTON && ani->_objtype == kObjTypeStaticANIObject && lift_getButtonIdP(static_cast<StaticANIObject *>(ani)->_statics->_staticsId)) {
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

void NGIEngine::initArcadeKeys(const char *varname) {
	_arcadeKeys.clear();

	GameVar *var = getGameLoaderGameVar()->getSubVarByName(varname)->getSubVarByName("KEYPOS");

	if (!var)
		return;

	int cnt = var->getSubVarsCount();
	_arcadeKeys.resize(cnt);
	for (int i = 0; i < cnt; i++) {
		Common::Point &point = _arcadeKeys[i];
		GameVar *sub = var->getSubVarByIndex(i);
		point.x = sub->getSubVarAsInt("X");
		point.y = sub->getSubVarAsInt("Y");
	}
}

void NGIEngine::processArcade(ExCommand *cmd) {
	if (!g_nmi->_aniMan2)
		return;

	int idx;

	if (cmd->_sceneClickX <= g_nmi->_aniMan2->_ox) {
		for (idx = (int)_arcadeKeys.size() - 1; idx >= 0; idx--) {
			if (_arcadeKeys[idx].x < g_nmi->_aniMan2->_ox)
				break;
		}

		if (idx < 0)
			return;
	} else {
		for (idx = 0; idx < (int)_arcadeKeys.size(); idx++) {
			if (_arcadeKeys[idx].x > g_nmi->_aniMan2->_ox)
				break;
		}

		if (idx >= (int)_arcadeKeys.size())
			return;
	}

	cmd->_sceneClickX = _arcadeKeys[idx].x;
	cmd->_sceneClickY = _arcadeKeys[idx].y;

	cmd->_x = cmd->_sceneClickX - g_nmi->_sceneRect.left;
	cmd->_y = cmd->_sceneClickY - g_nmi->_sceneRect.top;
}

void NGIEngine::setArcadeOverlay(int picId) {
	_arcadeOverlayX = 800;
	_arcadeOverlayY = 545;

	_arcadeOverlayHelper = accessScene(SC_INV)->getPictureObjectById(PIC_CSR_HELPERBGR, 0);
	_arcadeOverlay = accessScene(SC_INV)->getPictureObjectById(picId, 0);

	const Dims dims = _arcadeOverlay->getDimensions();
	const Dims dims2 = _arcadeOverlayHelper->getDimensions();

	// TODO: Only Y gets abs?
	_arcadeOverlayMidX = (dims2.x - dims.x) / 2;
	_arcadeOverlayMidY = abs(dims2.y - dims.y) / 2;
}

int NGIEngine::drawArcadeOverlay(int adjust) {
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

} // End of namespace NGI
