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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <FAppApplication.h>

#include "common/translation.h"
#include "base/main.h"

#include "backends/platform/bada/form.h"
#include "backends/platform/bada/system.h"

using namespace Osp::App;
using namespace Osp::Base;
using namespace Osp::Base::Runtime;
using namespace Osp::Graphics;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;

#define SHORTCUT_TIMEOUT 3000
#define SHORTCUT_SWAP_MOUSE 0
#define SHORTCUT_ESCAPE 1
#define SHORTCUT_F5 2
#define LEVEL_RANGE 5

//
// BadaAppForm
//
BadaAppForm::BadaAppForm() : 
	_gameThread(0), 
	_state(InitState),
	_buttonState(LeftButton),
	_shortcutTimer(0),
	_shortcutIndex(-1),
	_touchCount(0) {
	_eventQueueLock = new Mutex();
	_eventQueueLock->Create();
}

result BadaAppForm::Construct() {
	result r = Form::Construct(Controls::FORM_STYLE_NORMAL);
	if (IsFailed(r)) {
		return r;
	}

	BadaSystem *badaSystem = null;
	_gameThread = null;

	badaSystem = new BadaSystem(this);
	r = badaSystem != null ? E_SUCCESS : E_OUT_OF_MEMORY;
	
	if (!IsFailed(r)) {
		r = badaSystem->Construct();
	}

	if (!IsFailed(r)) {
		_gameThread = new Thread();
		r = _gameThread != null ? E_SUCCESS : E_OUT_OF_MEMORY;
	}

	if (!IsFailed(r)) {
		r = _gameThread->Construct(*this);
	}

	if (IsFailed(r)) {
		if (badaSystem != null) {
			delete badaSystem;
		}
		if (_gameThread != null) {
			delete _gameThread;
			_gameThread = null;
		}
	}	else {
		g_system = badaSystem;
	}

	return r;
}

BadaAppForm::~BadaAppForm() {
	logEntered();

	if (_gameThread) {
		terminate();
		_gameThread->Stop();
		_gameThread->Join();
		delete _gameThread;
		_gameThread = null;
	}

	if (_eventQueueLock) {
		delete _eventQueueLock;
		_eventQueueLock = null;
	}

	logLeaving();
}

//
// abort the game thread
//
void BadaAppForm::terminate() {
	if (_state == ActiveState) {
		((BadaSystem*) g_system)->setMute(true);

		_eventQueueLock->Acquire();

		Common::Event e;
		e.type = Common::EVENT_QUIT;
		_eventQueue.push(e);
		_state = ClosingState;

		_eventQueueLock->Release();

		// block while thread ends
		AppLog("waiting for shutdown");
		for (int i = 0; i < 10 && _state == ClosingState; i++) {
			Thread::Sleep(250);
		}
	}
}

void BadaAppForm::exitSystem() {
	_state = ErrorState;

	if (_gameThread) {
		_gameThread->Stop();
		delete _gameThread;
		_gameThread = null;
	}
}

result BadaAppForm::OnInitializing(void) {
	logEntered();

	SetOrientation(ORIENTATION_LANDSCAPE);
	AddOrientationEventListener(*this);
	AddTouchEventListener(*this);
	AddKeyEventListener(*this);

	Touch touch;
	touch.SetMultipointEnabled(*this, true);

	// set focus to enable receiving key events
	SetFocusable(true);
	SetFocus();

	return E_SUCCESS;
}

result BadaAppForm::OnDraw(void) {
	logEntered();

	if (g_system) {
		BadaSystem *system = (BadaSystem*)g_system;
		BadaGraphicsManager *graphics = system->getGraphics();
		if (graphics && graphics->isReady()) {
			g_system->updateScreen();
		}
	}

	return E_SUCCESS;
}

bool BadaAppForm::pollEvent(Common::Event &event) {
	bool result = false;

	_eventQueueLock->Acquire();
	if (!_eventQueue.empty()) {
		event = _eventQueue.pop();
		result = true;
	}
	_eventQueueLock->Release();

	return result;
}

void BadaAppForm::pushEvent(Common::EventType type,
														const Point &currentPosition) {
	BadaSystem *system = (BadaSystem*)g_system;
	BadaGraphicsManager *graphics = system->getGraphics();
	if (graphics) {
		Common::Event e;
		e.type = type;
		e.mouse.x = currentPosition.x;
		e.mouse.y = currentPosition.y;
		
		bool moved = graphics->moveMouse(e.mouse.x, e.mouse.y);

		_eventQueueLock->Acquire();

		if (moved && type != Common::EVENT_MOUSEMOVE) {
			Common::Event moveEvent;
			moveEvent.type = Common::EVENT_MOUSEMOVE;
			moveEvent.mouse = e.mouse;
			_eventQueue.push(moveEvent);
		}

		_eventQueue.push(e);
		_eventQueueLock->Release();
	}
}

void BadaAppForm::pushKey(Common::KeyCode keycode) {
	Common::Event e;
	e.synthetic = false;
	e.kbd.keycode = keycode;
	e.kbd.ascii = keycode;
	e.kbd.flags = 0;

	_eventQueueLock->Acquire();

	e.type = Common::EVENT_KEYDOWN;
	_eventQueue.push(e);
	e.type = Common::EVENT_KEYUP;
	_eventQueue.push(e);

	_eventQueueLock->Release();
}

void BadaAppForm::OnOrientationChanged(const Control &source, 
																			 OrientationStatus orientationStatus) {
	logEntered();
	if (_state == InitState) {
		_state = ActiveState;
		_gameThread->Start();
	}
}

Object *BadaAppForm::Run(void) {
	scummvm_main(0, 0);

	AppLog("scummvm_main completed");

	if (_state == ActiveState) {
		Application::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, null);
	}
	_state = DoneState;
	return null;
}

int BadaAppForm::getShortcutIndex() {
	if (_shortcutTimer) {
		uint32 nextTimer = g_system->getMillis();
		if (_shortcutTimer + SHORTCUT_TIMEOUT < nextTimer) {
			// double tap has expired
			_shortcutTimer = 0;
			_shortcutIndex = -1;
		}
	}
	return _shortcutIndex;
}

void BadaAppForm::setVolume(bool up, bool minMax) {
	int level = ((BadaSystem*)g_system)->setVolume(up, minMax);
	if (level != -1) {
		char message[32];
		char ind[LEVEL_RANGE]; // 1..5 (0=off)
		int j = LEVEL_RANGE - 1; // 0..4
		for (int i = 1; i <= LEVEL_RANGE; i++) {
			ind[j--] = level >= i ? '|' : ' ';
		}
		snprintf(message, sizeof(message), "Volume: [ %c%c%c%c%c ]",
						 ind[0], ind[1], ind[2], ind[3], ind[4]);
		g_system->displayMessageOnOSD(message);
	}
}

void BadaAppForm::OnTouchDoublePressed(const Control &source, 
																			 const Point &currentPosition, 
																			 const TouchEventInfo &touchInfo) {
	if (getShortcutIndex() == -1) {
		pushEvent(_buttonState == LeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
							currentPosition);
		pushEvent(_buttonState == LeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
							currentPosition);
	}
}

void BadaAppForm::OnTouchFocusIn(const Control &source, 
																 const Point &currentPosition, 
																 const TouchEventInfo &touchInfo) {
}

void BadaAppForm::OnTouchFocusOut(const Control &source, 
																	const Point &currentPosition, 
																	const TouchEventInfo &touchInfo) {
}

void BadaAppForm::OnTouchLongPressed(const Control &source, 
																		 const Point &currentPosition, 
																		 const TouchEventInfo &touchInfo) {
	if (getShortcutIndex() == -1 && _buttonState != LeftButton) {
		pushKey(Common::KEYCODE_RETURN);
	}
}

void BadaAppForm::OnTouchMoved(const Control &source, 
															 const Point &currentPosition, 
															 const TouchEventInfo &touchInfo) {
	if (getShortcutIndex() == -1) {
		pushEvent(Common::EVENT_MOUSEMOVE, currentPosition);
	}
}

void BadaAppForm::OnTouchPressed(const Control &source, 
																 const Point &currentPosition, 
																 const TouchEventInfo &touchInfo) {
	Touch touch;
	_touchCount = touch.GetPointCount();
	if (_touchCount > 1) {
		int index = getShortcutIndex();
		_shortcutIndex = (index == -1 ? 0 : index + 1);
		_shortcutTimer = g_system->getMillis();
		
		switch (_shortcutIndex) {
		case SHORTCUT_F5:
			g_system->displayMessageOnOSD(_("Game Menu"));
			break;
			
		case SHORTCUT_ESCAPE:
			g_system->displayMessageOnOSD(_("Escape"));
			break;
			
		default:
			g_system->displayMessageOnOSD(_("Swap Buttons"));
			_shortcutIndex = SHORTCUT_SWAP_MOUSE;
		}
	}	else if (getShortcutIndex() == -1) {
		pushEvent(_buttonState == LeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
							currentPosition);
	}
}

void BadaAppForm::OnTouchReleased(const Control &source, 
																	const Point &currentPosition, 
																	const TouchEventInfo &touchInfo) {
	if (getShortcutIndex() == -1) {
		pushEvent(_buttonState == LeftButton ? Common::EVENT_LBUTTONUP : Common::EVENT_RBUTTONUP,
							currentPosition);
		if (_buttonState == RightButtonOnce) {
			_buttonState = LeftButton;
		}
		// flick to skip dialog
		if (touchInfo.IsFlicked()) {
			pushKey(Common::KEYCODE_PERIOD);
		}
	}	else if (_touchCount == 1) {
		bool repeat = false;
		switch (_shortcutIndex) {
		case SHORTCUT_SWAP_MOUSE:
			switch (_buttonState) {
			case LeftButton:
				_buttonState = RightButtonOnce;
				g_system->displayMessageOnOSD(_("Right Once"));
				break;
			case RightButtonOnce:
				g_system->displayMessageOnOSD(_("Right Active"));
				_buttonState = RightButton;
				break;
			case RightButton:
				g_system->displayMessageOnOSD(_("Left Active"));
				_buttonState = LeftButton;
				break;
			}
			break;

		case SHORTCUT_F5:
			pushKey(Common::KEYCODE_F5);
			break;

		case SHORTCUT_ESCAPE:
			pushKey(Common::KEYCODE_ESCAPE);
			repeat = true;
			break;
		}

		// allow key repeat or terminate setup mode
		_shortcutTimer = repeat ? g_system->getMillis() : -1;
	}
}

void BadaAppForm::OnKeyLongPressed(const Control &source, KeyCode keyCode) {
	logEntered();
	switch (keyCode) {
	case KEY_SIDE_UP:
		setVolume(true, true);
		return;

	case KEY_SIDE_DOWN:
		setVolume(false, true);
		return;

	default:
		break;
	}
}

void BadaAppForm::OnKeyPressed(const Control &source, KeyCode keyCode) {
	switch (keyCode) {
	case KEY_SIDE_UP:
		setVolume(true, false);
		return;

	case KEY_SIDE_DOWN:
		setVolume(false, false);
		return;

	case KEY_CAMERA:
		// display the soft keyboard
		pushKey(Common::KEYCODE_F7);
		return;

	default:
		break;
	}
}

void BadaAppForm::OnKeyReleased(const Control &source, KeyCode keyCode) {
}

//
// end of form.cpp 
//
