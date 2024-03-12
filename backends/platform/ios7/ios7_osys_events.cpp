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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "gui/message.h"
#include "common/translation.h"
#include "common/config-manager.h"
#include "backends/graphics/ios/ios-graphics.h"
#include "backends/platform/ios7/ios7_osys_main.h"

static const int kQueuedInputEventDelay = 50;

bool OSystem_iOS7::pollEvent(Common::Event &event) {
	//printf("pollEvent()\n");

	long curTime = getMillis();

	if (_queuedInputEvent.type != Common::EVENT_INVALID && curTime >= _queuedEventTime) {
		event = _queuedInputEvent;
		_queuedInputEvent.type = Common::EVENT_INVALID;
		return true;
	}

	InternalEvent internalEvent;

	if (iOS7_fetchEvent(&internalEvent)) {
		switch (internalEvent.type) {
		case kInputTouchBegan:
			if (!handleEvent_touchBegan(event, internalEvent.value1, internalEvent.value2))
				return false;
			break;

		case kInputTouchMoved:
			if (!handleEvent_touchMoved(event, internalEvent.value1, internalEvent.value2))
				return false;
			break;

		case kInputMouseLeftButtonDown:
			handleEvent_mouseLeftButtonDown(event, internalEvent.value1, internalEvent.value2);
			break;

		case kInputMouseLeftButtonUp:
			handleEvent_mouseLeftButtonUp(event, internalEvent.value1, internalEvent.value2);
			break;

		case kInputMouseRightButtonDown:
			handleEvent_mouseRightButtonDown(event, internalEvent.value1, internalEvent.value2);
			break;

		case kInputMouseRightButtonUp:
			handleEvent_mouseRightButtonUp(event, internalEvent.value1, internalEvent.value2);
			break;

		case kInputMouseDelta:
			handleEvent_mouseDelta(event, internalEvent.value1, internalEvent.value2);
			break;

		case kInputOrientationChanged:
			handleEvent_orientationChanged(internalEvent.value1);
			return false;

		case kInputApplicationSuspended:
			handleEvent_applicationSuspended();
			return false;

		case kInputApplicationResumed:
			handleEvent_applicationResumed();
			return false;

		case kInputApplicationSaveState:
			handleEvent_applicationSaveState();
			return false;

		case kInputApplicationRestoreState:
			handleEvent_applicationRestoreState();
			return false;

		case kInputApplicationClearState:
			handleEvent_applicationClearState();
			return false;

		case kInputKeyPressed:
			handleEvent_keyPressed(event, internalEvent.value1, internalEvent.value2);
			break;

		case kInputSwipe:
			if (!handleEvent_swipe(event, internalEvent.value1, internalEvent.value2))
				return false;
			break;

		case kInputTap:
			if (!handleEvent_tap(event, (UIViewTapDescription) internalEvent.value1, internalEvent.value2))
				return false;
			break;

		case kInputLongPress:
			if (!handleEvent_longPress(event, (UIViewLongPressDescription) internalEvent.value1, internalEvent.value2))
				return false;
			break;

		case kInputMainMenu:
			event.type = Common::EVENT_MAINMENU;
			_queuedInputEvent.type = Common::EVENT_INVALID;
			_queuedEventTime = getMillis() + kQueuedInputEventDelay;
			break;

		case kInputJoystickAxisMotion:
			event.type = Common::EVENT_JOYAXIS_MOTION;
			event.joystick.axis = internalEvent.value1;
			event.joystick.position = internalEvent.value2;
			break;

		case kInputJoystickButtonDown:
			event.type = Common::EVENT_JOYBUTTON_DOWN;
			event.joystick.button = internalEvent.value1;
			break;

		case kInputJoystickButtonUp:
			event.type = Common::EVENT_JOYBUTTON_UP;
			event.joystick.button = internalEvent.value1;
			break;

		case kInputScreenChanged:
			rebuildSurface();
			dynamic_cast<iOSCommonGraphics *>(_graphicsManager)->notifyResize(getScreenWidth(), getScreenHeight());
			event.type = Common::EVENT_SCREEN_CHANGED;
			break;

		case kInputTouchModeChanged:
			handleEvent_touchModeChanged();
			break;

		default:
			break;
		}

		return true;
	}
	return false;
}

bool OSystem_iOS7::handleEvent_touchBegan(Common::Event &event, int x, int y) {
	_lastPadX = x;
	_lastPadY = y;

	if (_currentTouchMode == kTouchModeDirect) {
		Common::Point mouse(x, y);
		dynamic_cast<iOSCommonGraphics *>(_graphicsManager)->notifyMousePosition(mouse);
	}

	return false;
}

bool OSystem_iOS7::handleEvent_touchMoved(Common::Event &event, int x, int y) {
	int deltaX = _lastPadX - x;
	int deltaY = _lastPadY - y;
	_lastPadX = x;
	_lastPadY = y;

	if (_currentTouchMode == kTouchModeTouchpad) {
		handleEvent_mouseDelta(event, deltaX, deltaY);
	} else {
		// Update mouse position
		Common::Point mousePos(x, y);
		dynamic_cast<iOSCommonGraphics *>(_graphicsManager)->notifyMousePosition(mousePos);
		event.type = Common::EVENT_MOUSEMOVE;
		handleEvent_mouseEvent(event, deltaX, deltaY);
	}
	return true;
}

void OSystem_iOS7::handleEvent_mouseLeftButtonDown(Common::Event &event, int x, int y) {
	event.type = Common::EVENT_LBUTTONDOWN;
	handleEvent_mouseEvent(event, 0, 0);
}

void OSystem_iOS7::handleEvent_mouseLeftButtonUp(Common::Event &event, int x, int y) {
	event.type = Common::EVENT_LBUTTONUP;
	handleEvent_mouseEvent(event, 0, 0);
}

void OSystem_iOS7::handleEvent_mouseRightButtonDown(Common::Event &event, int x, int y) {
	event.type = Common::EVENT_RBUTTONDOWN;
	handleEvent_mouseEvent(event, 0, 0);
}

void OSystem_iOS7::handleEvent_mouseRightButtonUp(Common::Event &event, int x, int y) {
	event.type = Common::EVENT_RBUTTONUP;
	handleEvent_mouseEvent(event, 0, 0);
}

void OSystem_iOS7::handleEvent_mouseDelta(Common::Event &event, int deltaX, int deltaY) {
	Common::Point mouseOldPos = dynamic_cast<iOSCommonGraphics *>(_graphicsManager)->getMousePosition();

	Common::Point newMousePos((int)(mouseOldPos.x - (int)((float)deltaX * getMouseSpeed())), (int)(mouseOldPos.y - (int)((float)deltaY * getMouseSpeed())));

	// Update mouse position
	dynamic_cast<iOSCommonGraphics *>(_graphicsManager)->notifyMousePosition(newMousePos);

	event.type = Common::EVENT_MOUSEMOVE;
	handleEvent_mouseEvent(event, deltaX, deltaY);
}

void OSystem_iOS7::handleEvent_mouseEvent(Common::Event &event, int relX, int relY) {
	Common::Point mouse = dynamic_cast<iOSCommonGraphics *>(_graphicsManager)->getMousePosition();
	dynamic_cast<iOSCommonGraphics *>(_graphicsManager)->notifyMousePosition(mouse);

	event.relMouse.x = relX;
	event.relMouse.y = relY;
	event.mouse = mouse;
}


void  OSystem_iOS7::handleEvent_orientationChanged(int orientation) {
	//printf("Orientation: %i\n", orientation);

	ScreenOrientation newOrientation = (ScreenOrientation)orientation;

	if (_screenOrientation != newOrientation) {
		_screenOrientation = newOrientation;
		rebuildSurface();
	}
}

void OSystem_iOS7::handleEvent_touchModeChanged() {
	switch (_currentTouchMode) {
	case kTouchModeDirect:
		_currentTouchMode = kTouchModeTouchpad;
		break;
	case kTouchModeTouchpad:
	default:
		_currentTouchMode = kTouchModeDirect;
		break;
	}

	updateTouchMode();
}

void OSystem_iOS7::rebuildSurface() {
	updateOutputSurface();
}

void OSystem_iOS7::handleEvent_applicationSuspended() {
	suspendLoop();
}

void OSystem_iOS7::handleEvent_applicationResumed() {
	rebuildSurface();
}

void  OSystem_iOS7::handleEvent_keyPressed(Common::Event &event, int keyPressed, int modifierFlags) {
	int ascii = keyPressed;
	//printf("key: %i\n", keyPressed);

	// Map LF character to Return key/CR character
	if (keyPressed == 10) {
		keyPressed = Common::KEYCODE_RETURN;
		ascii = Common::ASCII_RETURN;
	}

	event.type = Common::EVENT_KEYDOWN;
	_queuedInputEvent.type = Common::EVENT_KEYUP;

	event.kbd.flags = _queuedInputEvent.kbd.flags = modifierFlags;
	event.kbd.keycode = _queuedInputEvent.kbd.keycode = (Common::KeyCode)keyPressed;
	event.kbd.ascii = _queuedInputEvent.kbd.ascii = ascii;
	_queuedEventTime = getMillis() + kQueuedInputEventDelay;
}

bool OSystem_iOS7::handleEvent_swipe(Common::Event &event, int direction, int touches) {
	if (touches == 3) {
		Common::KeyCode keycode = Common::KEYCODE_INVALID;
		switch ((UIViewSwipeDirection)direction) {
			case kUIViewSwipeUp:
				keycode = Common::KEYCODE_UP;
				break;
			case kUIViewSwipeDown:
				keycode = Common::KEYCODE_DOWN;
				break;
			case kUIViewSwipeLeft:
				keycode = Common::KEYCODE_LEFT;
				break;
			case kUIViewSwipeRight:
				keycode = Common::KEYCODE_RIGHT;
				break;
			default:
				return false;
		}

		event.kbd.keycode = _queuedInputEvent.kbd.keycode = keycode;
		event.kbd.ascii = _queuedInputEvent.kbd.ascii = 0;
		event.type = Common::EVENT_KEYDOWN;
		_queuedInputEvent.type = Common::EVENT_KEYUP;
		event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
		_queuedEventTime = getMillis() + kQueuedInputEventDelay;

		return true;
	}
	else if (touches == 2) {
		switch ((UIViewSwipeDirection)direction) {
		case kUIViewSwipeUp: {
			return false;
		}

		case kUIViewSwipeDown: {
			// Swipe down
			event.type = Common::EVENT_MAINMENU;
			_queuedInputEvent.type = Common::EVENT_INVALID;
			_queuedEventTime = getMillis() + kQueuedInputEventDelay;
			return true;
		}

		case kUIViewSwipeRight: {
			// Swipe right
			if (_currentTouchMode == kTouchModeDirect) {
				_currentTouchMode = kTouchModeTouchpad;
			} else {
				_currentTouchMode = kTouchModeDirect;
			}
			updateTouchMode();

			Common::U32String dialogMsg;
			if (_currentTouchMode == kTouchModeTouchpad)
				dialogMsg = _("Touchpad emulation");
			else
				dialogMsg = _("Direct mouse");
			GUI::TimedMessageDialog dialog(dialogMsg, 1500);
			dialog.runModal();
			return false;
		}

		case kUIViewSwipeLeft: {
			// Swipe left
			bool connect = !ConfMan.getBool("gamepad_controller");
			ConfMan.setBool("gamepad_controller", connect);
			ConfMan.flushToDisk();
			virtualController(connect);
			return false;
		}

		default:
			break;
		}
	}
	return false;
}

bool OSystem_iOS7::handleEvent_tap(Common::Event &event, UIViewTapDescription type, int touches) {
	if (touches == 1) {
		if (type == kUIViewTapSingle) {
			event.type = Common::EVENT_LBUTTONDOWN;
			handleEvent_mouseEvent(event, 0, 0);

			_queuedInputEvent.type = Common::EVENT_LBUTTONUP;
			_queuedEventTime = getMillis() + kQueuedInputEventDelay;
			handleEvent_mouseEvent(_queuedInputEvent, 0, 0);
			return true;
		}
	} else if (touches == 2) {
		if (type == kUIViewTapSingle) {
			event.type = Common::EVENT_RBUTTONDOWN;
			handleEvent_mouseEvent(event, 0, 0);

			_queuedInputEvent.type = Common::EVENT_RBUTTONUP;
			_queuedEventTime = getMillis() + kQueuedInputEventDelay;
			handleEvent_mouseEvent(_queuedInputEvent, 0, 0);
			return true;
		} else if (type == kUIViewTapDouble) {
			event.kbd.keycode = _queuedInputEvent.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = _queuedInputEvent.kbd.ascii = Common::ASCII_ESCAPE;
			event.type = Common::EVENT_KEYDOWN;
			_queuedInputEvent.type = Common::EVENT_KEYUP;
			event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
			_queuedEventTime = getMillis() + kQueuedInputEventDelay;
			return true;
		}
	}
	return false;
}

bool OSystem_iOS7::handleEvent_longPress(Common::Event &event, UIViewLongPressDescription type, int touches) {
	if (touches == 1) {
		if (type == UIViewLongPressStarted) {
			event.type = Common::EVENT_LBUTTONDOWN;
			handleEvent_mouseEvent(event, 0, 0);
		} else {
			event.type = Common::EVENT_LBUTTONUP;
			handleEvent_mouseEvent(event, 0, 0);
		}
		return true;
	} else if (touches == 2) {
		if (type == UIViewLongPressStarted) {
			event.type = Common::EVENT_RBUTTONDOWN;
			handleEvent_mouseEvent(event, 0, 0);
		} else {
			event.type = Common::EVENT_RBUTTONUP;
			handleEvent_mouseEvent(event, 0, 0);
		}
		return true;
	}
	return false;
}
