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

#include "zvision/core/console.h"
#include "zvision/cursors/cursor_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/animation/rlf_animation.h"
#include "zvision/core/menu.h"
#include "zvision/utility/win_keys.h"
#include "zvision/core/menu.h"
#include "zvision/sound/zork_raw.h"

#include "common/events.h"
#include "common/system.h"
#include "common/rational.h"

#include "engines/util.h"


namespace ZVision {

void ZVision::shortKeys(Common::Event event) {
	if (event.kbd.hasFlags(Common::KBD_CTRL)) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_s:
			if (getMenuBarEnable() & menuBar_Save)
				_scriptManager->changeLocation('g', 'j', 's', 'e', 0);
			break;
		case Common::KEYCODE_r:
			if (getMenuBarEnable() & menuBar_Restore)
				_scriptManager->changeLocation('g', 'j', 'r', 'e', 0);
			break;
		case Common::KEYCODE_p:
			if (getMenuBarEnable() & menuBar_Settings)
				_scriptManager->changeLocation('g', 'j', 'p', 'e', 0);
			break;
		case Common::KEYCODE_q:
			if (getMenuBarEnable() & menuBar_Exit)
				ifQuit();
			break;
		default:
			break;
		}
	}
}

void ZVision::cheatCodes(uint8 key) {
	pushKeyToCheatBuf(key);

	if (getGameId() == GID_GRANDINQUISITOR) {

		if (checkCode("IMNOTDEAF")) {
			// Unknow cheat
			showDebugMsg(Common::String::format("IMNOTDEAF cheat or debug, not implemented"));
		}

		if (checkCode("3100OPB")) {
			showDebugMsg(Common::String::format("Current location: %c%c%c%c",
			                                    _scriptManager->getStateValue(StateKey_World),
			                                    _scriptManager->getStateValue(StateKey_Room),
			                                    _scriptManager->getStateValue(StateKey_Node),
			                                    _scriptManager->getStateValue(StateKey_View)));
		}

		if (checkCode("KILLMENOW")) {
			_scriptManager->changeLocation('g', 'j', 'd', 'e', 0);
			_scriptManager->setStateValue(2201, 35);
		}

		if (checkCode("MIKESPANTS")) {
			_scriptManager->changeLocation('g', 'j', 't', 'm', 0);
		}
	} else if (getGameId() == GID_NEMESIS) {

		if (checkCode("CHLOE")) {
			_scriptManager->changeLocation('t', 'm', '2', 'g', 0);
			_scriptManager->setStateValue(224, 1);
		}

		if (checkCode("77MASSAVE")) {
			showDebugMsg(Common::String::format("Current location: %c%c%c%c",
			                                    _scriptManager->getStateValue(StateKey_World),
			                                    _scriptManager->getStateValue(StateKey_Room),
			                                    _scriptManager->getStateValue(StateKey_Node),
			                                    _scriptManager->getStateValue(StateKey_View)));
		}

		if (checkCode("IDKFA")) {
			_scriptManager->changeLocation('t', 'w', '3', 'f', 0);
			_scriptManager->setStateValue(249, 1);
		}

		if (checkCode("309NEWDORMA")) {
			_scriptManager->changeLocation('g', 'j', 'g', 'j', 0);
		}

		if (checkCode("HELLOSAILOR")) {
			Location loc = _scriptManager->getCurrentLocation();
			Audio::AudioStream *soundStream;
			if (loc.world == 'v' && loc.room == 'b' && loc.node == '1' && loc.view == '0') {
				soundStream = makeRawZorkStream("v000hpta.raw", this);
			} else {
				soundStream = makeRawZorkStream("v000hnta.raw", this);
			}
			Audio::SoundHandle handle;
			_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, soundStream);
		}
	}

	if (checkCode("FRAME"))
		showDebugMsg(Common::String::format("FPS: ???, not implemented"));

	if (checkCode("XYZZY"))
		_scriptManager->setStateValue(StateKey_DebugCheats, 1 - _scriptManager->getStateValue(StateKey_DebugCheats));

	if (checkCode("COMPUTERARCH"))
		showDebugMsg(Common::String::format("COMPUTERARCH: var-viewer not implemented"));

	if (_scriptManager->getStateValue(StateKey_DebugCheats) == 1)
		if (checkCode("GO????"))
			_scriptManager->changeLocation(getBufferedKey(3),
			                               getBufferedKey(2),
			                               getBufferedKey(1),
			                               getBufferedKey(0), 0);
}

void ZVision::processEvents() {
	while (_eventMan->pollEvent(_event)) {
		switch (_event.type) {
		case Common::EVENT_LBUTTONDOWN:
			_cursorManager->cursorDown(true);
			_scriptManager->setStateValue(StateKey_LMouse, 1);
			_menu->onMouseDown(_event.mouse);
			_scriptManager->addEvent(_event);
			break;

		case Common::EVENT_LBUTTONUP:
			_cursorManager->cursorDown(false);
			_scriptManager->setStateValue(StateKey_LMouse, 0);
			_menu->onMouseUp(_event.mouse);
			_scriptManager->addEvent(_event);
			break;

		case Common::EVENT_RBUTTONDOWN:
			_cursorManager->cursorDown(true);
			_scriptManager->setStateValue(StateKey_RMouse, 1);

			if (getGameId() == GID_NEMESIS)
				_scriptManager->invertory_cycle();
			break;

		case Common::EVENT_RBUTTONUP:
			_cursorManager->cursorDown(false);
			_scriptManager->setStateValue(StateKey_RMouse, 0);
			break;

		case Common::EVENT_MOUSEMOVE:
			onMouseMove(_event.mouse);
			break;

		case Common::EVENT_KEYDOWN: {
			switch (_event.kbd.keycode) {
			case Common::KEYCODE_d:
				if (_event.kbd.hasFlags(Common::KBD_CTRL)) {
					// Start the debugger
					_console->attach();
					_console->onFrame();
				}
				break;

			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_RIGHT:
				if (_renderManager->getRenderTable()->getRenderState() == RenderTable::PANORAMA)
					_kbdVelocity = (_event.kbd.keycode == Common::KEYCODE_LEFT ?
					                -_scriptManager->getStateValue(StateKey_KbdRotateSpeed) :
					                _scriptManager->getStateValue(StateKey_KbdRotateSpeed)) * 2;
			case Common::KEYCODE_q:
				if (_event.kbd.hasFlags(Common::KBD_CTRL))
					quitGame();
				break;

			case Common::KEYCODE_UP:
			case Common::KEYCODE_DOWN:
				if (_renderManager->getRenderTable()->getRenderState() == RenderTable::TILT)
					_kbdVelocity = (_event.kbd.keycode == Common::KEYCODE_UP ?
					                -_scriptManager->getStateValue(StateKey_KbdRotateSpeed) :
					                _scriptManager->getStateValue(StateKey_KbdRotateSpeed)) * 2;
				break;

			default:
				break;
			}

			uint8 vkKey = VKkey(_event.kbd.keycode);

			_scriptManager->setStateValue(StateKey_KeyPress, vkKey);

			_scriptManager->addEvent(_event);
			shortKeys(_event);
			cheatCodes(vkKey);
		}
		break;
		case Common::EVENT_KEYUP:
			_scriptManager->addEvent(_event);
			switch (_event.kbd.keycode) {
			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_RIGHT:
				if (_renderManager->getRenderTable()->getRenderState() == RenderTable::PANORAMA)
					_kbdVelocity = 0;
				break;
			case Common::KEYCODE_UP:
			case Common::KEYCODE_DOWN:
				if (_renderManager->getRenderTable()->getRenderState() == RenderTable::TILT)
					_kbdVelocity = 0;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
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
				_mouseVelocity  = (((pos.x - (ROTATION_SCREEN_EDGE_OFFSET + _workingWindow.left)) << 7) / ROTATION_SCREEN_EDGE_OFFSET * mspeed) >> 7;

				_cursorManager->changeCursor(CursorIndex_Left);
				cursorWasChanged = true;
			} else if (pos.x <= _workingWindow.right && pos.x > _workingWindow.right - ROTATION_SCREEN_EDGE_OFFSET) {

				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 400 >> 4;
				_mouseVelocity  = (((pos.x - (_workingWindow.right - ROTATION_SCREEN_EDGE_OFFSET)) << 7) / ROTATION_SCREEN_EDGE_OFFSET * mspeed) >> 7;

				_cursorManager->changeCursor(CursorIndex_Right);
				cursorWasChanged = true;
			} else {
				_mouseVelocity = 0;
			}
		} else if (renderState == RenderTable::TILT) {
			if (pos.y >= _workingWindow.top && pos.y < _workingWindow.top + ROTATION_SCREEN_EDGE_OFFSET) {

				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 400 >> 4;
				_mouseVelocity  = (((pos.y - (_workingWindow.top + ROTATION_SCREEN_EDGE_OFFSET)) << 7) / ROTATION_SCREEN_EDGE_OFFSET * mspeed) >> 7;

				_cursorManager->changeCursor(CursorIndex_UpArr);
				cursorWasChanged = true;
			} else if (pos.y <= _workingWindow.bottom && pos.y > _workingWindow.bottom - ROTATION_SCREEN_EDGE_OFFSET) {

				int16 mspeed = _scriptManager->getStateValue(StateKey_RotateSpeed) >> 4;
				if (mspeed <= 0)
					mspeed = 400 >> 4;
				_mouseVelocity  = (((pos.y - (_workingWindow.bottom - ROTATION_SCREEN_EDGE_OFFSET)) << 7) / ROTATION_SCREEN_EDGE_OFFSET * mspeed) >> 7;

				_cursorManager->changeCursor(CursorIndex_DownArr);
				cursorWasChanged = true;
			} else {
				_mouseVelocity = 0;
			}
		} else {
			_mouseVelocity = 0;
		}
	} else {
		_mouseVelocity = 0;
	}

	if (!cursorWasChanged) {
		_cursorManager->changeCursor(CursorIndex_Idle);
	}
}

} // End of namespace ZVision
