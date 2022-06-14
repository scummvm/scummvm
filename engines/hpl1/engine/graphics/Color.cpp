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

#include "hpl1/engine/graphics/Color.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cColor::cColor(float afR, float afG, float afB, float afA)
	{
		r = afR; g = afG; b = afB; a = afA;
	}

	//-----------------------------------------------------------------------

	cColor::cColor(float afR, float afG, float afB)
	{
		r = afR; g = afG; b = afB; a = 1;
	}

	//-----------------------------------------------------------------------

	cColor::cColor()
	{
		r = 0; g = 0; b = 0; a = 1;
	}

	//-----------------------------------------------------------------------

	cColor::cColor(float afVal)
	{
		r = afVal; g = afVal; b = afVal; a = 1;
	}

	//-----------------------------------------------------------------------

	cColor::cColor(float afVal, float afA)
	{
		r = afVal; g = afVal; b = afVal; a = afA;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cColor::FromVec(float *apV)
	{
		r = apV[0];
		g = apV[1];
		b = apV[2];
		a = apV[3];
	}

	//-----------------------------------------------------------------------

	cColor cColor::operator*(float afVal) const
	{
		cColor col;
		col.r = r * afVal;
		col.g = g * afVal;
		col.b = b * afVal;
		col.a = a * afVal;
		return col;
	}

	cColor cColor::operator/(float afVal) const
	{
		cColor col;
		col.r = r / afVal;
		col.g = g / afVal;
		col.b = b / afVal;
		col.a = a / afVal;
		return col;
	}

	//-----------------------------------------------------------------------


	cColor cColor::operator+(const cColor &aCol) const
	{
		return cColor(
			r + aCol.r,
			g + aCol.g,
			b + aCol.b,
			a + aCol.a
			);
	}

	cColor cColor::operator-(const cColor &aCol) const
	{
		return cColor(
			r - aCol.r,
			g - aCol.g,
			b - aCol.b,
			a - aCol.a
			);
	}

	cColor cColor::operator*(const cColor &aCol) const
	{
		return cColor(
			r * aCol.r,
			g * aCol.g,
			b * aCol.b,
			a * aCol.a
			);
	}

	cColor cColor::operator/(const cColor &aCol) const
	{
		return cColor(
			r / aCol.r,
			g / aCol.g,
			b / aCol.b,
			a / aCol.a
			);
	}

	//-----------------------------------------------------------------------

	bool cColor::operator==(cColor aCol) const
	{
		if(r == aCol.r && g == aCol.g && b == aCol.b && a == aCol.a)
			return true;
		else
			return false;
	}

	//-----------------------------------------------------------------------

	tString cColor::ToString() const
	{
		char buf[512];
		sprintf(buf,"%f : %f : %f : %f",r,g,b,a);
		tString str = buf;
		return str;
	}


	//-----------------------------------------------------------------------

	tString cColor::ToFileString() const
	{
		char buf[512];
		sprintf(buf,"%g %g %g %g",r,g,b,a);
		tString str = buf;
		return str;
	}


	//-----------------------------------------------------------------------
}
