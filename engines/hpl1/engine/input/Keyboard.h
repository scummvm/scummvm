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

#ifndef HPL_KEYBOARD_H
#define HPL_KEYBOARD_H

#include "common/bitarray.h"
#include "common/keyboard.h"
#include "common/queue.h"
#include "hpl1/engine/input/InputDevice.h"
#include "hpl1/engine/input/InputTypes.h"

namespace Common {
struct Event;
}

namespace hpl {

class LowLevelInput;

class Keyboard : public iInputDevice {
public:
	Keyboard(LowLevelInput *);

	/**
	 *
	 * \param aKey The key to check
	 * \return true if pressed else false
	 */
	bool KeyIsDown(eKey aKey);
	/**
	 * Can be checked many times to see all key presses
	 * \return key that is currently pressed. eKey_NONE is no key.
	 */
	cKeyPress GetKey();
	/**
	 *
	 * \return If ANY key is pressed
	 */
	bool KeyIsPressed();
	/**
	 * \return The current modifiers.
	 */
	eKeyModifier GetModifier();
	/**
	 * \todo Implement!
	 * \param eKey The key to change to string.
	 * \return The name of the key as a string.
	 */
	tString KeyToString(eKey);
	/**
	 * \todo Implement!
	 * \param tString NAme of the key
	 * \return enum of the key.
	 */
	eKey StringToKey(tString);

	void Update();

private:
	void processEvent(const Common::Event &ev);
	eKey AsciiToKey(int alChar);

	eKeyModifier _modifiers;
	Common::BitArray _downKeys;
	Common::Queue<cKeyPress> _pressedKeys;
	LowLevelInput *_lowLevelSystem;
};

} // namespace hpl

#endif // HPL_KEYBOARD_H
