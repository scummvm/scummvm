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

#ifndef HPL_MOUSE_SDL_H
#define HPL_MOUSE_SDL_H

#include "common/bitarray.h"
#include "hpl1/engine/input/Mouse.h"

namespace Common {
struct Event;
}

namespace hpl {

class iLowLevelGraphics;
class cLowLevelInputSDL;

class cMouseSDL : public iMouse {
public:
	cMouseSDL(cLowLevelInputSDL *apLowLevelInputSDL, iLowLevelGraphics *apLowLevelGraphics);

	bool ButtonIsDown(eMButton);

	void Update();

	/**
	 * \todo Fix so it works and handles screen size
	 * \return
	 */
	cVector2f GetAbsPosition();

	/**
	 * \todo Fix so it works and handles screen size
	 * \return
	 */
	cVector2f GetRelPosition();

	void Reset();

	/**
	 * Sets how much smoothening there will be in the RelPosition.
	 * The percentages are just ratios, so min/max 1/10 equals 0.1/1
	 * \param afMinPercent The influence of the oldest value
	 * \param afMaxPercent The influence of the newest value
	 * \param alBufferSize The number of values recorded
	 */
	void SetSmoothProperties(float afMinPercent,
							 float afMaxPercent, unsigned int alBufferSize);

private:
	void processEvent(const Common::Event &ev);
	cVector2f _absMousePos;
	cVector2f _relMousePos;
	Common::BitArray _buttonState;
	float mfMaxPercent;
	float mfMinPercent;
	int mlBufferSize;
	cLowLevelInputSDL *_lowLevelInputSDL;
	iLowLevelGraphics *_lowLevelGraphics;
};

} // namespace hpl

#endif // HPL_MOUSE_SDL_H
