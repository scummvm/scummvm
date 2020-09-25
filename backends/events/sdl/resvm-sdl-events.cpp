/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#if defined(SDL_BACKEND)

#include "backends/events/sdl/resvm-sdl-events.h"
#include "backends/graphics/resvm-sdl/resvm-sdl-graphics.h"
#include "engines/engine.h"
#include "gui/gui-manager.h"
#include "common/config-manager.h"

#if defined(ENABLE_VKEYBD) && SDL_VERSION_ATLEAST(2, 0, 0)
#define CONTROLLER_BUT_VKEYBOARD SDL_CONTROLLER_BUTTON_BACK
#endif

// FIXME move joystick defines out and replace with confile file options
// we should really allow users to map any key to a joystick button

// #define JOY_INVERT_Y
#define JOY_XAXIS 0
#define JOY_YAXIS 1
// buttons
#define JOY_BUT_LMOUSE 0
#define JOY_BUT_RMOUSE 2
#define JOY_BUT_ESCAPE 3
#define JOY_BUT_PERIOD 1
#define JOY_BUT_SPACE 4
#define JOY_BUT_F5 5
#ifdef ENABLE_VKEYBD
#define JOY_BUT_VKEYBOARD 7
#endif

ResVmSdlEventSource::ResVmSdlEventSource() {
	// Reset mouse state
	memset(&_km, 0, sizeof(_km));

	ConfMan.registerDefault("kbdmouse_speed", 3);
	ConfMan.registerDefault("joystick_deadzone", 3);
}

bool ResVmSdlEventSource::processMouseEvent(Common::Event &event, int x, int y, int relx, int rely) {
	event.relMouse.x = relx;
	event.relMouse.y = rely;

	return SdlEventSource::processMouseEvent(event, x, y);
}

bool ResVmSdlEventSource::pollEvent(Common::Event &event) {
	bool state = SdlEventSource::pollEvent(event);

	// Handle mouse control via analog joystick and keyboard
	if (!state && handleKbdMouse(event)) {
		return true;
	}

	return state;
}

bool ResVmSdlEventSource::handleMouseMotion(SDL_Event &ev, Common::Event &event) {
	// update KbdMouse
	_km.x = ev.motion.x * MULTIPLIER;
	_km.y = ev.motion.y * MULTIPLIER;

	return SdlEventSource::handleMouseMotion(ev, event);
}

bool ResVmSdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	// update KbdMouse
	_km.x = ev.motion.x * MULTIPLIER;
	_km.y = ev.motion.y * MULTIPLIER;

	return SdlEventSource::handleMouseButtonDown(ev, event);
}

bool ResVmSdlEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	// update KbdMouse
	_km.x = ev.motion.x * MULTIPLIER;
	_km.y = ev.motion.y * MULTIPLIER;

	return SdlEventSource::handleMouseButtonUp(ev, event);
}

bool ResVmSdlEventSource::handleJoyButtonDown(SDL_Event &ev, Common::Event &event) {
	if (shouldGenerateMouseEvents()) {
		if (ev.jbutton.button == JOY_BUT_LMOUSE) {
			event.type = Common::EVENT_LBUTTONDOWN;
			return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
		} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
			event.type = Common::EVENT_RBUTTONDOWN;
			return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
		}
	}

	return SdlEventSource::handleJoyButtonDown(ev, event);
}

bool ResVmSdlEventSource::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {
	if (shouldGenerateMouseEvents()) {
		if (ev.jbutton.button == JOY_BUT_LMOUSE) {
			event.type = Common::EVENT_LBUTTONUP;
			return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
		} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
			event.type = Common::EVENT_RBUTTONUP;
			return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
		}
	}

	return SdlEventSource::handleJoyButtonUp(ev, event);
}

bool ResVmSdlEventSource::handleJoyAxisMotion(SDL_Event &ev, Common::Event &event) {
	if (shouldGenerateMouseEvents()) {
		if (ev.jaxis.axis == JOY_XAXIS) {
			_km.joy_x = ev.jaxis.value;
			return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
		} else if (ev.jaxis.axis == JOY_YAXIS) {
			_km.joy_y = ev.jaxis.value;
			return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
		}
	}

	return SdlEventSource::handleJoyAxisMotion(ev, event);
}

#if SDL_VERSION_ATLEAST(2, 0, 0)

static int mapSDLControllerButtonToResVM(Uint8 sdlButton) {
	Common::JoystickButton resvmButtons[] = {
	    Common::JOYSTICK_BUTTON_A,
	    Common::JOYSTICK_BUTTON_B,
	    Common::JOYSTICK_BUTTON_X,
	    Common::JOYSTICK_BUTTON_Y,
	    Common::JOYSTICK_BUTTON_BACK,
	    Common::JOYSTICK_BUTTON_GUIDE,
	    Common::JOYSTICK_BUTTON_START,
	    Common::JOYSTICK_BUTTON_LEFT_STICK,
	    Common::JOYSTICK_BUTTON_RIGHT_STICK,
	    Common::JOYSTICK_BUTTON_LEFT_SHOULDER,
	    Common::JOYSTICK_BUTTON_RIGHT_SHOULDER,
	    Common::JOYSTICK_BUTTON_DPAD_UP,
	    Common::JOYSTICK_BUTTON_DPAD_DOWN,
	    Common::JOYSTICK_BUTTON_DPAD_LEFT,
	    Common::JOYSTICK_BUTTON_DPAD_RIGHT,
	};

	if (sdlButton >= ARRAYSIZE(resvmButtons)) {
		return -1;
	}

	return resvmButtons[sdlButton];
}

bool ResVmSdlEventSource::handleControllerButton(const SDL_Event &ev, Common::Event &event, bool buttonUp) {
	if (shouldGenerateMouseEvents()) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		if (event.joystick.button == Common::JOYSTICK_BUTTON_RIGHT_SHOULDER) {
			// Right shoulder is the modifier button that makes the mouse go slower.
			_km.modifier = !buttonUp;
		}
#endif
		return SdlEventSource::handleControllerButton(ev, event, buttonUp);
	} else {
#ifdef ENABLE_VKEYBD
		// Trigger virtual keyboard on long press of more than 1 second of configured button
		const uint32 vkeybdTime = 1000;
		static uint32 vkeybdThen = 0;

		if (ev.cbutton.button == CONTROLLER_BUT_VKEYBOARD) {
			if (!buttonUp) {
				vkeybdThen = g_system->getMillis();
			} else if ((vkeybdThen > 0) && (g_system->getMillis() - vkeybdThen >= vkeybdTime)) {
				vkeybdThen = 0;
				event.type = Common::EVENT_VIRTUAL_KEYBOARD;
				return true;
			}
		}
#endif
		int button = mapSDLControllerButtonToResVM(ev.cbutton.button);
		if (button == -1) {
			return false;
		}

		event.type = buttonUp ? Common::EVENT_JOYBUTTON_UP : Common::EVENT_JOYBUTTON_DOWN;
		event.joystick.button = button;
		return true;
	}
}

bool ResVmSdlEventSource::handleControllerAxisMotion(const SDL_Event &ev, Common::Event &event) {
	if (shouldGenerateMouseEvents()) {
		if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
			_km.joy_x = ev.caxis.value;
			return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
		} else if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
			_km.joy_y = ev.caxis.value;
			return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
		}
		return SdlEventSource::handleControllerAxisMotion(ev, event);
	} else {
		// Indicates if L2/R2 are currently pushed or not
		static bool l2Pushed = false, r2Pushed = false;
		// Simulate buttons from left and right triggers (needed for EMI)
		if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
			bool pushed = (ev.caxis.value > 0);
			if (l2Pushed == pushed)
				return false;
			event.type = (pushed) ? Common::EVENT_JOYBUTTON_DOWN : Common::EVENT_JOYBUTTON_UP;
			event.joystick.button = Common::JOYSTICK_BUTTON_LEFT_TRIGGER;
			l2Pushed = pushed;
			return true;
		} else if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
			bool pushed = (ev.caxis.value > 0);
			if (r2Pushed == pushed)
				return false;
			event.type = (pushed) ? Common::EVENT_JOYBUTTON_DOWN : Common::EVENT_JOYBUTTON_UP;
			event.joystick.button = Common::JOYSTICK_BUTTON_RIGHT_TRIGGER;
			r2Pushed = pushed;
			return true;
		} else {
			event.type = Common::EVENT_JOYAXIS_MOTION;
			event.joystick.axis = ev.caxis.axis;
			event.joystick.position = ev.caxis.value;
			return true;
		}
	}
}
#endif

bool ResVmSdlEventSource::shouldGenerateMouseEvents() {
	// Engine doesn't support joystick -> emulate mouse events
	if (g_engine && !g_engine->hasFeature(Engine::kSupportsJoystick)) {
		return true;
	}

	// Even if engine supports joystick, emulate mouse events if in GUI or in virtual keyboard
	if (g_gui.isActive() || g_engine->isPaused()) {
		return true;
	}

	return false;
}

bool ResVmSdlEventSource::handleKbdMouse(Common::Event &event) {
	// The ResidualVM version of this method handles relative mouse
	// movement, as required by Myst III.

	int32 oldKmX = _km.x;
	int32 oldKmY = _km.y;

	updateKbdMouse();

	if (_km.x != oldKmX || _km.y != oldKmY) {
		ResVmSdlGraphicsManager *graphicsManager = dynamic_cast<ResVmSdlGraphicsManager *>(_graphicsManager);

		int32 relX = _km.x - oldKmX;
		int32 relY = _km.y - oldKmY;

		if (graphicsManager) {
			if (graphicsManager->isMouseLocked()) {
				_km.x = oldKmX;
				_km.y = oldKmY;
			} else {
				// warpMouseInWindow causes SDL to generate mouse events. Don't use it when the mouse is locked
				// to avoid inconsistent events that would cause crazy camera movement in Myst III.
				graphicsManager->getWindow()->warpMouseInWindow((Uint16)(_km.x / MULTIPLIER), (Uint16)(_km.y / MULTIPLIER));
			}
		}

		event.type = Common::EVENT_MOUSEMOVE;
		return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER, relX / MULTIPLIER, relY / MULTIPLIER);
	}

	return false;
}

bool ResVmSdlEventSource::handleAxisToMouseMotion(int16 xAxis, int16 yAxis) {
#ifdef JOY_INVERT_Y
	yAxis = -yAxis;
#endif

	// conversion factor between keyboard mouse and joy axis value
	int vel_to_axis = (1500 / MULTIPLIER);

	// radial and scaled deadzone

	float analogX = (float)xAxis;
	float analogY = (float)yAxis;
	float deadZone = (float)ConfMan.getInt("joystick_deadzone") * 1000.0f;

	float magnitude = sqrt(analogX * analogX + analogY * analogY);

	if (magnitude >= deadZone) {
		_km.x_down_count = 0;
		_km.y_down_count = 0;
		float scalingFactor = 1.0f / magnitude * (magnitude - deadZone) / (32769.0f - deadZone);
		_km.x_vel = (int16)(analogX * scalingFactor * 32768.0f / vel_to_axis);
		_km.y_vel = (int16)(analogY * scalingFactor * 32768.0f / vel_to_axis);
	} else {
		_km.x_vel = 0;
		_km.y_vel = 0;
	}

	return false;
}

void ResVmSdlEventSource::resetKeyboardEmulation(int16 x_max, int16 y_max) {
	_km.x_max = x_max;
	_km.y_max = y_max;
	_km.delay_time = 12;
	_km.last_time = 0;
	_km.modifier = false;
	_km.joy_x = 0;
	_km.joy_y = 0;
}

void ResVmSdlEventSource::updateKbdMouse() {
	uint32 curTime = g_system->getMillis(true);
	if (curTime < _km.last_time + _km.delay_time) {
		return;
	}

	_km.last_time = curTime;
	if (_km.x_down_count == 1) {
		_km.x_down_time = curTime;
		_km.x_down_count = 2;
	}
	if (_km.y_down_count == 1) {
		_km.y_down_time = curTime;
		_km.y_down_count = 2;
	}

	if (_km.x_vel || _km.y_vel) {
		if (_km.x_down_count) {
			if (curTime > _km.x_down_time + 300) {
				if (_km.x_vel > 0)
					_km.x_vel += MULTIPLIER;
				else
					_km.x_vel -= MULTIPLIER;
			} else if (curTime > _km.x_down_time + 200) {
				if (_km.x_vel > 0)
					_km.x_vel = 5 * MULTIPLIER;
				else
					_km.x_vel = -5 * MULTIPLIER;
			}
		}
		if (_km.y_down_count) {
			if (curTime > _km.y_down_time + 300) {
				if (_km.y_vel > 0)
					_km.y_vel += MULTIPLIER;
				else
					_km.y_vel -= MULTIPLIER;
			} else if (curTime > _km.y_down_time + 200) {
				if (_km.y_vel > 0)
					_km.y_vel = 5 * MULTIPLIER;
				else
					_km.y_vel = -5 * MULTIPLIER;
			}
		}

		int16 speedFactor = computeJoystickMouseSpeedFactor();

		// - The modifier key makes the mouse movement slower
		// - The extra factor "delay/speedFactor" ensures velocities
		// are independent of the kbdMouse update rate
		// - all velocities were originally chosen
		// at a delay of 25, so that is the reference used here
		// - note: operator order is important to avoid overflow
		if (_km.modifier) {
			_km.x += ((_km.x_vel / 10) * ((int16)_km.delay_time)) / speedFactor;
			_km.y += ((_km.y_vel / 10) * ((int16)_km.delay_time)) / speedFactor;
		} else {
			_km.x += (_km.x_vel * ((int16)_km.delay_time)) / speedFactor;
			_km.y += (_km.y_vel * ((int16)_km.delay_time)) / speedFactor;
		}

		if (_km.x < 0) {
			_km.x = 0;
			_km.x_vel = -1 * MULTIPLIER;
			_km.x_down_count = 1;
		} else if (_km.x > _km.x_max * MULTIPLIER) {
			_km.x = _km.x_max * MULTIPLIER;
			_km.x_vel = 1 * MULTIPLIER;
			_km.x_down_count = 1;
		}

		if (_km.y < 0) {
			_km.y = 0;
			_km.y_vel = -1 * MULTIPLIER;
			_km.y_down_count = 1;
		} else if (_km.y > _km.y_max * MULTIPLIER) {
			_km.y = _km.y_max * MULTIPLIER;
			_km.y_vel = 1 * MULTIPLIER;
			_km.y_down_count = 1;
		}
	}
}

int16 ResVmSdlEventSource::computeJoystickMouseSpeedFactor() const {
	int16 speedFactor;

	switch (ConfMan.getInt("kbdmouse_speed")) {
	// 0.25 keyboard pointer speed
	case 0:
		speedFactor = 100;
		break;
	// 0.5 speed
	case 1:
		speedFactor = 50;
		break;
	// 0.75 speed
	case 2:
		speedFactor = 33;
		break;
	// 1.0 speed
	case 3:
		speedFactor = 25;
		break;
	// 1.25 speed
	case 4:
		speedFactor = 20;
		break;
	// 1.5 speed
	case 5:
		speedFactor = 17;
		break;
	// 1.75 speed
	case 6:
		speedFactor = 14;
		break;
	// 2.0 speed
	case 7:
		speedFactor = 12;
		break;
	default:
		speedFactor = 25;
	}

	// Scale the mouse cursor speed with the display size so moving across
	// the screen takes a reasonable amount of time at higher resolutions.
	return speedFactor * 480 / _km.y_max;
}

#endif
