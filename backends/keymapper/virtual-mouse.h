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

#ifndef BACKENDS_KEYMAPPER_VIRTUAL_MOUSE_H
#define BACKENDS_KEYMAPPER_VIRTUAL_MOUSE_H

#include "common/scummsys.h"

#include "common/events.h"

namespace Common {

class EventDispatcher;
class Keymap;

/**
 * The Virtual Mouse can produce mouse move events on systems without a physical mouse.
 *
 * It is useful for moving the mouse cursor using a gamepad or a keyboard.
 *
 * This class defines a keymap with actions for moving the cursor in all four directions.
 * The keymapper produces custom backend events whenever keys bound to these actions are
 * pressed. This class handles the events through its EventObserver interface and produces
 * mouse move events when necesssary through its EventSource interface.
 */
class VirtualMouse : public EventSource, public EventObserver {
public:
	VirtualMouse(EventDispatcher *eventDispatcher);
	~VirtualMouse() override;

	// EventSource API
	bool pollEvent(Event &event) override;

	// EventObserver API
	bool notifyEvent(const Event &event) override;

	/** Add the virtual mouse keymapper actions to a keymap */
	void addActionsToKeymap(Keymap *keymap);

private:
	static const int32 kUpdateDelay = 12;
	static const int32 kDefaultScreenWidth = 640;

	enum {
		kCustomActionVirtualAxisUp    = 10000,
		kCustomActionVirtualAxisDown  = 10001,
		kCustomActionVirtualAxisLeft  = 10002,
		kCustomActionVirtualAxisRight = 10003,
		kCustomActionVirtualMouseSlow = 10004
	};

	void handleAxisMotion(int16 axisPositionX, int16 axisPositionY);
	float computeJoystickMouseSpeedFactor() const;

	EventDispatcher *_eventDispatcher;

	int16 _inputAxisPositionX;
	int16 _inputAxisPositionY;

	float _mouseVelocityX;
	float _mouseVelocityY;
	float _slowModifier;

	float _subPixelRemainderX;
	float _subPixelRemainderY;

	uint32 _lastUpdateMillis;
};

} // End of namespace Common

#endif // #ifndef BACKENDS_KEYMAPPER_VIRTUAL_MOUSE_H
