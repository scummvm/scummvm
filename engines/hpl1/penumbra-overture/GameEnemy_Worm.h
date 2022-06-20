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

#ifndef GAME_GAME_ENEMY_WORM_H
#define GAME_GAME_ENEMY_WORM_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEnemy.h"

using namespace hpl;

//-----------------------------------------

class cGameEnemy_Worm;

// BASE STATE
class iGameEnemyState_Worm_Base : public iGameEnemyState {
public:
	iGameEnemyState_Worm_Base(int alId, cInit *apInit, iGameEnemy *apEnemy);

	virtual void OnSeePlayer(const cVector3f &avPosition, float afChance);
	virtual bool OnHearNoise(const cVector3f &avPosition, float afVolume);
	virtual void OnTakeHit(float afDamage);
	virtual void OnDeath(float afDamage);
	virtual void OnFlashlight(const cVector3f &avPosition);

	virtual void OnAnimationOver(const tString &asAnimName) {}

	virtual void OnDraw() {}
	virtual void OnPostSceneDraw() {}

protected:
	cGameEnemy_Worm *mpEnemyWorm;
};

//-----------------------------------------

// IDLE STATE
class cGameEnemyState_Worm_Idle : public iGameEnemyState_Worm_Base {
public:
	cGameEnemyState_Worm_Idle(int alId, cInit *apInit, iGameEnemy *apEnemy) : iGameEnemyState_Worm_Base(alId, apInit, apEnemy) {}

	void OnEnterState(iGameEnemyState *apPrevState);
	void OnLeaveState(iGameEnemyState *apNextState);

	void OnUpdate(float afTimeStep);

	// void OnSeePlayer(const cVector3f &avPosition, float afChance){}

private:
	bool mbStopped;
	float mfNextWalkTime;
};

//-----------------------------------------

// HUNT STATE
class cGameEnemyState_Worm_Hunt : public iGameEnemyState_Worm_Base {
public:
	cGameEnemyState_Worm_Hunt(int alId, cInit *apInit, iGameEnemy *apEnemy) : iGameEnemyState_Worm_Base(alId, apInit, apEnemy) {}

	void OnEnterState(iGameEnemyState *apPrevState);
	void OnLeaveState(iGameEnemyState *apNextState);

	void OnUpdate(float afTimeStep);

	void OnSeePlayer(const cVector3f &avPosition, float afChance);
	bool OnHearNoise(const cVector3f &avPosition, float afVolume);

	void OnDraw();
	void OnPostSceneDraw();

private:
	float mfUpdatePathCount;
	float mfUpdateFreq;
	bool mbFreePlayerPath;
	bool mbLostPlayer;
	float mfLostPlayerCount;
	float mfMaxLostPlayerCount;
	float mfAttackSoundCount;

	float mfAttackCount;

	float mfSoundCount;
};

//-----------------------------------------

// DEAD STATE
class cGameEnemyState_Worm_Dead : public iGameEnemyState_Worm_Base {
public:
	cGameEnemyState_Worm_Dead(int alId, cInit *apInit, iGameEnemy *apEnemy) : iGameEnemyState_Worm_Base(alId, apInit, apEnemy) {}

	void OnEnterState(iGameEnemyState *apPrevState);
	void OnLeaveState(iGameEnemyState *apNextState);

	void OnUpdate(float afTimeStep) {}

	void OnSeePlayer(const cVector3f &avPosition, float afChance) {}
	bool OnHearNoise(const cVector3f &avPosition, float afVolume) { return false; }

private:
};

//-----------------------------------------

class cWormTailSegment {
public:
	cWormTailSegment() {
		mpBone = NULL;
		mpChildSegment = NULL;
	}

	std::list<cVector3f> mlstPositions;
	cVector3f mvPostion;

	cVector3f mvGoalForward;

	cVector3f mvForward;
	cVector3f mvUp;
	cVector3f mvRight;

	float mfDistToFront;

	iPhysicsBody *mpBody;

	cBoneState *mpBone;

	cMatrixf m_mtxBaseRot;

	cWormTailSegment *mpChildSegment;
};

//-----------------------------------------

class cGameEnemy_Worm_MeshCallback : public cMeshEntityCallback {
public:
	cGameEnemy_Worm_MeshCallback(cGameEnemy_Worm *apWorm);

	void AfterAnimationUpdate(cMeshEntity *apMeshEntity, float afTimeStep);

private:
	cGameEnemy_Worm *mpWorm;
};

//-----------------------------------------

class cVector3Smoother {
public:
	cVector3Smoother();

	void Add(const cVector3f &avVec);
	cVector3f GetAverage();

	void SetMax(int alMax) { mlMaxVecs = alMax; }

private:
	int mlMaxVecs;
	std::list<cVector3f> mlstVecs;
};

//-----------------------------------------

class cGameEnemy_Worm : public iGameEnemy {
	friend class cGameEnemy_Worm_MeshCallback;

public:
	cGameEnemy_Worm(cInit *apInit, const tString &asName, TiXmlElement *apGameElem);
	~cGameEnemy_Worm();

	void OnLoad();

	void OnUpdate(float afTimeStep);

	void ExtraPostSceneDraw();

	void ShowPlayer(const cVector3f &avPlayerFeetPos);

	bool MoveToPos(const cVector3f &avFeetPos);

	bool IsFighting();

	iCollideShape *GetAttackShape() { return mpAttackShape; }

	// Worm specific
	void SetupTail();

	cSoundEntity *mpMoveSound;

	// State properties
	tString msMoveSound;

	float mfIdleFOV;
	tString msIdleFoundPlayerSound;
	float mfIdleMinSeeChance;
	float mfIdleMinHearVolume;
	float mfIdleMinWaitLength;
	float mfIdleMaxWaitLength;

	float mfHuntFOV;
	float mfHuntSpeed;
	float mfHuntForLostPlayerTime;
	float mfHuntMinSeeChance;
	float mfHuntMinHearVolume;

	tString msHuntSound;
	float mfHuntSoundMinInteraval;
	float mfHuntSoundMaxInteraval;

	float mfAttackDamage;
	float mfAttackInterval;
	tString msAttackHitSound;
	float mfAttackHitSoundInterval;
	float mfAttackMinMass;
	float mfAttackMaxMass;
	float mfAttackMinImpulse;
	float mfAttackMaxImpulse;
	int mlAttackStrength;

	cVector3f mvAttackDamageSize;

private:
	iCollideShape *mpAttackShape;

	cBoneState *mpRootBone;
	std::list<cVector3f> mlstRootPositions;
	cVector3Smoother mRootForwards;
	cVector3f mvRootPosition;
	cVector3f mvRootGoalForward;
	cVector3f mvRootForward;
	cVector3f mvRootRight;
	cVector3f mvRootUp;

	cVector3f mvRootBaseForward;

	cVector3f mvLastForward;

	int mlMaxSegmentPostions;
	float mfTurnSpeed;

	bool mbFirstUpdate;
	cVector3f mvFirstUpdatePos;

	cGameEnemy_Worm_MeshCallback *mpMeshCallback;

	cWormTailSegment *mpRootSegment;
	std::vector<cWormTailSegment *> mvTailSegments;
};

//-----------------------------------------

#endif // GAME_GAME_ENEMY_WORM_H
