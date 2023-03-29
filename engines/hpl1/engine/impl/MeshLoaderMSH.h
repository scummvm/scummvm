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

#ifndef HPL_MESH_LOADER_MSH_H
#define HPL_MESH_LOADER_MSH_H

#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/resources/MeshLoader.h"

class TiXmlElement;

namespace hpl {

class cMesh;
class cNode3D;
class iVertexBuffer;

class cMeshLoaderMSH : public iMeshLoader {
public:
	cMeshLoaderMSH(iLowLevelGraphics *apLowLevelGraphics);
	~cMeshLoaderMSH();

	cMesh *LoadMesh(const tString &asFile, tMeshLoadFlag aFlags);
	bool SaveMesh(cMesh *apMesh, const tString &asFile);

	cWorld3D *LoadWorld(const tString &asFile, cScene *apScene, tWorldLoadFlag aFlags);

	cAnimation *LoadAnimation(const tString &asFile) { return NULL; }

	bool IsSupported(const tString asFileType);

	void AddSupportedTypes(tStringVec *avFileTypes);

private:
	// Saving
	void SaveFloatData(TiXmlElement *apRoot, int alSize, const char *asName, float *apData);
	void SaveIntData(TiXmlElement *apRoot, int alSize, unsigned int *apData);

	// Loading
	void FillVtxArray(float *apArray, const char *apString, int alSize);
	void FillIdxArray(unsigned int *apArray, const char *apString, int alSize);

	// Common
	const char *GetVertexName(tVertexFlag aFlag);
};

} // namespace hpl

#endif // HPL_MESH_LOADER_MSH_H
