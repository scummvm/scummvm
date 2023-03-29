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

#include "hpl1/engine/input/ActionMouseButton.h"
#include "hpl1/engine/input/Input.h"
#include "hpl1/engine/input/Mouse.h"

namespace hpl {
//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cActionMouseButton::cActionMouseButton(tString asName, cInput *apInput, eMButton aButton) : iAction(asName) {
	mButton = aButton;
	mpInput = apInput;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cActionMouseButton::IsTriggerd() {
	return mpInput->GetMouse()->ButtonIsDown(mButton);
}

//-----------------------------------------------------------------------

float cActionMouseButton::GetValue() {
	if (IsTriggerd())
		return 1.0;
	else
		return 0.0;
}

//-----------------------------------------------------------------------

tString cActionMouseButton::GetInputName() {
	switch (mButton) {
	case eMButton_Left:
		return "LeftMouse";
	case eMButton_Middle:
		return "MiddleMouse";
	case eMButton_Right:
		return "RightMouse";
	case eMButton_WheelUp:
		return "WheelUp";
	case eMButton_WheelDown:
		return "WheelDown";
	case eMButton_6:
		return "Mouse6";
	case eMButton_7:
		return "Mouse7";
	case eMButton_8:
		return "Mouse8";
	case eMButton_9:
		return "Mouse9";
	default:
		break;
	}
	return "Unknown";
}

//-----------------------------------------------------------------------
} // namespace hpl
