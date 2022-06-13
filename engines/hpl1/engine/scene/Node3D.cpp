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
#include "hpl1/engine/scene/Node3D.h"
#include "hpl1/engine/scene/Entity3D.h"
#include "hpl1/engine/system/LowLevelSystem.h"
#include "hpl1/engine/math/Math.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cNode3D::cNode3D(const tString &asName, bool abAutoDeleteChildren) : iNode()
	{
		m_mtxLocalTransform = cMatrixf::Identity;
		m_mtxWorldTransform = cMatrixf::Identity;

		mvWorldPosition = cVector3f(0,0,0);

		mbTransformUpdated = true;

		mpParent = NULL;

		msName = asName;

		mbAutoDeleteChildren = abAutoDeleteChildren;

		m_mtxRotation = cMatrixf::Identity;
		mvScale = cVector3f(1,1,1);
		mvTranslation = cVector3f(0,0,0);
	}

	//-----------------------------------------------------------------------

	cNode3D::~cNode3D()
	{
		if(mbAutoDeleteChildren)
		{
			STLDeleteAll(mlstNode);
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iNode* cNode3D::CreateChild()
	{
		return CreateChild3D();
	}

	//-----------------------------------------------------------------------

	cNode3D* cNode3D::CreateChild3D(const tString &asName, bool abAutoDeleteChildren)
	{
		cNode3D *pNode = hplNew( cNode3D, (asName,abAutoDeleteChildren) );

		pNode->mpParent = this;
		mlstNode.push_back(pNode);

		return pNode;
	}

	//-----------------------------------------------------------------------

	cVector3f cNode3D::GetLocalPosition()
	{
		return m_mtxLocalTransform.GetTranslation();
	}

	//-----------------------------------------------------------------------

	cMatrixf& cNode3D::GetLocalMatrix()
	{
		return m_mtxLocalTransform;
	}

	//-----------------------------------------------------------------------

	cVector3f cNode3D::GetWorldPosition()
	{
		UpdateWorldTransform();

		return m_mtxWorldTransform.GetTranslation();
	}

	//-----------------------------------------------------------------------

	cMatrixf& cNode3D::GetWorldMatrix()
	{
		UpdateWorldTransform();

		return m_mtxWorldTransform;
	}

	//-----------------------------------------------------------------------


	void cNode3D::SetPosition(const cVector3f& avPos)
	{
		m_mtxLocalTransform.m[0][3] = avPos.x;
		m_mtxLocalTransform.m[1][3] = avPos.y;
		m_mtxLocalTransform.m[2][3] = avPos.z;

		SetWorldTransformUpdated();
	}

	void cNode3D::SetMatrix(const cMatrixf& a_mtxTransform, bool abSetChildrenUpdated)
	{
		m_mtxLocalTransform = a_mtxTransform;

		if(abSetChildrenUpdated)
			SetWorldTransformUpdated();
		else
			mbTransformUpdated = true;
	}

	//-----------------------------------------------------------------------

	void cNode3D::SetWorldPosition(const cVector3f& avWorldPos)
	{
		if(mpParent)
		{
			SetPosition(avWorldPos - mpParent->GetWorldPosition());
		}
		else
		{
			SetPosition(avWorldPos);
		}
	}

	//-----------------------------------------------------------------------


	void cNode3D::SetWorldMatrix(const cMatrixf& a_mtxWorldTransform)
	{
		if(mpParent)
		{
			SetMatrix(cMath::MatrixMul(cMath::MatrixInverse(mpParent->GetWorldMatrix()),
										a_mtxWorldTransform));
		}
		else
		{
			SetMatrix(a_mtxWorldTransform);
		}
	}

	//-----------------------------------------------------------------------

	const char* cNode3D::GetName()
	{
		return msName.c_str();
	}

	//-----------------------------------------------------------------------

	cNode3D* cNode3D::GetParent()
	{
		return mpParent;
	}

	//-----------------------------------------------------------------------

	void cNode3D::SetParent(cNode3D* apNode)
	{
		mpParent = apNode;

		mpParent->mlstNode.push_back(this);
	}

	void cNode3D::AddChild(cNode3D* apChild)
	{
		mlstNode.push_back(apChild);
	}

	//-----------------------------------------------------------------------

	void cNode3D::AddRotation(const cVector3f& avRot, eEulerRotationOrder aOrder)
	{
		m_mtxRotation = cMath::MatrixMul(cMath::MatrixRotate(avRot, aOrder), m_mtxRotation);
	}

	void cNode3D::AddRotation(const cQuaternion& aqRotation)
	{
		//Would be better to have the rotation as a quaternion and
		//then just multiply so that it NOT becomes order dependant.
		m_mtxRotation = cMath::MatrixMul(cMath::MatrixQuaternion(aqRotation), m_mtxRotation);
	}

	void cNode3D::AddScale(const cVector3f& avScale)
	{
		mvScale *= avScale;
	}

	void cNode3D::AddTranslation(const cVector3f& avTrans)
	{
		mvTranslation += avTrans;
	}

	void cNode3D::UpdateMatrix(bool abSetChildrenUpdated)
	{
		cMatrixf mtxTransform = GetLocalMatrix();

		//Save the translation and set it to 0 so that only the rotation is altered.
		cVector3f vPos = mtxTransform.GetTranslation();
		mtxTransform.SetTranslation(cVector3f(0,0,0));

		//Log("Startpos: %s",vPos.ToString().c_str());
		//Log("World pos: %s\n",GetWorldMatrix().GetTranslation().ToString().c_str());

		//The animation rotation is applied before the local.
		mtxTransform = cMath::MatrixMul(mtxTransform,m_mtxRotation);

		//Skip scale for now.
		//mtxTransform = cMath::MatrixMul(cMath::MatrixScale(mvScale), mtxTransform);

		mtxTransform.SetTranslation(vPos + mvTranslation);

		SetMatrix(mtxTransform,abSetChildrenUpdated);

		//Log("World pos: %s\n",GetWorldMatrix().GetTranslation().ToString().c_str());

		//Reset values
		m_mtxRotation = cMatrixf::Identity;
		mvScale = cVector3f(1,1,1);
		mvTranslation = cVector3f(0,0,0);
	}

	//-----------------------------------------------------------------------

	void cNode3D::SetSource(const tString &asSource)
	{
		msSource = asSource;
	}

	const char* cNode3D::GetSource()
	{
		return msSource.c_str();
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	void cNode3D::UpdateWorldTransform()
	{
		if(mbTransformUpdated)
		{
			//if(msName == "WeaponJoint") LogUpdate("  update world transform!\n");

			mbTransformUpdated = false;

			if(mpParent)
			{
				m_mtxWorldTransform = cMath::MatrixMul(mpParent->GetWorldMatrix(),m_mtxLocalTransform);
			}
			else
			{
				m_mtxWorldTransform = m_mtxLocalTransform;
			}
		}
	}
	//-----------------------------------------------------------------------

	void cNode3D::SetWorldTransformUpdated()
	{
		//if(msName == "WeaponJoint") LogUpdate("  setworldtransform updated!\n");

		mbTransformUpdated = true;

		//Set all entities as updated
		tEntityListIt EIt = mlstEntity.begin();
		for(;EIt != mlstEntity.end();++EIt)
		{
			iEntity3D* pEntity = static_cast<iEntity3D*>(*EIt);

			pEntity->SetTransformUpdated();
		}

		//Set all child nodes as updated
		tNodeListIt NIt = mlstNode.begin();
		for(;NIt != mlstNode.end();++NIt)
		{
			cNode3D* pNode = static_cast<cNode3D*>(*NIt);

			pNode->SetWorldTransformUpdated();
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// SAVE OBJECT STUFF
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	kBeginSerialize(cSaveData_cNode3D,cSaveData_iNode)
	kSerializeVar(msName,eSerializeType_String)
	kSerializeVar(msSource,eSerializeType_String)
	kSerializeVar(mbAutoDeleteChildren,eSerializeType_Bool)
	kSerializeVar(m_mtxLocalTransform,eSerializeType_Matrixf)
	kSerializeVar(mlParentId,eSerializeType_Int32)
	kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject* cSaveData_cNode3D::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler,cGame *apGame)
	{
		return hplNew( cNode3D,(msName,mbAutoDeleteChildren) );
	}

	int cSaveData_cNode3D::GetSaveCreatePrio()
	{
		return 0;
	}

	//-----------------------------------------------------------------------

	iSaveData* cNode3D::CreateSaveData()
	{
		return hplNew( cSaveData_cNode3D,() );
	}

	//-----------------------------------------------------------------------


	void cNode3D::SaveToSaveData(iSaveData *apSaveData)
	{
		kSaveData_SaveToBegin(cNode3D);

		kSaveData_SaveTo(msName);
		kSaveData_SaveTo(msSource);
		kSaveData_SaveTo(mbAutoDeleteChildren);
		kSaveData_SaveTo(m_mtxLocalTransform);

		kSaveData_SaveObject(mpParent,mlParentId);
	}

	//-----------------------------------------------------------------------

	void cNode3D::LoadFromSaveData(iSaveData *apSaveData)
	{
		kSaveData_LoadFromBegin(cNode3D);

		kSaveData_LoadFrom(msName);
		kSaveData_LoadFrom(msSource);
		kSaveData_LoadFrom(mbAutoDeleteChildren);
		kSaveData_LoadFrom(m_mtxLocalTransform);
	}

	//-----------------------------------------------------------------------

	void cNode3D::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame)
	{
		kSaveData_SetupBegin(cNode3D);

		kSaveData_LoadObject(mpParent,mlParentId,cNode3D*);

	}

	//-----------------------------------------------------------------------


}
