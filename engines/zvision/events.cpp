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
#include "common/events.h"
#include "engines/util.h"
#include "common/system.h"
#include "common/rational.h"

#include "zvision/cursor_manager.h"
#include "zvision/render_manager.h"
#include "zvision/script_manager.h"
#include "zvision/mouse_event.h"
#include "zvision/rlf_animation.h"

namespace ZVision {

void ZVision::registerMouseEvent(const MouseEvent &event) {
	_mouseEvents.push_back(event);
}

bool ZVision::removeMouseEvent(const uint32 key) {
	for (Common::List<MouseEvent>::iterator iter = _mouseEvents.begin(); iter != _mouseEvents.end(); iter++) {
		if ((*iter)._key == key) {
			_scriptManager->setStateValue((*iter)._key, 0);
			_mouseEvents.erase(iter);
			return true;
		}
	}

	return false;
}

void ZVision::clearAllMouseEvents() {
	// Clear the state values of all the events
	for (Common::List<MouseEvent>::iterator iter = _mouseEvents.begin(); iter != _mouseEvents.end(); iter++) {
		_scriptManager->setStateValue((*iter)._key, 0);
	}

	_mouseEvents.clear();
}

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
			case Common::KEYCODE_F1:
				cycleThroughCursors();
				break;
			case Common::KEYCODE_F2:
				{
					RlfAnimation *animation = new RlfAnimation("te2ea21c.rlf");
					playAnimation(animation, 0, 0);
					break;
				}
			default:
				break;
			}

			onKeyDown(_event.kbd.keycode);
			break;

		default:
			break;
		}
	}
}

void ZVision::onMouseDown(const Common::Point &pos) {
	_cursorManager->cursorDown(true);
}

void ZVision::onMouseUp(const Common::Point &pos) {
	_cursorManager->cursorDown(false);

	Common::Point imageCoord(_renderManager->screenSpaceToImageSpace(pos));

	for (Common::List<MouseEvent>::iterator iter = _mouseEvents.begin(); iter != _mouseEvents.end(); iter++) {
		if (iter->withinHotspot(imageCoord)) {
			iter->onClick(this);
		}
	}
}

void ZVision::onMouseMove(const Common::Point &pos) {
	Common::Point imageCoord(_renderManager->screenSpaceToImageSpace(pos));

	bool isWithinAHotspot = false;
	if (_workingWindow.contains(pos)) {
		for (Common::List<MouseEvent>::iterator iter = _mouseEvents.begin(); iter != _mouseEvents.end(); iter++) {
			if (iter->withinHotspot(imageCoord)) {
				_cursorManager->changeCursor(iter->getHoverCursor());
				isWithinAHotspot = true;
			}
		}
	}

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
				isWithinAHotspot = true;
			} else if (pos.x <= _workingWindow.right && pos.x > _workingWindow.right - ROTATION_SCREEN_EDGE_OFFSET) {
				// Linear function of distance to the right edge (y = mx)
				// We use fixed point math to get better accuracy
				Common::Rational velocity = Common::Rational(MAX_ROTATION_SPEED, ROTATION_SCREEN_EDGE_OFFSET) * (pos.x - _workingWindow.right + ROTATION_SCREEN_EDGE_OFFSET);
				_renderManager->setBackgroundVelocity(velocity.toInt());
				_cursorManager->setRightCursor();
				isWithinAHotspot = true;
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
				isWithinAHotspot = true;
			} else if (pos.y <= _workingWindow.bottom && pos.y > _workingWindow.bottom - ROTATION_SCREEN_EDGE_OFFSET) {
				// Linear function of distance to the bottom edge (y = mx)
				// We use fixed point math to get better accuracy
				Common::Rational velocity = Common::Rational(MAX_ROTATION_SPEED, ROTATION_SCREEN_EDGE_OFFSET) * (pos.y - _workingWindow.bottom + ROTATION_SCREEN_EDGE_OFFSET);
				_renderManager->setBackgroundVelocity(velocity.toInt());
				_cursorManager->setDownCursor();
				isWithinAHotspot = true;
			} else {
				_renderManager->setBackgroundVelocity(0);
			}
		}
	} else {
		_renderManager->setBackgroundVelocity(0);
	}

	if (!isWithinAHotspot) {
		_cursorManager->revertToIdle();
	}
}

void ZVision::onKeyDown(uint keyCode) {

}

} // End of namespace ZVision
