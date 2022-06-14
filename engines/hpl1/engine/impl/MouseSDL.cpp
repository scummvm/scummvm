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

#include "hpl1/engine/impl/MouseSDL.h"

//#include "SDL/SDL.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/impl/LowLevelInputSDL.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMouseSDL::cMouseSDL(cLowLevelInputSDL *apLowLevelInputSDL, iLowLevelGraphics *apLowLevelGraphics) : iMouse("SDL Portable Mouse") {
	mfMaxPercent = 0.7f;
	mfMinPercent = 0.1f;
	mlBufferSize = 6;

	mvMButtonArray.resize(eMButton_LastEnum);
	mvMButtonArray.assign(mvMButtonArray.size(), false);

	mpLowLevelInputSDL = apLowLevelInputSDL;
	mpLowLevelGraphics = apLowLevelGraphics;

	mvMouseRelPos = cVector2f(0, 0);
	mvMouseAbsPos = cVector2f(0, 0);

	mbWheelUpMoved = false;
	mbWheelDownMoved = false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cMouseSDL::Update() {
#if 0
  cVector2f vScreenSize = mpLowLevelGraphics->GetScreenSize();
		cVector2f vVirtualSize = mpLowLevelGraphics->GetVirtualSize();

		//mvMouseRelPos = cVector2f(0,0);

		//Log("Input start\n");
		mbWheelUpMoved = false;
		mbWheelDownMoved = false;

		std::list<SDL_Event>::iterator it = mpLowLevelInputSDL->mlstEvents.begin();
		for(; it != mpLowLevelInputSDL->mlstEvents.end(); ++it)
		{
			SDL_Event *pEvent = &(*it);

			if(	pEvent->type != SDL_MOUSEMOTION &&
				pEvent->type != SDL_MOUSEBUTTONDOWN &&
				pEvent->type != SDL_MOUSEBUTTONUP)
			{
				continue;
			}

			if(pEvent->type == SDL_MOUSEMOTION)
			{
				mvMouseAbsPos = cVector2f((float)pEvent->motion.x,(float)pEvent->motion.y);
				mvMouseAbsPos = (mvMouseAbsPos/vScreenSize)*vVirtualSize;

				Uint8 buttonState = pEvent->motion.state;

				//Set button here as well just to be sure
				/*if(buttonState & SDL_BUTTON(1)) mvMButtonArray[eMButton_Left] = true;
				if(buttonState & SDL_BUTTON(2)) mvMButtonArray[eMButton_Middle] = true;
				if(buttonState & SDL_BUTTON(3)) mvMButtonArray[eMButton_Right] = true;*/
			}
			else
			{
				bool bButtonIsDown = pEvent->type==SDL_MOUSEBUTTONDOWN;

				//if(pEvent->button.button == SDL_BUTTON_WHEELUP)Log(" Wheel %d!\n",bButtonIsDown);

				switch(pEvent->button.button)
				{
					case SDL_BUTTON_LEFT: mvMButtonArray[eMButton_Left] = bButtonIsDown;break;
					case SDL_BUTTON_MIDDLE: mvMButtonArray[eMButton_Middle] = bButtonIsDown;break;
					case SDL_BUTTON_RIGHT: mvMButtonArray[eMButton_Right] = bButtonIsDown;break;
					case SDL_BUTTON_WHEELUP:
						mvMButtonArray[eMButton_WheelUp] = bButtonIsDown;
						if(bButtonIsDown) mbWheelUpMoved = true;
						break;
					case SDL_BUTTON_WHEELDOWN:
						mvMButtonArray[eMButton_WheelDown] = bButtonIsDown;
						if(bButtonIsDown) mbWheelDownMoved = true;
						break;
				}
			}
		}

		if(mbWheelDownMoved)	mvMButtonArray[eMButton_WheelDown] = true;
		else					mvMButtonArray[eMButton_WheelDown] = false;
		if(mbWheelUpMoved)		mvMButtonArray[eMButton_WheelUp] = true;
		else					mvMButtonArray[eMButton_WheelUp] = false;

		int lX,lY;
		SDL_GetRelativeMouseState(&lX, &lY);

		mvMouseRelPos = cVector2f((float)lX,(float)lY);
		mvMouseRelPos = (mvMouseRelPos/vScreenSize)*vVirtualSize;
#endif
}

//-----------------------------------------------------------------------

bool cMouseSDL::ButtonIsDown(eMButton mButton) {
	return mvMButtonArray[mButton];
}

//-----------------------------------------------------------------------

cVector2f cMouseSDL::GetAbsPosition() {
	// Do a transform with the screen-size to the the float coordinates.
	cVector2f vPos = mvMouseAbsPos;

	return vPos;
}

//-----------------------------------------------------------------------

cVector2f cMouseSDL::GetRelPosition() {
	// Do a transform with the screen-size to the the float coordinates.
	cVector2f vPos = mvMouseRelPos;
	// Ok this is?
	mvMouseRelPos = cVector2f(0, 0);

	return vPos;
	/*cVector2f vNew;

	if((int)mlstMouseCoord.size() >= mlBufferSize)
		mlstMouseCoord.erase(mlstMouseCoord.begin());

	mlstMouseCoord.push_back(vPos);

	int lBufferSize = (int) mlstMouseCoord.size();

	cVector2f vSum(0,0);
	float fPercent = mfMinPercent;
	float fPercentAdd =  (mfMaxPercent - mfMinPercent)/((float)lBufferSize);
	float fTotalPercent=0;

	tVector2fListIt It = mlstMouseCoord.begin();
	while(It != mlstMouseCoord.end())
	{
		vSum.x +=It->x*fPercent;
		vSum.y +=It->y*fPercent;
		fTotalPercent+=fPercent;
		fPercent+=fPercentAdd;

		It++;
	}
	vNew.x = vSum.x/fTotalPercent;
	vNew.y = vSum.y/fTotalPercent;

	return vNew;*/
}

//-----------------------------------------------------------------------

void cMouseSDL::Reset() {
#if 0
  		mlstMouseCoord.clear();
		mvMouseRelPos = cVector2f(0,0);

		int lX,lY; //Just to clear the rel pos.

		SDL_PumpEvents();
		SDL_GetRelativeMouseState(&lX, &lY);
#endif
}

//-----------------------------------------------------------------------

void cMouseSDL::SetSmoothProperties(float afMinPercent,
									float afMaxPercent, unsigned int alBufferSize) {
	mfMaxPercent = afMaxPercent;
	mfMinPercent = afMinPercent;
	mlBufferSize = alBufferSize;
}

//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
/////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

} // namespace hpl
