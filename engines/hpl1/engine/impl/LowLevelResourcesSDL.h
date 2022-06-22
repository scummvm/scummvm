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

#ifndef HPL_LOWLEVELRESOURCES_SDL_H
#define HPL_LOWLEVELRESOURCES_SDL_H

#include "hpl1/engine/impl/LowLevelGraphicsSDL.h"
#include "hpl1/engine/resources/LowLevelResources.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cLowLevelResourcesSDL : public iLowLevelResources {
public:
	cLowLevelResourcesSDL(cLowLevelGraphicsSDL *apLowLevelGraphics);
	~cLowLevelResourcesSDL();

	void FindFilesInDir(tStringList &alstStrings, tString asDir, tString asMask);

	void GetSupportedImageFormats(tStringList &alstFormats);
	Bitmap2D *LoadBitmap2D(tString asFilePath, tString asType = "");

	void AddMeshLoaders(cMeshLoaderHandler *apHandler);
	void AddVideoLoaders(cVideoManager *apManager);

private:
	tString mvImageFormats[30];
	cLowLevelGraphicsSDL *mpLowLevelGraphics;
};
};     // namespace hpl
#endif // HPL_LOWLEVELRESOURCES_SDL_H
