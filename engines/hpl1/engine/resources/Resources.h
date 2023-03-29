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

#ifndef HPL_RESOURCES_H
#define HPL_RESOURCES_H

#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/std/map.h"

#include "hpl1/engine/graphics/GPUProgram.h"

#include "hpl1/engine/game/Updateable.h"

class TiXmlElement;

namespace hpl {

class cImageEntity;

class LowLevelResources;
class iLowLevelGraphics;
class LowLevelSystem;
class cSystem;
class iResourceManager;
class cFileSearcher;
class cImageManager;
class cGpuProgramManager;
class cTileSetManager;
class cImageEntityManager;
class cParticleManager;
class cSoundManager;
class cFontManager;
class cScriptManager;
class cTextureManager;
class cMaterialManager;
class cSoundEntityManager;
class cAnimationManager;
class cMeshManager;
class cVideoManager;
class cConfigFile;
class cArea2D;
class cSound;
class cMeshLoaderHandler;
class cScene;
class cGraphics;
class iRenderable;
class cWorld3D;
class iEntity3D;
class cLanguageFile;
class cGui;

//-------------------------------------------------------

class iEntity2DLoader {
public:
	iEntity2DLoader(const tString &asName) : msName(asName) {}
	virtual ~iEntity2DLoader() {}

	const tString &GetName() { return msName; }

	virtual void Load(cImageEntity *apImageEntity) = 0;

protected:
	tString msName;
};

typedef Hpl1::Std::map<tString, iEntity2DLoader *> tEntity2DLoaderMap;
typedef tEntity2DLoaderMap::iterator tEntity2DLoaderMapIt;

//-------------------------------------------------------

class iArea2DLoader {
public:
	iArea2DLoader(const tString &asName) : msName(asName) {}
	virtual ~iArea2DLoader() {}

	const tString &GetName() { return msName; }

	virtual void Load(cArea2D *apArea) = 0;

protected:
	tString msName;
};

typedef Hpl1::Std::map<tString, iArea2DLoader *> tArea2DLoaderMap;
typedef tArea2DLoaderMap::iterator tArea2DLoaderMapIt;

//-------------------------------------------------------

class iEntity3DLoader {
public:
	iEntity3DLoader(const tString &asName) : msName(asName) {}
	virtual ~iEntity3DLoader() {}

	const tString &GetName() { return msName; }

	virtual iEntity3D *Load(const tString &asName, TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform,
							cWorld3D *apWorld, const tString &asFileName, bool abLoadReferences) = 0;

protected:
	tString msName;
};

typedef Hpl1::Std::map<tString, iEntity3DLoader *> tEntity3DLoaderMap;
typedef tEntity3DLoaderMap::iterator tEntity3DLoaderMapIt;

//-------------------------------------------------------

class iArea3DLoader {
public:
	iArea3DLoader(const tString &asName) : msName(asName) {}
	virtual ~iArea3DLoader() {}

	const tString &GetName() { return msName; }

	virtual iEntity3D *Load(const tString &asName, const cVector3f &avSize, const cMatrixf &a_mtxTransform, cWorld3D *apWorld) = 0;

protected:
	tString msName;
};

typedef Hpl1::Std::map<tString, iArea3DLoader *> tArea3DLoaderMap;
typedef tArea3DLoaderMap::iterator tArea3DLoaderMapIt;

//-------------------------------------------------------

// Should this be made a map so you can do
// GetManager(tString) when getting a manager?
// This way you would be able to add your own resource types
// easily.
typedef Common::List<iResourceManager *> tResourceManagerList;
typedef tResourceManagerList::iterator tResourceManagerListIt;

//-------------------------------------------------------

class cResources : public iUpdateable {
public:
	cResources(LowLevelResources *apLowLevelResources, iLowLevelGraphics *apLowLevelGraphics);
	virtual ~cResources();

	void Init(cGraphics *apGraphics, cSystem *apSystem, cSound *apSound, cScene *apScene, cGui *apGui);

	void Update(float afTimeStep);

	LowLevelResources *GetLowLevel();
	cFileSearcher *GetFileSearcher();

	bool AddResourceDir(const tString &asDir, const tString &asMask = "*.*");
	void ClearResourceDirs();

	bool SetLanguageFile(const tString &asFile);
	const tWString &Translate(const tString &asCat, const tString &asName);

	void AddEntity2DLoader(iEntity2DLoader *apLoader);
	iEntity2DLoader *GetEntity2DLoader(const tString &asName);

	void AddEntity3DLoader(iEntity3DLoader *apLoader, bool abSetAsDefault = false);
	iEntity3DLoader *GetEntity3DLoader(const tString &asName);

	void AddArea3DLoader(iArea3DLoader *apLoader, bool abSetAsDefault = false);
	iArea3DLoader *GetArea3DLoader(const tString &asName);

	void AddArea2DLoader(iArea2DLoader *apLoader);
	iArea2DLoader *GetArea2DLoader(const tString &asName);

	bool LoadResourceDirsFile(const tString &asFile);

	cImageManager *GetImageManager() { return mpImageManager; }
	cGpuProgramManager *GetGpuProgramManager() { return mpGpuProgramManager; }
	cTileSetManager *GetTileSetManager() { return mpTileSetManager; }
	cImageEntityManager *GetImageEntityManager() { return mpImageEntityManager; }
	cParticleManager *GetParticleManager() { return mpParticleManager; }
	cSoundManager *GetSoundManager() { return mpSoundManager; }
	cFontManager *GetFontManager() { return mpFontManager; }
	cScriptManager *GetScriptManager() { return mpScriptManager; }
	cTextureManager *GetTextureManager() { return mpTextureManager; }
	cMaterialManager *GetMaterialManager() { return mpMaterialManager; }
	cMeshManager *GetMeshManager() { return mpMeshManager; }
	cMeshLoaderHandler *GetMeshLoaderHandler() { return mpMeshLoaderHandler; }
	cSoundEntityManager *GetSoundEntityManager() { return mpSoundEntityManager; }
	cAnimationManager *GetAnimationManager() { return mpAnimationManager; }
	cVideoManager *GetVideoManager() { return mpVideoManager; }

	LowLevelSystem *GetLowLevelSystem() { return mpLowLevelSystem; }

private:
	LowLevelResources *mpLowLevelResources;
	iLowLevelGraphics *mpLowLevelGraphics;
	LowLevelSystem *mpLowLevelSystem;
	cFileSearcher *mpFileSearcher;

	tResourceManagerList mlstManagers;
	cImageManager *mpImageManager;
	cGpuProgramManager *mpGpuProgramManager;
	cTileSetManager *mpTileSetManager;
	cImageEntityManager *mpImageEntityManager;
	cParticleManager *mpParticleManager;
	cSoundManager *mpSoundManager;
	cFontManager *mpFontManager;
	cScriptManager *mpScriptManager;
	cTextureManager *mpTextureManager;
	cMaterialManager *mpMaterialManager;
	cSoundEntityManager *mpSoundEntityManager;
	cAnimationManager *mpAnimationManager;
	cVideoManager *mpVideoManager;

	cLanguageFile *mpLanguageFile;

	cMeshManager *mpMeshManager;
	cMeshLoaderHandler *mpMeshLoaderHandler;

	tEntity2DLoaderMap m_mMapEntity2DLoaders;
	tArea2DLoaderMap m_mMapArea2DLoaders;

	tEntity3DLoaderMap m_mEntity3DLoaders;
	iEntity3DLoader *mpDefaultEntity3DLoader;

	tArea3DLoaderMap m_mArea3DLoaders;
	iArea3DLoader *mpDefaultArea3DLoader;

	tWString mwsEmptyString;
};

} // namespace hpl

#endif // HPL_RESOURCES_H
