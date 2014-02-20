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

#include "common/scummsys.h"

#include "zvision/zvision.h"

#include "zvision/core/console.h"
#include "zvision/cursors/cursor_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/animation/rlf_animation.h"

#include "common/events.h"
#include "common/system.h"
#include "common/rational.h"

#include "engines/util.h"


namespace ZVision {

void ZVision::processEvents() {
	while (_eventMan->pollEvent(_event)) {
		switch (_event.type) {
		case Common::EVENT_LBUTTONDOWN:
			onMouseDown(_event.mouse);
			break;

		case Common::EVENT_LBUTTONUP:
			onMouseUp(_event.mouse);
			break;

		case Common::EVENT_RBUTTONDOWN:
			// TODO: Inventory logic
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
	_cursorManager->cursorDown(true);

	Common::Point imageCoord(_renderManager->screenSpaceToImageSpace(pos));
	_scriptManager->onMouseDown(pos, imageCoord);
}

void ZVision::onMouseUp(const Common::Point &pos) {
	_cursorManager->cursorDown(false);

	Common::Point imageCoord(_renderManager->screenSpaceToImageSpace(pos));
	_scriptManager->onMouseUp(pos, imageCoord);
}

void ZVision::onMouseMove(const Common::Point &pos) {
	Common::Point imageCoord(_renderManager->screenSpaceToImageSpace(pos));

	bool cursorWasChanged = _scriptManager->onMouseMove(pos, imageCoord);

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
		RenderTable::RenderState renderState = _renderManager->getRenderTable()->getRenderState();
		if (renderState == RenderTable::PANORAMA) {
			if (pos.x >= _workingWindow.left && pos.x < _workingWindow.left + ROTATION_SCREEN_EDGE_OFFSET) {
				// Linear function of distance to the left edge (y = -mx + b)
				// We use fixed point math to get better accuracy
				Common::Rational velocity = (Common::Rational(MAX_ROTATION_SPEED, ROTATION_SCREEN_EDGE_OFFSET) * (pos.x - _workingWindow.left)) - MAX_ROTATION_SPEED;
				_renderManager->setBackgroundVelocity(velocity.toInt());
				_cursorManager->setLeftCursor();
				cursorWasChanged = true;
			} else if (pos.x <= _workingWindow.right && pos.x > _workingWindow.right - ROTATION_SCREEN_EDGE_OFFSET) {
				// Linear function of distance to the right edge (y = mx)
				// We use fixed point math to get better accuracy
				Common::Rational velocity = Common::Rational(MAX_ROTATION_SPEED, ROTATION_SCREEN_EDGE_OFFSET) * (pos.x - _workingWindow.right + ROTATION_SCREEN_EDGE_OFFSET);
				_renderManager->setBackgroundVelocity(velocity.toInt());
				_cursorManager->setRightCursor();
				cursorWasChanged = true;
			} else {
				_renderManager->setBackgroundVelocity(0);
			}
		} else if (renderState == RenderTable::TILT) {
			if (pos.y >= _workingWindow.top && pos.y < _workingWindow.top + ROTATION_SCREEN_EDGE_OFFSET) {
				// Linear function of distance to top edge
				// We use fixed point math to get better accuracy
				Common::Rational velocity = (Common::Rational(MAX_ROTATION_SPEED, ROTATION_SCREEN_EDGE_OFFSET) * (pos.y - _workingWindow.top)) - MAX_ROTATION_SPEED;
				_renderManager->setBackgroundVelocity(velocity.toInt());
				_cursorManager->setUpCursor();
				cursorWasChanged = true;
			} else if (pos.y <= _workingWindow.bottom && pos.y > _workingWindow.bottom - ROTATION_SCREEN_EDGE_OFFSET) {
				// Linear function of distance to the bottom edge (y = mx)
				// We use fixed point math to get better accuracy
				Common::Rational velocity = Common::Rational(MAX_ROTATION_SPEED, ROTATION_SCREEN_EDGE_OFFSET) * (pos.y - _workingWindow.bottom + ROTATION_SCREEN_EDGE_OFFSET);
				_renderManager->setBackgroundVelocity(velocity.toInt());
				_cursorManager->setDownCursor();
				cursorWasChanged = true;
			} else {
				_renderManager->setBackgroundVelocity(0);
			}
		}
	} else {
		_renderManager->setBackgroundVelocity(0);
	}

	if (!cursorWasChanged) {
		_cursorManager->revertToIdle();
	}
}

} // End of namespace ZVision
