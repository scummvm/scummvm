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

#ifndef HPL_SUB_MESH_ENTITY_H
#define HPL_SUB_MESH_ENTITY_H

#include "common/array.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Renderable.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/math/MeshTypes.h"
#include "hpl1/engine/scene/Entity3D.h"
#include "hpl1/engine/scene/SectorVisibility.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/std/map.h"
#include "hpl1/std/multimap.h"

namespace hpl {

class cMaterialManager;
class cMeshManager;
class cMesh;
class cSubMesh;
class cMeshEntity;
class cAnimationState;
class cNodeState;
class cBone;
class cNode3D;
class iPhysicsBody;
class iMaterial;
class cBoneState;

//-----------------------------------------------------------------------

typedef Common::Array<cAnimationState *> tAnimationStateVec;
typedef tAnimationStateVec::iterator tAnimationStateVecIt;

typedef Hpl1::Std::map<tString, int> tAnimationStateIndexMap;
typedef tAnimationStateIndexMap::iterator tAnimationStateIndexMapIt;

typedef Common::Array<cBoneState *> tNodeStateVec;
typedef tNodeStateVec::iterator tNodeStateVecIt;

typedef Hpl1::Std::map<tString, int> tNodeStateIndexMap;
typedef tNodeStateIndexMap::iterator tNodeStateIndexMapIt;

//-----------------------------------------------------------------------

class cSubMeshEntityBodyUpdate : public iEntityCallback {
public:
	void OnTransformUpdate(iEntity3D *apEntity);
};

//-----------------------------------------------------------------------

kSaveData_ChildClass(iRenderable, cSubMeshEntity) {
	kSaveData_ClassInit(cSubMeshEntity) public : tString msMaterial;

	bool mbCastShadows;

	int mlBodyId;

	bool mbUpdateBody;

	iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame) { return NULL; }
	int GetSaveCreatePrio() { return 0; }
};

//-----------------------------------------------------------------------

class cSubMeshEntity : public iRenderable {
	typedef iRenderable super;
	friend class cMeshEntity;

public:
	cSubMeshEntity(const tString &asName, cMeshEntity *apMeshEntity, cSubMesh *apSubMesh, cMaterialManager *apMaterialManager);
	virtual ~cSubMeshEntity();

	iMaterial *GetMaterial();

	void UpdateGraphics(cCamera3D *apCamera, float afFrameTime, cRenderList *apRenderList);

	iVertexBuffer *GetVertexBuffer();

	void SetCastsShadows(bool abX) { mbCastShadows = abX; }
	bool IsShadowCaster() { return mbCastShadows; }

	cBoundingVolume *GetBoundingVolume();

	cMatrixf *GetModelMatrix(cCamera3D *apCamera);

	int GetMatrixUpdateCount();

	eRenderableType GetRenderType() { return eRenderableType_Normal; }

	cSubMesh *GetSubMesh() const { return mpSubMesh; }

	void SetLocalNode(cNode3D *apNode);
	cNode3D *GetLocalNode();

	tRenderContainerDataList *GetRenderContainerDataList();

	// Entity implementation
	tString GetEntityType() { return "SubMesh"; }
	bool IsVisible() { return IsRendered(); }
	void SetVisible(bool abVisible) { SetRendered(abVisible); }

	void UpdateLogic(float afTimeStep);

	cTriangleData &GetTriangle(int alIndex);
	int GetTriangleNum();
	tTriangleDataVec *GetTriangleVecPtr();

	iPhysicsBody *GetBody() { return mpBody; }
	void SetBody(iPhysicsBody *apBody) { mpBody = apBody; }

	void SetUpdateBody(bool abX);
	bool GetUpdateBody();

	void SetCustomMaterial(iMaterial *apMaterial, bool abDestroyOldCustom = true);
	iMaterial *GetCustomMaterial() { return mpMaterial; }

	cSector *GetCurrentSector() const;

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

private:
	bool mbCastShadows;
	cSubMesh *mpSubMesh;
	cMeshEntity *mpMeshEntity;

	iMaterial *mpMaterial;

	cNode3D *mpLocalNode;

	cMaterialManager *mpMaterialManager;

	iVertexBuffer *mpDynVtxBuffer;
	tTriangleDataVec mvDynTriangles;

	cSubMeshEntityBodyUpdate *mpEntityCallback;
	bool mbUpdateBody;

	bool mbGraphicsUpdated;

	iPhysicsBody *mpBody;
};

typedef Common::Array<cSubMeshEntity *> tSubMeshEntityVec;
typedef Common::Array<cSubMeshEntity *>::iterator tSubMeshEntityVecIt;

typedef Hpl1::Std::multimap<tString, cSubMeshEntity *> tSubMeshEntityMap;
typedef tSubMeshEntityMap::iterator tSubMeshEntityMapIt;

} // namespace hpl

#endif // HPL_SUB_MESH_ENTITY_H
