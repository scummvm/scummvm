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

#ifndef HPL_MESH_LOADER_HANDLER_H
#define HPL_MESH_LOADER_HANDLER_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

#include "common/list.h"
#include "hpl1/engine/resources/MeshLoader.h"

namespace hpl {

class cMesh;
class iMeshLoader;
class cResources;
class cWorld3D;
class cScene;
class cAnimation;

typedef Common::List<iMeshLoader *> tMeshLoaderList;
typedef tMeshLoaderList::iterator tMeshLoaderListIt;

class cMeshLoaderHandler {
public:
	cMeshLoaderHandler(cResources *apResources, cScene *apScene);
	~cMeshLoaderHandler();

	cMesh *LoadMesh(const tString &asFile, tMeshLoadFlag aFlags);
	bool SaveMesh(cMesh *apMesh, const tString &asFile);

	cWorld3D *LoadWorld(const tString &asFile, tWorldLoadFlag aFlags);

	cAnimation *LoadAnimation(const tString &asFile);

	void AddLoader(iMeshLoader *apLoader);

	tStringVec *GetSupportedTypes() { return &mvSupportedTypes; }

private:
	tStringVec mvSupportedTypes;

	tMeshLoaderList mlstLoaders;

	cResources *mpResources;
	cScene *mpScene;
};

} // namespace hpl

#endif // HPL_MESH_LOADER_HANDLER_H
