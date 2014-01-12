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

#include "common/scummsys.h"

#include "zvision/zvision.h"

#include "zvision/console.h"
#include "zvision/cursor_manager.h"
#include "zvision/render_manager.h"
#include "zvision/script_manager.h"
#include "zvision/rlf_animation.h"
#include "zvision/menu.h"

#include "common/events.h"
#include "common/system.h"
#include "common/rational.h"

#include "engines/util.h"


namespace ZVision {

void ZVision::processEvents() {
	while (_eventMan->pollEvent(_event)) {
		switch (_event.type) {
		case Common::EVENT_LBUTTONDOWN:
			_cursorManager->cursorDown(true);
			_scriptManager->setStateValue(StateKey_LMouse, 1);
			onMouseDown(_event.mouse);
			break;

		case Common::EVENT_LBUTTONUP:
			_cursorManager->cursorDown(false);
			_scriptManager->setStateValue(StateKey_LMouse, 0);
			onMouseUp(_event.mouse);
			break;

		case Common::EVENT_RBUTTONDOWN:
			_cursorManager->cursorDown(true);
			_scriptManager->setStateValue(StateKey_RMouse, 1);
			// TODO: Inventory logic
			break;

		case Common::EVENT_RBUTTONUP:
			_cursorManager->cursorDown(false);
			_scriptManager->setStateValue(StateKey_RMouse, 0);
			break;

		case Common::EVENT_MOUSEMOVE:
			onMouseMove(_event.mouse);
			break;

		case Common::EVENT_KEYDOWN:
			switch (_event.kbd.keycode) {
			case Common::KEYCODE_d:
				if (_event.kbd.hasFlags(Common::KBD_CTRL)) {
					// Start the debugger
					_console->attach();
					_console->onFrame();
				}
				break;
			case Common::KEYCODE_q:
				if (_event.kbd.hasFlags(Common::KBD_CTRL))
					quitGame();
				break;
			default:
				break;
			}

			_scriptManager->onKeyDown(_event.kbd);
			break;
		case Common::EVENT_KEYUP:
			_scriptManager->onKeyUp(_event.kbd);
			break;
		default:
			break;
		}
	}
}

void ZVision::onMouseDown(const Common::Point &pos) {
	_menu->onMouseDown(pos);

	Common::Point imageCoord(_renderManager->screenSpaceToImageSpace(pos));
	_scriptManager->onMouseDown(pos, imageCoord);
}

void ZVision::onMouseUp(const Common::Point &pos) {
	_menu->onMouseUp(pos);

	Common::Point imageCoord(_renderManager->screenSpaceToImageSpace(pos));
	_scriptManager->onMouseUp(pos, imageCoord);
}

void ZVision::onMouseMove(const Common::Point &pos) {
	_menu->onMouseMove(pos);
	Common::Point imageCoord(_renderManager->screenSpaceToImageSpace(pos));

	bool cursorWasChanged = false;

	// Graph of the function governing rotation velocity:
	//
	//                                    |---------------- working window ------------------|
	//               ^                    |---------|
	//               |                          |
	// +Max velocity |                        rotation screen edge offset
	//               |                                                                      /|
	//               |                                                                     / |
	//               |                                                                    /  |
	//               |                                                                   /   |
	//               |                                                                  /    |
	//               |                                                                 /     |
	//               |                                                                /      |
	//               |                                                               /       |
	//               |                                                              /        |
	// Zero velocity |______________________________ ______________________________/_________|__________________________>
	//               | Position ->        |         /
	//               |                    |        /
	//               |                    |       /
	//               |                    |      /
	//               |                    |     /
	//               |                    |    /
	//               |                    |   /
	//               |                    |  /
	//               |                    | /
	// -Max velocity |                    |/
	//               |
	//               |
	//               ^

	if (_workingWindow.contains(pos)) {
		cursorWasChanged = _scriptManager->onMouseMove(pos, imageCoord);

		RenderTable::RenderState renderState = _renderManager->getRenderTable()->getRenderState();
		if (renderState == RenderTable::PANORAMA) {
			if (pos.x >= _workingWindow.left && pos.x < _workingWindow.left + ROTATION_SCREEN_EDGE_OFFSET) {

				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 400 >> 4;
				_velocity  = (((pos.x - (ROTATION_SCREEN_EDGE_OFFSET + _workingWindow.left)) << 7) / ROTATION_SCREEN_EDGE_OFFSET * mspeed) >> 7;

				_cursorManager->changeCursor(CursorIndex_Left);
				cursorWasChanged = true;
			} else if (pos.x <= _workingWindow.right && pos.x > _workingWindow.right - ROTATION_SCREEN_EDGE_OFFSET) {

				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 400 >> 4;
				_velocity  = (((pos.x - (_workingWindow.right - ROTATION_SCREEN_EDGE_OFFSET)) << 7) / ROTATION_SCREEN_EDGE_OFFSET * mspeed) >> 7;

				_cursorManager->changeCursor(CursorIndex_Right);
				cursorWasChanged = true;
			} else {
				_velocity = 0;
			}
		} else if (renderState == RenderTable::TILT) {
			if (pos.y >= _workingWindow.top && pos.y < _workingWindow.top + ROTATION_SCREEN_EDGE_OFFSET) {

				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 400 >> 4;
				_velocity  = (((pos.y - (_workingWindow.top + ROTATION_SCREEN_EDGE_OFFSET)) << 7) / ROTATION_SCREEN_EDGE_OFFSET * mspeed) >> 7;

				_cursorManager->changeCursor(CursorIndex_UpArr);
				cursorWasChanged = true;
			} else if (pos.y <= _workingWindow.bottom && pos.y > _workingWindow.bottom - ROTATION_SCREEN_EDGE_OFFSET) {

				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 400 >> 4;
				_velocity  = (((pos.y - (_workingWindow.bottom - ROTATION_SCREEN_EDGE_OFFSET)) << 7) / ROTATION_SCREEN_EDGE_OFFSET * mspeed) >> 7;

				_cursorManager->changeCursor(CursorIndex_DownArr);
				cursorWasChanged = true;
			} else {
				_velocity = 0;
			}
		} else {
			_velocity = 0;
		}
	} else {
		_velocity = 0;
	}

	if (!cursorWasChanged) {
		_cursorManager->changeCursor(CursorIndex_Idle);
	}
}

} // End of namespace ZVision
