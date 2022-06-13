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
#include "hpl1/engine/haptic/Haptic.h"
#include "hpl1/engine/system/LowLevelSystem.h"
#include "hpl1/engine/haptic/LowLevelHaptic.h"


namespace hpl {

	bool cHaptic::mbIsUsed = false;

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cHaptic::cHaptic(iLowLevelHaptic *apLowLevelHaptic) : iUpdateable("HPL_Haptic")
	{
		mpLowLevelHaptic = apLowLevelHaptic;
	}

	//-----------------------------------------------------------------------

	cHaptic::~cHaptic()
	{
		Log("Exiting Haptic Module\n");
		Log("--------------------------------------------------------\n");


		Log("--------------------------------------------------------\n\n");
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cHaptic::Update(float afTimeStep)
	{
		if(mbIsUsed==false) return;

		mpLowLevelHaptic->Update(afTimeStep);
	}

	//-----------------------------------------------------------------------

	void cHaptic::Init(cResources *apResources)
	{
		if(mbIsUsed==false) return;

		Log("Initializing Haptic Module\n");
		Log("--------------------------------------------------------\n");

		if(mpLowLevelHaptic->Init(apResources)==false)
		{
			SetIsUsed(false);
		}

		Log("--------------------------------------------------------\n\n");
	}

	//-----------------------------------------------------------------------

}
