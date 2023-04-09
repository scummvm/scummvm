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
#include "hpl1/engine/input/InputTypes.h"
#include "hpl1/engine/input/LowLevelInput.h"
#include "hpl1/engine/input/Mouse.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

Mouse::Mouse(LowLevelInput *apLowLevelInputSDL, iLowLevelGraphics *apLowLevelGraphics) : iInputDevice("Mouse", eInputDeviceType_Mouse) {
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

void Mouse::processEvent(const Common::Event &ev) {
	if (!Common::isMouseEvent(ev))
		return;
	if (ev.type == Common::EVENT_MOUSEMOVE) {
		_absMousePos = cVector2f(ev.mouse.x, ev.mouse.y);
	} else {
		setMouseState(ev.type, _buttonState);
	}
	_relMousePos = cVector2f(ev.relMouse.x, ev.relMouse.y);
}

void Mouse::Update() {
	for (const Common::Event &ev : _lowLevelInputSDL->_events)
		processEvent(ev);
}

//-----------------------------------------------------------------------

bool Mouse::ButtonIsDown(eMButton mButton) {
	return _buttonState.get(mButton);
}

//-----------------------------------------------------------------------

cVector2f Mouse::GetAbsPosition() {
	cVector2f vPos = _absMousePos;

	return vPos;
}

//-----------------------------------------------------------------------

cVector2f Mouse::GetRelPosition() {
	cVector2f vPos = _relMousePos;
	_relMousePos = cVector2f(0, 0);
	return vPos;
}

//-----------------------------------------------------------------------

void Mouse::Reset() {
	error("call to unimplemented function Mouse::Reset");
}

//-----------------------------------------------------------------------

void Mouse::SetSmoothProperties(float afMinPercent,
								float afMaxPercent, unsigned int alBufferSize) {
	mfMaxPercent = afMaxPercent;
	mfMinPercent = afMinPercent;
	mlBufferSize = alBufferSize;
}

//-----------------------------------------------------------------------

} // namespace hpl
