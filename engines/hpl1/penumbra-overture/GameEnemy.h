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

#ifndef GAME_GAME_ENEMY_H
#define GAME_GAME_ENEMY_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEntity.h"

#include "hpl1/penumbra-overture/CharacterMove.h"

using namespace hpl;

class cGameTrigger;
class cCharacterMove;
class iGameEntity;

//-----------------------------------------

#define STATE_IDLE 0
#define STATE_HUNT 1
#define STATE_ATTACK 2
#define STATE_FLEE 3
#define STATE_KNOCKDOWN 4
#define STATE_DEAD 5
#define STATE_PATROL 6
#define STATE_INVESTIGATE 7
#define STATE_BREAKDOOR 8
#define STATE_CALLBACKUP 9
#define STATE_MOVETO 10
#define STATE_EAT 11
#define STATE_ATTENTION 12
#define STATE_NUM 13

//-----------------------------------------

enum eEnemyMoveState {
	eEnemyMoveState_Backward,
	eEnemyMoveState_Stopped,
	eEnemyMoveState_Walking,
	eEnemyMoveState_Running,
	eEnemyMoveState_LastEnum
};

//-----------------------------------------

class cEnemyPatrolNode : public iSerializable {
	kSerializableClassInit(cEnemyPatrolNode) public : cEnemyPatrolNode() {}
	cEnemyPatrolNode(const tString &asNode, float afTime, const tString &asAnim) : msNodeName(asNode), mfWaitTime(afTime), msAnimation(asAnim) {}

	tString msNodeName;
	float mfWaitTime;
	tString msAnimation;
};

//-----------------------------------------

class cLineOfSightRayCallback : public iPhysicsRayCallback {
public:
	void Reset();
	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);
	bool Intersected();

private:
	bool mbIntersected;
	iPhysicsBody *mpGrabBody;
};

//------------------------------------------

class cEnemyFindGround : public iPhysicsRayCallback {
public:
	bool GetGround(const cVector3f &avStartPos, const cVector3f &avDir,
				   cVector3f *apDestPosition, cVector3f *apDestNormal,
				   float afMaxDistance = 10);

	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

private:
	bool mbIntersected;
	float mfMinDist;
	cVector3f mvPos;
	cVector3f mvNormal;
	float mfMaxDistance;
};

//------------------------------------------

class cEnemyCheckForDoor : public iPhysicsRayCallback {
public:
	bool CheckDoor(const cVector3f &avStart, const cVector3f &avEnd);

	bool BeforeIntersect(iPhysicsBody *pBody);
	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

private:
	bool mbIntersected;
};

//------------------------------------------

class iGameEnemy_SaveData : public iGameEntity_SaveData {
	kSerializableClassInit(iGameEnemy_SaveData);

public:
	bool mbHasBeenActivated;

	cVector3f mvCharBodyPosition;
	cVector3f mvCharBodyRotation;
	int mlCurrentPatrolNode;

	float mfDisappearTime;
	bool mbDisappearActive;
	bool mbHasDisappeared;

	bool mbUsesTriggers;

	cVector3f mvLastPlayerPos;
	tString msOnDeathCallback;
	tString msOnAttackCallback;
	cContainerVec<cEnemyPatrolNode> mvPatrolNodes;

	iGameEntity *CreateEntity();
};

//-----------------------------------------

class iGameEnemyState {
public:
	iGameEnemyState(int alId, cInit *apInit, iGameEnemy *apEnemy);
	virtual ~iGameEnemyState() = default;

	virtual void OnEnterState(iGameEnemyState *apPrevState) = 0;
	virtual void OnLeaveState(iGameEnemyState *apNextState) = 0;

	virtual void OnUpdate(float afTimeStep) = 0;

	virtual void OnDraw() = 0;
	virtual void OnPostSceneDraw() = 0;

	virtual void OnSeePlayer(const cVector3f &avPosition, float afChance) = 0;
	virtual bool OnHearNoise(const cVector3f &avPosition, float afVolume) = 0;
	virtual void OnTakeHit(float afDamage) = 0;
	virtual void OnDeath(float afDamage) = 0;
	virtual void OnAnimationOver(const tString &asAnimName) = 0;
	virtual void OnFlashlight(const cVector3f &avPosition) = 0;

	void SetPreviousState(int alX) { mlPreviousState = alX; }
	int GetPreviousState() { return mlPreviousState; }

	int GetId() { return mlId; }

protected:
	int mlId;
	cInit *mpInit;
	cPlayer *mpPlayer;
	iGameEnemy *mpEnemy;
	cCharacterMove *mpMover;

	int mlPreviousState;
};

//-----------------------------------------

class iGameEnemy : public iGameEntity {
	typedef iGameEntity super;
	friend class cEntityLoader_GameEnemy;

public:
	iGameEnemy(cInit *apInit, const tString &asName, TiXmlElement *apGameElem);
	virtual ~iGameEnemy(void);

	// Special
	void LoadBaseProperties(TiXmlElement *apGameElem);

	// Actions
	void OnPlayerInteract();
	void OnPlayerPick();

	void Setup(cWorld3D *apWorld);

	void OnWorldLoad();
	void OnPostLoadScripts();
	void OnWorldExit();
	void OnDraw();
	void OnPostSceneDraw();
	virtual void ExtraPostSceneDraw() {}

	void Update(float afTimeStep);

	bool IsLoading() { return mbLoading; }

	void OnDamage(float afX);
	void OnDeath(float afX);

	void OnFlashlight(const cVector3f &avPos);

	virtual void OnSetActive(bool abX);

	virtual bool MoveToPos(const cVector3f &avFeetPos) = 0;

	virtual void ShowPlayer(const cVector3f &avPlayerFeetPos) {}

	virtual bool IsFighting() { return false; }

	// Physics
	void SetupBody();

	// Properties
	cVector3f GetPosition();
	void SetFOV(float afAngle) { mfFOV = afAngle; }

	const tString &GetHitPS() { return msHitPS; }
	const tString &GetEnemyType() { return msEnemyType; }

	const tString &GetCloseMusic() { return msCloseMusic; }
	int GetCloseMusicPrio() { return mlCloseMusicPrio; }
	float GetCloseMusicStartDist() { return mfCloseMusicStartDist; }
	float GetCloseMusicStopDist() { return mfCloseMusicStopDist; }

	const tString &GetAttackMusic() { return msAttackMusic; }
	int GetAttackMusicPrio() { return mlAttackMusicPrio; }

	// States
	void AddState(iGameEnemyState *apState);
	void ChangeState(int alId);
	iGameEnemyState *GetState(int alId);
	int GetCurrentStateId() { return mlCurrentState; }

	// Triggers
	bool HandleTrigger(cGameTrigger *apTrigger);

	eGameTriggerType GetTriggerTypes() { return mTriggerTypes; }

	float GetTriggerUpdateCount() { return mfTriggerUpdateCount; }
	void SetTriggerUpdateCount(float afX) { mfTriggerUpdateCount = afX; }
	float GetTriggerUpdateRate() { return mfTriggerUpdateRate; }

	void SetSkipSoundTriggerCount(float afX) { mfSkipSoundTriggerCount = afX; }

	// Content control
	void PlayAnim(const tString &asName, bool abLoop, float afFadeTime,
				  bool abDependsOnSpeed = false, float afSpeedMul = 1.0f,
				  bool abSyncWithPrevFrame = false,
				  bool abOverideMoveState = true);
	void UseMoveStateAnimations();

	cAnimationState *GetCurrentAnimation() { return mpCurrentAnimation; }

	void PlaySound(const tString &asName);

	// Callabcks
	void SetOnDeathCallback(const tString &asFunc) { msOnDeathCallback = asFunc; }
	void SetOnAttackCallback(const tString &asFunc) { msOnAttackCallback = asFunc; }
	tString GetOnAttackCallback() { return msOnAttackCallback; }

	// AI
	bool CanSeePlayer();
	cCharacterMove *GetMover() { return mpMover; }
	cVector3f GetLastPlayerPos() { return mvLastPlayerPos; }
	void SetLastPlayerPos(const cVector3f &avPos) { mvLastPlayerPos = avPos; }

	void SetTempPosition(const cVector3f &avPos) { mvTempPosition = avPos; }
	cVector3f GetTempPosition() { return mvTempPosition; }

	void SetTempFloat(float afX) { mfTempFloat = afX; }
	float GetTempFloat() { return mfTempFloat; }

	cEnemyFindGround *GetGroundFinder() { return &mFindGround; }

	cEnemyCheckForDoor *GetDoorChecker() { return &mDoorCheck; }
	bool CheckForDoor();

	bool CheckForTeamMate(float afMaxDist, bool abCheckIfFighting);

	bool IsAttracted() { return mbIsAttracted; }
	void SetAttracted(bool abX) { mbIsAttracted = abX; }

	bool GetUsesTriggers() { return mbUsesTriggers; }
	void SetUsesTriggers(bool abX) { mbUsesTriggers = abX; }

	// Patrolling
	void AddPatrolNode(const tString &asNode, float afTime, const tString &asAnimation);
	cEnemyPatrolNode *GetPatrolNode(int alIdx) { return &mvPatrolNodes[alIdx]; }
	int GetPatrolNodeNum() { return (int)mvPatrolNodes.size(); }
	cEnemyPatrolNode *CurrentPatrolNode() { return &mvPatrolNodes[mlCurrentPatrolNode]; }

	void ClearPatrolNodes();

	int GetCurrentPatrolNode() { return mlCurrentPatrolNode; }
	float GetWaitTime() { return mfWaitTime; }
	float GetWaitTimeCount() { return mfWaitTimeCount; }

	void SetCurrentPatrolNode(int alX) { mlCurrentPatrolNode = alX; }
	void IncCurrentPatrolNode() {
		mlCurrentPatrolNode++;
		if (mlCurrentPatrolNode >= (int)mvPatrolNodes.size())
			mlCurrentPatrolNode = 0;
	}
	void SetWaitTime(float afX) { mfWaitTime = afX; }
	void SetWaitTimeCount(float afX) { mfWaitTimeCount = afX; }
	void AddWaitTimeCount(float afX) { mfWaitTimeCount += afX; }

	float GetDoorBreakCount() { return mfDoorBreakCount; }
	void SetDoorBreakCount(float afX) { mfDoorBreakCount = afX; }
	void AddDoorBreakCount(float afX) { mfDoorBreakCount += afX; }

	const cVector3f &GetModelOffsetAngles() { return mvModelOffsetAngles; }

	// Virtual
	virtual void OnUpdate(float afTimeStep) = 0;
	virtual void OnLoad() = 0;

	// SaveObject implementation
	iGameEntity_SaveData *CreateSaveData();
	void SaveToSaveData(iGameEntity_SaveData *apSaveData);
	void LoadFromSaveData(iGameEntity_SaveData *apSaveData);

protected:
	void UpdateEnemyPose(float afTimeStep);

	void UpdateAnimations(float afTimeStep);
	void UpdateCheckForPlayer(float afTimeStep);

	bool HandleSoundTrigger(cGameTrigger *apTrigger);

	bool LineOfSight(const cVector3f &avPos, const cVector3f &avSize);

	bool mbSetFeetAtGroundOnStart;
	bool mbAttachMeshToBody;
	bool mbRemoveAttackerOnDisable;

	tVector3fVec mvRayStartPoses;
	tVector3fVec mvRayEndPoses;

	cEnemyFindGround mFindGround;

	eGameTriggerType mTriggerTypes;

	cCharacterMove *mpMover;

	tString msEnemyType;

	float mfTriggerUpdateCount;
	float mfTriggerUpdateRate;

	float mfSkipSoundTriggerCount;

	cAStarHandler *mpAStarAir;
	cAStarHandler *mpAStarGround;
	cAINodeContainer *mpNodeContainerAir;
	cAINodeContainer *mpNodeContainerGround;

	cLineOfSightRayCallback mRayCallback;

	std::vector<iGameEnemyState *> mvStates;
	int mlCurrentState;

	cEnemyCheckForDoor mDoorCheck;

	cAnimationState *mpCurrentAnimation;
	bool mbAnimationIsSpeedDependant;
	float mfAnimationSpeedMul;

	bool mbOverideMoveState;
	eEnemyMoveState mMoveState;

	cVector3f mvTempPosition;
	float mfTempFloat;

	tStringVec mvPreloadSounds;

	bool mbLoading;

	bool mbAlignToGroundNormal;
	cMatrixf m_mtxStartPose;
	cMatrixf m_mtxGoalPose;
	float mfPoseCount;

	bool mbHasBeenActivated;

	bool mbUsesTriggers;

	///////////////////////////////
	// Variable Properties
	cVector3f mvLastPlayerPos;
	float mfCanSeePlayerCount;
	float mfCheckForPlayerCount;
	float mfCheckForPlayerRate;
	bool mbCanSeePlayer;
	int mlPlayerInLOSCount;
	int mlMaxPlayerInLOSCount;

	float mfCalcPlayerHiddenPosCount;

	float mfDamageSoundTimer;

	tString msOnDeathCallback;
	tString msOnAttackCallback;

	std::vector<cEnemyPatrolNode> mvPatrolNodes;

	int mlCurrentPatrolNode;
	float mfWaitTime;
	float mfWaitTimeCount;

	float mfDoorBreakCount;

	bool mbIsAttracted;

	/////////////////////////////////
	// Properties to be implemented
	bool mbDisappear;
	float mfDisappearMinTime;
	float mfDisappearMaxTime;
	float mfDisappearMinDistance;
	tString msDisappearPS;
	tString msDisappearSound;
	bool mbDisappearFreezesRagdoll;

	float mfDisappearTime;
	bool mbDisappearActive;
	bool mbHasDisappeared;

	tString msCloseMusic;
	int mlCloseMusicPrio;
	float mfCloseMusicStartDist;
	float mfCloseMusicStopDist;

	tString msAttackMusic;
	int mlAttackMusicPrio;

	bool mbShowDebug;

	tString msGroundNodeType;

	cVector3f mvBodySize;
	float mfBodyMass;

	float mfMaxForwardSpeed;
	float mfMaxBackwardSpeed;

	float mfAcceleration;
	float mfDeacceleration;

	float mfMaxTurnSpeed;
	float mfAngleDistTurnMul;

	float mfMinBreakAngle;
	float mfBreakAngleMul;

	float mfSpeedMoveAnimMul;
	float mfTurnSpeedMoveAnimMul;

	cMatrixf m_mtxModelOffset;
	cVector3f mvModelOffsetAngles;

	float mfIdleToMoveLimit;
	float mfMoveToLideLimit;

	float mfMaxPushMass;
	float mfPushForce;

	float mfMaxSeeDist;

	float mfMinAttackDist;

	float mfFOV;
	float mfFOVXMul;

	float mfMaxHealth;

	float mfStoppedToWalkSpeed;
	float mfWalkToStoppedSpeed;
	float mfWalkToRunSpeed;
	float mfRunToWalkSpeed;
	float mfMoveAnimSpeedMul;
	tString msBackwardAnim;
	tString msStoppedAnim;
	tString msWalkAnim;
	tString msRunAnim;

	float mfMinStunSpeed;
	float mfMinStunImpulse;
	float mfObjectCollideStun;
	float mfStunDamageMul;

	tString msHitPS;
};

//--------------------------------------

class cEntityLoader_GameEnemy : public cEntityLoader_Object {

public:
	cEntityLoader_GameEnemy(const tString &asName, cInit *apInit);
	~cEntityLoader_GameEnemy();

private:
	void BeforeLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);
	void AfterLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);

	cInit *mpInit;
};

//--------------------------------------

#endif // GAME_GAME_ENEMY_H
