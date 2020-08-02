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

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "backends/platform/3ds/osystem.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper-defaults.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"
#include "backends/platform/3ds/config.h"
#include "backends/platform/3ds/options-dialog.h"
#include "backends/timer/default/default-timer.h"
#include "common/translation.h"
#include "engines/engine.h"
#include "gui/gui-manager.h"

namespace _3DS {

static Common::Mutex *eventMutex;
static InputMode inputMode = MODE_DRAG;
static InputMode savedInputMode = MODE_DRAG;
static aptHookCookie cookie;

static const Common::HardwareInputTableEntry ctrJoystickButtons[] = {
    { "JOY_A",              Common::JOYSTICK_BUTTON_A,              _s("A")           },
    { "JOY_B",              Common::JOYSTICK_BUTTON_B,              _s("B")           },
    { "JOY_X",              Common::JOYSTICK_BUTTON_X,              _s("X")           },
    { "JOY_Y",              Common::JOYSTICK_BUTTON_Y,              _s("Y")           },
    { "JOY_BACK",           Common::JOYSTICK_BUTTON_BACK,           _s("Select")      },
    { "JOY_START",          Common::JOYSTICK_BUTTON_START,          _s("Start")       },
    { "JOY_LEFT_STICK",     Common::JOYSTICK_BUTTON_LEFT_STICK,     _s("ZL")          },
    { "JOY_RIGHT_STICK",    Common::JOYSTICK_BUTTON_RIGHT_STICK,    _s("ZR")          },
    { "JOY_LEFT_SHOULDER",  Common::JOYSTICK_BUTTON_LEFT_SHOULDER,  _s("L")           },
    { "JOY_RIGHT_SHOULDER", Common::JOYSTICK_BUTTON_RIGHT_SHOULDER, _s("R")           },
    { "JOY_UP",             Common::JOYSTICK_BUTTON_DPAD_UP,        _s("D-pad Up")    },
    { "JOY_DOWN",           Common::JOYSTICK_BUTTON_DPAD_DOWN,      _s("D-pad Down")  },
    { "JOY_LEFT",           Common::JOYSTICK_BUTTON_DPAD_LEFT,      _s("D-pad Left")  },
    { "JOY_RIGHT",          Common::JOYSTICK_BUTTON_DPAD_RIGHT,     _s("D-pad Right") },
    { nullptr,              0,                                      nullptr           }
};

static const Common::AxisTableEntry ctrJoystickAxes[] = {
    { "JOY_LEFT_STICK_X", Common::JOYSTICK_AXIS_LEFT_STICK_X, Common::kAxisTypeFull, _s("C-Pad X") },
    { "JOY_LEFT_STICK_Y", Common::JOYSTICK_AXIS_LEFT_STICK_Y, Common::kAxisTypeFull, _s("C-Pad Y") },
    { nullptr,            0,                                  Common::kAxisTypeFull, nullptr       }
};

const Common::HardwareInputTableEntry ctrMouseButtons[] = {
    { "MOUSE_LEFT",   Common::MOUSE_BUTTON_LEFT,   _s("Touch") },
    { nullptr,        0,                           nullptr     }
};

static const int16 CIRCLE_MAX = 160;

static void pushEventQueue(Common::Queue<Common::Event> *queue, Common::Event &event) {
	Common::StackLock lock(*eventMutex);
	queue->push(event);
}

static void doJoyEvent(Common::Queue<Common::Event> *queue, u32 keysPressed, u32 keysReleased, u32 ctrKey, uint8 svmButton) {
	if (keysPressed & ctrKey || keysReleased & ctrKey) {
		Common::Event event;
		event.type = (keysPressed & ctrKey) ? Common::EVENT_JOYBUTTON_DOWN : Common::EVENT_JOYBUTTON_UP;
		event.joystick.button = svmButton;

		pushEventQueue(queue, event);
	}
}

static void eventThreadFunc(void *arg) {
	OSystem_3DS *osys = dynamic_cast<OSystem_3DS *>(g_system);
	Common::Queue<Common::Event> *eventQueue = (Common::Queue<Common::Event> *)arg;

	uint32 touchStartTime = osys->getMillis();
	touchPosition  lastTouch  = {0, 0};
	circlePosition lastCircle = {0, 0};
	int borderSnapZone = 6;
	Common::Event event;

	while (!osys->exiting) {
		do {
			osys->delayMillis(10);
		} while (osys->sleeping && !osys->exiting);

		hidScanInput();
		u32 held = hidKeysHeld();
		u32 keysPressed = hidKeysDown();
		u32 keysReleased = hidKeysUp();

		// Touch screen events
		if (held & KEY_TOUCH) {
			touchPosition touch;
			hidTouchRead(&touch);
			if (config.snapToBorder) {
				if (touch.px < borderSnapZone) {
					touch.px = 0;
				}
				if (touch.px > 319 - borderSnapZone) {
					touch.px = 319;
				}
				if (touch.py < borderSnapZone) {
					touch.py = 0;
				}
				if (touch.py > 239 - borderSnapZone) {
					touch.py = 239;
				}
			}

			osys->transformPoint(touch);

			event.mouse.x = touch.px;
			event.mouse.y = touch.py;

			if (keysPressed & KEY_TOUCH) {
				touchStartTime = osys->getMillis();
				if (inputMode == MODE_DRAG) {
					event.type = Common::EVENT_LBUTTONDOWN;
					pushEventQueue(eventQueue, event);
				}
			} else if (touch.px != lastTouch.px || touch.py != lastTouch.py) {
				event.type = Common::EVENT_MOUSEMOVE;
				pushEventQueue(eventQueue, event);
			}

			lastTouch = touch;
		} else if (keysReleased & KEY_TOUCH) {
			event.mouse.x = lastTouch.px;
			event.mouse.y = lastTouch.py;
			if (inputMode == MODE_DRAG) {
				event.type = Common::EVENT_LBUTTONUP;
				pushEventQueue(eventQueue, event);
			} else if (osys->getMillis() - touchStartTime < 200) {
				// Process click in MODE_HOVER
				event.type = Common::EVENT_MOUSEMOVE;
				pushEventQueue(eventQueue, event);
				event.type = Common::EVENT_LBUTTONDOWN;
				pushEventQueue(eventQueue, event);
				event.type = Common::EVENT_LBUTTONUP;
				pushEventQueue(eventQueue, event);
			}
		}

		// C-Pad events
		circlePosition circle;
		hidCircleRead(&circle);

		if (circle.dx != lastCircle.dx) {
			int32 position = (int32)circle.dx * Common::JOYAXIS_MAX / CIRCLE_MAX;

			event.type              = Common::EVENT_JOYAXIS_MOTION;
			event.joystick.axis     = Common::JOYSTICK_AXIS_LEFT_STICK_X;
			event.joystick.position = CLIP<int32>(position, Common::JOYAXIS_MIN, Common::JOYAXIS_MAX);
			pushEventQueue(eventQueue, event);
		}

		if (circle.dy != lastCircle.dy) {
			int32 position = -(int32)circle.dy * Common::JOYAXIS_MAX / CIRCLE_MAX;

			event.type              = Common::EVENT_JOYAXIS_MOTION;
			event.joystick.axis     = Common::JOYSTICK_AXIS_LEFT_STICK_Y;
			event.joystick.position = CLIP<int32>(position, Common::JOYAXIS_MIN, Common::JOYAXIS_MAX);
			pushEventQueue(eventQueue, event);
		}

		lastCircle = circle;

		// Button events
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_L,      Common::JOYSTICK_BUTTON_LEFT_SHOULDER);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_R,      Common::JOYSTICK_BUTTON_RIGHT_SHOULDER);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_A,      Common::JOYSTICK_BUTTON_A);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_B,      Common::JOYSTICK_BUTTON_B);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_X,      Common::JOYSTICK_BUTTON_X);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_Y,      Common::JOYSTICK_BUTTON_Y);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_DUP,    Common::JOYSTICK_BUTTON_DPAD_UP);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_DDOWN,  Common::JOYSTICK_BUTTON_DPAD_DOWN);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_DLEFT,  Common::JOYSTICK_BUTTON_DPAD_LEFT);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_DRIGHT, Common::JOYSTICK_BUTTON_DPAD_RIGHT);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_START,  Common::JOYSTICK_BUTTON_START);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_SELECT, Common::JOYSTICK_BUTTON_BACK);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_ZL,     Common::JOYSTICK_BUTTON_LEFT_STICK);
		doJoyEvent(eventQueue, keysPressed, keysReleased, KEY_ZR,     Common::JOYSTICK_BUTTON_RIGHT_STICK);
	}
}

static void aptHookFunc(APT_HookType hookType, void *param) {
	OSystem_3DS *osys = dynamic_cast<OSystem_3DS *>(g_system);

	switch (hookType) {
		case APTHOOK_ONSUSPEND:
		case APTHOOK_ONSLEEP:
			if (g_engine) {
				osys->_sleepPauseToken = g_engine->pauseEngine();
			}
			osys->sleeping = true;
			if (R_SUCCEEDED(gspLcdInit())) {
				GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);
				gspLcdExit();
			}
			break;
		case APTHOOK_ONRESTORE:
		case APTHOOK_ONWAKEUP:
			if (g_engine) {
				osys->_sleepPauseToken.clear();
			}
			osys->sleeping = false;
			loadConfig();
			break;
		case APTHOOK_ONEXIT:
			break;
		default:
			warning("Unhandled APT hook, type: %d", hookType);
	}
}

static void timerThreadFunc(void *arg) {
	OSystem_3DS *osys = (OSystem_3DS *)arg;
	DefaultTimerManager *tm = (DefaultTimerManager *)osys->getTimerManager();
	while (!osys->exiting) {
		g_system->delayMillis(10);
		tm->handler();
	}
}

Common::HardwareInputSet *OSystem_3DS::getHardwareInputSet() {
	using namespace Common;

	CompositeHardwareInputSet *inputSet = new CompositeHardwareInputSet();
	// Touch input sends mouse events for now, so we need to declare we have a mouse...
	inputSet->addHardwareInputSet(new MouseHardwareInputSet(ctrMouseButtons));
	inputSet->addHardwareInputSet(new JoystickHardwareInputSet(ctrJoystickButtons, ctrJoystickAxes));

	return inputSet;
}

void OSystem_3DS::initEvents() {
	eventMutex = new Common::Mutex();
	s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	_timerThread = threadCreate(&timerThreadFunc, this, 32 * 1024, prio - 1, -2, false);
	_eventThread = threadCreate(&eventThreadFunc, &_eventQueue, 32 * 1024, prio - 1, -2, false);

	aptHook(&cookie, aptHookFunc, this);
	_eventManager->getEventDispatcher()->registerObserver(this, 10, false);
}

void OSystem_3DS::destroyEvents() {
	_eventManager->getEventDispatcher()->unregisterObserver(this);

	threadJoin(_timerThread, U64_MAX);
	threadFree(_timerThread);

	threadJoin(_eventThread, U64_MAX);
	threadFree(_eventThread);
	delete eventMutex;
}

void OSystem_3DS::transformPoint(touchPosition &point) {
	if (!_overlayVisible) {
		point.px = static_cast<float>(point.px) / _gameBottomTexture.getScaleX() - _gameBottomTexture.getPosX();
		point.py = static_cast<float>(point.py) / _gameBottomTexture.getScaleY() - _gameBottomTexture.getPosY();
	}

	clipPoint(point);
}

void OSystem_3DS::clipPoint(touchPosition &point) {
	if (_overlayVisible) {
		point.px = CLIP<uint16>(point.px, 0, getOverlayWidth()  - 1);
		point.py = CLIP<uint16>(point.py, 0, getOverlayHeight() - 1);
	} else {
		point.px = CLIP<uint16>(point.px, 0, _gameTopTexture.actualWidth  - 1);
		point.py = CLIP<uint16>(point.py, 0, _gameTopTexture.actualHeight - 1);
	}
}

enum _3DSCustomEvent {
	k3DSEventToggleDragMode,
	k3DSEventToggleMagnifyMode,
	k3DSEventOpenSettings
};

Common::KeymapArray OSystem_3DS::getGlobalKeymaps() {
	using namespace Common;

	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGlobal, "3ds", "3DS");

	Action *act;

	act = new Action("DRAGM", _("Toggle Drag Mode"));
	act->setCustomBackendActionEvent(k3DSEventToggleDragMode);
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	keymap->addAction(act);

	act = new Action("MAGM", _("Toggle Magnify Mode"));
	act->setCustomBackendActionEvent(k3DSEventToggleMagnifyMode);
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	keymap->addAction(act);

	act = new Action("OPTS", _("Open 3DS Settings"));
	act->setCustomBackendActionEvent(k3DSEventOpenSettings);
	act->addDefaultInputMapping("JOY_BACK");
	keymap->addAction(act);

	return Keymap::arrayOf(keymap);
}

Common::KeymapperDefaultBindings *OSystem_3DS::getKeymapperDefaultBindings() {
	Common::KeymapperDefaultBindings *keymapperDefaultBindings = new Common::KeymapperDefaultBindings();

	// Bind the virtual keyboard to X so SELECT can be used for the 3DS options dialog
	keymapperDefaultBindings->setDefaultBinding(Common::kGlobalKeymapName, "VIRT", "JOY_X");

	// Unmap the main menu standard action so LEFT_SHOULDER can be used for drag mode
	keymapperDefaultBindings->setDefaultBinding("engine-default", Common::kStandardActionOpenMainMenu, "");

	return keymapperDefaultBindings;
}

bool OSystem_3DS::pollEvent(Common::Event &event) {
	if (!aptMainLoop()) {
		// The system requested us to quit
		if (_sleepPauseToken.isActive()) {
			_sleepPauseToken.clear();
		}

		event.type = Common::EVENT_QUIT;
		return true;
	}

	// If magnify mode is on when returning to Launcher, turn it off
	if (_eventManager->shouldReturnToLauncher()) {
		if (_magnifyMode == MODE_MAGON) {
			_magnifyMode = MODE_MAGOFF;
			updateSize();
			if (savedInputMode == MODE_DRAG) {
				inputMode = savedInputMode;
				displayMessageOnOSD(_("Magnify Mode Off. Reactivating Drag Mode.\nReturning to Launcher..."));
			} else {
				displayMessageOnOSD(_("Magnify Mode Off. Returning to Launcher..."));
			}
		}
	}

	Common::StackLock lock(*eventMutex);

	if (_eventQueue.empty()) {
		return false;
	}

	event = _eventQueue.pop();

	if (Common::isMouseEvent(event)) {
		warpMouse(event.mouse.x, event.mouse.y);
	}

	return true;
}

bool OSystem_3DS::notifyEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_CUSTOM_BACKEND_ACTION_START
	        && event.type != Common::EVENT_CUSTOM_BACKEND_ACTION_END) {
		return false; // We're only interested in custom backend events
	}

	if (event.type == Common::EVENT_CUSTOM_BACKEND_ACTION_END) {
		return true; // We'll say we have handled the event so it is not propagated
	}

	switch ((_3DSCustomEvent)event.customType) {
	case k3DSEventToggleDragMode:
		if (inputMode == MODE_DRAG) {
			inputMode = savedInputMode = MODE_HOVER;
			displayMessageOnOSD(_("Hover Mode"));
		} else {
			if (_magnifyMode == MODE_MAGOFF) {
				inputMode = savedInputMode = MODE_DRAG;
				displayMessageOnOSD(_("Drag Mode"));
			} else {
				displayMessageOnOSD(_("Cannot Switch to Drag Mode while Magnify Mode is On"));
			}
		}
		return true;

	case k3DSEventToggleMagnifyMode:
		if (_overlayVisible) {
			displayMessageOnOSD(_("Magnify Mode cannot be activated in menus."));
		} else if (config.screen != kScreenBoth && _magnifyMode == MODE_MAGOFF) {
			// TODO: Automatically enable both screens while magnify mode is on
			displayMessageOnOSD(_("Magnify Mode can only be activated\n when both screens are enabled."));
		} else if (_gameWidth <= 400 && _gameHeight <= 240) {
			displayMessageOnOSD(_("In-game resolution too small to magnify."));
		} else {
			if (_magnifyMode == MODE_MAGOFF) {
				_magnifyMode = MODE_MAGON;
				if (inputMode == MODE_DRAG) {
					inputMode = MODE_HOVER;
					displayMessageOnOSD(_("Magnify Mode On. Switching to Hover Mode..."));
				} else {
					displayMessageOnOSD(_("Magnify Mode On"));
				}
			} else {
				_magnifyMode = MODE_MAGOFF;
				updateSize();
				if (savedInputMode == MODE_DRAG) {
					inputMode = savedInputMode;
					displayMessageOnOSD(_("Magnify Mode Off. Reactivating Drag Mode..."));
				} else {
					displayMessageOnOSD(_("Magnify Mode Off"));
				}
			}
		}
		return true;

	case k3DSEventOpenSettings:
		runOptionsDialog();
		return true;
	}

	return false;
}

void OSystem_3DS::runOptionsDialog() {
	static bool optionsDialogRunning = false;

	// Prevent opening the options dialog multiple times
	if (optionsDialogRunning) {
		return;
	}

	optionsDialogRunning = true;

	PauseToken pauseToken;
	OptionsDialog dialog;
	if (g_engine) {
		pauseToken = g_engine->pauseEngine();
	}
	int result = dialog.runModal();
	if (g_engine) {
		pauseToken.clear();
	}

	if (result > 0) {
		int oldScreen = config.screen;

		config.showCursor   = dialog.getShowCursor();
		config.snapToBorder = dialog.getSnapToBorder();
		config.stretchToFit = dialog.getStretchToFit();
		config.screen       = dialog.getScreen();

		saveConfig();
		loadConfig();

		if (config.screen != oldScreen) {
			_screenChangeId++;
			g_gui.checkScreenChange();
		}
	}

	optionsDialogRunning = false;
}

} // namespace _3DS
