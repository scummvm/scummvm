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
 * This file is part of Penumbra Overture.
 */

#ifndef GAME_GRAPHICS_HELPER_H
#define GAME_GRAPHICS_HELPER_H

#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameTypes.h"

using namespace hpl;

class cInit;

//---------------------------
class cGraphicsHelper {
public:
	cGraphicsHelper(cInit *apInit);
	~cGraphicsHelper();

	void ClearScreen(const cColor &aColor);
	void DrawTexture(iTexture *apTex, const cVector3f &avPos, const cVector3f &avSize, const cColor &aColor);

	void DrawLoadingScreen(const tString &asFile);

	void SwapBuffers();

private:
	tVertexVec mvVtx;

	cInit *mpInit;
	iLowLevelGraphics *mpLowLevelGfx;
	cTextureManager *mpTexManager;

	cGraphicsDrawer *mpDrawer;

	iFontData *mpFont;
};

//---------------------------------------------

#endif // GAME_GRAPHICS_HELPER_H
