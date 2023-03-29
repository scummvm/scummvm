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

#include "hpl1/engine/graphics/RenderList.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/RenderState.h"
#include "hpl1/engine/graphics/Renderable.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/graphics/RendererPostEffects.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/scene/Camera3D.h"
#include "hpl1/engine/scene/MeshEntity.h"

namespace hpl {

int cRenderList::mlGlobalRenderCount = 0;

//////////////////////////////////////////////////////////////////////////
// MOTION BLUR OBJECT COMPARE
//////////////////////////////////////////////////////////////////////////

bool cMotionBlurObject_Compare::operator()(iRenderable *pObjectA,
										   iRenderable *pObjectB) const {
	// TODO: perhaps Z here...

	return pObjectA->GetVertexBuffer() < pObjectB->GetVertexBuffer();
}

//////////////////////////////////////////////////////////////////////////
// TRANSPERANT OBJECT COMPARE
//////////////////////////////////////////////////////////////////////////

bool cTransperantObject_Compare::operator()(iRenderable *pObjectA, iRenderable *pObjectB) const {
	if (pObjectA->GetMaterial()->GetDepthTest() != pObjectA->GetMaterial()->GetDepthTest()) {
		return pObjectA->GetMaterial()->GetDepthTest() < pObjectA->GetMaterial()->GetDepthTest();
	}

	return pObjectA->GetZ() < pObjectB->GetZ();
}

//////////////////////////////////////////////////////////////////////////
// QUERY COMPARE
//////////////////////////////////////////////////////////////////////////

bool cOcclusionQueryObject_Compare::operator()(const cOcclusionQueryObject *pObjectA,
											   const cOcclusionQueryObject *pObjectB) const {
	if (pObjectA->mpVtxBuffer != pObjectB->mpVtxBuffer)
		return pObjectA->mpVtxBuffer < pObjectB->mpVtxBuffer;

	if (pObjectA->mpMatrix != pObjectB->mpMatrix)
		return pObjectA->mpMatrix < pObjectB->mpMatrix;

	return (int)pObjectA->mbDepthTest < (int)pObjectB->mbDepthTest;
}

//////////////////////////////////////////////////////////////////////////
// RENDER NODE
//////////////////////////////////////////////////////////////////////////

cMemoryPool<iRenderState> *g_poolRenderState = NULL;
cMemoryPool<cRenderNode> *g_poolRenderNode = NULL;

//-----------------------------------------------------------------------

bool cRenderNodeCompare::operator()(cRenderNode *apNodeA, cRenderNode *apNodeB) const {
	int val = apNodeA->mpState->Compare(apNodeB->mpState);
	bool ret = val > 0 ? true : false;
	return ret;
}

//-----------------------------------------------------------------------

void cRenderNode::DeleteChildren() {
	tRenderNodeSetIt it = m_setNodes.begin();
	for (; it != m_setNodes.end(); ++it) {
		cRenderNode *pNode = *it;

		pNode->DeleteChildren();
		g_poolRenderNode->Release(pNode);
	}

	if (mpState)
		g_poolRenderState->Release(mpState);
	mpState = NULL;
	m_setNodes.clear();
}

//-----------------------------------------------------------------------

void cRenderNode::Render(cRenderSettings *apSettings) {
	tRenderNodeSetIt it = m_setNodes.begin();
	for (; it != m_setNodes.end(); ++it) {
		cRenderNode *pNode = *it;

		pNode->mpState->SetMode(apSettings);
		pNode->Render(apSettings);
	}
}

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cRenderList::cRenderList(cGraphics *apGraphics) {
	mfFrameTime = 0;
	mTempNode.mpState = hplNew(iRenderState, ());

	mlRenderCount = 0;
	mlLastRenderCount = 0;

	mpGraphics = apGraphics;

	m_poolRenderState = hplNew(cMemoryPool<iRenderState>, (3000, NULL));
	m_poolRenderNode = hplNew(cMemoryPool<cRenderNode>, (3000, NULL));

	g_poolRenderState = m_poolRenderState;
	g_poolRenderNode = m_poolRenderNode;
}

//-----------------------------------------------------------------------

cRenderList::~cRenderList() {
	Clear();
	hplDelete(mTempNode.mpState);

	hplDelete(m_poolRenderState);
	hplDelete(m_poolRenderNode);

	g_poolRenderState = NULL;
	g_poolRenderNode = NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cRenderList::Clear() {
	++mlGlobalRenderCount;

	// Clear containers.
	m_setLights.clear();
	m_setObjects.clear();
	m_setQueries.clear();
	m_setMotionBlurObjects.clear();
	m_setTransperantObjects.clear();

	mRootNodeDepth.DeleteChildren();
	mRootNodeDiffuse.DeleteChildren();
	mRootNodeTrans.DeleteChildren();

	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		mRootNodeLight[i].DeleteChildren();
	}

	mlLastRenderCount = mlRenderCount;
	mlRenderCount++;
}

//-----------------------------------------------------------------------

void cRenderList::AddOcclusionQuery(cOcclusionQueryObject *apObject) {
	m_setQueries.insert(apObject);
}

//-----------------------------------------------------------------------

cOcclusionQueryObjectIterator cRenderList::GetQueryIterator() {
	return cOcclusionQueryObjectIterator(&m_setQueries);
}

//-----------------------------------------------------------------------

cMotionBlurObjectIterator cRenderList::GetMotionBlurIterator() {
	return cMotionBlurObjectIterator(&m_setMotionBlurObjects);
}

//-----------------------------------------------------------------------

cTransperantObjectIterator cRenderList::GetTransperantIterator() {
	return cTransperantObjectIterator(&m_setTransperantObjects);
}

//-----------------------------------------------------------------------

cLight3DIterator cRenderList::GetLightIt() {
	return cLight3DIterator(&m_setLights);
}

//-----------------------------------------------------------------------

cRenderableIterator cRenderList::GetObjectIt() {
	return cRenderableIterator(&m_setObjects);
}

//-----------------------------------------------------------------------

int cRenderList::GetLightNum() {
	return (int)m_setLights.size();
}

int cRenderList::GetObjectNum() {
	return (int)m_setObjects.size();
}

//-----------------------------------------------------------------------

cRenderNode *cRenderList::GetRootNode(eRenderListDrawType aObjectType, eMaterialRenderType aPassType, int alLightNum) {
	if (aObjectType == eRenderListDrawType_Normal) {
		if (aPassType == eMaterialRenderType_Z)
			return &mRootNodeDepth;
		if (aPassType == eMaterialRenderType_Diffuse)
			return &mRootNodeDiffuse;
		if (aPassType == eMaterialRenderType_Light)
			return &mRootNodeLight[alLightNum];
		return NULL;
	} else {
		return &mRootNodeTrans;
	}
}

//-----------------------------------------------------------------------

void cRenderList::Compile() {
	int lLightNum = (int)m_setLights.size();
	if (lLightNum > MAX_NUM_OF_LIGHTS)
		lLightNum = MAX_NUM_OF_LIGHTS;

	for (int i = 0; i < lLightNum; ++i)
		mvObjectsPerLight[i] = 0;

	// Iterate the objects to be rendered and build trees with render states.
	tRenderableSetIt it = m_setObjects.begin();
	for (; it != m_setObjects.end(); ++it) {
		iRenderable *pObject = *it;
		iMaterial *pMat = pObject->GetMaterial();

		// Skip meshes...
		if (pObject->GetRenderType() == eRenderableType_Mesh)
			continue;

		// If the object is transparent add tot eh trans tree
		if (pMat->IsTransperant()) {
			// Use the set cotainer instead for now:
			m_setTransperantObjects.insert(pObject);
			/*for(int lPass=0; lPass< pMat->GetNumOfPasses(eMaterialRenderType_Diffuse, NULL);lPass++)
			{
				AddToTree(pObject,eRenderListDrawType_Trans,
							eMaterialRenderType_Light,0,NULL, true,lPass);
			}*/
		}
		// If object is not trans add to another tree.
		else {
			// If the object uses z pass add to z tree.
			if (pMat->UsesType(eMaterialRenderType_Z)) {
				for (int lPass = 0; lPass < pMat->GetNumOfPasses(eMaterialRenderType_Z, NULL); lPass++) {
					AddToTree(pObject, eRenderListDrawType_Normal, eMaterialRenderType_Z,
							  0, NULL, false, lPass);
				}
			}

			// If object uses light add the object to each light's tree.
			if (pMat->UsesType(eMaterialRenderType_Light)) {
				// Light trees that the object will belong to.
				int lLightCount = 0;

				tLight3DSetIt lightIt = m_setLights.begin();
				for (; lightIt != m_setLights.end(); ++lightIt) {
					iLight3D *pLight = *lightIt;

					if ((pLight->GetOnlyAffectInSector() == false || pObject->IsInSector(pLight->GetCurrentSector())) &&
						pLight->CheckObjectIntersection(pObject)) {
						if (lLightCount >= MAX_NUM_OF_LIGHTS)
							break;

						++mvObjectsPerLight[lLightCount];

						for (int lPass = 0; lPass < pMat->GetNumOfPasses(eMaterialRenderType_Light, pLight); ++lPass) {
							AddToTree(pObject, eRenderListDrawType_Normal,
									  eMaterialRenderType_Light, lLightCount, pLight, false, lPass);
						}
					}
					++lLightCount;
				}
			}

			// If it has a diffuse pass, add to the diffuse tree.
			if (pObject->GetMaterial()->UsesType(eMaterialRenderType_Diffuse)) {
				for (int lPass = 0; lPass < pMat->GetNumOfPasses(eMaterialRenderType_Diffuse, NULL); lPass++) {
					AddToTree(pObject, eRenderListDrawType_Normal,
							  eMaterialRenderType_Diffuse, 0, NULL, false, lPass);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

bool cRenderList::Add(iRenderable *apObject) {
	if (apObject->IsVisible() == false)
		return false;

	// Check if the object is culled by fog.
	cRenderer3D *pRenderer = mpGraphics->GetRenderer3D();
	if (pRenderer->GetFogActive() && pRenderer->GetFogCulling()) {
		if (cMath::CheckCollisionBV(*pRenderer->GetFogBV(), *apObject->GetBoundingVolume())) {

		} else {
			return false;
		}
	}

	// Check if the object as already been added.
	if (mlRenderCount == apObject->GetRenderCount())
		return false;
	apObject->SetRenderCount(mlRenderCount);

	// Update the graphics of object.
	apObject->UpdateGraphics(cRenderList::GetCamera(), mfFrameTime, this);

	// Set that the object is to be rendered.
	apObject->SetGlobalRenderCount(mlGlobalRenderCount);

	switch (apObject->GetRenderType()) {
		// Add a normal renderable
	case eRenderableType_Normal:
	case eRenderableType_ParticleSystem:
		if (apObject->GetMaterial()->IsTransperant()) {
			// Calculate the Z for the trans object
			cVector3f vCameraPos = cMath::MatrixMul(cRenderList::GetCamera()->GetViewMatrix(),
													apObject->GetBoundingVolume()->GetWorldCenter());
			apObject->SetZ(vCameraPos.z);

			m_setObjects.insert(apObject);
		} else {
			m_setObjects.insert(apObject);

			// MotionBlur
			if (mpGraphics->GetRendererPostEffects()->GetMotionBlurActive() || mpGraphics->GetRenderer3D()->GetRenderSettings()->mbFogActive || mpGraphics->GetRendererPostEffects()->GetDepthOfFieldActive()) {
				m_setMotionBlurObjects.insert(apObject);

				if (apObject->GetPrevRenderCount() != GetLastRenderCount()) {
					cMatrixf *pMtx = apObject->GetModelMatrix(mpCamera);
					if (pMtx) {
						apObject->SetPrevMatrix(*pMtx);
					}
				}
				apObject->SetPrevRenderCount(mlRenderCount);
			}
		}

		break;
		// Add all sub meshes of the mesh
	case eRenderableType_Mesh: {
		cMeshEntity *pMesh = static_cast<cMeshEntity *>(apObject);
		for (int i = 0; i < pMesh->GetSubMeshEntityNum(); i++) {
			Add(pMesh->GetSubMeshEntity(i));
		}
		break;
	}
		// Add a light to a special container
	case eRenderableType_Light: {
		iLight3D *pLight = static_cast<iLight3D *>(apObject);

		m_setLights.insert(pLight);
	} break;
	default:
		break;
	}

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cRenderNode *cRenderList::InsertNode(cRenderNode *apListNode, cRenderNode *apTempNode) {
	// Log("Searching for node... ");
	// Create a node with the state.

	tRenderNodeSetIt it = apListNode->m_setNodes.find(apTempNode);

	if (it != apListNode->m_setNodes.end()) {
		// Log("Node found!\n");
		// node found, return it.
		return *it;
	} else {
		// Log("Node NOT found, creating new.\n");
		// no node found, create one and return it.
		cRenderNode *pNode = m_poolRenderNode->Create();
		pNode->mpState = m_poolRenderState->Create();

		// Copy the relevant values to the state.
		pNode->mpState->Set(apTempNode->mpState);

		apListNode->m_setNodes.insert(pNode);

		return pNode;
	}
}

//-----------------------------------------------------------------------

/*cRenderNode* cRenderList::InsertNode(cRenderNode* apListNode, cRenderNode* apTempNode)
{
	Std::pair<tRenderNodeSetIt, bool> ret = apListNode->m_setNodes.insert(apTempNode);
	if(ret.second == false)
	{
		m_poolRenderState->Release(apTempNode->mpState);
		m_poolRenderNode->Release(apTempNode);
		return *ret.first;
	}

	return apTempNode;
}*/

//-----------------------------------------------------------------------

void cRenderList::AddToTree(iRenderable *apObject, eRenderListDrawType aObjectType,
							eMaterialRenderType aPassType, int alLightNum, iLight3D *apLight,
							bool abUseDepth, int alPass) {
	// Log("------ OBJECT ------------\n");

	cRenderNode *pNode = GetRootNode(aObjectType, aPassType, alLightNum);
	iMaterial *pMaterial = apObject->GetMaterial();
	iRenderState *pTempState = mTempNode.mpState;
	cRenderNode *pTempNode = &mTempNode;

	//-------------- EXPLAINATION ----------------------------------
	// Go through each render state type and set the appropriate
	// variables for each type. The most important states are set first.
	// The state is then inserted to a tree structure, where each state type is a level.
	//                       Example:
	//                 |----------root--------|
	//           |---pass              |-----pass----|
	//       depth-test            depth-test     depth-test
	//              ............etc......................
	// After each insertion a pointer to the inserted or existing (if variables already existed)
	// node is returned
	// This is then passed on the next state, the state inserted (or exist found) there and so on.
	// At each insertion, it is first checked if a state with variables exist, and if so, use that.
	// if not, a new node is created and inserted.
	// The temp node state is used as storage for the data to skip the overhead of deletion
	// and creation when testing if state exist. Instead new data is only created if the state
	// is new.
	// Each Render state has the same class, but uses different var depending on type, this to
	// skip the overhead of using inheritance.
	//-------------------------------------------------------------

	/////// SECTOR //////////////
	if (aPassType == eMaterialRenderType_Z) {
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_Sector;
		pTempState->mpSector = apObject->GetCurrentSector();

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// PASS //////////////
	{
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_Pass;
		pTempState->mlPass = alPass;

		pNode = InsertNode(pNode, pTempNode);
	}
	/////// DEPTH TEST //////////////
	{
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		// Log("\nDepth level, Z: %f\n",apObject->GetZ());

		pTempState->mType = eRenderStateType_DepthTest;
		pTempState->mbDepthTest = pMaterial->GetDepthTest();

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// DEPTH //////////////
	if (abUseDepth) {
		// Log("\nDepth level, Z: %f\n",apObject->GetZ());
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_Depth;
		pTempState->mfZ = apObject->GetZ();

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// ALPHA MODE //////////////
	{
		// Log("\nAlpha level %d\n", pMaterial->GetAlphaMode(mType,alPass));
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_AlphaMode;
		pTempState->mAlphaMode = pMaterial->GetAlphaMode(aPassType, alPass, apLight);

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// BLEND MODE //////////////
	{
		// Log("\nBlend level %d : %d\n",pMaterial->GetBlendMode(mType,alPass),pMaterial->GetChannelMode(mType,alPass));
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_BlendMode;

		pTempState->mBlendMode = pMaterial->GetBlendMode(aPassType, alPass, apLight);
		pTempState->mChannelMode = pMaterial->GetChannelMode(aPassType, alPass, apLight);

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// GPU PROGRAM //////////////
	{
		pTempState->mType = eRenderStateType_GpuProgram;

		pTempState->gpuProgram = pMaterial->getGpuProgram(aPassType, alPass, apLight);
		pTempState->gpuProgramSetup = pMaterial->getGpuProgramSetup(aPassType, alPass, apLight);
		pTempState->mbUsesLight = pMaterial->VertexProgramUsesLight(aPassType, alPass, apLight);
		pTempState->mbUsesEye = pMaterial->VertexProgramUsesEye(aPassType, alPass, apLight);
		pTempState->mpLight = apLight;

		pNode = InsertNode(pNode, pTempNode);
	}
#if 0
	/////// VERTEX PROGRAM //////////////
	{
		// Log("\nVertex program level\n");
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_VertexProgram;

		pTempState->mpVtxProgram = pMaterial->GetVertexProgram(aPassType, alPass, apLight);
		pTempState->mpVtxProgramSetup = pMaterial->GetVertexProgramSetup(aPassType, alPass, apLight);
		pTempState->mbUsesLight = pMaterial->VertexProgramUsesLight(aPassType, alPass, apLight);
		pTempState->mbUsesEye = pMaterial->VertexProgramUsesEye(aPassType, alPass, apLight);
		pTempState->mpLight = apLight;

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// FRAGMENT PROGRAM //////////////
	{
		// Log("\nFragment program level\n");
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_FragmentProgram;

		pTempState->mpFragProgram = pMaterial->GetFragmentProgram(aPassType, alPass, apLight);
		pTempState->mpFragProgramSetup = pMaterial->GetFragmentProgramSetup(aPassType, alPass, apLight);

		pNode = InsertNode(pNode, pTempNode);
	}
#endif
	/////// TEXTURE //////////////
	{
		// Log("\nTexture level\n");
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_Texture;

		for (int i = 0; i < MAX_TEXTUREUNITS; i++) {
			pTempState->mpTexture[i] = pMaterial->GetTexture(i, aPassType, alPass, apLight);
		}

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// VERTEX BUFFER //////////////
	{
		// Log("\nVertex buffer level\n");
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_VertexBuffer;

		pTempState->mpVtxBuffer = apObject->GetVertexBuffer();

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// MATRIX //////////////
	{
		// Log("\nMatrix level\n");
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_Matrix;

		pTempState->mpModelMatrix = apObject->GetModelMatrix(mpCamera);
		pTempState->mpInvModelMatrix = apObject->GetInvModelMatrix();
		pTempState->mvScale = apObject->GetCalcScale();

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// RENDER //////////////
	{
		// Log("\nRender leaf!\n");
		// pTempNode = m_poolRenderNode->Create();
		// pTempState = m_poolRenderState->Create();
		// pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_Render;

		pTempState->mpObject = apObject;

		InsertNode(pNode, pTempNode);
	}

	// Log("-------------------------\n");
}

//----------------------------------------------------------------------------------------------

/*void cRenderList::AddToTree(iRenderable* apObject,eRenderListDrawType aObjectType,
							eMaterialRenderType aPassType, int alLightNum,iLight3D* apLight,
							bool abUseDepth, int alPass)
{
	//Log("------ OBJECT ------------\n");

	cRenderNode *pNode = GetRootNode(aObjectType,aPassType, alLightNum);
	iMaterial *pMaterial = apObject->GetMaterial();
	iRenderState *pTempState = NULL;//mTempNode.mpState;
	cRenderNode *pTempNode = NULL;

	//-------------- EXPLAINATION ----------------------------------
	// Go through each render state type and set the appropriate
	// variables for each type. The most important states are set first.
	// The state is then inserted to a tree structure, where each state type is a level.
	//                       Example:
	//                 |----------root--------|
	//           |---pass              |-----pass----|
	//       depth-test            depth-test     depth-test
	//              ............etc......................
	// After each insertion a pointer to the inserted or existing (if variables already existed)
	// node is returned
	// This is then passed on the next state, the state inserted (or exist found) there and so on.
	// At each insertion, it is first checked if a state with variables exist, and if so, use that.
	// if not, a new node is created and inserted.
	// The temp node state is used as storage for the data to skip the overhead of deletion
	// and creation when testing if state exist. Instead new data is only created if the state
	// is new.
	// Each Render state has the same class, but uses different var depending on type, this to
	// skip the overhead of using inheritance.
	//-------------------------------------------------------------

	/////// SECTOR //////////////
	if(aPassType == eMaterialRenderType_Z)
	{
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_Sector;
		pTempState->mpSector = apObject->GetCurrentSector();

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// PASS //////////////
	{
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;


		pTempState->mType = eRenderStateType_Pass;
		pTempState->mlPass = alPass;

		pNode = InsertNode(pNode, pTempNode);
	}
	/////// DEPTH TEST //////////////
	{
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;

		//Log("\nDepth level, Z: %f\n",apObject->GetZ());

		pTempState->mType = eRenderStateType_DepthTest;
		pTempState->mbDepthTest = pMaterial->GetDepthTest();

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// DEPTH //////////////
	if(abUseDepth)
	{
		//Log("\nDepth level, Z: %f\n",apObject->GetZ());
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;


		pTempState->mType = eRenderStateType_Depth;
		pTempState->mfZ = apObject->GetZ();

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// ALPHA MODE //////////////
	{
		//Log("\nAlpha level %d\n", pMaterial->GetAlphaMode(mType,alPass));
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;


		pTempState->mType = eRenderStateType_AlphaMode;
		pTempState->mAlphaMode = pMaterial->GetAlphaMode(aPassType,alPass, apLight);

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// BLEND MODE //////////////
	{
		//Log("\nBlend level %d : %d\n",pMaterial->GetBlendMode(mType,alPass),pMaterial->GetChannelMode(mType,alPass));
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;


		pTempState->mType = eRenderStateType_BlendMode;

		pTempState->mBlendMode = pMaterial->GetBlendMode(aPassType,alPass,apLight);
		pTempState->mChannelMode = pMaterial->GetChannelMode(aPassType,alPass,apLight);

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// VERTEX PROGRAM //////////////
	{
		//Log("\nVertex program level\n");
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;

		pTempState->mType = eRenderStateType_VertexProgram;

		pTempState->mpVtxProgram = pMaterial->GetVertexProgram(aPassType,alPass,apLight);
		pTempState->mpVtxProgramSetup = pMaterial->GetVertexProgramSetup(aPassType,alPass,apLight);
		pTempState->mbUsesLight = pMaterial->VertexProgramUsesLight(aPassType, alPass,apLight);
		pTempState->mbUsesEye = pMaterial->VertexProgramUsesEye(aPassType, alPass,apLight);
		pTempState->mpLight = apLight;

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// FRAGMENT PROGRAM //////////////
	{
		//Log("\nFragment program level\n");
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;


		pTempState->mType = eRenderStateType_FragmentProgram;

		pTempState->mpFragProgram = pMaterial->GetFragmentProgram(aPassType,alPass,apLight);

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// TEXTURE //////////////
	{
		//Log("\nTexture level\n");
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;


		pTempState->mType = eRenderStateType_Texture;

		for(int i=0; i<MAX_TEXTUREUNITS;i++)
		{
			pTempState->mpTexture[i] = pMaterial->GetTexture(i,aPassType, alPass,apLight);
		}

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// VERTEX BUFFER //////////////
	{
		//Log("\nVertex buffer level\n");
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;


		pTempState->mType = eRenderStateType_VertexBuffer;

		pTempState->mpVtxBuffer = apObject->GetVertexBuffer();

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// MATRIX //////////////
	{
		//Log("\nMatrix level\n");
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;


		pTempState->mType = eRenderStateType_Matrix;

		pTempState->mpModelMatrix = apObject->GetModelMatrix(mpCamera);
		pTempState->mpInvModelMatrix = apObject->GetInvModelMatrix();
		pTempState->mvScale = apObject->GetCalcScale();

		pNode = InsertNode(pNode, pTempNode);
	}

	/////// RENDER //////////////
	{
		//Log("\nRender leaf!\n");
		pTempNode = m_poolRenderNode->Create();
		pTempState = m_poolRenderState->Create();
		pTempNode->mpState = pTempState;


		pTempState->mType = eRenderStateType_Render;

		pTempState->mpObject = apObject;

		InsertNode(pNode, pTempNode);
	}

	//Log("-------------------------\n");
}*/

//-----------------------------------------------------------------------

} // namespace hpl
