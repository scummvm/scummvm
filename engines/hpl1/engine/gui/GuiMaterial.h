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

#ifndef HPL_GUI_MATERIAL_H
#define HPL_GUI_MATERIAL_H

#include "hpl1/engine/system/SystemTypes.h"
namespace hpl {

class iLowLevelGraphics;

class iGuiMaterial {
public:
	iGuiMaterial(const tString &asName, iLowLevelGraphics *apLowLevelGraphics) : msName(asName), mpLowLevelGraphics(apLowLevelGraphics) {}
	virtual ~iGuiMaterial() {}

	virtual void BeforeRender() = 0;
	virtual void AfterRender() = 0;

	const tString &GetName() const { return msName; }

protected:
	tString msName;
	iLowLevelGraphics *mpLowLevelGraphics;
};

} // namespace hpl

#endif // HPL_GUI_MATERIAL_H
