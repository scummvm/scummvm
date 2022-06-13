/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "hpl1/engine/scene/SubMeshEntity.h"

#include "hpl1/engine/scene/MeshEntity.h"

#include "hpl1/engine/resources/MaterialManager.h"
#include "hpl1/engine/resources/MeshManager.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/Mesh.h"
#include "hpl1/engine/graphics/SubMesh.h"

#include "hpl1/engine/graphics/Animation.h"
#include "hpl1/engine/graphics/AnimationTrack.h"
#include "hpl1/engine/graphics/Skeleton.h"
#include "hpl1/engine/graphics/Bone.h"

#include "hpl1/engine/scene/AnimationState.h"
#include "hpl1/engine/scene/NodeState.h"

#include "hpl1/engine/physics/PhysicsBody.h"

#include "hpl1/engine/math/Math.h"

#include <string.h>

namespace hpl {
	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	cSubMeshEntity::cSubMeshEntity(const tString &asName, cMeshEntity *apMeshEntity, cSubMesh * apSubMesh,
								cMaterialManager* apMaterialManager) : iRenderable(asName)
	{
		mpMeshEntity = apMeshEntity;
		mpSubMesh = apSubMesh;

		mbIsOneSided = mpSubMesh->GetIsOneSided();
		mvOneSidedNormal = mpSubMesh->GetOneSidedNormal();

		mpMaterialManager = apMaterialManager;

		mbCastShadows = false;

		mbGraphicsUpdated = false;

		mpBody = NULL;

		if(mpMeshEntity->GetMesh()->GetSkeleton())
		{
			mpDynVtxBuffer = mpSubMesh->GetVertexBuffer()->CreateCopy(eVertexBufferUsageType_Dynamic);
			mvDynTriangles = *mpSubMesh->GetTriangleVecPtr();
		}
		else
		{
			mpDynVtxBuffer = NULL;
		}

		mpLocalNode = NULL;

		mpEntityCallback = hplNew( cSubMeshEntityBodyUpdate, () );
		mbUpdateBody = false;

		mpMaterial = NULL;
	}

	cSubMeshEntity::~cSubMeshEntity()
	{
		hplDelete(mpEntityCallback);

		if(mpDynVtxBuffer) hplDelete(mpDynVtxBuffer);

		/* Clear any custom textures here*/
		if(mpMaterial) mpMaterialManager->Destroy(mpMaterial);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// BODY CALLBACK
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cSubMeshEntityBodyUpdate::OnTransformUpdate(iEntity3D* apEntity)
	{
		/*cSubMeshEntity *pSubEntity = static_cast<cSubMeshEntity*>(apEntity);

		if(pSubEntity->GetBody())
		{
			if(apEntity->GetWorldMatrix() != pSubEntity->GetBody()->GetLocalMatrix())
			{
				Log("Setting matrix on %s from\n",pSubEntity->GetBody()->GetName().c_str());
				Log("  %s\n",apEntity->GetWorldMatrix().ToString().c_str());
				Log("  %s\n",pSubEntity->GetBody()->GetLocalMatrix().ToString().c_str());

				pSubEntity->GetBody()->SetMatrix(apEntity->GetWorldMatrix());
			}
		}*/
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cSubMeshEntity::UpdateLogic(float afTimeStep)
	{
		if(mbUpdateBody && mpBody)
		{
			mpBody->SetMatrix(GetWorldMatrix());
		}
	}

	//-----------------------------------------------------------------------


	iMaterial* cSubMeshEntity::GetMaterial()
	{
		if(mpMaterial==NULL && mpSubMesh->GetMaterial()==NULL)
		{
			Error("Materials for sub entity %s are NULL!\n",GetName().c_str());
		}

		if(mpMaterial)
			return mpMaterial;
		else
			return mpSubMesh->GetMaterial();
	}

	//-----------------------------------------------------------------------

	// Set Src as private variable to give this a little boost! Or?
	static inline void MatrixFloatTransformSet(float *pDest, const cMatrixf &a_mtxA, const float* pSrc, const float fWeight)
	{
		pDest[0] = ( a_mtxA.m[0][0] * pSrc[0] + a_mtxA.m[0][1] * pSrc[1] + a_mtxA.m[0][2] * pSrc[2] + a_mtxA.m[0][3] ) * fWeight;
		pDest[1] = ( a_mtxA.m[1][0] * pSrc[0] + a_mtxA.m[1][1] * pSrc[1] + a_mtxA.m[1][2] * pSrc[2] + a_mtxA.m[1][3] ) * fWeight;
		pDest[2] = ( a_mtxA.m[2][0] * pSrc[0] + a_mtxA.m[2][1] * pSrc[1] + a_mtxA.m[2][2] * pSrc[2] + a_mtxA.m[2][3] ) * fWeight;
	}

	static inline void MatrixFloatRotateSet(float *pDest, const cMatrixf &a_mtxA, const float* pSrc, const float fWeight)
	{
		pDest[0] = ( a_mtxA.m[0][0] * pSrc[0] + a_mtxA.m[0][1] * pSrc[1] + a_mtxA.m[0][2] * pSrc[2] ) * fWeight;
		pDest[1] = ( a_mtxA.m[1][0] * pSrc[0] + a_mtxA.m[1][1] * pSrc[1] + a_mtxA.m[1][2] * pSrc[2] ) * fWeight;
		pDest[2] = ( a_mtxA.m[2][0] * pSrc[0] + a_mtxA.m[2][1] * pSrc[1] + a_mtxA.m[2][2] * pSrc[2] ) * fWeight;
	}

	////////////////////////////////////////////////////////////

	// Set Src as private variable to give this a little boost!Or?
	static inline void MatrixFloatTransformAdd(float *pDest, const cMatrixf &a_mtxA, const float* pSrc, const float fWeight)
	{
		pDest[0] += ( a_mtxA.m[0][0] * pSrc[0] + a_mtxA.m[0][1] * pSrc[1] + a_mtxA.m[0][2] * pSrc[2] + a_mtxA.m[0][3] ) * fWeight;
		pDest[1] += ( a_mtxA.m[1][0] * pSrc[0] + a_mtxA.m[1][1] * pSrc[1] + a_mtxA.m[1][2] * pSrc[2] + a_mtxA.m[1][3] ) * fWeight;
		pDest[2] += ( a_mtxA.m[2][0] * pSrc[0] + a_mtxA.m[2][1] * pSrc[1] + a_mtxA.m[2][2] * pSrc[2] + a_mtxA.m[2][3] ) * fWeight;
	}

	static inline void MatrixFloatRotateAdd(float *pDest, const cMatrixf &a_mtxA, const float* pSrc, const float fWeight)
	{
		pDest[0] += ( a_mtxA.m[0][0] * pSrc[0] + a_mtxA.m[0][1] * pSrc[1] + a_mtxA.m[0][2] * pSrc[2] ) * fWeight;
		pDest[1] += ( a_mtxA.m[1][0] * pSrc[0] + a_mtxA.m[1][1] * pSrc[1] + a_mtxA.m[1][2] * pSrc[2] ) * fWeight;
		pDest[2] += ( a_mtxA.m[2][0] * pSrc[0] + a_mtxA.m[2][1] * pSrc[1] + a_mtxA.m[2][2] * pSrc[2] ) * fWeight;
	}

	////////////////////////////////////////////////////////////

	void cSubMeshEntity::UpdateGraphics(cCamera3D *apCamera, float afFrameTime, cRenderList *apRenderList)
	{
		if(mpDynVtxBuffer)
		{
			if(mpMeshEntity->mbSkeletonPhysicsSleeping && mbGraphicsUpdated)
			{
				return;
			}

			mbGraphicsUpdated = true;

			const float *pBindPos = mpSubMesh->GetVertexBuffer()->GetArray(eVertexFlag_Position);
			const float *pBindNormal = mpSubMesh->GetVertexBuffer()->GetArray(eVertexFlag_Normal);
			const float *pBindTangent = mpSubMesh->GetVertexBuffer()->GetArray(eVertexFlag_Texture1);

			float *pSkinPos = mpDynVtxBuffer->GetArray(eVertexFlag_Position);
			float *pSkinNormal = mpDynVtxBuffer->GetArray(eVertexFlag_Normal);
			float *pSkinTangent = mpDynVtxBuffer->GetArray(eVertexFlag_Texture1);

			const int lVtxStride = kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)];
			const int lVtxNum = mpDynVtxBuffer->GetVertexNum();

			for(int vtx=0; vtx < lVtxNum; vtx++)
			{
				//To count the bone bindings
				int lCount = 0;
				//Get pointer to weights and bone index.
				const float *pWeight = &mpSubMesh->mpVertexWeights[vtx*4];
				if(*pWeight==0) continue;

				const unsigned char *pBoneIdx = &mpSubMesh->mpVertexBones[vtx*4];

				const cMatrixf &mtxTransform = mpMeshEntity->mvBoneMatrices[*pBoneIdx];

				//Log("Vtx: %d\n",vtx);
				//Log("Boneidx: %d Count %d Weight: %f vtx: %d\n",(int)*pBoneIdx,lCount, *pWeight,vtx);

				//ATTENTION: Some optimizing might be done by accumulating the matrix instead.
								//THIS is really unsure since it seems like it will result in more math, matrix mul = 8*4 calc
								//Vertex mul with matrix is 3 * 3 calculations
								//this means: vertex= 9*3 = 27, matrix = 32

				MatrixFloatTransformSet(pSkinPos,mtxTransform, pBindPos, *pWeight);

				MatrixFloatRotateSet(pSkinNormal,mtxTransform, pBindNormal, *pWeight);

				MatrixFloatRotateSet(pSkinTangent,mtxTransform, pBindTangent, *pWeight);

				++pWeight; ++pBoneIdx; ++lCount;

				//Iterate weights until 0 is found or count < 4
				while(*pWeight != 0 && lCount < 4)
				{
					//Log("Boneidx: %d Count %d Weight: %f\n",(int)*pBoneIdx,lCount, *pWeight);
					const cMatrixf &mtxTransform = mpMeshEntity->mvBoneMatrices[*pBoneIdx];

					//Transform with the local movement of the bone.
					MatrixFloatTransformAdd(pSkinPos,mtxTransform, pBindPos, *pWeight);

					MatrixFloatRotateAdd(pSkinNormal,mtxTransform, pBindNormal, *pWeight);

					MatrixFloatRotateAdd(pSkinTangent,mtxTransform, pBindTangent, *pWeight);

					++pWeight; ++pBoneIdx; ++lCount;
				}

				pBindPos += lVtxStride;
				pSkinPos += lVtxStride;

				pBindNormal += 3;
				pSkinNormal += 3;

				pBindTangent += 4;
				pSkinTangent += 4;
			}

			float *pSkinPosArray = mpDynVtxBuffer->GetArray(eVertexFlag_Position);
			if(mpMeshEntity->IsShadowCaster())
			{
				//Update the shadow double
				memcpy(&pSkinPosArray[lVtxStride*lVtxNum],pSkinPosArray,sizeof(float)*lVtxStride*lVtxNum);
				for(int vtx=lVtxStride*lVtxNum + lVtxStride-1; vtx < lVtxStride*lVtxNum*2; vtx+=lVtxStride)
				{
					pSkinPosArray[vtx] = 0;
				}
			}

			//Update buffer
			mpDynVtxBuffer->UpdateData(eVertexFlag_Position | eVertexFlag_Normal | eVertexFlag_Texture1,false);

			if(mpMeshEntity->IsShadowCaster())
			{
				//Update triangles
				cMath::CreateTriangleData(mvDynTriangles,
					mpDynVtxBuffer->GetIndices(), mpDynVtxBuffer->GetIndexNum(),
					pSkinPosArray, lVtxStride, lVtxNum);
			}
		}



		/*if(mpDynVtxBuffer)
		{
		const float *pBindPosArray = mpSubMesh->GetVertexBuffer()->GetArray(eVertexFlag_Position);
		const float *pBindNormalArray = mpSubMesh->GetVertexBuffer()->GetArray(eVertexFlag_Normal);
		const float *pBindTangentArray = mpSubMesh->GetVertexBuffer()->GetArray(eVertexFlag_Texture1);

		float *pSkinPosArray = mpDynVtxBuffer->GetArray(eVertexFlag_Position);
		float *pSkinNormalArray = mpDynVtxBuffer->GetArray(eVertexFlag_Normal);
		float *pSkinTangentArray = mpDynVtxBuffer->GetArray(eVertexFlag_Texture1);

		int lVtxStride = kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)];
		int lVtxNum = mpDynVtxBuffer->GetVertexNum();

		memset(pSkinPosArray,0,sizeof(float)*lVtxStride*lVtxNum);
		memset(pSkinNormalArray,0,sizeof(float)*3*lVtxNum);
		memset(pSkinTangentArray,0,sizeof(float)*4*lVtxNum);

		int lSize = mpSubMesh->GetVertexBonePairNum();
		for(int i=0; i<lSize; i++)
		{
		const cVertexBonePair& VBPair = mpSubMesh->GetVertexBonePair(i);

		//Log("%d: Vtx: %d Bone: %d\n",i,VBPair.vtxIdx, VBPair.boneIdx);

		const float* pBindPos = &pBindPosArray[VBPair.vtxIdx*lVtxStride];
		float* pSkinPos = &pSkinPosArray[VBPair.vtxIdx*lVtxStride];

		const float* pBindNorm = &pBindNormalArray[VBPair.vtxIdx*3];
		float* pSkinNorm = &pSkinNormalArray[VBPair.vtxIdx*3];

		const float* pBindTan = &pBindTangentArray[VBPair.vtxIdx*4];
		float* pSkinTan = &pSkinTangentArray[VBPair.vtxIdx*4];

		const cMatrixf &mtxTransform = mpMeshEntity->mvBoneMatrices[VBPair.boneIdx];

		//Transform with the local movement of the bone.
		MatrixFloatTransform(pSkinPos,mtxTransform, pBindPos, VBPair.weight);
		pSkinPos[3] = 1;

		MatrixFloatRotate(pSkinNorm,mtxTransform, pBindNorm, VBPair.weight);

		MatrixFloatRotate(pSkinTan,mtxTransform, pBindTan, VBPair.weight);
		pSkinTan[3] = pBindTan[3];

		//cVector3f vSkin = cMath::MatrixMul(mpMeshEntity->mvBoneMatrices[VBPair.boneIdx],
		//									cVector3f(pBindPos[0],pBindPos[1],pBindPos[2]));

		//pSkinPos[0] += vSkin.x * VBPair.weight;
		//pSkinPos[1] += vSkin.y * VBPair.weight;
		//pSkinPos[2] += vSkin.z * VBPair.weight;
		}

		//Update the shadow double
		memcpy(&pSkinPosArray[lVtxStride*lVtxNum],pSkinPosArray,sizeof(float)*lVtxStride*lVtxNum);
		for(int vtx=lVtxStride*lVtxNum + lVtxStride-1; vtx < lVtxStride*lVtxNum*2; vtx+=lVtxStride)
		{
		pSkinPosArray[vtx] = 0;
		}

		//Update buffer
		mpDynVtxBuffer->UpdateData(eVertexFlag_Position | eVertexFlag_Normal | eVertexFlag_Texture1,false);

		//Update triangles
		cMath::CreateTriangleData(mvDynTriangles,
		mpDynVtxBuffer->GetIndices(), mpDynVtxBuffer->GetIndexNum(),
		pSkinPosArray, lVtxStride, lVtxNum);
		}*/
	}

	iVertexBuffer* cSubMeshEntity::GetVertexBuffer()
	{
		if(mpDynVtxBuffer)
		{
			return mpDynVtxBuffer;
		}
		else
		{
			return mpSubMesh->GetVertexBuffer();
		}
	}

	cBoundingVolume* cSubMeshEntity::GetBoundingVolume()
	{
		return mpMeshEntity->GetBoundingVolume();
	}

	int cSubMeshEntity::GetMatrixUpdateCount()
	{
		if(mpMeshEntity->HasNodes())
		{
			return GetTransformUpdateCount();
		}
		else
		{
			return mpMeshEntity->GetMatrixUpdateCount();
		}
	}

	cMatrixf* cSubMeshEntity::GetModelMatrix(cCamera3D* apCamera)
	{
		if(mpMeshEntity->HasNodes())
		{
			//Log("%s Matrix from local node!\n",msName.c_str());
			return &GetWorldMatrix();
		}
		else
		{
			//Log("%s Matrix from mesh!\n",msName.c_str());

			if(mpMeshEntity->IsStatic()) return NULL;

			return mpMeshEntity->GetModelMatrix(NULL);
		}
	}

	//-----------------------------------------------------------------------

	void cSubMeshEntity::SetLocalNode(cNode3D *apNode)
	{
		mpLocalNode = apNode;

		mpLocalNode->AddEntity(this);
	}

	//-----------------------------------------------------------------------

	cNode3D* cSubMeshEntity::GetLocalNode()
	{
		return mpLocalNode;
	}

	//-----------------------------------------------------------------------

	tRenderContainerDataList* cSubMeshEntity::GetRenderContainerDataList()
	{
		//Log("Get from parent %s\n",mpMeshEntity->GetName().c_str());
		return mpMeshEntity->GetRenderContainerDataList();
	}

	//-----------------------------------------------------------------------

	void cSubMeshEntity::SetUpdateBody(bool abX)
	{
		mbUpdateBody = abX;

		/*if(mbUpdateBody)
		{
			AddCallback(mpEntityCallback);
		}
		else
		{
			RemoveCallback(mpEntityCallback);
		}*/
	}

	bool cSubMeshEntity::GetUpdateBody()
	{
		return mbUpdateBody;
	}

	//-----------------------------------------------------------------------

	cTriangleData& cSubMeshEntity::GetTriangle(int alIndex)
	{
		if(mpDynVtxBuffer)
			return mvDynTriangles[alIndex];
		else
			return (*mpSubMesh->GetTriangleVecPtr())[alIndex];
	}
	int cSubMeshEntity::GetTriangleNum()
	{
		if(mpDynVtxBuffer)
			return (int)mvDynTriangles.size();
		else
			return (int)mpSubMesh->GetTriangleVecPtr()->size();
	}

	tTriangleDataVec* cSubMeshEntity::GetTriangleVecPtr()
	{
		if(mpDynVtxBuffer)
			return &mvDynTriangles;
		else
			return mpSubMesh->GetTriangleVecPtr();
	}

	//-----------------------------------------------------------------------

	void cSubMeshEntity::SetCustomMaterial(iMaterial *apMaterial, bool abDestroyOldCustom)
	{
		if(abDestroyOldCustom)
		{
			if(mpMaterial) mpMaterialManager->Destroy(mpMaterial);
		}

		mpMaterial = apMaterial;
	}

	//-----------------------------------------------------------------------

	cSector* cSubMeshEntity::GetCurrentSector() const
	{
		return mpMeshEntity->GetCurrentSector();
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// SAVE OBJECT STUFF
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	kBeginSerialize(cSaveData_cSubMeshEntity,cSaveData_iRenderable)
	kSerializeVar(msMaterial, eSerializeType_String)
	kSerializeVar(mbCastShadows, eSerializeType_Bool)
	kSerializeVar(mlBodyId, eSerializeType_Int32)
	kSerializeVar(mbUpdateBody, eSerializeType_Bool)
	kEndSerialize()


	//-----------------------------------------------------------------------

	iSaveData* cSubMeshEntity::CreateSaveData()
	{
		return hplNew( cSaveData_cSubMeshEntity, () );
	}

	//-----------------------------------------------------------------------

	void cSubMeshEntity::SaveToSaveData(iSaveData *apSaveData)
	{
		kSaveData_SaveToBegin(cSubMeshEntity);

		kSaveData_SaveTo(mbCastShadows);
		kSaveData_SaveTo(mbUpdateBody);

		pData->msMaterial = mpMaterial==NULL ?"" : mpMaterial->GetName();

		kSaveData_SaveObject(mpBody, mlBodyId);
	}

	//-----------------------------------------------------------------------

	void cSubMeshEntity::LoadFromSaveData(iSaveData *apSaveData)
	{
		kSaveData_LoadFromBegin(cSubMeshEntity);

		kSaveData_LoadFrom(mbCastShadows);
		kSaveData_LoadFrom(mbUpdateBody);

		if(pData->msMaterial != "")
		{
			iMaterial *pMat = mpMaterialManager->CreateMaterial(pData->msMaterial);
			if(pMat) SetCustomMaterial(pMat);
		}
	}

	//-----------------------------------------------------------------------

	void cSubMeshEntity::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame)
	{
		kSaveData_SetupBegin(cSubMeshEntity);

		kSaveData_LoadObject(mpBody, mlBodyId,iPhysicsBody*);

		if(mpBody && mbUpdateBody == false)
		{
			mpBody->CreateNode()->AddEntity(this);
		}
	}

	//-----------------------------------------------------------------------
}
