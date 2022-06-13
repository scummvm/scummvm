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
#ifndef HPL_KEYBOARD_SDL_H
#define HPL_KEYBOARD_SDL_H

#include <vector>
#include <list>
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/engine/input/Keyboard.h"

namespace hpl {

#define MAX_KEY_PRESSES (20)

	class cLowLevelInputSDL;

	class cKeyboardSDL : public iKeyboard
	{
	public:
		cKeyboardSDL(cLowLevelInputSDL *apLowLevelInputSDL);

		void Update();

		//Keyboard specific
		bool KeyIsDown(eKey aKey);
		cKeyPress GetKey();
		bool KeyIsPressed();
		eKeyModifier GetModifier();
		tString KeyToString(eKey);
		eKey StringToKey(tString);

	private:
		eKey SDLToKey(int alKey);
		void ClearKeyList();
		eKey AsciiToKey(int alChar);
		eKeyModifier mModifier;

		std::vector<bool> mvKeyArray;

		std::list<cKeyPress> mlstKeysPressed;

		cLowLevelInputSDL *mpLowLevelInputSDL;
	};

};

#endif // HPL_KEYBOARD_SDL_H
