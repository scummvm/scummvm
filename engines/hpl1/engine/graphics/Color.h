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

#ifndef HPL_COLOR_H
#define HPL_COLOR_H

#include "common/array.h"
#include "common/list.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cColor {
public:
	float r, g, b, a;

	cColor(float afR, float afG, float afB, float afA);
	cColor(float afR, float afG, float afB);
	cColor();
	cColor(float afVal);
	cColor(float afVal, float afA);

	cColor operator*(float afVal) const;
	cColor operator/(float afVal) const;

	cColor operator+(const cColor &aCol) const;
	cColor operator-(const cColor &aCol) const;
	cColor operator*(const cColor &aCol) const;
	cColor operator/(const cColor &aCol) const;

	bool operator==(cColor aCol) const;

	tString ToString() const;

	tString ToFileString() const;

	void FromVec(float *apV);
};

typedef Common::List<cColor> tColorList;
typedef tColorList::iterator tColorListIt;

typedef Common::Array<cColor> tColorVec;
typedef tColorVec::iterator tColorVecIt;

} // namespace hpl

#endif // HPL_COLOR_H
