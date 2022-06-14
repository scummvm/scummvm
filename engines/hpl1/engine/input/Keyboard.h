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
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HPL_KEYBOARD_H
#define HPL_KEYBOARD_H

#include "hpl1/engine/input/InputTypes.h"
#include "hpl1/engine/input/InputDevice.h"

namespace hpl {

	//------------------------------

	class iKeyboard : public iInputDevice
	{
	public:
		iKeyboard(tString asName);
		virtual ~iKeyboard(){}

		/**
		 *
		 * \param aKey The key to check
		 * \return true if pressed else false
		 */
		virtual bool KeyIsDown(eKey aKey)=0;
		/**
		 * Can be checked many times to see all key presses
		 * \return key that is currently pressed. eKey_NONE is no key.
		 */
		virtual cKeyPress GetKey()=0;
		/**
		 *
		 * \return If ANY key is pressed
		 */
		virtual bool KeyIsPressed()=0;
		/**
		 * \return The current modifiers.
		 */
		virtual eKeyModifier GetModifier()=0;
		/**
		 * \todo Implement!
		 * \param eKey The key to change to string.
		 * \return The name of the key as a string.
		 */
		virtual tString KeyToString(eKey)=0;
		/**
		 * \todo Implement!
		 * \param tString NAme of the key
		 * \return enum of the key.
		 */
		virtual eKey StringToKey(tString)=0;
	};

};

#endif // HPL_KEYBOARD_H
