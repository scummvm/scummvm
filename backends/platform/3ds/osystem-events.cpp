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
static bool optionMenuOpening = false;

static void pushEventQueue(Common::Queue<Common::Event> *queue, Common::Event &event) {
	Common::StackLock lock(*eventMutex);
	queue->push(event);
}

static void eventThreadFunc(void *arg) {
	OSystem_3DS *osys = (OSystem_3DS *)g_system;
	auto eventQueue = (Common::Queue<Common::Event> *)arg;

	uint32 touchStartTime = osys->getMillis();
	touchPosition lastTouch = {0, 0};
	bool isRightClick = false;
	float cursorDeltaX = 0;
	float cursorDeltaY = 0;
	int circleDeadzone = 20;
	int borderSnapZone = 6;
	Common::Event event;

	while (!osys->exiting) {
		do {
			osys->delayMillis(10);
		} while (osys->sleeping && !osys->exiting);

		hidScanInput();
		touchPosition touch;
		circlePosition circle;
		u32 held = hidKeysHeld();
		u32 keysPressed = hidKeysDown();
		u32 keysReleased = hidKeysUp();

		// C-Pad used to control the cursor
		hidCircleRead(&circle);
		if (circle.dx < circleDeadzone && circle.dx > -circleDeadzone)
			circle.dx = 0;
		if (circle.dy < circleDeadzone && circle.dy > -circleDeadzone)
			circle.dy = 0;
		cursorDeltaX = (0.0002f + config.sensitivity / 100000.f) * circle.dx * abs(circle.dx);
		cursorDeltaY = (0.0002f + config.sensitivity / 100000.f) * circle.dy * abs(circle.dy);

		// Touch screen events
		if (held & KEY_TOUCH) {
			hidTouchRead(&touch);
			if (config.snapToBorder) {
				if (touch.px < borderSnapZone)
					touch.px = 0;
				if (touch.px > 319 - borderSnapZone)
					touch.px = 319;
				if (touch.py < borderSnapZone)
					touch.py = 0;
				if (touch.py > 239 - borderSnapZone)
					touch.py = 239;
			}

			osys->transformPoint(touch);

			osys->warpMouse(touch.px, touch.py);
			event.mouse.x = touch.px;
			event.mouse.y = touch.py;

			if (keysPressed & KEY_TOUCH) {
				touchStartTime = osys->getMillis();
				isRightClick = (held & KEY_X || held & KEY_DUP);
				if (inputMode == MODE_DRAG) {
					event.type = isRightClick ? Common::EVENT_RBUTTONDOWN : Common::EVENT_LBUTTONDOWN;
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
				event.type = isRightClick ? Common::EVENT_RBUTTONUP : Common::EVENT_LBUTTONUP;
				pushEventQueue(eventQueue, event);
			} else if (osys->getMillis() - touchStartTime < 200) {
				// Process click in MODE_HOVER
				event.type = Common::EVENT_MOUSEMOVE;
				pushEventQueue(eventQueue, event);
				event.type = isRightClick ? Common::EVENT_RBUTTONDOWN : Common::EVENT_LBUTTONDOWN;
				pushEventQueue(eventQueue, event);
				event.type = isRightClick ? Common::EVENT_RBUTTONUP : Common::EVENT_LBUTTONUP;
				pushEventQueue(eventQueue, event);
			}
		} else if (cursorDeltaX != 0 || cursorDeltaY != 0) {
			float scaleRatio = osys->getScaleRatio();

			lastTouch.px += cursorDeltaX / scaleRatio;
			lastTouch.py -= cursorDeltaY / scaleRatio;

			osys->clipPoint(lastTouch);
			osys->warpMouse(lastTouch.px, lastTouch.py);

			event.mouse.x = lastTouch.px;
			event.mouse.y = lastTouch.py;
			event.type = Common::EVENT_MOUSEMOVE;
			pushEventQueue(eventQueue, event);
		}

		// Button events
		if (keysPressed & KEY_L) {
			if (g_gui.isActive()) {
				osys->displayMessageOnOSD(_("Magnify Mode cannot be activated in menus."));
			} else if (config.screen != kScreenBoth && osys->getMagnifyMode() == MODE_MAGOFF) {
				// TODO: Automatically enable both screens while magnify mode is on
				osys->displayMessageOnOSD(_("Magnify Mode can only be activated\n when both screens are enabled."));
			} else if (osys->getWidth() <= 400 && osys->getHeight() <= 240) {
				osys->displayMessageOnOSD(_("In-game resolution too small to magnify."));
			} else {
				if (osys->getMagnifyMode() == MODE_MAGOFF) {
					osys->setMagnifyMode(MODE_MAGON);
					if (inputMode == MODE_DRAG) {
						inputMode = MODE_HOVER;
						osys->displayMessageOnOSD(_("Magnify Mode On. Switching to Hover Mode..."));
					} else {
						osys->displayMessageOnOSD(_("Magnify Mode On"));
					}
				} else {
					osys->setMagnifyMode(MODE_MAGOFF);
					osys->updateSize();
					if (savedInputMode == MODE_DRAG) {
						inputMode = savedInputMode;
						osys->displayMessageOnOSD(_("Magnify Mode Off. Reactivating Drag Mode..."));
					} else {
						osys->displayMessageOnOSD(_("Magnify Mode Off"));
					}
				}
			}
		}
		if (keysPressed & KEY_R) {
			if (inputMode == MODE_DRAG) {
				inputMode = savedInputMode = MODE_HOVER;
				osys->displayMessageOnOSD(_("Hover Mode"));
			} else {
				if (osys->getMagnifyMode() == MODE_MAGOFF) {
					inputMode = savedInputMode = MODE_DRAG;
					osys->displayMessageOnOSD(_("Drag Mode"));
				} else
					osys->displayMessageOnOSD(_("Cannot Switch to Drag Mode while Magnify Mode is On"));
			}
		}
		if (keysPressed & KEY_A || keysPressed & KEY_DLEFT || keysReleased & KEY_A || keysReleased & KEY_DLEFT) {
			// SIMULATE LEFT CLICK
			event.mouse.x = lastTouch.px;
			event.mouse.y = lastTouch.py;
			if (keysPressed & KEY_A || keysPressed & KEY_DLEFT)
				event.type = Common::EVENT_LBUTTONDOWN;
			else
				event.type = Common::EVENT_LBUTTONUP;
			pushEventQueue(eventQueue, event);
		}
		if (keysPressed & KEY_B || keysReleased & KEY_B || keysPressed & KEY_DDOWN || keysReleased & KEY_DDOWN) {
			if (keysPressed & KEY_B || keysPressed & KEY_DDOWN)
				event.type = Common::EVENT_KEYDOWN;
			else
				event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = Common::ASCII_ESCAPE;
			event.kbd.flags = 0;
			pushEventQueue(eventQueue, event);
		}
		if (keysPressed & KEY_X || keysPressed & KEY_DUP || keysReleased & KEY_X || keysReleased & KEY_DUP) {
			// SIMULATE RIGHT CLICK
			event.mouse.x = lastTouch.px;
			event.mouse.y = lastTouch.py;
			if (keysPressed & KEY_X || keysPressed & KEY_DUP)
				event.type = Common::EVENT_RBUTTONDOWN;
			else
				event.type = Common::EVENT_RBUTTONUP;
			pushEventQueue(eventQueue, event);
		}
		if (keysPressed & KEY_Y || keysPressed & KEY_DRIGHT) {
			event.type = Common::EVENT_VIRTUAL_KEYBOARD;
			pushEventQueue(eventQueue, event);
		}
		if (keysPressed & KEY_START) {
			event.type = Common::EVENT_MAINMENU;
			pushEventQueue(eventQueue, event);
		}
		if (keysPressed & KEY_SELECT) {
			if (!optionMenuOpened)
				optionMenuOpening = true;
		}

		// If magnify mode is on when returning to Launcher, turn it off
		if (g_system->getEventManager()->shouldRTL()) {
			if (osys->getMagnifyMode() == MODE_MAGON) {
				osys->setMagnifyMode(MODE_MAGOFF);
				osys->updateSize();
				if (savedInputMode == MODE_DRAG) {
					inputMode = savedInputMode;
					osys->displayMessageOnOSD(_("Magnify Mode Off. Reactivating Drag Mode.\nReturning to Launcher..."));
				} else
					osys->displayMessageOnOSD(_("Magnify Mode Off. Returning to Launcher..."));
			}
		}

		// TODO: EVENT_PREDICTIVE_DIALOG
		// EVENT_SCREEN_CHANGED
	}
}

static void aptHookFunc(APT_HookType hookType, void *param) {
	OSystem_3DS *osys = (OSystem_3DS *)g_system;

	switch (hookType) {
		case APTHOOK_ONSUSPEND:
		case APTHOOK_ONSLEEP:
			if (g_engine)
				g_engine->pauseEngine(true);
			osys->sleeping = true;
			if (R_SUCCEEDED(gspLcdInit())) {
				GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);
				gspLcdExit();
			}
			break;
		case APTHOOK_ONRESTORE:
		case APTHOOK_ONWAKEUP:
			if (g_engine)
				g_engine->pauseEngine(false);
			osys->sleeping = false;
			loadConfig();
			break;
		default: {
			Common::StackLock lock(*eventMutex);
			Common::Event event;
			event.type = Common::EVENT_QUIT;
			g_system->getEventManager()->pushEvent(event);
		}
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

void OSystem_3DS::initEvents() {
	eventMutex = new Common::Mutex();
	s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	_timerThread = threadCreate(&timerThreadFunc, this, 32 * 1024, prio - 1, -2, false);
	_eventThread = threadCreate(&eventThreadFunc, &_eventQueue, 32 * 1024, prio - 1, -2, false);

	aptHook(&cookie, aptHookFunc, this);
}

void OSystem_3DS::destroyEvents() {
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

void OSystem_3DS::setMagnifyMode(MagnifyMode mode) {
	_magnifyMode = mode;
}

bool OSystem_3DS::pollEvent(Common::Event &event) {
	aptMainLoop(); // Call apt hook when necessary

	if (optionMenuOpening) {
		optionMenuOpening = false;
		runOptionsDialog();
	}

	Common::StackLock lock(*eventMutex);

	if (_eventQueue.empty())
		return false;

	event = _eventQueue.pop();
	return true;
}

void OSystem_3DS::runOptionsDialog() {
	OptionsDialog dialog;
	if (g_engine)
		g_engine->pauseEngine(true);
	int result = dialog.runModal();
	if (g_engine)
		g_engine->pauseEngine(false);

	if (result > 0) {
		int oldScreen = config.screen;

		config.showCursor   = dialog.getShowCursor();
		config.snapToBorder = dialog.getSnapToBorder();
		config.stretchToFit = dialog.getStretchToFit();
		config.sensitivity  = dialog.getSensitivity();
		config.screen       = dialog.getScreen();

		saveConfig();
		loadConfig();

		if (config.screen != oldScreen) {
			_screenChangeId++;
			g_gui.checkScreenChange();
		}
	}
}

} // namespace _3DS
