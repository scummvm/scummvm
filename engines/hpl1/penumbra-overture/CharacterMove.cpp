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

#include "hpl1/penumbra-overture/CharacterMove.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/GameEntity.h"

//////////////////////////////////////////////////////////////////////////
// A STAR CALLBHCK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cCharacterAStarCallback::cCharacterAStarCallback(cCharacterMove *apMove) {
	mbCheckDynamic = false;

	mpMove = apMove;
}

//-----------------------------------------------------------------------

bool cCharacterAStarCallback::CanAddNode(cAINode *apParentNode, cAINode *apChildNode) {
	if (mbCheckDynamic == false && mpMove->GetMaxDoorToughness() == -1)
		return true;

	bool bRet = mpMove->GetNodeContainer()->FreePath(apParentNode->GetPosition(), apChildNode->GetPosition(),
													 1, 0, this);
	// Log("Checking %s -> %s, ret: %d \n",apParentNode->GetName().c_str(),
	//									apChildNode->GetName().c_str(),bRet?1:0);

	return bRet;
}

//-----------------------------------------------------------------------

bool cCharacterAStarCallback::Intersects(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	// if(pBody->GetCollide()==false) return false;

	if (pBody->IsCharacter())
		return false;
	if (pBody->GetCollideCharacter() == false)
		return false;

	if (pBody->GetMass() == 0)
		return false;

	iGameEntity *pEntity = static_cast<iGameEntity *>(pBody->GetUserData());

	if (pEntity && pEntity->GetType() == eGameEntityType_SwingDoor) {
		if (pEntity->GetToughness() >= mpMove->GetMaxDoorToughness() && mpMove->GetMaxDoorToughness() >= 0) {
			// Log("Door was too strong!\n T: %d Max: %d\n",pEntity->GetToughness(),mpMove->GetMaxDoorToughness());
			return true;
		}

		return false;
	} else {
		if (pBody->GetMass() < mpMove->mfMaxPushMass)
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// RAY CALLBHCK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cMoverRayCallback::Reset() {
	mbIntersection = false;
}

bool cMoverRayCallback::BeforeIntersect(iPhysicsBody *pBody) {
	if (pBody->IsCharacter() || pBody->GetCollide() == false || pBody->GetCollideCharacter() == false ||
		pBody->GetMass() > 0) {
		return false;
	}

	return true;
}

bool cMoverRayCallback::OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	if (pBody->IsCharacter() || pBody->GetCollide() == false || pBody->GetCollideCharacter() == false ||
		pBody->GetMass() > 0) {
		return true;
	}

	mbIntersection = true;

	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cCharacterMove::cCharacterMove(cInit *apInit) {
	mpAStar = NULL;
	mpCharBody = NULL;
	mpContainer = NULL;

	mpInit = apInit;

	mfMaxTurnSpeed = 9999.0f;  // The maximum speed the enemy will turn in.
	mfAngleDistTurnMul = 1.0f; // Multiplied with angle distance to determine turn speed.

	mfMinBreakAngle = cMath::ToRad(20.0f); // Minimum angle distance at which break start.
	mfBreakAngleMul = 1.0f;                // Multiplied with angle distance to determine break force.

	mbMoving = false;
	mbTurning = false;

	mfTurnSpeed = 0;

	mfStuckLimit = 0.3f;
	mfStuckCounter = 0;

	mfMaxPushMass = 5;

	mbMoveToNewNode = false;

	mpAStarCallback = hplNew(cCharacterAStarCallback, (this));

	mvTempStart = 0;
	mvTempEnd = 0;

	mlMaxDoorToughness = -1;

	mlMaxNodeDistances = 150;
	mfNodeDistAvg = 0;
}

//-----------------------------------------------------------------------

cCharacterMove::~cCharacterMove() {
	if (mpAStarCallback)
		hplDelete(mpAStarCallback);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cCharacterMove::Update(float afTimeStep) {
	if (mpCharBody->IsActive() == false)
		return;

	////////////////////////////////////
	// Update Movement
	if (mbMoving) {
		cAINode *pCurrentNode = NULL;
		cVector3f vGoal;
		cVector3f vPos = mpCharBody->GetPosition();

		/////////////////////////////////////////
		// Get the postion to move towards and current node if there is any.
		if (mlstNodes.empty()) {
			vGoal = mvGoalPos;
		} else {
			pCurrentNode = mlstNodes.back();
			vGoal = pCurrentNode->GetPosition();
		}

		/////////////////////////////////////////
		// Check if the newer node is reachable
		if (mbMoveToNewNode) {
			mbMoveToNewNode = false;

			// if(mpContainer->FreePath(mpCharBody->GetFeetPosition()+cVector3f(0,0.1f,0),
			//						vGoal,1,0,mpAStarCallback)==false)
			/*if(mpContainer->FreePath(mpCharBody->GetFeetPosition()+cVector3f(0,0.1f,0),
									vGoal,-1,0,mpAStarCallback)==false)
			{
				//Do nothing here right now, might wanna remove the path from the two connected
				//Nodes, but that might turn out pretty hard, leave for now, this takes too much cpu.
				//------------------------
				//mpInit->mpEffectHandler->GetSubTitle()->Add("Path was blocked!\n",2.0f,true);
				mbMoving = false;
				mlstNodes.clear();

				//Log("-- Begin Dynamic path find !\n");
				mpAStarCallback->mbCheckDynamic = true;
				MoveToPos(mvGoalPos);
				mpAStarCallback->mbCheckDynamic = false;
				//Log("-- End Dynamic path find !\n");

			}*/
		}

		float fGoalAngle = -cMath::GetAngleFromPoints2D(cVector2f(vPos.x, vPos.z), cVector2f(vGoal.x, vGoal.z));
		TurnToAngle(fGoalAngle);
		mpCharBody->SetPitch(0);

		mpCharBody->Move(eCharDir_Forward, 1.0f, afTimeStep);

		////////////////////////////////
		// Update bounding volume
		mBoundingVolume.SetPosition(vPos);
		mBoundingVolume.UpdateSize();

		bool bStuckAtNode = false;
		float fNodeDist = cMath::Vector3DistSqr(vGoal, mpCharBody->GetFeetPosition());
		mlstNodeDistances.push_back(fNodeDist);
		if ((int)mlstNodeDistances.size() > mlMaxNodeDistances) {
			mlstNodeDistances.pop_front();
			mfNodeDistAvg = 0;

			std::list<float>::iterator it = mlstNodeDistances.begin();
			float fPreviousDistance = *it;
			++it;
			for (; it != mlstNodeDistances.end(); ++it) {
				float fChange = *it - fPreviousDistance;
				mfNodeDistAvg += fChange;
			}

			if (mfNodeDistAvg > 0 &&
				fNodeDist < mpCharBody->GetSize().x * 1.5f) {
				bStuckAtNode = true;
			}
		}

		////////////////////////////////
		// Check if node is reached:
		if (bStuckAtNode || cMath::PointBVCollision(vGoal, mBoundingVolume)) {
			if (mlstNodes.empty()) {
				mbMoving = false;
			} else {
				mlstNodes.pop_back(); // Go to next node next update.
			}
			mbMoveToNewNode = true;
			mlstNodeDistances.clear();
		}
	}

	////////////////////////////////////
	// Update turning
	if (mbTurning) {
		float fAngleDist = cMath::GetAngleDistanceRad(mpCharBody->GetYaw(), mfGoalAngle);

		/////////////////
		// Rotate the body
		if (std::abs(fAngleDist) < 0.001f) {
			mbTurning = false;
			mfTurnSpeed = 0;
		}
		if (mbTurning) {
			mfTurnSpeed = cMath::Min(mfAngleDistTurnMul * std::abs(fAngleDist), mfMaxTurnSpeed);

			if (fAngleDist < 0)
				mpCharBody->AddYaw(-mfTurnSpeed * afTimeStep);
			else
				mpCharBody->AddYaw(mfTurnSpeed * afTimeStep);

			//////////////////////
			// Break when making short turns
			if (std::abs(fAngleDist) >= mfMinBreakAngle && mpCharBody->GetMoveSpeed(eCharDir_Forward) > 0.15f) {
				float fBreakAcc = -mfBreakAngleMul * std::abs(fAngleDist);
				mpCharBody->Move(eCharDir_Forward, fBreakAcc, afTimeStep);

				mfCurrentBreak = fBreakAcc;
			} else {
				mfCurrentBreak = 0;
			}
		}
	}

	//////////////////////////////////////
	/// Update stuck counter
	float fWantedSpeed = mpCharBody->GetMoveSpeed(eCharDir_Forward);
	float fRealSpeed = cMath::Vector3Dist(mpCharBody->GetPosition(), mpCharBody->GetLastPosition());
	fRealSpeed = fRealSpeed / afTimeStep;

	cVector3f vWantedDir = mpCharBody->GetForward();
	cVector3f vRealDir = mpCharBody->GetPosition() - mpCharBody->GetLastPosition();
	vRealDir.Normalise();

	float fCos = cMath::Vector3Dot(vWantedDir, vRealDir);

	if (fRealSpeed / fWantedSpeed < mfStuckLimit ||
		(std::abs(fCos) < 0.3f && fWantedSpeed > 0.001f)) {
		mfStuckCounter += afTimeStep;
		// mpInit->mpEffectHandler->GetSubTitle()->Add(_W("ADD!\n"),1.0f/60.0f,false);
	} else {
		// mpInit->mpEffectHandler->GetSubTitle()->Add(_W("NEG!\n"),1.0f/60.0f,false);
		mfStuckCounter -= afTimeStep;
		if (mfStuckCounter < 0)
			mfStuckCounter = 0;
	}
}

//-----------------------------------------------------------------------

bool cCharacterMove::MoveToPos(const cVector3f &avPos) {
	if (mpAStar == NULL)
		return false;

	// Log(" Moving to %s\n",avPos.ToString().c_str());

	// mpInit->mpEffectHandler->GetSubTitle()->Add("Get newer path!\n",2.0f,true);

	// Get the start and goal position
	cVector3f vStartPos = mpCharBody->GetPosition();
	cVector3f vGoalPos = avPos;
	if (mpContainer->GetNodeIsAtCenter() == false) {
		vStartPos -= cVector3f(0, mpCharBody->GetSize().y / 2.0f, 0);
	}

	// Get the nodes to be following
	mlstNodes.clear();
	// Log(" Getting path!\n");
	bool bRet = mpAStar->GetPath(vStartPos, vGoalPos, &mlstNodes);

	if (bRet == false) {
		// Log("Did NOT find path\n");
		// mpInit->mpEffectHandler->GetSubTitle()->Add(_W("Did not find path!\n"),2,false);
	}

	mvGoalPos = vGoalPos;

	mbMoving = true;
	mlstNodeDistances.clear();

	return bRet;
}

//-----------------------------------------------------------------------

void cCharacterMove::MoveDirectToPos(const cVector3f &avFeetPos, float afTimeStep) {
	TurnToPos(avFeetPos);
	GetCharBody()->Move(eCharDir_Forward, 1.0f, afTimeStep);
}

//-----------------------------------------------------------------------

void cCharacterMove::TurnToAngle(float afAngle) {
	mbTurning = true;

	mfGoalAngle = afAngle;
}

void cCharacterMove::TurnToPos(const cVector3f &avPos) {
	cVector3f vStartPos = mpCharBody->GetPosition();

	float fGoalAngle = -cMath::GetAngleFromPoints2D(cVector2f(vStartPos.x, vStartPos.z),
													cVector2f(avPos.x, avPos.z));
	TurnToAngle(fGoalAngle);
}

//-----------------------------------------------------------------------

void cCharacterMove::Stop() {
	mbMoving = false;
	mlstNodes.clear();
	mlstNodeDistances.clear();
}

//-----------------------------------------------------------------------

bool cCharacterMove::FreeDirectPathToChar(iCharacterBody *apBody) {
	float fHeight = fabs(GetCharBody()->GetFeetPosition().y - apBody->GetFeetPosition().y);

	if (fHeight > 0.8f)
		return false;

	return mpContainer->FreePath(GetCharBody()->GetFeetPosition() + cVector3f(0, 0.05f, 0),
								 apBody->GetFeetPosition() + cVector3f(0, 0.05f, 0),
								 -1, eAIFreePathFlag_SkipDynamic);
}

//-----------------------------------------------------------------------

float cCharacterMove::DistanceToChar(iCharacterBody *apBody) {
	return cMath::Vector3Dist(mpCharBody->GetFeetPosition(), apBody->GetFeetPosition());
}

float cCharacterMove::DistanceToChar2D(iCharacterBody *apBody) {
	cVector3f vStart = mpCharBody->GetFeetPosition();
	cVector3f vEnd = apBody->GetFeetPosition();
	vStart.y = 0;
	vEnd.y = 0;

	return cMath::Vector3Dist(vStart, vEnd);
}

//-----------------------------------------------------------------------

cAINode *cCharacterMove::GetAINodeInRange(float afMinDistance, float afMaxDistance) {
	float fMaxDistSqr = afMaxDistance * afMaxDistance;
	float fMinDistSqr = afMinDistance * afMinDistance;

	int i = cMath::RandRectl(0, mpContainer->GetNodeNum() - 1);
	int lCount = 0;

	// Log("StartNode i: %d\n",i);

	while (lCount < mpContainer->GetNodeNum()) {
		cAINode *pNode = mpContainer->GetNode(i);
		// Log("Testing node: %d '%s'\n",i,pNode->GetName().c_str());

		float fDistSqr = cMath::Vector3DistSqr(pNode->GetPosition(), mpCharBody->GetFeetPosition());
		if (fDistSqr <= fMaxDistSqr && fDistSqr >= fMinDistSqr) {
			return pNode;
		}

		++lCount;
		++i;
		if (i >= mpContainer->GetNodeNum())
			i = 0;
	}

	return NULL;

	/*for(int i=0; i<mpContainer->GetNodeNum(); ++i)
	{
		cAINode* pNode = mpContainer->GetNode(i);

		MoveToPos(pNode->GetPosition());
	}
	return NULL;*/
}

//-----------------------------------------------------------------------

cAINode *cCharacterMove::GetAINodeAtPosInRange(const cVector3f &avPos, float afMinDistance, float afMaxDistance,
											   bool abRayCheck, float afEndOffset) {
	float fMaxDistSqr = afMaxDistance * afMaxDistance;
	float fMinDistSqr = afMinDistance * afMinDistance;

	int i = cMath::RandRectl(0, mpContainer->GetNodeNum() - 1);
	int lCount = 0;

	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	while (lCount < mpContainer->GetNodeNum()) {
		cAINode *pNode = mpContainer->GetNode(i);

		float fDistSqr = cMath::Vector3DistSqr(pNode->GetPosition(), avPos);
		if (fDistSqr <= fMaxDistSqr && fDistSqr >= fMinDistSqr) {
			if (abRayCheck) {
				mRayCallback.Reset();

				cVector3f vStart = pNode->GetPosition();

				// Calculate a postion offset closer to the node.
				float fDist = cMath::Vector3Dist(vStart, avPos);
				fDist = fDist - afEndOffset;
				if (fDist < 0)
					fDist = 0;
				cVector3f vDir = cMath::Vector3Normalize(avPos - vStart);

				cVector3f vEnd = vStart + vDir * fDist;

				pPhysicsWorld->CastRay(&mRayCallback, vStart, vEnd, false, false, false);

				if (mRayCallback.mbIntersection == false) {
					mvTempStart = vStart;
					mvTempEnd = vEnd;

					return pNode;
				}
			} else {
				return pNode;
			}
		}

		++lCount;
		++i;
		if (i >= mpContainer->GetNodeNum())
			i = 0;
	}

	return NULL;
}

//-----------------------------------------------------------------------

void cCharacterMove::SetCharBody(iCharacterBody *apCharBody) {
	mpCharBody = apCharBody;

	cVector3f vSize = mpCharBody->GetSize();
	vSize.y *= 2.5f;
	mBoundingVolume.SetSize(vSize);
}

void cCharacterMove::SetNodeContainer(cAINodeContainer *apContainer) {
	mpContainer = apContainer;
}

void cCharacterMove::SetAStar(cAStarHandler *apAStar) {
	mpAStar = apAStar;
	mpAStar->SetCallback(mpAStarCallback);
}

//-----------------------------------------------------------------------

void cCharacterMove::OnDraw(cInit *apInit) {
	// return;

	// apInit->mpDefaultFont->Draw(cVector3f(5,64,100),14,cColor(1,1,1,1),eFontAlign_Left,
	//	"StuckCount %f",mfStuckCounter);

	/*apInit->mpDefaultFont->Draw(cVector3f(5,64,100),14,cColor(1,1,1,1),eFontAlign_Left,
		"Speed: %f",mpCharBody->GetMoveSpeed(eCharDir_Forward));
	apInit->mpDefaultFont->Draw(cVector3f(5,79,100),14,cColor(1,1,1,1),eFontAlign_Left,
		"Break: %f",mfCurrentBreak);*/

	if (mbMoving)
		apInit->mpDefaultFont->draw(cVector3f(5, 79, 100), 14, cColor(1, 1, 1, 1), eFontAlign_Left,
									_W("NodeDistAvg: %f"), mfNodeDistAvg);

	/*apInit->mpDefaultFont->Draw(cVector3f(5,64,100),14,cColor(1,1,1,1),eFontAlign_Left,
		"Yaw: %f Pitch %f",cMath::ToDeg(mpCharBody->GetYaw()), cMath::ToDeg(mpCharBody->GetPitch()));

	apInit->mpDefaultFont->Draw(cVector3f(5,79,100),14,cColor(1,1,1,1),eFontAlign_Left,
		"Speed: %f",mpCharBody->GetMoveSpeed(eCharDir_Forward));

	apInit->mpDefaultFont->Draw(cVector3f(5,94,100),14,cColor(1,1,1,1),eFontAlign_Left,
		"Fwd: %s",mpCharBody->GetForward().ToString().c_str());

	apInit->mpDefaultFont->Draw(cVector3f(5,110,100),14,cColor(1,1,1,1),eFontAlign_Left,
		"Moving: %d",mbMoving?1:0);*/
}

//-----------------------------------------------------------------------

void cCharacterMove::OnPostSceneDraw(iLowLevelGraphics *apLowLevelGfx) {
	apLowLevelGfx->SetDepthTestActive(true);

	for (int i = 0; i < mpContainer->GetNodeNum(); ++i) {
		cAINode *pNode = mpContainer->GetNode(i);

		apLowLevelGfx->DrawSphere(pNode->GetPosition(), 0.15f, cColor(0.6f, 0.6f, 0.6f, 1));

		for (int j = 0; j < pNode->GetEdgeNum(); ++j) {
			cAINodeEdge *pEdge = pNode->GetEdge(j);

			apLowLevelGfx->DrawLine(pNode->GetPosition(), pEdge->mpNode->GetPosition(), cColor(0.4f, 0.4f, 0.4f, 1));
		}
	}

	mpCharBody->GetBody()->RenderDebugGeometry(apLowLevelGfx, cColor(1, 1, 1, 1));

	// return;
	if (mbMoving == false)
		return;

	//////////////////////////////
	// GoalPos
	cVector3f vGoalPos = mvGoalPos;
	if (mpContainer->GetNodeIsAtCenter() == false) {
		vGoalPos += cVector3f(0, mpContainer->GetCollideSize().y / 2, 0);
	}

	apLowLevelGfx->DrawSphere(vGoalPos, 0.2f, cColor(1, 0, 1));
	cVector3f vLastVec = vGoalPos;

	//////////////////////////////
	// Nodes
	tAINodeListIt it = mlstNodes.begin();
	for (; it != mlstNodes.end(); ++it) {
		cAINode *pNode = *it;

		cVector3f vNodePos = pNode->GetPosition();
		if (mpContainer->GetNodeIsAtCenter() == false) {
			vNodePos += cVector3f(0, mpContainer->GetCollideSize().y / 2, 0);
		}

		apLowLevelGfx->DrawSphere(vNodePos, 0.2f, cColor(1, 0, 1));
		apLowLevelGfx->DrawLine(vLastVec, vNodePos, cColor(1, 0, 1));

		vLastVec = vNodePos;
	}

	//////////////////////////////
	// Start pos
	cVector3f vStartPos = mpCharBody->GetPosition();
	// if(mpContainer->GetNodeIsAtCenter()==false){
	// vStartPos += cVector3f(0,mpContainer->GetCollideSize().y/2, 0);
	//}
	apLowLevelGfx->DrawSphere(vStartPos, 0.2f, cColor(1, 0, 1));
	apLowLevelGfx->DrawLine(vLastVec, vStartPos, cColor(1, 0, 1));

	apLowLevelGfx->DrawSphere(mvTempStart, 0.2f, cColor(0, 1, 1));
	apLowLevelGfx->DrawSphere(mvTempEnd, 0.2f, cColor(0, 1, 1));
	apLowLevelGfx->DrawLine(mvTempStart, mvTempEnd, cColor(0, 1, 1));
}

//-----------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
