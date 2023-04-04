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

#include "common/bitarray.h"
#include "common/events.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/impl/MouseSDL.h"
#include "hpl1/engine/input/InputTypes.h"
#include "hpl1/engine/input/LowLevelInput.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMouseSDL::cMouseSDL(iLowLevelInput *apLowLevelInputSDL, iLowLevelGraphics *apLowLevelGraphics) : iMouse("SDL Portable Mouse") {
	mfMaxPercent = 0.7f;
	mfMinPercent = 0.1f;
	mlBufferSize = 6;

	_buttonState.set_size(eMButton_LastEnum);

	_lowLevelInputSDL = apLowLevelInputSDL;
	_lowLevelGraphics = apLowLevelGraphics;

	_relMousePos = cVector2f(0, 0);
	_absMousePos = cVector2f(0, 0);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

static void setMouseState(const int state, Common::BitArray &states) {
	if (state != Common::EVENT_WHEELDOWN)
		states.unset(eMButton_WheelDown);
	if (state != Common::EVENT_WHEELUP)
		states.unset(eMButton_WheelUp);

	switch (state) {
	case Common::EVENT_LBUTTONDOWN:
		return states.set(eMButton_Left);
	case Common::EVENT_LBUTTONUP:
		return states.unset(eMButton_Left);
	case Common::EVENT_RBUTTONDOWN:
		return states.set(eMButton_Right);
	case Common::EVENT_RBUTTONUP:
		return states.unset(eMButton_Right);
	case Common::EVENT_MBUTTONDOWN:
		return states.set(eMButton_Middle);
	case Common::EVENT_MBUTTONUP:
		return states.unset(eMButton_Middle);
	case Common::EVENT_WHEELUP:
		return states.set(eMButton_WheelUp);
	case Common::EVENT_WHEELDOWN:
		return states.set(eMButton_WheelDown);
	}
}

void cMouseSDL::processEvent(const Common::Event &ev) {
	if (!Common::isMouseEvent(ev))
		return;
	// const cVector2f screenSize = _lowLevelGraphics->GetScreenSize();
	// const cVector2f virtualSize = _lowLevelGraphics->GetVirtualSize();
	if (ev.type == Common::EVENT_MOUSEMOVE) {
		_absMousePos = cVector2f(ev.mouse.x, ev.mouse.y);
		// mvMouseAbsPos = (mvMouseAbsPos / screenSize) * virtualSize;
	} else {
		setMouseState(ev.type, _buttonState);
	}
	_relMousePos = cVector2f(ev.relMouse.x, ev.relMouse.y);
	// mvMouseRelPos = (mvMouseRelPos / screenSize) * virtualSize;
}

void cMouseSDL::Update() {
	for (const Common::Event &ev : _lowLevelInputSDL->_events)
		processEvent(ev);
}

//-----------------------------------------------------------------------

bool cMouseSDL::ButtonIsDown(eMButton mButton) {
	return _buttonState.get(mButton);
}

//-----------------------------------------------------------------------

cVector2f cMouseSDL::GetAbsPosition() {
	// Do a transform with the screen-size to the the float coordinates.
	cVector2f vPos = _absMousePos;

	return vPos;
}

//-----------------------------------------------------------------------

cVector2f cMouseSDL::GetRelPosition() {
	// Do a transform with the screen-size to the the float coordinates.
	cVector2f vPos = _relMousePos;
	// Ok this is?
	_relMousePos = cVector2f(0, 0);

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
	error("call to unimplemented function Mouse::Reset");
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
