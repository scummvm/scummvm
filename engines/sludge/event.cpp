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

#include "common/events.h"
#include "common/keyboard.h"

#include "sludge/event.h"
#include "sludge/freeze.h"
#include "sludge/function.h"
#include "sludge/graphics.h"
#include "sludge/newfatal.h"
#include "sludge/region.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/variable.h"

namespace Sludge {

extern Variable *launchResult;
extern VariableStack *noStack;

EventManager::EventManager(SludgeEngine *vm) {
	_vm = vm;
	_currentEvents = new EventHandlers;
	init();
}

EventManager::~EventManager() {
	kill();
	if (_currentEvents) {
		delete _currentEvents;
		_currentEvents = nullptr;
	}
}

void EventManager::init() {
	_weAreDoneSoQuit = 0;
	_reallyWantToQuit = false;

	_input.leftClick = _input.rightClick = _input.justMoved = _input.leftRelease = _input.rightRelease = false;
	_input.keyPressed = 0;

	for (uint i = 0; i < EVENT_FUNC_NB; ++i) {
		_currentEvents->func[i] = 0;
	}
}

void EventManager::kill() {
}

void EventManager::checkInput() {
	float cameraZoom = _vm->_gfxMan->getCamZoom();
#if 0
	static bool fakeRightclick = false;
#endif
	Common::Event event;

	/* Check for events */
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
#if 0
			case SDL_VIDEORESIZE:
			realWinWidth = event.resize.w;
			realWinHeight = event.resize.h;
			setGraphicsWindow(false, true, true);
			break;
#endif
			case Common::EVENT_MOUSEMOVE:
				_input.justMoved = true;
				_input.mouseX = event.mouse.x * cameraZoom;
				_input.mouseY = event.mouse.y * cameraZoom;
				break;

			case Common::EVENT_LBUTTONDOWN:
				_input.leftClick = true;
				_input.mouseX = event.mouse.x * cameraZoom;
				_input.mouseY = event.mouse.y * cameraZoom;
#if 0
				if (SDL_GetModState() & KMOD_CTRL) {
					input.rightClick = true;
					fakeRightclick = true;
				} else {
					input.leftClick = true;
					fakeRightclick = false;
				}
#endif
				break;

			case Common::EVENT_RBUTTONDOWN:
				_input.rightClick = true;
				_input.mouseX = event.mouse.x * cameraZoom;
				_input.mouseY = event.mouse.y * cameraZoom;
				break;

			case Common::EVENT_LBUTTONUP:
				_input.leftRelease = true;
				_input.mouseX = event.mouse.x * cameraZoom;
				_input.mouseY = event.mouse.y * cameraZoom;
				break;

			case Common::EVENT_RBUTTONUP:
				_input.rightRelease = true;
				_input.mouseX = event.mouse.x * cameraZoom;
				_input.mouseY = event.mouse.y * cameraZoom;
				break;

			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {

					case Common::KEYCODE_BACKSPACE:
						// fall through
					case Common::KEYCODE_DELETE:
						_input.keyPressed = Common::KEYCODE_DELETE;
						break;
					default:
						_input.keyPressed = event.kbd.keycode;
						break;
				}
				break;

			case Common::EVENT_QUIT:
				_weAreDoneSoQuit = 1;
				// TODO: if _reallyWantToQuit, popup a message box to confirm
				break;

			default:
				break;
		}
	}
}

bool EventManager::handleInput() {
	static int l = 0;

	if (!_vm->launchMe.empty()) {
		if (l) {
			// Still paused because of spawned thingy...
		} else {
			l = 1;

			launchResult->setVariable(SVT_INT, 0/*launch(launchMe) > 31*/); //TODO:false value
			_vm->launchMe.clear();
			launchResult = nullptr;
		}
		return true;
	} else {
		l = 0;
	}

	if (!_vm->_regionMan->getOverRegion())
		_vm->_regionMan->updateOverRegion();

	if (_input.justMoved) {
		if (_currentEvents->func[kMoveMouse]) {
			if (!startNewFunctionNum(_currentEvents->func[kMoveMouse], 0, nullptr, noStack))
				return false;
		}
	}
	_input.justMoved = false;

	if (_vm-> _regionMan->isRegionChanged()&& _currentEvents->func[kFocus]) {
		VariableStack *tempStack = new VariableStack;
		if (!checkNew(tempStack))
			return false;

		ScreenRegion *overRegion = _vm->_regionMan->getOverRegion();
		if (overRegion) {
			tempStack->thisVar.setVariable(SVT_OBJTYPE, overRegion->thisType->objectNum);
		} else {
			tempStack->thisVar.setVariable(SVT_INT, 0);
		}
		tempStack->next = nullptr;
		if (!startNewFunctionNum(_currentEvents->func[kFocus], 1, nullptr, tempStack))
			return false;
	}
	if (_input.leftRelease && _currentEvents->func[kLeftMouseUp]) {
		if (!startNewFunctionNum(_currentEvents->func[kLeftMouseUp], 0, nullptr, noStack))
			return false;
	}
	if (_input.rightRelease && _currentEvents->func[kRightMouseUp]) {
		if (!startNewFunctionNum(_currentEvents->func[kRightMouseUp], 0, nullptr, noStack))
			return false;
	}
	if (_input.leftClick && _currentEvents->func[kLeftMouse])
		if (!startNewFunctionNum(_currentEvents->func[kLeftMouse], 0, nullptr, noStack))
			return false;
	if (_input.rightClick && _currentEvents->func[kRightMouse]) {
		if (!startNewFunctionNum(_currentEvents->func[kRightMouse], 0, nullptr, noStack))
			return false;
	}
	if (_input.keyPressed && _currentEvents->func[kSpace]) {
		Common::String tempString = "";
		switch (_input.keyPressed) {
			case 127:
				tempString = "BACKSPACE";
				break;
			case 9:
				tempString = "TAB";
				break;
			case 13:
				tempString = "ENTER";
				break;
			case 27:
				tempString = "ESCAPE";
				break;
				/*
				 case 1112:  tempString = copyString ("ALT+F1");     break;
				 case 1113:  tempString = copyString ("ALT+F2");     break;
				 case 1114:  tempString = copyString ("ALT+F3");     break;
				 case 1115:  tempString = copyString ("ALT+F4");     break;
				 case 1116:  tempString = copyString ("ALT+F5");     break;
				 case 1117:  tempString = copyString ("ALT+F6");     break;
				 case 1118:  tempString = copyString ("ALT+F7");     break;
				 case 1119:  tempString = copyString ("ALT+F8");     break;
				 case 1120:  tempString = copyString ("ALT+F9");     break;
				 case 1121:  tempString = copyString ("ALT+F10");    break;
				 case 1122:  tempString = copyString ("ALT+F11");    break;
				 case 1123:  tempString = copyString ("ALT+F12");    break;

				 case 2019:  tempString = copyString ("PAUSE");      break;
				 */
			case 63276:
				tempString = "PAGE UP";
				break;
			case 63277:
				tempString = "PAGE DOWN";
				break;
			case 63275:
				tempString = "END";
				break;
			case 63273:
				tempString = "HOME";
				break;
			case 63234:
				tempString = "LEFT";
				break;
			case 63232:
				tempString = "UP";
				break;
			case 63235:
				tempString = "RIGHT";
				break;
			case 63233:
				tempString = "DOWN";
				break;
				/*
				 case 2045:   tempString = copyString ("INSERT");     break;
				 case 2046:   tempString = copyString ("DELETE");     break;
				 */
			case 63236:
				tempString = "F1";
				break;
			case 63237:
				tempString = "F2";
				break;
			case 63238:
				tempString = "F3";
				break;
			case 63239:
				tempString = "F4";
				break;
			case 63240:
				tempString = "F5";
				break;
			case 63241:
				tempString = "F6";
				break;
			case 63242:
				tempString = "F7";
				break;
			case 63243:
				tempString = "F8";
				break;
			case 63244:
				tempString = "F9";
				break;
			case 63245:
				tempString = "F10";
				break;
			case 63246:
				tempString = "F11";
				break;
			case 63247:
				tempString = "F12";
				break;

			default:
				if (_input.keyPressed >= 256) {
					char tmp[7] = "ABCDEF";
					sprintf(tmp, "%i", _input.keyPressed);
					tempString = tmp;
					//}
				} else {
					char tmp[2] = " ";
					tmp[0] = _input.keyPressed;
					tempString = tmp;
				}
		}

		if (!tempString.empty()) {
			VariableStack *tempStack = new VariableStack;
			if (!checkNew(tempStack))
				return false;
			tempStack->thisVar.makeTextVar(tempString);
			tempStack->next = nullptr;
			if (!startNewFunctionNum(_currentEvents->func[kSpace], 1, nullptr, tempStack))
				return false;
		}
	}
	_input.rightClick = false;
	_input.leftClick = false;
	_input.rightRelease = false;
	_input.leftRelease = false;
	_input.keyPressed = 0;
	_vm->_regionMan->updateLastRegion();
	return true;
}

void EventManager::loadHandlers(Common::SeekableReadStream *stream) {
	for (uint i = 0; i < EVENT_FUNC_NB; ++i) {
		_currentEvents->func[i] = stream->readUint16BE();
	}
}

void EventManager::saveHandlers(Common::WriteStream *stream) {
	for (uint i = 0; i < EVENT_FUNC_NB; ++i) {
		stream->writeUint16BE(_currentEvents->func[i]);
	}
}

bool EventManager::freeze(FrozenStuffStruct *frozenStuff) {
	frozenStuff->currentEvents = _currentEvents;
	_currentEvents = new EventHandlers;
	if (!checkNew(_currentEvents))
		return false;
	for (uint i = 0; i < EVENT_FUNC_NB; ++i) {
		_currentEvents->func[i] = 0;
	}
	return true;
}

void EventManager::restore(FrozenStuffStruct *frozenStuff) {
	delete _currentEvents;
	_currentEvents = frozenStuff->currentEvents;
}

} /* namespace Sludge */
