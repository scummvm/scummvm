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

#ifndef GAME_GAME_ENEMY_SPIDER_H
#define GAME_GAME_ENEMY_SPIDER_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEnemy.h"

using namespace hpl;

//-----------------------------------------

class cGameEnemy_Spider;

// BASE STATE
class iGameEnemyState_Spider_Base : public iGameEnemyState {
public:
	iGameEnemyState_Spider_Base(int alId, cInit *apInit, iGameEnemy *apEnemy);

	virtual void OnSeePlayer(const cVector3f &avPosition, float afChance);
	virtual bool OnHearNoise(const cVector3f &avPosition, float afVolume);
	virtual void OnTakeHit(float afDamage);
	virtual void OnDeath(float afDamage);
	virtual void OnFlashlight(const cVector3f &avPosition);

	virtual void OnAnimationOver(const tString &asAnimName) {}

	virtual void OnDraw() {}
	virtual void OnPostSceneDraw() {}

protected:
	cGameEnemy_Spider *mpEnemySpider;
};

//-----------------------------------------

// IDLE STATE
class cGameEnemyState_Spider_Idle : public iGameEnemyState_Spider_Base {
public:
	cGameEnemyState_Spider_Idle(int alId, cInit *apInit, iGameEnemy *apEnemy) : iGameEnemyState_Spider_Base(alId, apInit, apEnemy) {}

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
class cGameEnemyState_Spider_Hunt : public iGameEnemyState_Spider_Base {
public:
	cGameEnemyState_Spider_Hunt(int alId, cInit *apInit, iGameEnemy *apEnemy) : iGameEnemyState_Spider_Base(alId, apInit, apEnemy) {}

	void OnEnterState(iGameEnemyState *apPrevState);
	void OnLeaveState(iGameEnemyState *apNextState);

	void OnUpdate(float afTimeStep);

	void OnSeePlayer(const cVector3f &avPosition, float afChance);
	bool OnHearNoise(const cVector3f &avPosition, float afVolume);

	void OnDraw();

private:
	float mfUpdatePathCount;
	float mfUpdateFreq;
	bool mbFreePlayerPath;
	bool mbLostPlayer;
	float mfLostPlayerCount;
	float mfMaxLostPlayerCount;
};

//-----------------------------------------

// ATTACK STATE
class cGameEnemyState_Spider_Attack : public iGameEnemyState_Spider_Base {
public:
	cGameEnemyState_Spider_Attack(int alId, cInit *apInit, iGameEnemy *apEnemy) : iGameEnemyState_Spider_Base(alId, apInit, apEnemy) {}

	void OnEnterState(iGameEnemyState *apPrevState);
	void OnLeaveState(iGameEnemyState *apNextState);

	void OnUpdate(float afTimeStep);

	void OnAnimationOver(const tString &asName);

	void OnPostSceneDraw();

	void OnSeePlayer(const cVector3f &avPosition, float afChance) {}
	bool OnHearNoise(const cVector3f &avPosition, float afVolume) { return false; }
	void OnFlashlight(const cVector3f &avPosition);

private:
	float mfDamageTimer;
	float mfJumpTimer;
	bool mbAttacked;
};

//-----------------------------------------

// FLEE STATE
class cGameEnemyState_Spider_Flee : public iGameEnemyState_Spider_Base {
public:
	cGameEnemyState_Spider_Flee(int alId, cInit *apInit, iGameEnemy *apEnemy) : iGameEnemyState_Spider_Base(alId, apInit, apEnemy) {}

	void OnEnterState(iGameEnemyState *apPrevState);
	void OnLeaveState(iGameEnemyState *apNextState);

	void OnUpdate(float afTimeStep);

	void OnSeePlayer(const cVector3f &avPosition, float afChance) {}
	void OnFlashlight(const cVector3f &avPosition) {}

private:
	// float mfTimer;
	// float mfBackAngle;
	// bool mbBackwards;
};

//-----------------------------------------

// KNOCKDOWN STATE
class cGameEnemyState_Spider_KnockDown : public iGameEnemyState_Spider_Base {
public:
	cGameEnemyState_Spider_KnockDown(int alId, cInit *apInit, iGameEnemy *apEnemy) : iGameEnemyState_Spider_Base(alId, apInit, apEnemy) {}

	void OnEnterState(iGameEnemyState *apPrevState);
	void OnLeaveState(iGameEnemyState *apNextState);

	void OnUpdate(float afTimeStep);

	void OnSeePlayer(const cVector3f &avPosition, float afChance) {}
	bool OnHearNoise(const cVector3f &avPosition, float afVolume) { return false; }
	void OnTakeHit(float afDamage) {}

	void OnAnimationOver(const tString &asName);

private:
	float mfTimer;
	bool mbCheckAnim;
};

//-----------------------------------------

// DEAD STATE
class cGameEnemyState_Spider_Dead : public iGameEnemyState_Spider_Base {
public:
	cGameEnemyState_Spider_Dead(int alId, cInit *apInit, iGameEnemy *apEnemy) : iGameEnemyState_Spider_Base(alId, apInit, apEnemy) {}

	void OnEnterState(iGameEnemyState *apPrevState);
	void OnLeaveState(iGameEnemyState *apNextState);

	void OnUpdate(float afTimeStep);

	void OnSeePlayer(const cVector3f &avPosition, float afChance) {}
	bool OnHearNoise(const cVector3f &avPosition, float afVolume) { return false; }
	void OnFlashlight(const cVector3f &avPosition) {}
	void OnTakeHit(float afDamage) {}
	void OnDeath(float afDamage) {}
};

//-----------------------------------------

class cGameEnemy_Spider : public iGameEnemy {
public:
	cGameEnemy_Spider(cInit *apInit, const tString &asName, TiXmlElement *apGameElem);
	~cGameEnemy_Spider();

	void OnLoad();

	void OnUpdate(float afTimeStep);

	void ShowPlayer(const cVector3f &avPlayerFeetPos);

	bool MoveToPos(const cVector3f &avFeetPos);

	bool IsFighting();

	iCollideShape *GetAttackShape() { return mpAttackShape; }

	// State properties
	bool mbPathFind;

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

	float mfAttackDistance;
	float mfAttackForce;
	float mfAttackJumpTime;
	float mfAttackDamageTime;
	cVector3f mvAttackDamageSize;
	float mfAttackDamageRange;
	float mfAttackMinDamage;
	float mfAttackMaxDamage;
	tString msAttackStartSound;
	tString msAttackHitSound;
	float mfAttackMinMass;
	float mfAttackMaxMass;
	float mfAttackMinImpulse;
	float mfAttackMaxImpulse;
	int mlAttackStrength;

	float mfFleeMinDistance;
	float mfFleeMaxDistance;
	bool mbFleeFromFlashlight;

	tString msKnockDownSound;

	tString msDeathSound;

private:
	iCollideShape *mpAttackShape;
};

//-----------------------------------------

#endif // GAME_GAME_ENEMY_SPIDER_H
