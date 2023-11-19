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

#ifndef HPL_BODY_H
#define HPL_BODY_H

#include "hpl1/engine/math/MathTypes.h"

#include "common/list.h"
#include "hpl1/engine/scene/Entity2D.h"
#include "common/stablemap.h"

namespace hpl {

class cMesh2D;
class cNode2D;
class cCollider2D;
class cCollisionMesh2D;

class cBody2D;

typedef Common::List<cBody2D *> tBody2DList;
typedef tBody2DList::iterator tBody2DListIt;

class cBody2D : public iEntity2D {
public:
	cBody2D(const tString &asName, cMesh2D *apMesh, cVector2f avSize, cCollider2D *apCollider, int alID);
	~cBody2D();

	const cRect2f &GetBoundingBox();
	bool UpdateBoundingBox();

	void Move(float afValue);

	void UpdateLogic(float afTimeStep);

	tString GetEntityType() { return "Body"; };

	cVector3f &GetPosition() { return mvPosition; }
	cVector3f &GetLastPosition() { return mvLastPosition; }
	void ResetLastPosition() { mvLastPosition = mvPosition; }

	float GetVelocity();
	float GetMaxVelocity() { return mfMaxVel; }
	float GetAcceleration() { return mfAcc; }
	float GetGravity() { return mfGravity; }
	float GetMaxGravityVel() { return mfMaxGravityVel; }
	bool GetCollidable() { return mbCollidable; }
	bool GetCollides() { return mbCollides; }
	float GetAirFriction() { return mfAirFriction; }
	float GetGroundFriction() { return mfGroundFriction; }
	const cVector2f &GetSize() { return mvSize; }

	const cVector3f &GetMovement() { return mvMovement; }

	void SetMaxVelocity(float afMaxVel) { mfMaxVel = afMaxVel; }
	void SetAcceleration(float afAcc) { mfAcc = afAcc; }
	void SetGravity(float afGravity) { mfGravity = afGravity; }
	void SetMaxGravityVel(float afMaxGravityVel) { mfMaxGravityVel = afMaxGravityVel; }
	void SetCollidable(bool abCollidable) { mbCollidable = abCollidable; }
	void SetCollides(bool abCollides) { mbCollides = abCollides; }
	void SetAirFriction(float afAirFriction) { mfAirFriction = afAirFriction; }
	void SetGroundFriction(float afGroundFriction) { mfGroundFriction = afGroundFriction; }

	void AddForce(float afAngle, float afStrength);
	void AddForce(const cVector2f &avForce);

	void SetForce(float afAngle, float afStrength);
	void SetForce(const cVector2f &avForce);

	const cVector2f &GetForce() const { return mvForce; }

	/**
	 * Sets the things that the body can collide with, default is eFlagBit_0 (tiles)
	 * \param alFlag
	 */
	void SetCollideFlag(tFlag alFlag) { mlCollideFlag = alFlag; }
	tFlag GetCollideFlag() { return mlCollideFlag; }

	/**
	 * Sets if the body is attached to moving obejcts and moves with the, default is false.
	 * \param abX
	 */
	void SetAttachToGround(bool abX) { mbAttachToGround = abX; }
	bool GetAttachToGround() { return mbAttachToGround; }

	/**
	 * Sets if other objects can attach themselves to this body. Default is true.
	 * \param abX
	 */
	void SetAttachBodies(bool abX) { mbAttachBodies = abX; }
	bool GetAttachBodies() { return mbAttachBodies; }

	void AttachBody(cBody2D *apBody);
	void DetachBody(cBody2D *apBody);
	void SetParentBody(cBody2D *apBody);

	/**
	 * Sets the types of collider the body is, default is eFlagBit_1. On collision checking, the
	 * other objects specify what it can collide with. And for every body this value is checked.
	 * \param alFlag
	 */
	void SetCollideType(tFlag alFlag) { mlCollideType = alFlag; }
	tFlag GetCollideType() { return mlCollideType; }

	void AttachNode(cNode2D *apNode) { mpNode = apNode; }
	void DetachNode() { mpNode = NULL; }

	int GetID() { return mlID; }

	bool OnGround() { return mbOnGround; }

	void UpdateCollisionMesh();
	cCollisionMesh2D *GetCollisionMesh();

private:
	float mfMaxVel;
	float mfAcc;
	float mfGravity;
	float mfMaxGravityVel;
	float mfAirFriction;
	float mfGroundFriction;

	bool mbCollides;
	bool mbCollidable;
	bool mbMoved;

	cVector3f mvMovement;

	bool mbAttachToGround;
	bool mbAttachBodies;
	tBody2DList mlstAttachedBodies;
	cBody2D *mpParentBody;

	int mlID;

	tFlag mlCollideFlag;
	tFlag mlCollideType;

	cMesh2D *mpMesh;
	cCollider2D *mpCollider;
	cNode2D *mpNode;

	cCollisionMesh2D *mpCollMesh;
	cCollisionMesh2D *mpBaseCollMesh;

	bool mbOnGround;
	bool mbGroundFrictionX;
	bool mbGroundFrictionY;

	cVector2f mvForce;
	cVector2f mvSize;

	cVector2l mvCollideCount;
	cVector2f mvLastCollidePos;

	void AddPosXY(cVector2f avPosAdd);
};

} // namespace hpl

#endif // HPL_BODY_H
