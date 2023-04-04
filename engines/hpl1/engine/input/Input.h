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

#ifndef HPL_INPUT_H
#define HPL_INPUT_H

#include "common/list.h"
#include "hpl1/engine/game/Updateable.h"
#include "hpl1/engine/input/InputTypes.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/std/map.h"
#include "hpl1/std/multimap.h"

namespace hpl {

class iKeyboard;
class iMouse;
class LowLevelInput;
class iInputDevice;
class iAction;

typedef Hpl1::Std::map<tString, iAction *> tActionMap;
typedef tActionMap::iterator tActionMapIt;

typedef Hpl1::Std::multimap<tString, iAction *> tActionMultiMap;
typedef tActionMultiMap::iterator tActionMultiMapIt;

typedef Common::List<iInputDevice *> tInputDeviceList;
typedef tInputDeviceList::iterator tInputDeviceListIt;

class cInput : public iUpdateable {
public:
	cInput(LowLevelInput *apLowLevelInput);
	~cInput();

	/**
	 * Updates the input, called by cGame
	 */
	void Update(float afTimeStep);

	/**
	 * Add a new action.
	 * \param *apAction The action to add.
	 */
	void AddAction(iAction *apAction);

	/**
	 * Check if an action is triggered.
	 * \param asName
	 * \return
	 */
	bool IsTriggerd(tString asName);
	/**
	 *
	 * \param asName name of the action.
	 * \return
	 */
	bool WasTriggerd(tString asName);

	/**
	 *
	 * \param asName name of the action.
	 * \return
	 */
	bool BecameTriggerd(tString asName);

	/**
	 *
	 * \param asName name of the action.
	 * \return
	 */
	bool DoubleTriggerd(tString asName, float afLimit);

	/**
	 *
	 * \return currently used keyboard
	 */
	iKeyboard *GetKeyboard();

	/**
	 *
	 * \return currently used mouse
	 */
	iMouse *GetMouse();

	/**
	 * Get action from map.
	 * \param asName name of action.
	 * \return Pointer to action, if not found NULL.
	 */
	iAction *GetAction(tString asName);

	/**
	 * Destroys an action if it exists.
	 * \param asName
	 */
	void DestroyAction(tString asName);

	/**
	 * Checks if any input is given.
	 * \return true if any input is given, else false.
	 */
	bool CheckForInput();

	/**
	 * Creates an action from the latest input made. Any action with the same name is destroyed.
	 * \param &asName Name of action be be created.
	 * \return NULL if no input was given.
	 */
	iAction *InputToAction(const tString &asName);

	LowLevelInput *GetLowLevel() { return mpLowLevelInput; }

private:
	tActionMap m_mapActions;
	tInputDeviceList mlstInputDevices;

	LowLevelInput *mpLowLevelInput;

	iMouse *mpMouse;
	iKeyboard *mpKeyboard;
};
} // namespace hpl

#endif // HPL_INPUT_H
