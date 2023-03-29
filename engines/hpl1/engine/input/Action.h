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

#ifndef HPL_ACTION_H
#define HPL_ACTION_H

#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class iAction {
public:
	iAction(tString asName);
	virtual ~iAction() {}

	/**
	 *
	 * \return returns true if the action just was triggered, else false
	 */
	bool WasTriggerd();

	/**
	 *
	 * \return true if the action just became triggered, else false
	 */
	bool BecameTriggerd();

	/**
	 *
	 * \return true if the action just was double triggered (double clicked), else false
	 */
	bool DoubleTriggerd(float afLimit);

	/**
	 *Update the Action, called by cInput
	 */
	void Update(float afTimeStep);

	/**
	 *
	 * \return the name of the action.
	 */
	tString GetName();

	/**
	 * Update special logic for the action. Normally empty
	 */
	virtual void UpdateLogic(float afTimeStep);

	/**
	 * Filled in by the class that inherits from Action.
	 * \return true if the action is being triggered
	 */
	virtual bool IsTriggerd() = 0;

	/**
	 *
	 * \return A value from the input, ie the relative mouse x position.
	 */
	virtual float GetValue() = 0;

	/**
	 * The name of the input, ie for keyboard the name of the key is returned.
	 */
	virtual tString GetInputName() = 0;

	/**
	 * The name of the input type.
	 */
	virtual tString GetInputType() = 0;

private:
	tString msName;

	bool mbBecameTriggerd;
	bool mbIsTriggerd;

	bool mbTriggerDown;

	double mfTimeCount;
};

} // namespace hpl

#endif // HPL_ACTION_H
