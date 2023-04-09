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

#ifndef HPL_MOUSE_H
#define HPL_MOUSE_H

#include "common/bitarray.h"
#include "hpl1/engine/input/InputDevice.h"
#include "hpl1/engine/input/InputTypes.h"
#include "hpl1/engine/math/MathTypes.h"

namespace Common {
struct Event;
}

namespace hpl {

class LowLevelInput;
class iLowLevelGraphics;

class iMouse : public iInputDevice {
public:
	iMouse(LowLevelInput *apLowLevelInputSDL, iLowLevelGraphics *apLowLevelGraphics);
	~iMouse() {}

	/**
	 * Check if a mouse button is down
	 * \param eMButton the button to check
	 * \return
	 */
	bool ButtonIsDown(eMButton);
	/**
	 * Get the absolute pos of the mouse.
	 * \return
	 */
	cVector2f GetAbsPosition();
	/**
	 * Get the relative movement.
	 * \return
	 */
	cVector2f GetRelPosition();

	/**
	 * Reset smoothing and relative movement.
	 */
	void Reset();
	/**
	 * Set parameters for mouse smoothing
	 * \param afMinPercent Influence of the oldest position.
	 * \param afMaxPercent Influence of the latest position.
	 * \param alBufferSize number of saved positions, 1 = no smoothing
	 */
	void SetSmoothProperties(float afMinPercent,
							 float afMaxPercent, unsigned int alBufferSize);

	void Update();

private:
	void processEvent(const Common::Event &ev);
	cVector2f _absMousePos;
	cVector2f _relMousePos;
	Common::BitArray _buttonState;
	float mfMaxPercent;
	float mfMinPercent;
	int mlBufferSize;
	LowLevelInput *_lowLevelInputSDL;
	iLowLevelGraphics *_lowLevelGraphics;
};

} // namespace hpl

#endif // HPL_MOUSE_H
