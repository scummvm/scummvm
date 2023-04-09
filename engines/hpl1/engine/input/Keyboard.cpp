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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#include "common/events.h"
#include "hpl1/debug.h"
#include "hpl1/engine/input/Keyboard.h"
#include "hpl1/engine/input/LowLevelInput.h"

namespace hpl {

//-----------------------------------------------------------------------

Keyboard::Keyboard(LowLevelInput *lowLevelInput) : iInputDevice("Keyboard", eInputDeviceType_Keyboard) {
	_lowLevelSystem = lowLevelInput;

	_downKeys.set_size(Common::KEYCODE_LAST);
	_downKeys.clear();
}

//-----------------------------------------------------------------------

void Keyboard::processEvent(const Common::Event &ev) {
	if (ev.type != Common::EVENT_KEYDOWN && ev.type != Common::EVENT_KEYUP)
		return;

	if (ev.type == Common::EVENT_KEYDOWN) {
		_downKeys.set(ev.kbd.keycode);
		_modifiers = ev.kbd.flags;
		_pressedKeys.push(ev.kbd);
	} else {
		_downKeys.unset(ev.kbd.keycode);
	}
}

void Keyboard::Update() {
	_pressedKeys.clear();
	for (const Common::Event &ev : _lowLevelSystem->_events)
		processEvent(ev);
}

//-----------------------------------------------------------------------

bool Keyboard::KeyIsDown(Common::KeyCode key) {
	return _downKeys.get(key);
}

//-----------------------------------------------------------------------

Common::KeyState Keyboard::GetKey() {
	return _pressedKeys.pop();
}

//-----------------------------------------------------------------------

bool Keyboard::KeyIsPressed() {
	return _pressedKeys.empty() == false;
}

//-----------------------------------------------------------------------

int Keyboard::GetModifier() {
	return _modifiers;
}

//-----------------------------------------------------------------------

tString Keyboard::KeyToString(Common::KeyCode eKey) {
	HPL1_UNIMPLEMENTED(Keyboard::KeyToString);
}

//-----------------------------------------------------------------------

Common::KeyCode Keyboard::StringToKey(tString) {
	HPL1_UNIMPLEMENTED(Keyboard::StringToKey);
}

//-----------------------------------------------------------------------

} // namespace hpl
