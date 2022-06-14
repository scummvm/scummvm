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

#include "hpl1/engine/physics/Body2D.h"

#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/physics/CollideData2D.h"
#include "hpl1/engine/graphics/Mesh2d.h"
#include "hpl1/engine/scene/Node2D.h"
#include "hpl1/engine/scene/Entity2D.h"
#include "hpl1/engine/physics/Collider2D.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cBody2D::cBody2D(const tString& asName,cMesh2D *apMesh, cVector2f avSize, cCollider2D* apCollider, int alID)
	: iEntity2D(asName)
	{
		mvSize = avSize;
		mpMesh = apMesh;
		mpCollider = apCollider;

		mpCollMesh = mpMesh->CreateCollisonMesh(0,mvSize);
		mpBaseCollMesh = mpMesh->CreateCollisonMesh(0,mvSize);

		/*for(int i=0;i<(int)mpCollMesh->mvPos.size();i++)
		{
			Log("Pos%d: %s\n", i,mpCollMesh->mvPos[i].ToString().c_str());
		}

		for(int i=0;i<(int)mpCollMesh->mvNormal.size();i++)
		{
			Log("Norm%d: %s\n", i,mpCollMesh->mvNormal[i].ToString().c_str());
		}*/

		//Log("------------\n");

		//Set some default values to the properties
		mfMaxVel =0;
		mfAcc =1;

		mfAirFriction = 0.005f;
		mfGroundFriction = 0.3f;
		mfGravity = 0.4f;
		mfMaxGravityVel = 3;

		mbCollidable = false;
		mbCollides = true;
		mbMoved = false;
		mbOnGround = false;
		mbGroundFrictionX = false;
		mbGroundFrictionY = false;

		mvCollideCount =0;

		mvMovement =0;

		mbAttachToGround = false;
		mbAttachBodies = true;

		mpParentBody = NULL;

		mpNode = NULL;
		mlID = alID;

		mlCollideFlag = eFlagBit_0;
		mlCollideType = eFlagBit_1;
	}

	//-----------------------------------------------------------------------

	cBody2D::~cBody2D()
	{
		hplDelete(mpCollMesh);
		hplDelete(mpBaseCollMesh);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cBody2D::UpdateLogic(float afTimeStep)
	{
		cVector3f vStartPos = GetPosition();

		if(mbAttachToGround)
		{
			if(mpParentBody != NULL)
			{
				//What can be done is to add this to the force, and then att the end redcue the same amout
				//This will allow the body to add the velocity to it's jumps for example.
				cVector3f vMovement = mpParentBody->GetMovement();//mpParentBody->GetPosition() - mpParentBody->GetLastPosition();
				SetPosition(GetPosition() + vMovement);
			}
		}

		cCollideData2D CollideData;
		//Update Gravity

		mbGroundFrictionX = false;

		if(mbCollides && mfGravity > 0)
		{
			cVector3f vTempPos = GetPosition();

			float fXSize = mvSize.x;
			cRect2f Rect(vTempPos.x - fXSize/2,vTempPos.y+mfGravity,
						fXSize, mvSize.y/2);

			if(mpCollider->CollideRect(Rect,GetCollideFlag(),NULL))
			{
				//We are still on ground.
				mbOnGround = true;
				mbGroundFrictionX = true;
				//Log("Standing on ground\n");
			}
			else
			{
				//If the player used to be on ground, add some extra force
				if(mbOnGround)
				{
					mvForce.y += mfGravity*2;
				}

				mbOnGround = false;
				mvForce.y += mfGravity;
				if(mvForce.y>mfMaxGravityVel)mvForce.y = mfMaxGravityVel;
			}
		}

		//Add the Y Axis and check for collision
		AddPosXY(cVector2f(0,mvForce.y));

		if(mbCollides)
		{
			UpdateCollisionMesh();
			UpdateBoundingBox();
			if(mpCollider->CollideBody(this, &CollideData))
			{
				if(mvLastCollidePos.y == GetPosition().y)	mvCollideCount.y++;

				if(mvForce.y>0)
				{
					if(mbAttachToGround)
					{
						//If collided with a  body, make it the new reference frame.
						if(CollideData.mlstBodies.size()>0)
						{
							tBody2DListIt it = CollideData.mlstBodies.begin();
							if(*it != mpParentBody)
							{
								if(mpParentBody != NULL)DetachBody(mpParentBody);

								SetParentBody(*it);
								(*it)->AttachBody(this);
								//Log("Attaching!\n");
							}
						}
						else
						{
							if(mpParentBody != NULL)
							{
								DetachBody(mpParentBody);
								mpParentBody = NULL;
								//Log("Detaching!\n");
							}
						}
					}

					mbOnGround = true;
					mvForce.y=0;
				}

				if(mvCollideCount.y>0){

					mvForce.y=0;
				}

				mvLastCollidePos.y = GetPosition().y;
				mbGroundFrictionX = true;
			}
			else{
				mvCollideCount.y =0;
				if(!mbOnGround)
					mvLastCollidePos.y = -10000;
				//mbGroundFrictionX = false;
			}

			if(mbOnGround==false && mbAttachToGround)
			{
				if(mpParentBody != NULL)
				{
					DetachBody(mpParentBody);
					mpParentBody = NULL;
					//Log("Detaching!\n");
				}
			}
		}

		//Add the X Axis and check for collision
		AddPosXY(cVector2f(mvForce.x, 0));

		if(mbCollides)
		{
			UpdateCollisionMesh();
			UpdateBoundingBox();
			if(mpCollider->CollideBody(this,NULL))
			{
				if(mvLastCollidePos.x == GetPosition().x)	mvCollideCount.x++;

				if(mvCollideCount.x>0){
					mvForce.x=0;
				}

				mvLastCollidePos.x = GetPosition().x;
			}
			else{
				mvCollideCount.x =0;
				mvLastCollidePos.x = -10000;
			}
		}

		// Update the force
		float fAngle=0,fStrength=0;
		cMath::GetAngleFromVector(mvForce, &fAngle, &fStrength);

		fStrength -= mfAirFriction; //This is the air friction.
							//It should be combined with the friction of the collided material.

		if(fStrength<0)fStrength=0;

		SetForce(fAngle,fStrength);

		//Don't do any friction if the body has moved.
		//if(!mbMoved) Or?
		{
			if(mbGroundFrictionX && mvForce.x!=0){
				//Log("GroundFriction!\n");
				//Log("Force: %s\n",mvForce.ToString().c_str());
				if(mvForce.x>0)
				{
					mvForce.x -= mfGroundFriction;
					if(mvForce.x<0)mvForce.x =0;
				}
				else
				{
					mvForce.x += mfGroundFriction;
					if(mvForce.x>0)mvForce.x =0;
				}
				//Log("Force: %s\n",mvForce.ToString().c_str());
			}
		}

		mvMovement = GetPosition() - vStartPos;


		/*tBody2DListIt BodyIt = mlstAttachedBodies.begin();
		for(;BodyIt != mlstAttachedBodies.end();BodyIt++)
		{
			cBody2D* pBody = *BodyIt;

			pBody->SetPosition(pBody->GetPosition() + vMovement);
		}*/

		if(mpNode){
			mpNode->SetPosition(GetWorldPosition());
		}

		mbMoved = false;
	}

	//-----------------------------------------------------------------------

	void cBody2D::Move(float afValue)
	{
		float fAngle=0,fStrength=0;

		cVector2f vForwardVec = cMath::GetVectorFromAngle2D(mvRotation.z, 1);
		cVector2f vMovement = cMath::ProjectVector2D(mvForce,vForwardVec);
		cMath::GetAngleFromVector(vMovement,&fAngle,&fStrength);

		if(fStrength < mfMaxVel)
		{
			float fTempAcc = mfAcc;

			if(fStrength + fTempAcc > mfMaxVel){
				fTempAcc -=  (fStrength + fTempAcc) -  mfMaxVel;
			}

			mvForce += vForwardVec * fTempAcc;

			mbMoved = true;
		}
	}

	//-----------------------------------------------------------------------

	void cBody2D::AddForce(float afAngle, float afStrength)
	{
		cVector2f vForce = cMath::GetVectorFromAngle2D(afAngle, afStrength);

		AddForce(vForce);
	}

	//-----------------------------------------------------------------------

	void cBody2D::AddForce(const cVector2f& avForce)
	{
		mvForce += avForce;
	}

	//-----------------------------------------------------------------------

	void cBody2D::SetForce(float afAngle, float afStrength)
	{
		cVector2f vForce = cMath::GetVectorFromAngle2D(afAngle, afStrength);
		SetForce(vForce);
	}

	//-----------------------------------------------------------------------

	void cBody2D::SetForce(const cVector2f& avForce)
	{
		mvForce = avForce;
	}

	//-----------------------------------------------------------------------

	const cRect2f& cBody2D::GetBoundingBox()
	{
		return mBoundingBox;
	}

	//-----------------------------------------------------------------------

	bool cBody2D::UpdateBoundingBox()
	{
		cVector2f vSize;

		/*if(mvRotation.z != 0)
		{
			//Only Temp...
			float fMaxSize = sqrt(mvSize.x*mvSize.x + mvSize.y*mvSize.y);

			vSize.x = fMaxSize;
			vSize.y = fMaxSize;
		}
		else*/
		{
			vSize = mvSize;
		}

		mBoundingBox = cRect2f(cVector2f(GetWorldPosition().x-vSize.x/2,
			GetWorldPosition().y-vSize.y/2),vSize);

		return true;
	}

	//-----------------------------------------------------------------------

	void cBody2D::UpdateCollisionMesh()
	{
		cVector2f vPos(GetPosition().x, GetPosition().y);
		for(int i=0;i<(int)mpCollMesh->mvPos.size();i++)
		{
			mpCollMesh->mvPos[i] =  vPos + mpBaseCollMesh->mvPos[i];
		}
	}

	//-----------------------------------------------------------------------

	cCollisionMesh2D* cBody2D::GetCollisionMesh()
	{
		return mpCollMesh;
	}

	//-----------------------------------------------------------------------

	void cBody2D::AttachBody(cBody2D* apBody)
	{
		mlstAttachedBodies.push_back(apBody);
	}

	//-----------------------------------------------------------------------

	void cBody2D::DetachBody(cBody2D* apBody)
	{
		tBody2DListIt it = mlstAttachedBodies.begin();
		for(;it != mlstAttachedBodies.end();it++)
		{
			if(*it == apBody)
			{
				mlstAttachedBodies.erase(it);
				break;
			}
		}
	}

	//-----------------------------------------------------------------------

	void cBody2D::SetParentBody(cBody2D* apBody)
	{
		mpParentBody = apBody;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cBody2D::AddPosXY(cVector2f avPosAdd)
	{
		cVector3f vPos = GetPosition();
		vPos += avPosAdd;
		SetPosition(vPos);
	}

	//-----------------------------------------------------------------------
}
