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

#ifndef GAME_CHARACTER_MOVE_H
#define GAME_CHARACTER_MOVE_H

#include "hpl1/engine/engine.h"

using namespace hpl;

class iGameEnemy;

#include "hpl1/penumbra-overture/Init.h"

//------------------------------------------

class cCharacterMove;

class cCharacterAStarCallback : public iAStarCallback, public iAIFreePathCallback {
public:
	cCharacterAStarCallback(cCharacterMove *apMove);

	bool CanAddNode(cAINode *apParentNode, cAINode *apChildNode);

	bool Intersects(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	bool mbCheckDynamic;

	cCharacterMove *mpMove;
};

//------------------------------------------

class cMoverRayCallback : public iPhysicsRayCallback {
public:
	void Reset();
	bool BeforeIntersect(iPhysicsBody *pBody);
	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	bool mbIntersection;
};

//------------------------------------------

class cCharacterMove {
	friend class cCharacterAStarCallback;

public:
	cCharacterMove(cInit *apInit);
	~cCharacterMove();

	void Update(float afTimeStep);

	void OnDraw(cInit *apInit);
	void OnPostSceneDraw(iLowLevelGraphics *apLowLevelGfx);

	///////////////////////////////////
	// Actions
	/**
	 * returns false if no path could be found.
	 */
	bool MoveToPos(const cVector3f &avPos);
	void MoveDirectToPos(const cVector3f &avFeetPos, float afTimeStep);

	void TurnToAngle(float afAngle);
	void TurnToPos(const cVector3f &avPos);

	void Stop();

	///////////////////////////////////
	// Helpers

	bool FreeDirectPathToChar(iCharacterBody *apBody);

	float DistanceToChar(iCharacterBody *apBody);
	float DistanceToChar2D(iCharacterBody *apBody);

	cAINode *GetAINodeInRange(float afMinDistance, float afMaxDistance);
	cAINode *GetAINodeAtPosInRange(const cVector3f &avPos, float afMinDistance, float afMaxDistance,
								   bool abRayCheck, float afEndOffset);

	//////////////////////////////////////////
	// Properties
	bool IsMoving() { return mbMoving; }

	bool IsTurning() { return mbTurning; }

	void SetCharBody(iCharacterBody *apCharBody);
	iCharacterBody *GetCharBody() { return mpCharBody; }

	void SetNodeContainer(cAINodeContainer *apContainer);
	cAINodeContainer *GetNodeContainer() { return mpContainer; }

	void SetAStar(cAStarHandler *apAStar);
	cAStarHandler *GetAStar() { return mpAStar; }

	void SetMaxTurnSpeed(float afX) { mfMaxTurnSpeed = afX; }
	void SetAngleDistTurnMul(float afX) { mfAngleDistTurnMul = afX; };

	void SetMinBreakAngle(float afX) { mfMinBreakAngle = afX; }
	void SetBreakAngleMul(float afX) { mfBreakAngleMul = afX; }

	void SetMaxPushMass(float afX) { mfMaxPushMass = afX; }

	float GetTurnSpeed() { return mfTurnSpeed; }

	const cVector3f &GetGoalPos() { return mvGoalPos; }

	float GetStuckCounter() { return mfStuckCounter; }
	void ResetStuckCounter() { mfStuckCounter = 0; }

	void SetMaxDoorToughness(int alX) { mlMaxDoorToughness = alX; }
	int GetMaxDoorToughness() { return mlMaxDoorToughness; }

private:
	cInit *mpInit;

	iCharacterBody *mpCharBody;

	cAINodeContainer *mpContainer;
	cAStarHandler *mpAStar;
	cBoundingVolume mBoundingVolume;

	tAINodeList mlstNodes;

	bool mbMoving;
	bool mbTurning;

	float mfTurnSpeed;

	cVector3f mvGoalPos;

	float mfGoalAngle;

	float mfStuckCounter;
	float mfStuckLimit;

	std::list<float> mlstNodeDistances;
	int mlMaxNodeDistances;
	float mfNodeDistAvg;

	// Properties
	float mfMaxTurnSpeed;
	float mfAngleDistTurnMul;

	float mfMinBreakAngle;
	float mfBreakAngleMul;

	float mfMaxPushMass;

	cMoverRayCallback mRayCallback;

	int mlMaxDoorToughness;

	// Debug:
	float mfCurrentBreak;
	cVector3f mvTempStart;
	cVector3f mvTempEnd;

	bool mbMoveToNewNode;

	cCharacterAStarCallback *mpAStarCallback;
};

//--------------------------------------

#endif // GAME_CHARACTER_MOVE_H
