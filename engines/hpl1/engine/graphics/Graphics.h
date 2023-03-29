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

#ifndef HPL_GRAPHICS_H
#define HPL_GRAPHICS_H

#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cResources;
class cRenderer2D;
class cRenderer3D;
class cRendererPostEffects;
class cRenderList;
class cGraphicsDrawer;
class LowLevelResources;
class iLowLevelGraphics;
class cMeshCreator;
class cMaterialHandler;

class cGraphics {
public:
	cGraphics(iLowLevelGraphics *apLowLevelGraphics, LowLevelResources *apLowLevelResources);
	~cGraphics();

	bool Init(int alWidth, int alHeight, int alBpp, int abFullscreen, int alMultisampling,
			  const tString &asWindowCaption, cResources *apResources);

	/**
	 * Get low level graphics routines
	 * \return pointer to the low level class
	 */
	iLowLevelGraphics *GetLowLevel();

	/**
	 * Get the drawer
	 * \return
	 */
	cGraphicsDrawer *GetDrawer();

	cRenderer2D *GetRenderer2D();
	cRenderer3D *GetRenderer3D() { return mpRenderer3D; }
	cRendererPostEffects *GetRendererPostEffects() { return mpRendererPostEffects; }
	cMeshCreator *GetMeshCreator() { return mpMeshCreator; }
	cMaterialHandler *GetMaterialHandler() { return mpMaterialHandler; }

private:
	iLowLevelGraphics *mpLowLevelGraphics;
	LowLevelResources *mpLowLevelResources;
	cGraphicsDrawer *mpDrawer;
	cMeshCreator *mpMeshCreator;
	cMaterialHandler *mpMaterialHandler;
	cRenderer2D *mpRenderer2D;
	cRenderer3D *mpRenderer3D;
	cRendererPostEffects *mpRendererPostEffects;
	cRenderList *mpRenderList;
};

} // namespace hpl

#endif // HPL_GRAPHICS_H
