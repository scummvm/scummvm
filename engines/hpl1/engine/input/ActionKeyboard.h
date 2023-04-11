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

#ifndef HPL_ACTIONKEYBOARD_H
#define HPL_ACTIONKEYBOARD_H

#include "common/keyboard.h"
#include "hpl1/engine/input/Action.h"
#include "hpl1/engine/input/InputTypes.h"

namespace hpl {

class cInput;

class cActionKeyboard : public iAction {
public:
	cActionKeyboard(tString asName, cInput *apInput, Common::KeyState key) : iAction(asName), _key(key), mpInput(apInput) {}

	bool IsTriggerd();
	float GetValue();

	tString GetInputName();

	tString GetInputType() { return "Keyboard"; }

	Common::KeyCode GetKey() { return _key.keycode; }
	int GetModifier() { return _key.flags; }

private:
	Common::KeyState _key;
	cInput *mpInput;
};

} // namespace hpl

#endif // HPL_ACTIONKEYBOARD_H
