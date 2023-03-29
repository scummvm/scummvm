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
 * This file is part of Penumbra Overture.
 */

#include "hpl1/penumbra-overture/PlayerHands.h"

#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHelper.h"

#include "hpl1/penumbra-overture/HudModel_Throw.h"
#include "hpl1/penumbra-overture/HudModel_Weapon.h"

//////////////////////////////////////////////////////////////////////////
// HELP FUNCTIONS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cHudModelPose GetPoseFromElem(const tString &asName, TiXmlElement *apElem) {
	cHudModelPose hudPose;

	tString sAttrPos = asName + "_Pos";
	tString sAttrRot = asName + "_Rot";

	hudPose.mvPos = cString::ToVector3f(apElem->Attribute(sAttrPos.c_str()), 0);
	hudPose.mvRot = cMath::Vector3ToRad(
		cString::ToVector3f(apElem->Attribute(sAttrRot.c_str()), 0));

	return hudPose;
}

//-----------------------------------------------------------------------

cMatrixf InterpolatePosesToMatrix(float afT, const cHudModelPose &aPoseA,
								  const cHudModelPose &aPoseB) {
	cVector3f vPos = aPoseA.mvPos * (1 - afT) + aPoseB.mvPos * afT;

	cMatrixf mtxRotA = cMath::MatrixRotate(aPoseA.mvRot, eEulerRotationOrder_XYZ);
	cMatrixf mtxRotB = cMath::MatrixRotate(aPoseB.mvRot, eEulerRotationOrder_XYZ);

	cQuaternion qA;
	qA.FromRotationMatrix(mtxRotA);
	cQuaternion qB;
	qB.FromRotationMatrix(mtxRotB);

	cQuaternion qFinal = cMath::QuaternionSlerp(afT, qA, qB, true);

	cMatrixf mtxFinal = cMath::MatrixQuaternion(qFinal);
	mtxFinal.SetTranslation(vPos);

	return mtxFinal;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// HUD MODEL
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iHudModel::iHudModel(ePlayerHandType aType) {
	mType = aType;
	mState = eHudModelState_Idle;
}

//-----------------------------------------------------------------------

void iHudModel::LoadEntities() {
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	// Mesh entity
	mpMesh->IncUserCount(); // entity is newer user.
	mpEntity = pWorld->CreateMeshEntity(msName, mpMesh);

	// Create lights
	for (int i = 0; i < mpMesh->GetLightNum(); i++) {
		iLight3D *pLight = mpMesh->CreateLightInWorld(msName, mpMesh->GetLight(i), mpEntity, pWorld);
		if (pLight) {
			mvLightColors.push_back(pLight->GetDiffuseColor());
			mvLightRadii.push_back(pLight->GetFarAttenuation());
			mvLights.push_back(pLight);
		}
	}

	// Create billboards
	for (int i = 0; i < mpMesh->GetBillboardNum(); i++) {
		cBillboard *pBill = mpMesh->CreateBillboardInWorld(msName, mpMesh->GetBillboard(i), mpEntity, pWorld);
		if (pBill)
			mvBillboards.push_back(pBill);
	}

	// Create particle systems
	for (int i = 0; i < mpMesh->GetParticleSystemNum(); i++) {
		cParticleSystem3D *pPS = mpMesh->CreateParticleSystemInWorld(msName, mpMesh->GetParticleSystem(i), mpEntity, pWorld);
		if (pPS)
			mvParticleSystems.push_back(pPS);
	}

	// Create sounds entities
	for (int i = 0; i < mpMesh->GetSoundEntityNum(); i++) {
		cSoundEntity *pSound = mpMesh->CreateSoundEntityInWorld(msName, mpMesh->GetSoundEntity(i), mpEntity, pWorld);
		if (pSound)
			mvSoundEntities.push_back(pSound);
	}

	LoadExtraEntites();
}

//-----------------------------------------------------------------------

void iHudModel::DestroyEntities() {
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	// Mesh entity
	pWorld->DestroyMeshEntity(mpEntity);
	mpEntity = NULL;

	// Particle systems
	for (size_t i = 0; i < mvParticleSystems.size(); ++i) {
		pWorld->DestroyParticleSystem(mvParticleSystems[i]);
	}
	mvParticleSystems.clear();

	// Billboards
	for (size_t i = 0; i < mvBillboards.size(); ++i) {
		pWorld->DestroyBillboard(mvBillboards[i]);
	}
	mvBillboards.clear();

	// Lights
	for (size_t i = 0; i < mvLights.size(); ++i) {
		pWorld->DestroyLight(mvLights[i]);
	}
	mvLights.clear();

	// Sound entities
	for (size_t i = 0; i < mvSoundEntities.size(); ++i) {
		if (pWorld->SoundEntityExists(mvSoundEntities[i]))
			pWorld->DestroySoundEntity(mvSoundEntities[i]);
	}
	mvSoundEntities.clear();

	DestroyExtraEntities();
}

//-----------------------------------------------------------------------

void iHudModel::Reset() {
	mfTime = 0;
	ResetExtraData();
}

//-----------------------------------------------------------------------

void iHudModel::EquipEffect(bool abJustCreated) {
	if (msEquipSound != "") {
		cSoundHandler *pSoundHanlder = mpInit->mpGame->GetSound()->GetSoundHandler();
		pSoundHanlder->PlayGui(msEquipSound, false, 1);
	}

	for (size_t i = 0; i < mvLights.size(); ++i) {
		if (abJustCreated)
			mvLights[i]->SetDiffuseColor(cColor(0, 0));
		mvLights[i]->FadeTo(mvLightColors[i], mvLightRadii[i], 0.3f);
	}
}

void iHudModel::UnequipEffect() {
	if (msUnequipSound != "") {
		cSoundHandler *pSoundHanlder = mpInit->mpGame->GetSound()->GetSoundHandler();
		pSoundHanlder->PlayGui(msUnequipSound, false, 1);
	}

	for (size_t i = 0; i < mvLights.size(); ++i) {
		mvLights[i]->FadeTo(cColor(0, 0), mvLightRadii[i], 0.3f);
	}
}

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerHands::cPlayerHands(cInit *apInit) : iUpdateable("FadeHandler") {
	mpInit = apInit;

	mpMeshManager = mpInit->mpGame->GetResources()->GetMeshManager();

	mlMaxPositions = 3;
	mlMaxRotations = 16;

	mlCurrentModelNum = 2;

	for (int i = 0; i < mlCurrentModelNum; ++i) {
		mvCurrentHudModels[i] = NULL;
	}
}

//-----------------------------------------------------------------------

cPlayerHands::~cPlayerHands(void) {
	///////////////////////////////////////////
	// Replace this and use some cache instead
	tHudModelMapIt it = m_mapHudModels.begin();
	for (; it != m_mapHudModels.end(); ++it) {
		iHudModel *pHandModel = it->second;

		mpMeshManager->Destroy(pHandModel->mpMesh);
	}

	STLMapDeleteAll(m_mapHudModels);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPlayerHands::OnStart() {
}

//-----------------------------------------------------------------------

void cPlayerHands::Update(float afTimeStep) {
	UpdatePrevPostions();

	///////////////////////////////////
	// Get the camera properties
	cCamera3D *pCam = mpInit->mpPlayer->GetCamera();

	cVector3f vRot = cVector3f(pCam->GetPitch(), pCam->GetYaw(), pCam->GetRoll());
	cMatrixf mtxSmoothCam = cMath::MatrixRotate(vRot * -1.0f, eEulerRotationOrder_YXZ);
	cVector3f vUp = mtxSmoothCam.GetUp();                   // pCam->GetUp();
	cVector3f vRight = mtxSmoothCam.GetRight();             // pCam->GetRight();
	cVector3f vForward = mtxSmoothCam.GetForward() * -1.0f; // pCam->GetForward();

	/////////////////////////////////////
	// Update the current model
	for (int i = 0; i < mlCurrentModelNum; ++i) {
		iHudModel *pHudModel = mvCurrentHudModels[i];
		if (pHudModel == NULL)
			continue;

		cMatrixf mtxPose;

		////////////////////
		// Update state
		switch (pHudModel->mState) {
		// Idle
		case eHudModelState_Idle: {
			if (pHudModel->UpdatePoseMatrix(mtxPose, afTimeStep) == false) {
				mtxPose = cMath::MatrixRotate(pHudModel->mEquipPose.mvRot, eEulerRotationOrder_XYZ);
				mtxPose.SetTranslation(pHudModel->mEquipPose.mvPos);
			}
			break;
		}
		// Equip
		case eHudModelState_Equip: {
			float fT = cMath::Clamp(pHudModel->mfTime, 0, 1);
			mtxPose = InterpolatePosesToMatrix(fT, pHudModel->mUnequipPose, pHudModel->mEquipPose);

			pHudModel->mfTime += afTimeStep / pHudModel->mfEquipTime;
			if (pHudModel->mfTime >= 1) {
				pHudModel->mState = eHudModelState_Idle;
				pHudModel->mfTime = 1;
			}
			break;
		}
		// Unequip
		case eHudModelState_Unequip: {
			float fT = cMath::Clamp(pHudModel->mfTime, 0, 1);
			mtxPose = InterpolatePosesToMatrix(fT, pHudModel->mEquipPose, pHudModel->mUnequipPose);

			pHudModel->mfTime += afTimeStep / pHudModel->mfUnequipTime;
			if (pHudModel->mfTime >= 1) {
				// Log("Creating next model and destroying current!\n");

				pHudModel->mState = eHudModelState_Idle;
				pHudModel->mfTime = 0;

				pHudModel->DestroyEntities();

				mvCurrentHudModels[i] = NULL;

				if (pHudModel->msNextModel != "") {
					SetCurrentModel(i, pHudModel->msNextModel);
				}
				pHudModel->Reset();
				continue;
			}
			break;
		}
		}

		////////////////////
		// Set rotation
		cMatrixf mtxTransform = cMath::MatrixMul(
			cMath::MatrixRotate(mvSmoothCameraRot, eEulerRotationOrder_XYZ),
			mtxPose.GetRotation());

		// pHudModel->mpEntity->SetMatrix(mtxRot);

		/////////////////////////
		// Set position
		const cVector3f &vLocalPos = mtxPose.GetTranslation();
		cVector3f vRealLocalPos = vUp * vLocalPos.y +
								  vRight * vLocalPos.x +
								  vForward * vLocalPos.z +
								  cVector3f(0, -mpInit->mpPlayer->GetHeadMove()->GetPos() * 0.1f, 0);
		;

		mtxTransform.SetTranslation(pCam->GetPosition() + vRealLocalPos);

		pHudModel->mpEntity->SetMatrix(mtxTransform);
	}
}

//-----------------------------------------------------------------------

void cPlayerHands::Reset() {
	for (int i = 0; i < mlCurrentModelNum; ++i) {
		iHudModel *pHudModel = mvCurrentHudModels[i];
		if (pHudModel) {
			pHudModel->DestroyEntities();
		}

		mvCurrentHudModels[i] = NULL;
	}

	tHudModelMapIt it = m_mapHudModels.begin();
	for (; it != m_mapHudModels.end(); ++it) {
		iHudModel *pModel = it->second;
		pModel->Reset();
	}
}

//-----------------------------------------------------------------------

void cPlayerHands::OnDraw() {
}

//-----------------------------------------------------------------------

void cPlayerHands::AddHudModel(iHudModel *apHudModel) {
	apHudModel->mpMesh = mpMeshManager->CreateMesh(apHudModel->msModelFile);
	apHudModel->mpInit = mpInit;

	m_mapHudModels.insert(tHudModelMap::value_type(cString::ToLowerCase(apHudModel->msName),
												   apHudModel));
}

//-----------------------------------------------------------------------

static ePlayerHandType ToHandType(const char *apString) {
	if (apString == NULL)
		return ePlayerHandType_Normal;

	tString sType = cString::ToLowerCase(apString);

	if (sType == "normal")
		return ePlayerHandType_Normal;
	else if (sType == "weaponmelee")
		return ePlayerHandType_WeaponMelee;
	else if (sType == "throw")
		return ePlayerHandType_Throw;

	return ePlayerHandType_Normal;
}

////////////////////////////////////

bool cPlayerHands::AddModelFromFile(const tString &asFile) {
	tString sFileName = cString::SetFileExt(asFile, "hud");
	tString sPath = mpInit->mpGame->GetResources()->GetFileSearcher()->GetFilePath(sFileName);
	if (sPath == "") {
		Error("Couldn't find '%s' in resource directories!\n", sFileName.c_str());
		return false;
	}

	////////////////////////////////////////////////
	// Load the document
	TiXmlDocument *pXmlDoc = hplNew(TiXmlDocument, (sPath.c_str()));
	if (pXmlDoc->LoadFile() == false) {
		Error("Couldn't load XML document '%s'\n", sPath.c_str());
		hplDelete(pXmlDoc);
		return false;
	}

	////////////////////////////////////////////////
	// Load the root
	TiXmlElement *pRootElem = pXmlDoc->FirstChildElement();
	if (pRootElem == NULL) {
		Error("Couldn't load root from XML document '%s'\n", sPath.c_str());
		hplDelete(pXmlDoc);
		return false;
	}

	////////////////////////////////////////////////
	// Load the MAIN element.
	TiXmlElement *pMainElem = pRootElem->FirstChildElement("MAIN");
	if (pMainElem == NULL) {
		Error("Couldn't load MAIN element from XML document '%s'\n", sPath.c_str());
		hplDelete(pXmlDoc);
		return false;
	}

	ePlayerHandType handType = ToHandType(pMainElem->Attribute("Type"));
	iHudModel *pHudModel = NULL;

	switch (handType) {
	case ePlayerHandType_Normal:
		pHudModel = hplNew(cHudModel_Normal, ());
		break;
	case ePlayerHandType_WeaponMelee:
		pHudModel = hplNew(cHudModel_WeaponMelee, ());
		break;
	case ePlayerHandType_Throw:
		pHudModel = hplNew(cHudModel_Throw, ());
		break;
	default:
		break;
	}

	pHudModel->msName = cString::ToString(pMainElem->Attribute("Name"), "");
	pHudModel->msModelFile = cString::ToString(pMainElem->Attribute("ModelFile"), "");
	pHudModel->mfEquipTime = cString::ToFloat(pMainElem->Attribute("EquipTime"), 0.3f);
	pHudModel->mfUnequipTime = cString::ToFloat(pMainElem->Attribute("UnequipTime"), 0.3f);

	pHudModel->mEquipPose = GetPoseFromElem("EquipPose", pMainElem);

	pHudModel->mUnequipPose = GetPoseFromElem("UnequipPose", pMainElem);

	pHudModel->msEquipSound = cString::ToString(pMainElem->Attribute("EquipSound"), "");
	pHudModel->msUnequipSound = cString::ToString(pMainElem->Attribute("UnequipSound"), "");

	pHudModel->LoadData(pRootElem);

	AddHudModel(pHudModel);

	hplDelete(pXmlDoc);
	return true;
}

//-----------------------------------------------------------------------

void cPlayerHands::SetCurrentModel(int alNum, const tString &asName) {
	// Log("Setting current %d to '%s'\n",alNum,asName.c_str());

	//////////////////////////////////////////////
	// Check so that it is not already equipped
	if (mvCurrentHudModels[alNum] &&
		cString::ToLowerCase(asName) == cString::ToLowerCase(mvCurrentHudModels[alNum]->msName) &&
		mvCurrentHudModels[alNum]->mState == eHudModelState_Idle) {
		// Log(" model already active!\n");
		return;
	}

	///////////////////////////
	// Add a newer hud model
	if (asName != "") {
		tHudModelMapIt it = m_mapHudModels.find(cString::ToLowerCase(asName));
		if (it == m_mapHudModels.end()) {
			Log(" Couldn't find hud model '%s'!\n", asName.c_str());
			return;
		}

		iHudModel *pHandModel = it->second;

		/*cWorld3D *pWorld = */ mpInit->mpGame->GetScene()->GetWorld3D();

		if (mvCurrentHudModels[alNum]) {
			if (mvCurrentHudModels[alNum] != pHandModel) {
				if (mvCurrentHudModels[alNum]->mState != eHudModelState_Unequip) {
					mvCurrentHudModels[alNum]->UnequipEffect();

					mvCurrentHudModels[alNum]->mState = eHudModelState_Unequip;
					mvCurrentHudModels[alNum]->mfTime = 1.0f - mvCurrentHudModels[alNum]->mfTime;
				}
				mvCurrentHudModels[alNum]->msNextModel = asName;
				// Log(" Unequipping %s, time: %f\n",mvCurrentHudModels[alNum]->msName.c_str(),
				//							mvCurrentHudModels[alNum]->mfTime);
			} else {
				pHandModel->EquipEffect(false);

				mvCurrentHudModels[alNum]->mState = eHudModelState_Equip;
				mvCurrentHudModels[alNum]->mfTime = 1.0f - mvCurrentHudModels[alNum]->mfTime;
				mvCurrentHudModels[alNum]->msNextModel = asName;
			}
		} else {
			pHandModel->LoadEntities();
			pHandModel->EquipEffect(true);

			if (mvCurrentHudModels[alNum] == pHandModel) {
				pHandModel->mfTime = 1.0f - pHandModel->mfTime;
			} else {
				pHandModel->mfTime = 0;
			}

			pHandModel->mState = eHudModelState_Equip;

			mvCurrentHudModels[alNum] = pHandModel;
		}
	}
	///////////////////////////
	// Only remove old
	else {
		if (mvCurrentHudModels[alNum]) {
			mvCurrentHudModels[alNum]->UnequipEffect();

			mvCurrentHudModels[alNum]->mState = eHudModelState_Unequip;
			mvCurrentHudModels[alNum]->mfTime = 1.0f - mvCurrentHudModels[alNum]->mfTime;
			mvCurrentHudModels[alNum]->msNextModel = asName;
			// Log(" Destroying old\n");
		} else {
			// Log("No old to destroy!!!\n");
		}
	}
}

//-----------------------------------------------------------------------

iHudModel *cPlayerHands::GetModel(const tString &asName) {
	tHudModelMapIt it = m_mapHudModels.find(cString::ToLowerCase(asName));
	if (it == m_mapHudModels.end()) {
		return NULL;
	}

	return it->second;
}

//-----------------------------------------------------------------------

void cPlayerHands::OnWorldExit() {
	for (int i = 0; i < mlCurrentModelNum; ++i) {
		iHudModel *pHudModel = mvCurrentHudModels[i];
		if (pHudModel) {
			pHudModel->DestroyEntities();
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerHands::OnWorldLoad() {
	mlstRotations.clear();
	mlstPositions.clear();

	for (int i = 0; i < mlCurrentModelNum; ++i) {
		iHudModel *pHudModel = mvCurrentHudModels[i];
		if (pHudModel) {
			pHudModel->LoadEntities();
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
void cPlayerHands::UpdatePrevPostions() {
	///////////////////////////////////
	// Get current position
	cCamera3D *pCam = mpInit->mpPlayer->GetCamera();
	cVector3f vCamRotation(pCam->GetPitch(), pCam->GetYaw(), 0);
	cVector3f vCamPosition = pCam->GetPosition();

	mlstRotations.push_back(vCamRotation);
	mlstPositions.push_back(vCamPosition);

	// Delete if there are too many values stored.
	if ((int)mlstPositions.size() > mlMaxPositions)
		mlstPositions.pop_front();
	if ((int)mlstRotations.size() > mlMaxRotations)
		mlstRotations.pop_front();

	///////////////////////////////////////
	// Get the current camera postion and rotation
	cVector3f vRotation(0, 0, 0);
	cVector3f vPosition(0, 0, 0);
	float fRotNum = 0;
	float fPosNum = 0;

	// float fRotMulStart = 1.0f;
	// float fRotMulEnd = 0.1f;
	// float fSize = (float)mlstRotations.size();
	// float fD = (fRotMulStart - fRotMulEnd) / fSize;
	float fMul = 1.0f; // fRotMulEnd;

	for (tVector3fListIt it = mlstRotations.begin(); it != mlstRotations.end(); ++it) {
		vRotation += *it * fMul;
		fRotNum += fMul;
		// fMul += fD;
	}

	for (tVector3fListIt it = mlstPositions.begin(); it != mlstPositions.end(); ++it) {
		vPosition += *it;
		fPosNum++;
	}

	mvSmoothCameraPos = vCamPosition; // vPosition / fPosNum;
	mvSmoothCameraRot = vRotation / fRotNum;
}

//-----------------------------------------------------------------------
