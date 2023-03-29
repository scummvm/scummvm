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

#ifndef HPL_WORLD2D_H
#define HPL_WORLD2D_H

#include "hpl1/engine/game/GameTypes.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

#include "hpl1/engine/scene/SoundSource.h"
#include "hpl1/std/map.h"

class TiXmlElement;

namespace hpl {

class cGraphics;
class cResources;
class cSound;

class iCamera;
class cCamera2D;
class cTileMap;
class cTile;
class cNode2D;
class cBody2D;
class cCollider2D;
class cGridMap2D;
class cLight2DPoint;
class iLight2D;
class cImageEntity;
class cParticleManager;
class cArea2D;
class iScript;
class cMesh2D;
class iParticleSystem2D;

class cWorld2D {
	friend class cCollider2D;

public:
	cWorld2D(tString asName, cGraphics *apGraphics, cResources *apResources, cSound *apSound, cCollider2D *apCollider);
	~cWorld2D();

	tString GetName() { return msName; }

	bool CreateFromFile(tString asFile);

	void Render(cCamera2D *apCamera);

	void Update(float afTimeStep);

	cVector2f GetWorldSize() { return mvWorldSize; }

	iScript *GetScript() { return mpScript; }

	void ResetBodyIDCount() { mlBodyIDCount = 0; }

	///// LIGHT METHODS ////////////////////

	cLight2DPoint *CreateLightPoint(tString asName = "");
	void DestroyLight(iLight2D *apLight);
	cGridMap2D *GetGridMapLights();
	iLight2D *GetLight(const tString &asName);

	///// BODY METHODS ////////////////////

	cBody2D *CreateBody2D(const tString &asName, cMesh2D *apMesh, cVector2f avSize);
	cGridMap2D *GetGridMapBodies() { return mpMapBodies; }

	///// AREA METHODS ////////////////////
	cGridMap2D *GetGridMapAreas() { return mpMapAreas; }

	/**
	 * Gets an area on the map. This does a linear search and very fast.
	 * \param asName The name, if "" then the first of the correct type is returned
	 * \param asType The typem if "" then type doesn't matter.
	 * \return
	 */
	cArea2D *GetArea(const tString &asName, const tString &asType);

	///// SOUNDSOURCE METHODS ////////////////////

	cSoundSource *CreateSoundSource(const tString &asName, const tString &asSoundName, bool abVolatile);
	void DestroySoundSource(cSoundSource *apSound);

	///// ENTITY METHODS ////////////////////

	cImageEntity *CreateImageEntity(tString asName, tString asDataName);
	cGridMap2D *GetGridMapImageEntities() { return mpMapImageEntities; }
	void DestroyImageEntity(cImageEntity *apEntity);
	cImageEntity *GetImageEntity(const tString &asName);

	///// PARTICLE METHODS ////////////////////

	iParticleSystem2D *CreateParticleSystem(const tString &asName, const cVector3f &avSize);
	void DestroyParticleSystem(iParticleSystem2D *apPS);

	///// TILE METHODS ////////////////////

	cTileMap *GetTileMap() { return mpTileMap; }
	// Test!
	void CreateTileMap();

	/// NODE METHODS //////////////////////
	cNode2D *GetRootNode() { return mpRootNode; }

private:
	tString msName;
	cGraphics *mpGraphics;
	cSound *mpSound;
	cResources *mpResources;
	cCollider2D *mpCollider;

	cVector2f mvWorldSize;

	iScript *mpScript;

	cGridMap2D *mpMapLights;
	cGridMap2D *mpMapImageEntities;
	cGridMap2D *mpMapBodies;
	cGridMap2D *mpMapParticles;
	cGridMap2D *mpMapAreas;

	tSoundSourceList mlstSoundSources;

	int mlBodyIDCount;

	cTileMap *mpTileMap;
	cNode2D *mpRootNode;

	tString msMapName;
	float mfLightZ;
	cColor mAmbientColor;

	// Update
	void UpdateEntities();
	void UpdateBodies();
	void UpdateParticles();
	void UpdateSoundSources();
	void UpdateLights();

	// Render
	void RenderImagesEntities(cCamera2D *apCamera);
	void RenderParticles(cCamera2D *apCamera);

	// Load helper
	int LoadTileData(cTile *apTile, tString *asData, int alStart);
};

} // namespace hpl

#endif // HPL_WOLRD2D_H
