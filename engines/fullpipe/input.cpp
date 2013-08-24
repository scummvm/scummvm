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
#include "fullpipe/constants.h"

namespace Fullpipe {

CInputController::CInputController() {
	g_fullpipe->_inputController = this;

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

void CInputController::setInputDisabled(bool state) {
	_flag = state;
	g_fullpipe->_inputDisabled = state;
}

void setInputDisabled(bool state) {
	g_fullpipe->_inputController->setInputDisabled(state);
}

void CInputController::addCursor(CursorInfo *cursor) {
	CursorInfo *newc = new CursorInfo(cursor);
	Common::Point p;
	
	cursor->picture->getDimensions(&p);

	newc->width = p.x;
	newc->height = p.y;

	newc->picture->_x = -1;
	newc->picture->_y = -1;

	_cursorsArray.push_back(newc);
}

void CInputController::setCursorMode(bool enabled) {
	if (enabled)
		_inputFlags |= 1;
	else
		_inputFlags &= ~1;
}

void CInputController::drawCursor(int x, int y) {
	warning("STUB: CInputController::drawCursor(%d, %d)", x, y);
}

void CInputController::setCursor(int id) {
	warning("STUB: CInputController::setCursor(%d)", id);
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

void FullpipeEngine::defHandleKeyDown(int key) {
	warning("STUB: FullpipeEngine::defHandleKeyDown(%d)", key);
}

void FullpipeEngine::updateCursorsCommon() {
	GameObject *ani = _currentScene->getStaticANIObjectAtPos(_mouseVirtX, _mouseVirtY);

	GameObject *pic = _currentScene->getPictureObjectAtPos(_mouseVirtX, _mouseVirtY);
	if (!ani || pic && pic->_priority < ani->_priority )
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
		if (_aniMan->canInteractAny(ani, selId)) {
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
		if (_mouseVirtX - _sceneRect.left < 47 && _sceneRect.left > 0 ) {
			_cursorId = PIC_CSR_GOFAR_L;
			return;
		}
	}

	_cursorId = PIC_CSR_DEFAULT_INV;
}

} // End of namespace Fullpipe
