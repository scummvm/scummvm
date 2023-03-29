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

#ifndef HPL_KEYBOARD_SDL_H
#define HPL_KEYBOARD_SDL_H

#include "common/bitarray.h"
#include "common/queue.h"
#include "hpl1/engine/input/Keyboard.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace Common {

struct Event;

}

namespace hpl {

#define MAX_KEY_PRESSES (20)

class cLowLevelInputSDL;

class cKeyboardSDL : public iKeyboard {
public:
	cKeyboardSDL(cLowLevelInputSDL *apLowLevelInputSDL);

	void Update();

	// Keyboard specific
	bool KeyIsDown(eKey aKey);
	cKeyPress GetKey();
	bool KeyIsPressed();
	eKeyModifier GetModifier();
	tString KeyToString(eKey);
	eKey StringToKey(tString);

private:
	void processEvent(const Common::Event &ev);
	eKey AsciiToKey(int alChar);

	eKeyModifier _modifiers;
	Common::BitArray _downKeys;
	Common::Queue<cKeyPress> _pressedKeys;
	cLowLevelInputSDL *_lowLevelSystem;
};

} // namespace hpl

#endif // HPL_KEYBOARD_SDL_H
