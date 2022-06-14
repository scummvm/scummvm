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

#include "hpl1/engine/input/ActionHaptic.h"
#include "hpl1/engine/input/Input.h"
#include "hpl1/engine/haptic/Haptic.h"
#include "hpl1/engine/haptic/LowLevelHaptic.h"

namespace hpl
{
	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cActionHaptic::cActionHaptic(tString asName,cHaptic *apHaptic, int alButton) : iAction(asName)
	{
		mlButton = alButton;
		mpHaptic = apHaptic;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cActionHaptic::IsTriggerd()
	{
		if(mlButton >= mpHaptic->GetLowLevel()->GetNumberOfButtons()) return false;

		return mpHaptic->GetLowLevel()->ButtonIsPressed(mlButton);
	}

	//-----------------------------------------------------------------------

	float cActionHaptic::GetValue()
	{
		if(IsTriggerd())return 1.0;
		else return 0.0;
	}

	//-----------------------------------------------------------------------

	tString cActionHaptic::GetInputName()
	{
		switch(mlButton)
		{
		case 0: return "centre Controller button";
		case 1: return "left Controller button";
		case 2: return "forward Controller button";
		case 3: return "right Controller button";
		}
		return "unknown Controller button";
	}

	//-----------------------------------------------------------------------
}
