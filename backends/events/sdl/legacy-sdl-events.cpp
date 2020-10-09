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

#if defined(SDL_BACKEND)

#include "backends/events/sdl/legacy-sdl-events.h"

// #define JOY_INVERT_Y
#define JOY_XAXIS 0
#define JOY_YAXIS 1

LegacySdlEventSource::LegacySdlEventSource() {
	// Reset mouse state
	memset(&_km, 0, sizeof(_km));

	ConfMan.registerDefault("kbdmouse_speed", 3);
	ConfMan.registerDefault("joystick_deadzone", 3);
}

void LegacySdlEventSource::updateKbdMouse() {
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

bool LegacySdlEventSource::handleKbdMouse(Common::Event &event) {
	int32 oldKmX = _km.x;
	int32 oldKmY = _km.y;

	updateKbdMouse();

	if (_km.x != oldKmX || _km.y != oldKmY) {
		if (_graphicsManager) {
			dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->getWindow()->warpMouseInWindow((Uint16)(_km.x / MULTIPLIER), (Uint16)(_km.y / MULTIPLIER));
		}

		event.type = Common::EVENT_MOUSEMOVE;
		return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
	}

	return false;
}

bool LegacySdlEventSource::handleJoyAxisMotion(SDL_Event &ev, Common::Event &event) {
	if (ev.jaxis.axis == JOY_XAXIS) {
		_km.joy_x = ev.jaxis.value;
		return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
	} else if (ev.jaxis.axis == JOY_YAXIS) {
		_km.joy_y = ev.jaxis.value;
		return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
	}

	return SdlEventSource::handleJoyAxisMotion(ev, event);
}

int16 LegacySdlEventSource::computeJoystickMouseSpeedFactor() const {
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

bool LegacySdlEventSource::handleAxisToMouseMotion(int16 xAxis, int16 yAxis) {
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

void LegacySdlEventSource::resetKeyboardEmulation(int16 x_max, int16 y_max) {
	_km.x_max = x_max;
	_km.y_max = y_max;
	_km.delay_time = 12;
	_km.last_time = 0;
	_km.modifier = false;
	_km.joy_x = 0;
	_km.joy_y = 0;
}

void LegacySdlEventSource::checkScreenChange() {
	if (!_graphicsManager) {
		return;
	}

	int newMaxX = dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->getWindowWidth()  - 1;
	int newMaxY = dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->getWindowHeight() - 1;

	if (_km.x_max != newMaxX || _km.y_max != newMaxY) {
		resetKeyboardEmulation(newMaxX, newMaxY);
	}
}

bool LegacySdlEventSource::pollEvent(Common::Event &event) {
	checkScreenChange();

	bool handled = SdlEventSource::pollEvent(event);
	if (handled) {
		return true;
	}

	// Handle mouse control via analog joystick and keyboard
	if (handleKbdMouse(event)) {
		return true;
	}

	return false;
}

bool LegacySdlEventSource::handleMouseMotion(SDL_Event &ev, Common::Event &event) {
	// update KbdMouse
	_km.x = ev.motion.x * MULTIPLIER;
	_km.y = ev.motion.y * MULTIPLIER;

	return SdlEventSource::handleMouseMotion(ev, event);
}

bool LegacySdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	// update KbdMouse
	_km.x = ev.motion.x * MULTIPLIER;
	_km.y = ev.motion.y * MULTIPLIER;

	return SdlEventSource::handleMouseButtonDown(ev, event);
}

bool LegacySdlEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	// update KbdMouse
	_km.x = ev.motion.x * MULTIPLIER;
	_km.y = ev.motion.y * MULTIPLIER;

	return SdlEventSource::handleMouseButtonUp(ev, event);
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
bool LegacySdlEventSource::handleControllerButton(const SDL_Event &ev, Common::Event &event, bool buttonUp) {
	if (event.joystick.button == Common::JOYSTICK_BUTTON_RIGHT_SHOULDER) {
		// Right shoulder is the modifier button that makes the mouse go slower.
		_km.modifier = !buttonUp;
	}

	return 	SdlEventSource::handleControllerButton(ev, event, buttonUp);
}

bool LegacySdlEventSource::handleControllerAxisMotion(const SDL_Event &ev, Common::Event &event) {
	if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
		_km.joy_x = ev.caxis.value;
		return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
	} else if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
		_km.joy_y = ev.caxis.value;
		return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
	}

	return SdlEventSource::handleControllerAxisMotion(ev, event);
}
#endif

#endif
