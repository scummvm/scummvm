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

#include "hpl1/penumbra-overture/HapticGameCamera.h"

#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/GameEntity.h"
#include "hpl1/penumbra-overture/GameLiquidArea.h"
#include "hpl1/penumbra-overture/ButtonHandler.h"

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cHapticGameCamera::cHapticGameCamera(cInit *apInit, cPlayer *apPlayer)
{
	mpInit = apInit;
	mpLowLevelHaptic = mpInit->mpGame->GetHaptic()->GetLowLevel();
	mpScene = mpInit->mpGame->GetScene();

	mfFrameW = 200;
	mfFrameH = 180;

	mbUseFrame = true;

	mfFrameForceStrength = 0.0002f;
	mfFrameMoveSpeedX =	0.00033f;
	mfFrameMoveSpeedY = 0.0002f;

	mfInteractModeCameraSpeed = 1;
	mfActionModeCameraSpeed = 1;

	mfFrameMaxForce = 0.3f;//1;
	mfFrameMaxMoveSpeed = 0.1f;

	mvCentrePos = cVector3f(0,-25,0);

	mpForce = apInit->mpGame->GetHaptic()->GetLowLevel()->CreateImpulseForce(0);
	mpForce->SetForce(0);

	mpVtxProgram = mpInit->mpGame->GetResources()->GetGpuProgramManager()->CreateProgram(
												"Diffuse_Color_vp.cg","main",
												eGpuProgramType_Vertex);

	mpPlayer = apPlayer;

	Reset();
}

//-----------------------------------------------------------------------

cHapticGameCamera::~cHapticGameCamera()
{
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cHapticGameCamera::OnWorldLoad()
{
	cMesh *pMesh = mpInit->mpGame->GetResources()->GetMeshManager()->CreateMesh("player_gui_hand.dae");
	mpHandEntity = mpScene->GetWorld3D()->CreateMeshEntity("Hand",pMesh);

	float fRadius = mpLowLevelHaptic->GetProxyRadius();
	mpHandEntity->SetMatrix(cMath::MatrixScale(fRadius));
	mpHandEntity->SetVisible(mbHandVisible);
	mpHandEntity->SetCastsShadows(true);

	mpHandShape = mpScene->GetWorld3D()->GetPhysicsWorld()->CreateSphereShape(GetHandReachRadius(),NULL);
    	
	mvPrevPrxoyPos = mpLowLevelHaptic->GetProxyPosition();

	//Disable all shapes
	cHapticShapeIterator it = mpLowLevelHaptic->GetShapeIterator();
	while(it.HasNext())
	{
		iHapticShape *pShape = it.Next();
		pShape->SetEnabled(false);
	}
}

void cHapticGameCamera::OnWorldExit()
{
	mpHandShape = NULL;
	mpHandEntity = NULL;
}

//-----------------------------------------------------------------------

void cHapticGameCamera::Reset()
{	
	mpHandShape = NULL;
	mpHandEntity = NULL;

	mbIsTurning = false;

	mbActive = true;
	mbCamMove = true;

	mbHandVisible = true;

	mbRenderActive = true;

	mlContactCount =0;

	mbAtEdge = false;

	mType = eHapticGameCameraType_Frame;
}

//-----------------------------------------------------------------------

void cHapticGameCamera::SetActive(bool abX)
{
	if(mbActive == abX) return;

	mbActive = abX;

	if(mbActive)
	{
		if(mpHandEntity) mpHandEntity->SetVisible(mbHandVisible);
		mpInit->mpGame->GetHaptic()->GetLowLevel()->SetRenderingActive(mbRenderActive);
	}
	else
	{
		mpForce->SetActive(false);
		if(mpHandEntity)mpHandEntity->SetVisible(false);
		mpInit->mpGame->GetHaptic()->GetLowLevel()->SetRenderingActive(false);
	}
}

//-----------------------------------------------------------------------

void cHapticGameCamera::SetType(eHapticGameCameraType aType)
{
	mType = aType;
}

//-----------------------------------------------------------------------

void cHapticGameCamera::Update(float afTimeStep)
{	
	if(	mpPlayer->IsActive() == false || 
		mpInit->mpButtonHandler->GetState() != eButtonHandlerState_Game)
	{
		//Log("Player is not active!!\n");
		mpForce->SetActive(false);
		mpLowLevelHaptic->SetRenderingActive(false);
		if(mpHandEntity) mpHandEntity->SetActive(false);
		return;
	}
	
	if(mbActive==false){
		mpLowLevelHaptic->SetRenderingActive(false);
		return;
	}
	
	if(mpHandEntity) mpHandEntity->SetActive(mbHandVisible);
	if(mbHandVisible && mbAtEdge==false)
		mpLowLevelHaptic->SetRenderingActive(mbRenderActive);
	else
		mpLowLevelHaptic->SetRenderingActive(false);
	
	
	UpdateCameraOrientation(afTimeStep);
	UpdateHand(afTimeStep);
	UpdateProxyCovered(afTimeStep);
	UpdateProxyInteraction(afTimeStep);

	cInput *pInput = mpInit->mpGame->GetInput();
	if(pInput->IsTriggerd("InteractMode"))
	{
		mpLowLevelHaptic->SetRenderingActive(false);
	}
}


//-----------------------------------------------------------------------

void cHapticGameCamera::OnPostSceneDraw()
{
	if(mpHandEntity==NULL) return;
	if(mbHandVisible==false) return;
	if(mbActive==false) return;
	if(mpHandEntity->IsVisible()==false) return;

	cCamera3D *pCam = static_cast<cCamera3D*>(mpScene->GetCamera());
	iLowLevelGraphics *pLowGfx = mpInit->mpGame->GetGraphics()->GetLowLevel();
	eCrossHairState crossHairState = mpPlayer->GetCrossHairState();

	if(	crossHairState != eCrossHairState_Inactive &&
		crossHairState != eCrossHairState_None)
	{
		pLowGfx->SetMatrix(eMatrix_ModelView,cMath::MatrixMul(	pCam->GetViewMatrix(),
																mpHandEntity->GetWorldMatrix()));
		cSubMesh *pSubMesh = mpHandEntity->GetMesh()->GetSubMesh(0);
		iVertexBuffer *pVtxBuff = pSubMesh->GetVertexBuffer();

		pLowGfx->SetDepthTestActive(true);
		pLowGfx->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);
		pLowGfx->SetBlendActive(true);
		pLowGfx->SetBlendFunc(eBlendFunc_One,eBlendFunc_One);
		pLowGfx->SetTexture(0, pSubMesh->GetMaterial()->GetTexture(eMaterialTexture_Diffuse));

		mpVtxProgram->Bind();
		mpVtxProgram->SetMatrixf(	"worldViewProj",eGpuProgramMatrix_ViewProjection,
									eGpuProgramMatrixOp_Identity);

		pVtxBuff->Bind();
		pVtxBuff->Draw();
		pVtxBuff->Draw();
		pVtxBuff->UnBind();

		mpVtxProgram->UnBind();

		pLowGfx->SetTexture(0, NULL);
		pLowGfx->SetBlendActive(false);
		pLowGfx->SetDepthTestActive(true);
	}
}

//-----------------------------------------------------------------------

bool cHapticGameCamera::ShowCrosshair()
{
	if(mbActive==false) return true;

	if(mbHandVisible==false) return true;

	return false;
}

//-----------------------------------------------------------------------

void cHapticGameCamera::SetHandVisible(bool abX)
{
	if(mbHandVisible == abX) return;

	mbHandVisible = abX;
	
	if(mpHandEntity)
	{
		mpHandEntity->SetVisible(mbHandVisible);
	}
}

bool cHapticGameCamera::GetHandVisible()
{
	return mbHandVisible;
}


/**
 * The radius from proxy center that will be able to affect entities.
 */
float cHapticGameCamera::GetHandReachRadius()
{
	return mpLowLevelHaptic->GetProxyRadius()*7.0f;
}

//-----------------------------------------------------------------------

void cHapticGameCamera::SetRenderActive(bool abX)
{
	if(mbRenderActive == abX) return;

	mbRenderActive = abX;

    if(mbActive) mpLowLevelHaptic->SetRenderingActive(mbRenderActive);
}

//-----------------------------------------------------------------------

void cHapticGameCamera::SetInteractModeCameraSpeed(float afSpeed)
{
	mfInteractModeCameraSpeed = afSpeed;
	mfFrameMoveSpeedX =	0.00033f * mfInteractModeCameraSpeed;
	mfFrameMoveSpeedY = 0.0002f * mfInteractModeCameraSpeed;
}
void cHapticGameCamera::SetActionModeCameraSpeed(float afSpeed)
{
	mfActionModeCameraSpeed = afSpeed;
}


//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------

void cHapticGameCamera::UpdateCameraOrientation(float afTimeStep)
{
	cCamera3D *pCamera = static_cast<cCamera3D*>(mpScene->GetCamera());
	cVector3f vLocalPoxy = cMath::MatrixMul(pCamera->GetViewMatrix(), 
											mpLowLevelHaptic->GetProxyPosition());
	//Log("Local %s\n",vLocalPoxy.ToString().c_str());
	if(vLocalPoxy.z >-0.34){
		//mpLowLevelHaptic->SetRenderingActive(false);
		return;
	}

	mbAtEdge = false;

    ///////////////////////////
	// Frame Type
	cVector3f vForce(0);
	cVector2f vSpeed(0);
	if(mType == eHapticGameCameraType_Frame)
	{
		if(mbUseFrame==false) return;

		///////////////////////////////////
		//Get force and movement
		cVector2f vPos = mpLowLevelHaptic->GetProxyScreenPos(cVector2f(800,600));
		if(vPos.x < mfFrameW){ 
			vSpeed.x =	(mfFrameW - vPos.x) * -mfFrameMoveSpeedX; 
			vForce.x = (mfFrameW - vPos.x)*mfFrameForceStrength;
		}
		if(vPos.x > (800 - mfFrameW)){ 
			vSpeed.x = 	(vPos.x - (800 - mfFrameW)) *  mfFrameMoveSpeedX;
			vForce.x = (vPos.x - (800 - mfFrameW))*-mfFrameForceStrength;
		}
		if(vPos.y < mfFrameH){ 
			vSpeed.y  = (mfFrameH - vPos.y) *  -mfFrameMoveSpeedY;
			vForce.y = (mfFrameH - vPos.y)*-mfFrameForceStrength;
		}
		if(vPos.y > (600-mfFrameW)){ 
			vSpeed.y = (vPos.y - (600-mfFrameW)) *  mfFrameMoveSpeedY;
			vForce.y = (vPos.y - (600-mfFrameW))*mfFrameForceStrength;
		}

		///////////////////////////////////
		//Set Max speed
		if(vSpeed.x > mfFrameMaxMoveSpeed)	vSpeed.x = mfFrameMaxMoveSpeed;
		if(vSpeed.x < -mfFrameMaxMoveSpeed)	vSpeed.x = -mfFrameMaxMoveSpeed;
		if(vSpeed.y > mfFrameMaxMoveSpeed)	vSpeed.y = mfFrameMaxMoveSpeed;
		if(vSpeed.y < -mfFrameMaxMoveSpeed)	vSpeed.y = -mfFrameMaxMoveSpeed;

		if(mbCamMove) 
		{
			mpPlayer->AddYaw(vSpeed.x * mpInit->mfHapticMoveScreenSpeedMul);
			mpPlayer->AddPitch(vSpeed.y * mpInit->mfHapticMoveScreenSpeedMul);
		}


		///////////////////////////////////
		//Set Max Force
		if(vForce.x > mfFrameMaxForce)	vForce.x = mfFrameMaxForce;
		if(vForce.x < -mfFrameMaxForce)	vForce.x = -mfFrameMaxForce;
		if(vForce.y > mfFrameMaxForce)	vForce.y = mfFrameMaxForce;
		if(vForce.y < -mfFrameMaxForce)	vForce.y = -mfFrameMaxForce;

		////////////////////////////////////
		//Turn on Force
		if(vForce != 0)
		{
			mpForce->SetActive(true);
			mpForce->SetForce(vForce);

			//Do this better.
			mbHapticWasActive = mpLowLevelHaptic->IsRenderingActive();
			mbIsTurning = true;
		}
		else
		{
			mpForce->SetActive(false);

			mbIsTurning = false;
		}

		//////////////////////////////////////////
		//Turn off rendering
		bool bAtEdge = false;
		if(vPos.x < mfFrameW*.1f){ 
			bAtEdge = true;
		}
		if(vPos.x > (800 - mfFrameW*.1f)){ 
			bAtEdge = true;
		}
		if(vPos.y < mfFrameH*.1f){ 
			bAtEdge = true;
		}
		if(vPos.y > (600-mfFrameH*.1f)){ 
			bAtEdge = true;
		}

		mbAtEdge = bAtEdge;

		/*if(bAtEdge)
		{
			mpLowLevelHaptic->SetRenderingActive(false);	
		}
		else
		{
			mpLowLevelHaptic->SetRenderingActive(mbRenderActive);
		}*/
	}
	///////////////////////////
	// Centre Type
	else if(mType == eHapticGameCameraType_Centre)
	{
		cVector3f vDevicePos = mpLowLevelHaptic->GetHardwarePosition();
		cVector3f vDeltaPos = mvCentrePos - vDevicePos;
		
		float fLength = vDeltaPos.Length() -0.9f;
		

		if(fLength > 0.0f)
		{
			cVector3f vDir = cMath::Vector3Normalize(vDeltaPos);

			//Force
			cVector3f vForce2 = vDir * fLength * 0.012f;
			mpForce->SetActive(true);
			mpForce->SetForce(vForce2);

			//Movement
			if(mbCamMove) 
			{
				cVector3f vMov = vDir * fLength * 0.0015f * mfActionModeCameraSpeed;
				mpPlayer->AddPitch(vMov.y);
				mpPlayer->AddYaw(-vMov.x);
			}
		}
		else
		{
			mpForce->SetActive(false);
		}
	}
}
//-----------------------------------------------------------------------

void cHapticGameCamera::UpdateHand(float afTimeStep)
{
	///////////////////////////
	// GUI Hand
	cVector3f vProxyPos = mpLowLevelHaptic->GetProxyPosition();
		
	mlstProxyPos.push_back(vProxyPos);
	if(mlstProxyPos.size() > 8) mlstProxyPos.pop_front();

	vProxyPos =0;
	Common::List<cVector3f>::iterator it = mlstProxyPos.begin();
	for(; it != mlstProxyPos.end(); ++it)
	{
		vProxyPos += *it;
	}
	vProxyPos = vProxyPos / (float)mlstProxyPos.size();

	mpHandEntity->SetPosition(vProxyPos);

}
//-----------------------------------------------------------------------

void cHapticGameCamera::UpdateProxyCovered(float afTimeStep)
{
	/////////////////////////////////////
	// Check if proxy is covered
	/*if(false)//cHaptic::GetIsUsed())
	{
	if(mState == ePlayerState_Normal && mpHapticCamera->IsTurning()==false)
	{
	cVector3f vStart = mpCamera->GetPosition();
	cVector3f vEnd = mpHandEntity->GetWorldPosition();

	//Log("Casting ray!\n");
	gTempCheckProxy.mbCollided =false;
	pPhysicsWorld->CastRay(&gTempCheckProxy,vStart,vEnd,false,false,false);

	if(gTempCheckProxy.mbCollided)
	{
	//Log("Disable!\n");
	mpLowLevelHaptic->SetRenderingActive(false);
	}
	else
	{
	//Log("Enabel!\n");
	mpLowLevelHaptic->SetRenderingActive(true);
	}
	}
	}*/
}

//-----------------------------------------------------------------------

void cHapticGameCamera::UpdateProxyInteraction(float afTimeStep)
{
	cVector3f vProxyPos = mpLowLevelHaptic->GetProxyPosition();
	cVector3f vProxyVel = (vProxyPos - mvPrevPrxoyPos) / afTimeStep;
	float fProxySpeed = vProxyVel.Length();
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	bool bHadContact = false;

	/////////////////////////////////
	//Disable previously enabled 
	cHapticShapeIterator shapeIt = mpLowLevelHaptic->GetShapeIterator();
	while(shapeIt.HasNext())
	{
		iHapticShape *pShape = shapeIt.Next();
		pShape->SetEnabled(false);
	}

	//////////////////////////////////
	// Iterate physics bodies
	cBoundingVolume bv;
	bv.SetSize(GetHandReachRadius()*2);
	bv.SetPosition(vProxyPos);

	cPortalContainerEntityIterator it = pWorld->GetPortalContainer()->GetEntityIterator(&bv);
	while(it.HasNext())
	{
		iPhysicsBody *pBody = static_cast<iPhysicsBody*>(it.Next());
		
		if(pBody->IsCharacter()) continue;
		if(pBody->IsActive()==false) continue;

		if(pBody->GetHapticShape()==NULL)
		{
			if(cMath::PointBVCollision(vProxyPos,*pBody->GetBV())==false) continue;

			/////////////////////////////////
			//Special case for water
			iGameEntity *pEntity = (iGameEntity*)pBody->GetUserData();
			if(pEntity==NULL || pEntity->GetType() != eGameEntityType_LiquidArea) continue;
			
			cGameLiquidArea *pLiquid = static_cast<cGameLiquidArea*>(pEntity);
            
			bHadContact = true;

			if(	mlContactCount <=0)
			{
				float fY =	pBody->GetWorldPosition().y + 
							pBody->GetShape()->GetSize().y/2;

				cVector3f vWaterPos = vProxyPos; vProxyPos.y = fY;
				
				cSurfaceData *pSurface = pLiquid->GetPhysicsMaterial()->GetSurfaceData();
				cSurfaceImpactData *pImpact = pSurface->GetImpactDataFromSpeed(fProxySpeed * 0.3f);
				if(pImpact == NULL) continue;

				if(pImpact->GetPSName() != "")
				{
					pWorld->CreateParticleSystem(	"Splash", pImpact->GetPSName(),1, 
													cMath::MatrixTranslate(vWaterPos));
				}

				if(pImpact->GetSoundName() != "")
				{
					cSoundEntity *pSound = pWorld->CreateSoundEntity("Splash",
																	pImpact->GetSoundName(),true);
					if(pSound) pSound->SetPosition(vWaterPos);
				}
			}
			continue;
		}
		
		/////////////////////////////////////
		//Get Shape
		iHapticShape *pContactShape = pBody->GetHapticShape();

		pContactShape->SetEnabled(true);
		//mlstActiveShapes.push_back(pContactShape);

		//////////////////////////////////////
		//Check if any of the haptic shapes are touched.
		if(mpLowLevelHaptic->ShapeIsInContact(pContactShape))
		{
			bHadContact = true;
		}
		else
		{
			continue;
		}

		/////////////////////////////////////////
		// Add force to body and play material effects

		//Log("Body: %s Force: %s\n",	pBody->GetName().c_str(), 
			//						pContactShape->GetAppliedForce().ToString().c_str());

		
		//Force
		pBody->AddForceAtPosition(	pContactShape->GetAppliedForce() * 60.0f,//33.0f,
									vProxyPos);
		
        //Effects
		cSurfaceData *pSurface = pBody->GetMaterial()->GetSurfaceData();
		if(mlContactCount <=0 && pSurface)
		{
			//Sound
			cSurfaceImpactData* pImpact =  pSurface->GetImpactDataFromSpeed(fProxySpeed*0.3f);
			if(pImpact)
			{
				cSoundEntity *pSound = pWorld->CreateSoundEntity(	"Hit",pImpact->GetSoundName(),
																	true);
				pSound->SetWorldPosition(vProxyPos);
			}
				
			//Particle system (and other effects)
            pSurface->CreateImpactEffect(fProxySpeed*0.3f,vProxyPos,1,NULL);
		}
	}
	
	////////////////////////////
	//Update contact count
	if(bHadContact)
	{
		mlContactCount = 10;
	}
	else
	{
		mlContactCount--;
		if(mlContactCount<=0) mlContactCount=0;
	}

	mvPrevPrxoyPos = vProxyPos;
}

//-----------------------------------------------------------------------

