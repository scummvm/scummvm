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

#ifndef HPL_LIGHT2D_H
#define HPL_LIGHT2D_H

#include "common/list.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/scene/Entity2D.h"
#include "hpl1/engine/scene/Light.h"

namespace hpl {

class iLowLevelGraphics;

class iLight2D : public iEntity2D, public iLight {
public:
	iLight2D(tString asName);
	virtual ~iLight2D() {}

	tString GetEntityType() { return "iLight2D"; }

	void UpdateLogic(float afTimeStep);

	void SetFarAttenuation(float afX);
	void SetNearAttenuation(float afX);

	cVector3f GetLightPosition();

	virtual int Render(iLowLevelGraphics *apLowLevel, int alFirstIndex) = 0;

protected:
};

typedef Common::List<iLight2D *> tLightList;
typedef tLightList::iterator tLightListIt;

} // namespace hpl

#endif // HPL_LIGHT2D_H
