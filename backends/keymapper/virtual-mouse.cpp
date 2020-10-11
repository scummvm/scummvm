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

#include "backends/keymapper/virtual-mouse.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/translation.h"

#include "gui/gui-manager.h"

namespace Common {

VirtualMouse::VirtualMouse(EventDispatcher *eventDispatcher) :
		_eventDispatcher(eventDispatcher),
		_inputAxisPositionX(0),
		_inputAxisPositionY(0),
		_mouseVelocityX(0.f),
		_mouseVelocityY(0.f),
		_slowModifier(1.f),
		_subPixelRemainderX(0.f),
		_subPixelRemainderY(0.f),
		_lastUpdateMillis(0) {
	ConfMan.registerDefault("kbdmouse_speed", 3);
	ConfMan.registerDefault("joystick_deadzone", 3);

	_eventDispatcher->registerSource(this, false);
	_eventDispatcher->registerObserver(this, 10, false);
}

VirtualMouse::~VirtualMouse() {
	_eventDispatcher->unregisterObserver(this);
	_eventDispatcher->unregisterSource(this);
}

bool VirtualMouse::pollEvent(Event &event) {
	// Update the virtual mouse once per frame (assuming 60Hz)
	uint32 curTime = g_system->getMillis(true);
	if (curTime < _lastUpdateMillis + kUpdateDelay) {
		return false;
	}
	_lastUpdateMillis = curTime;

	// Adjust the speed of the cursor according to the virtual screen resolution
	Common::Rect screenSize;
	if (g_system->isOverlayVisible()) {
		screenSize = Common::Rect(g_system->getOverlayWidth(), g_system->getOverlayHeight());
	} else {
		screenSize = Common::Rect(g_system->getWidth(), g_system->getHeight());
	}

	float screenSizeSpeedModifier = screenSize.width() / (float)kDefaultScreenWidth;

	// Compute the movement delta when compared to the previous update
	float deltaX = _subPixelRemainderX + _mouseVelocityX * _slowModifier * screenSizeSpeedModifier * 10.f;
	float deltaY = _subPixelRemainderY + _mouseVelocityY * _slowModifier * screenSizeSpeedModifier * 10.f;

	Common::Point delta;
	delta.x = deltaX;
	delta.y = deltaY;

	// Keep track of sub-pixel movement so the cursor ultimately moves,
	// even when configured at very low speeds.
	_subPixelRemainderX = deltaX - delta.x;
	_subPixelRemainderY = deltaY - delta.y;

	if (delta.x == 0 && delta.y == 0) {
		return false;
	}

	// Send a mouse event
	Common::Point oldPos = g_system->getEventManager()->getMousePos();

	event.type = Common::EVENT_MOUSEMOVE;
	event.mouse = oldPos + delta;

	event.mouse.x = CLIP<int16>(event.mouse.x, 0, screenSize.width());
	event.mouse.y = CLIP<int16>(event.mouse.y, 0, screenSize.height());

	event.relMouse.x = delta.x;
	event.relMouse.y = delta.y;

	g_system->warpMouse(event.mouse.x, event.mouse.y);

	return true;
}

bool VirtualMouse::notifyEvent(const Event &event) {
	if (event.type != EVENT_CUSTOM_BACKEND_ACTION_AXIS) {
		return false;
	}

	switch (event.customType) {
	case kCustomActionVirtualAxisUp:
		if (event.joystick.position == 0 && _inputAxisPositionY > 0) {
			return true; // Ignore axis reset events if we are already going in the other direction
		}

		handleAxisMotion(_inputAxisPositionX, -event.joystick.position);
		return true;
	case kCustomActionVirtualAxisDown:
		if (event.joystick.position == 0 && _inputAxisPositionY < 0) {
			return true;
		}

		handleAxisMotion(_inputAxisPositionX, event.joystick.position);
		return true;
	case kCustomActionVirtualAxisLeft:
		if (event.joystick.position == 0 && _inputAxisPositionX > 0) {
			return true;
		}

		handleAxisMotion(-event.joystick.position, _inputAxisPositionY);
		return true;
	case kCustomActionVirtualAxisRight:
		if (event.joystick.position == 0 && _inputAxisPositionX < 0) {
			return true;
		}

		handleAxisMotion(event.joystick.position, _inputAxisPositionY);
		return true;
	case kCustomActionVirtualMouseSlow:
		_slowModifier = 0.9f * (1.f - event.joystick.position / (float)JOYAXIS_MAX) + 0.1f;
		return true;
	default:
		break;
	}

	return false;
}

void VirtualMouse::addActionsToKeymap(Keymap *keymap) {
	Action *act;

	act = new Action("VMOUSEUP", _("Virtual mouse up"));
	act->addDefaultInputMapping("JOY_LEFT_STICK_Y-");
	act->setCustomBackendActionAxisEvent(VirtualMouse::kCustomActionVirtualAxisUp);
	keymap->addAction(act);

	act = new Action("VMOUSEDOWN", _("Virtual mouse down"));
	act->addDefaultInputMapping("JOY_LEFT_STICK_Y+");
	act->setCustomBackendActionAxisEvent(VirtualMouse::kCustomActionVirtualAxisDown);
	keymap->addAction(act);

	act = new Action("VMOUSELEFT", _("Virtual mouse left"));
	act->addDefaultInputMapping("JOY_LEFT_STICK_X-");
	act->setCustomBackendActionAxisEvent(VirtualMouse::kCustomActionVirtualAxisLeft);
	keymap->addAction(act);

	act = new Action("VMOUSERIGHT", _("Virtual mouse right"));
	act->addDefaultInputMapping("JOY_LEFT_STICK_X+");
	act->setCustomBackendActionAxisEvent(VirtualMouse::kCustomActionVirtualAxisRight);
	keymap->addAction(act);

	act = new Action("VMOUSESLOW", _("Slow down virtual mouse"));
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	act->setCustomBackendActionAxisEvent(VirtualMouse::kCustomActionVirtualMouseSlow);
	keymap->addAction(act);
}

void VirtualMouse::handleAxisMotion(int16 axisPositionX, int16 axisPositionY) {
	_inputAxisPositionX = axisPositionX;
	_inputAxisPositionY = axisPositionY;

	float analogX  = (float)_inputAxisPositionX;
	float analogY  = (float)_inputAxisPositionY;
	float deadZone = (float)ConfMan.getInt("joystick_deadzone") * 1000.0f;

	float magnitude = sqrt(analogX * analogX + analogY * analogY);

	if (magnitude >= deadZone) {
		float scalingFactor = 1.0f / magnitude * (magnitude - deadZone) / (JOYAXIS_MAX - deadZone);
		float speedFactor = computeJoystickMouseSpeedFactor();
		_mouseVelocityX = analogX * scalingFactor * speedFactor;
		_mouseVelocityY = analogY * scalingFactor * speedFactor;
	} else {
		_mouseVelocityX = 0.f;
		_mouseVelocityY = 0.f;
	}
}

float VirtualMouse::computeJoystickMouseSpeedFactor() const {
	switch (ConfMan.getInt("kbdmouse_speed")) {
	case 0:
		return 0.25; // 0.25 keyboard pointer speed
	case 1:
		return 0.5;  // 0.5 speed
	case 2:
		return 0.75; // 0.75 speed
	case 3:
		return 1.0;  // 1.0 speed
	case 4:
		return 1.25; // 1.25 speed
	case 5:
		return 1.5;  // 1.5 speed
	case 6:
		return 1.75; // 1.75 speed
	case 7:
		return 2.0;  // 2.0 speed
	default:
		return 1.0;
	}
}

} // End of namespace Common
