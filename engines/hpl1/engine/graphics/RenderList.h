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

#ifndef HPL_RENDER_LIST_H
#define HPL_RENDER_LIST_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/OcclusionQuery.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/scene/Light3D.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/std/multiset.h"
#include "hpl1/std/set.h"

namespace hpl {

class iRenderable;
class cCamera3D;
class cGraphics;

class iRenderState;
class cRenderSettings;

//-------------------------------------------------------------

enum eRenderListDrawType {
	eRenderListDrawType_Normal,
	eRenderListDrawType_Trans,
	eRenderListDrawType_LastEnum
};

//-------------------------------------------------------------

class cRenderNode;

class cRenderNodeCompare {
public:
	bool operator()(cRenderNode *apNodeA, cRenderNode *apNodeB) const;
};

typedef Hpl1::Std::set<cRenderNode *, cRenderNodeCompare> tRenderNodeSet;
typedef tRenderNodeSet::iterator tRenderNodeSetIt;

class cRenderNode {
public:
	cRenderNode() : mpState(NULL) {}

	iRenderState *mpState;

	tRenderNodeSet m_setNodes;

	void DeleteChildren();
	void Render(cRenderSettings *apSettings);
};

//-------------------------------------------------------------

class cMotionBlurObject_Compare {
public:
	bool operator()(iRenderable *pObjectA, iRenderable *pObjectB) const;
};

typedef Hpl1::Std::multiset<iRenderable *, cMotionBlurObject_Compare> tMotionBlurObjectSet;
typedef tMotionBlurObjectSet::iterator tMotionBlurObjectSetIt;

typedef cSTLIterator<iRenderable *, tMotionBlurObjectSet,
					 tMotionBlurObjectSetIt>
	cMotionBlurObjectIterator;

//-------------------------------------------------------------

class cTransperantObject_Compare {
public:
	bool operator()(iRenderable *pObjectA, iRenderable *pObjectB) const;
};

typedef Hpl1::Std::multiset<iRenderable *, cTransperantObject_Compare> tTransperantObjectSet;
typedef tTransperantObjectSet::iterator tTransperantObjectSetIt;

typedef cSTLIterator<iRenderable *, tTransperantObjectSet, tTransperantObjectSetIt> cTransperantObjectIterator;

//-------------------------------------------------------------

class cOcclusionQueryObject {
public:
	cOcclusionQueryObject() : mpQuery(NULL), mpVtxBuffer(NULL), mpMatrix(NULL) {}

	iOcclusionQuery *mpQuery;
	iVertexBuffer *mpVtxBuffer;
	cMatrixf *mpMatrix;
	bool mbDepthTest;
};

class cOcclusionQueryObject_Compare {
public:
	bool operator()(const cOcclusionQueryObject *pObjectA, const cOcclusionQueryObject *pObjectB) const;
};

typedef Hpl1::Std::set<cOcclusionQueryObject *, cOcclusionQueryObject_Compare> tOcclusionQueryObjectSet;
typedef tOcclusionQueryObjectSet::iterator tOcclusionQueryObjectSetIt;

typedef cSTLIterator<cOcclusionQueryObject *, tOcclusionQueryObjectSet,
					 tOcclusionQueryObjectSetIt>
	cOcclusionQueryObjectIterator;

//-------------------------------------------------------------

typedef Hpl1::Std::set<iRenderable *> tRenderableSet;
typedef tRenderableSet::iterator tRenderableSetIt;

typedef Hpl1::Std::set<iLight3D *> tLight3DSet;
typedef tLight3DSet::iterator tLight3DSetIt;

typedef cSTLIterator<iLight3D *, tLight3DSet, tLight3DSetIt> cLight3DIterator;
typedef cSTLIterator<iRenderable *, tRenderableSet, tRenderableSetIt> cRenderableIterator;

//-------------------------------------------------------------

class cRenderList {
public:
	cRenderList(cGraphics *apGraphics);
	~cRenderList();

	void SetCamera(cCamera3D *apCamera) { mpCamera = apCamera; }
	cCamera3D *GetCamera() { return mpCamera; }

	bool Add(iRenderable *apObject);
	void Clear();

	void AddOcclusionQuery(cOcclusionQueryObject *apObject);

	cOcclusionQueryObjectIterator GetQueryIterator();

	cMotionBlurObjectIterator GetMotionBlurIterator();

	cTransperantObjectIterator GetTransperantIterator();

	cLight3DIterator GetLightIt();
	cRenderableIterator GetObjectIt();

	int GetLightNum();
	int GetObjectNum();

	void Compile();

	int GetRenderCount() { return mlRenderCount; }
	int GetLastRenderCount() { return mlLastRenderCount; }

	int GetLightObjects(int alLightIdx) { return mvObjectsPerLight[alLightIdx]; }

	void SetFrameTime(float afTime) { mfFrameTime = afTime; }

	cRenderNode *GetRootNode(eRenderListDrawType aObjectType, eMaterialRenderType aPassType, int alLightNum);

	static inline int GetGlobalRenderCount() { return mlGlobalRenderCount; }

private:
	cRenderNode *InsertNode(cRenderNode *apListNode, cRenderNode *apTempNode);

	void AddToTree(iRenderable *apObject, eRenderListDrawType aObjectType,
				   eMaterialRenderType mPassType, int alLightNum, iLight3D *apLight,
				   bool abUseDepth, int alPass);

	static int mlGlobalRenderCount;

	tLight3DSet m_setLights;
	int mvObjectsPerLight[MAX_NUM_OF_LIGHTS];

	tRenderableSet m_setObjects;

	tOcclusionQueryObjectSet m_setQueries;

	tMotionBlurObjectSet m_setMotionBlurObjects;
	tTransperantObjectSet m_setTransperantObjects;

	cRenderNode mRootNodeDepth;
	cRenderNode mRootNodeDiffuse;
	cRenderNode mRootNodeLight[MAX_NUM_OF_LIGHTS];
	cRenderNode mRootNodeTrans;

	cRenderNode mTempNode;

	int mlRenderCount;
	int mlLastRenderCount;

	float mfFrameTime;

	cMemoryPool<iRenderState> *m_poolRenderState;
	cMemoryPool<cRenderNode> *m_poolRenderNode;

	cCamera3D *mpCamera;

	cGraphics *mpGraphics;
};

} // namespace hpl

#endif // HPL_RENDER_LIST_H
