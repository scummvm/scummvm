/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_SCRIPT_AI_H
#define BLADERUNNER_SCRIPT_AI_H

#include "bladerunner/script/script.h"

namespace BladeRunner {

class BladeRunnerEngine;

class AIScriptBase : public ScriptBase {
protected:
	int _animationState;
	int _animationFrame;
	int _animationStateNext;
	int _animationNext;

public:
	AIScriptBase(BladeRunnerEngine *vm) : ScriptBase(vm) {
		_animationState = 0;
		_animationFrame = 0;
		_animationStateNext = 0;
		_animationNext = 0;
	}

	virtual void Initialize() = 0;
	virtual bool Update() = 0;
	virtual void TimerExpired(int timer) = 0;
	virtual void CompletedMovementTrack() = 0;
	virtual void ReceivedClue(int clueId, int fromActorId) = 0;
	virtual void ClickedByPlayer() = 0;
	virtual void EnteredSet(int setId) = 0;
	virtual void OtherAgentEnteredThisSet(int otherActorId) = 0;
	virtual void OtherAgentExitedThisSet(int otherActorId) = 0;
	virtual void OtherAgentEnteredCombatMode(int otherActorId, int combatMode) = 0;
	virtual void ShotAtAndMissed() = 0;
	virtual bool ShotAtAndHit() = 0;
	virtual void Retired(int byActorId) = 0;
	virtual int GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) = 0;
	virtual bool GoalChanged(int currentGoalNumber, int newGoalNumber) = 0;
	virtual bool UpdateAnimation(int *animation, int *frame) = 0;
	virtual bool ChangeAnimationMode(int mode) = 0;
	virtual void QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) = 0;
	virtual void SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) = 0;
	virtual bool ReachedMovementTrackWaypoint(int waypointId) = 0;
	virtual void FledCombat() = 0;
};

#define DECLARE_SCRIPT(name) \
class AIScript##name : public AIScriptBase { \
public: \
	AIScript##name(BladeRunnerEngine *vm); \
	void Initialize(); \
	bool Update(); \
	void TimerExpired(int timer); \
	void CompletedMovementTrack(); \
	void ReceivedClue(int clueId, int fromActorId); \
	void ClickedByPlayer(); \
	void EnteredSet(int setId); \
	void OtherAgentEnteredThisSet(int otherActorId); \
	void OtherAgentExitedThisSet(int otherActorId); \
	void OtherAgentEnteredCombatMode(int otherActorId, int combatMode); \
	void ShotAtAndMissed(); \
	bool ShotAtAndHit(); \
	void Retired(int byActorId); \
	int GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId); \
	bool GoalChanged(int currentGoalNumber, int newGoalNumber); \
	bool UpdateAnimation(int *animation, int *frame); \
	bool ChangeAnimationMode(int mode); \
	void QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext); \
	void SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext); \
	bool ReachedMovementTrackWaypoint(int waypointId); \
	void FledCombat(); \
private:
#define END_SCRIPT };

DECLARE_SCRIPT(McCoy)
	int _animationLoopCounter;
	int _animationLoopLength;
	int _animationLoopDirection;
	int _animationLoopFrameMin;
	int _animationLoopFrameMax;
	int _animationStateNextSpecial;
	int _animationNextSpecial;
	int _nextSoundId;
	bool _NR10SteeleShooting;
	float _fallSpeed;
	float _fallHeightCurrent;
	float _fallHeightTarget;

	void fallDown();
	void UG15fall();
	void dodge();
	void walkStairsLeft(float stepHeight);
	void walkStairsRight(float stepHeight);
END_SCRIPT

DECLARE_SCRIPT(Steele)
	bool _resumeIdleAfterFramesetCompletesFlag;
	int _varChooseIdleAnimation;
	int _varNumOfTimesToHoldCurrentFrame;

	double comp_distance(int actorId, float a5, float a6, int a1, float a2, float a3, float a4);
END_SCRIPT

DECLARE_SCRIPT(Gordo)
	bool _resumeIdleAfterFramesetCompletesFlag;
	int _counter;
	int _counterTarget;
	int _frameMin;
	int _frameDelta;
	int _frameMax;
	int _state;

	void talkToMcCoyInCity();
	void talkToMcCoyAtNR02();
	void dialogue2();
	void dialogue1();
	void unknown();
END_SCRIPT

DECLARE_SCRIPT(Dektora)
	bool _resumeIdleAfterFramesetCompletesFlag;
	float _x, _y, _z;

	double comp_distance(int actorId, float x1, float y1, float z1);
	void checkCombat();
END_SCRIPT

DECLARE_SCRIPT(Guzza)
	int _frameDelta;
	int _counter;
	int _state;
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Clovis)
	int _varChooseIdleAnimation;
	int _varNumOfTimesToHoldCurrentFrame;
	int _var3;
	int _var4;
	int _var5;
	bool _resumeIdleAfterFramesetCompletesFlag;

	void shotAnim();
	void someAnim();
END_SCRIPT

DECLARE_SCRIPT(Lucy)
	bool _resumeIdleAfterFramesetCompletesFlag;

	void voightKampffTest();
	void checkCombat();
END_SCRIPT

DECLARE_SCRIPT(Izo)
	int _var1;
	int _var2;
	int _varNumOfTimesToHoldCurrentFrame;
	int _varChooseIdleAnimation;
	bool _resumeIdleAfterFramesetCompletesFlag;

	void dialogueWithIzo();
	void modifyWaypoints();
END_SCRIPT

DECLARE_SCRIPT(Sadik)
	int _nextSoundId;
	int _varChooseIdleAnimation;
	int _varNumOfTimesToHoldCurrentFrame;
	int _var4;
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Crazylegs)
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Luther)
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Grigorian)
	int _varChooseIdleAnimation;
	int _varNumOfTimesToHoldCurrentFrame;
END_SCRIPT

DECLARE_SCRIPT(Transient)
END_SCRIPT

DECLARE_SCRIPT(Lance)
END_SCRIPT

DECLARE_SCRIPT(BulletBob)
	int _varChooseIdleAnimation;
	int _var2;
	int _var3;
	int _varNumOfTimesToHoldCurrentFrame;
END_SCRIPT

DECLARE_SCRIPT(Runciter)
	int _varChooseIdleAnimation;
	int var_45CD7C;
	int var_45CD80;
	int _varNumOfTimesToHoldCurrentFrame;
	int var_45CD88;
END_SCRIPT

DECLARE_SCRIPT(InsectDealer)
	bool _resumeIdleAfterFramesetCompletesFlag;
	int _state;
	int _frameDelta;
	int _var2;
	int _counter;
END_SCRIPT

DECLARE_SCRIPT(TyrellGuard)
	int _frameDelta;
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(EarlyQ)
	int _varNumOfTimesToHoldCurrentFrame;
	int _varChooseIdleAnimation;
	int _var3;
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Zuben)
	int _animationLoopCounter;
	int _animationLoopLength;
	int _animationLoopFrameMin;
	int _animationLoopDirection;
	int _animationLoopFrameMax;

	void dialogue();
END_SCRIPT

DECLARE_SCRIPT(Hasan)
	int _var1;
	int _var2;
	int _varNumOfTimesToHoldCurrentFrame;
	int _var4;
	int _var5;
	int _varChooseIdleAnimation;
END_SCRIPT

DECLARE_SCRIPT(Marcus)
END_SCRIPT

DECLARE_SCRIPT(Mia)
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(OfficerLeary)
	int _varChooseIdleAnimation;
	bool _idleModeRequestedWhileInTalkingState;
END_SCRIPT

DECLARE_SCRIPT(OfficerGrayford)
	int _varChooseIdleAnimation;
	bool _resumeIdleAfterFramesetCompletesFlag;
	int _varNumOfTimesToHoldCurrentFrame;
END_SCRIPT

DECLARE_SCRIPT(Hanoi)
	bool _resumeIdleAfterFramesetCompletesFlag;
	int _varChooseIdleAnimation;
	int _varNumOfTimesToHoldCurrentFrame;
	int _var4;
END_SCRIPT

DECLARE_SCRIPT(Baker)
END_SCRIPT

DECLARE_SCRIPT(DeskClerk)
	int  _varChooseIdleAnimation;
	bool _resumeIdleAfterFramesetCompletesFlag;
	int  _varNumOfTimesToHoldCurrentFrame;
END_SCRIPT

DECLARE_SCRIPT(HowieLee)
	int _varIdleStatesToggle;
END_SCRIPT

DECLARE_SCRIPT(FishDealer)
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Klein)
END_SCRIPT

DECLARE_SCRIPT(Murray)
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(HawkersBarkeep)
	int _varChooseIdleAnimation;
	int _varNumOfTimesToHoldCurrentFrame;
	int _var3;
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Holloway)
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(SergeantWalls)
END_SCRIPT

DECLARE_SCRIPT(Moraji)
	int _var1;
	int _varNumOfTimesToHoldCurrentFrame;
END_SCRIPT

DECLARE_SCRIPT(TheBard)
END_SCRIPT

DECLARE_SCRIPT(Photographer)
	int _varNumOfTimesToHoldCurrentFrame;
	int _var2;
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Dispatcher)
END_SCRIPT

DECLARE_SCRIPT(AnsweringMachine)
END_SCRIPT

DECLARE_SCRIPT(Rajif)
END_SCRIPT

DECLARE_SCRIPT(GovernorKolvig)
END_SCRIPT

DECLARE_SCRIPT(EarlyQBartender)
	int _varChooseIdleAnimation;
	int _var2;
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(HawkersParrot)
END_SCRIPT

DECLARE_SCRIPT(TaffyPatron)
END_SCRIPT

DECLARE_SCRIPT(LockupGuard)
END_SCRIPT

DECLARE_SCRIPT(Teenager)
END_SCRIPT

DECLARE_SCRIPT(HysteriaPatron1)
END_SCRIPT

DECLARE_SCRIPT(HysteriaPatron2)
END_SCRIPT

DECLARE_SCRIPT(HysteriaPatron3)
END_SCRIPT

DECLARE_SCRIPT(ShoeshineMan)
	bool _state;
END_SCRIPT

DECLARE_SCRIPT(Tyrell)
	bool _resumeIdleAfterFramesetCompletesFlag;
	int _varChooseIdleAnimation;
END_SCRIPT

DECLARE_SCRIPT(Chew)
	int _varNumOfTimesToHoldCurrentFrame;
	int _varChooseIdleAnimation;
	int _var3;
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Gaff)
END_SCRIPT

DECLARE_SCRIPT(Bryant)
END_SCRIPT

DECLARE_SCRIPT(Taffy)
END_SCRIPT

DECLARE_SCRIPT(Sebastian)
	bool _resumeIdleAfterFramesetCompletesFlag;

	void dialogue();
	void setMcCoyIsABladeRunner();
END_SCRIPT

DECLARE_SCRIPT(Rachael)
	bool _resumeIdleAfterFramesetCompletesFlag;

	void dialogue_start();
	void dialogue_agenda1();
	void dialogue_agenda2();
	void dialogue_act4();
END_SCRIPT

DECLARE_SCRIPT(GeneralDoll)
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Isabella)
	int _var1;
	int _varNumOfTimesToHoldCurrentFrame;
	int _varChooseIdleAnimation;
	int _var4;
END_SCRIPT

DECLARE_SCRIPT(BlimpGuy)
END_SCRIPT

DECLARE_SCRIPT(Newscaster)
END_SCRIPT

DECLARE_SCRIPT(Leon)
	bool _resumeIdleAfterFramesetCompletesFlag;
	float _mcCoyPositionX;
	float _mcCoyPositionY;
	float _mcCoyPositionZ;

	float distanceTo(int actorId, float x, float y, float z);
END_SCRIPT

DECLARE_SCRIPT(MaleAnnouncer)
END_SCRIPT

DECLARE_SCRIPT(FreeSlotA)
	int _var1;
	int _var2;
	float _fallSpeed;
	float _fallHeightCurrent;
	float _fallHeightTarget;

	void checkIfOnBridge();
	void goToRandomUGxx();
END_SCRIPT

DECLARE_SCRIPT(FreeSlotB)
	int _var1;
	int _var2;

	void goToRandomUGxx();
END_SCRIPT

DECLARE_SCRIPT(Maggie)
	int var_45F3F8;
	int var_45F3FC;
	int var_45F400; // only set to 0. unused
	int var_45F404; // only set to 0. unused
	int var_45F408; // only set to 0. unused

	int randomWaypointMA02();
	float distanceToActor(int actorId, float x, float y, float z);
END_SCRIPT

DECLARE_SCRIPT(GenericWalkerA)
	bool isInside;
	float deltaX;
	float deltaZ;

	void movingStart();
	void movingUpdate();
	bool prepareWalker();
	bool preparePath();
END_SCRIPT

DECLARE_SCRIPT(GenericWalkerB)
	bool isInside;
	float deltaX;
	float deltaZ;

	void movingStart();
	void movingUpdate();
	bool prepareWalker();
	bool preparePath();
END_SCRIPT

DECLARE_SCRIPT(GenericWalkerC)
	bool isInside;
	float deltaX;
	float deltaZ;

	void movingStart();
	void movingUpdate();
	bool prepareWalker();
	bool preparePath();
END_SCRIPT

DECLARE_SCRIPT(Mutant1)
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Mutant2)
	int _var1;
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

DECLARE_SCRIPT(Mutant3)
	int _var1;
	bool _resumeIdleAfterFramesetCompletesFlag;
END_SCRIPT

#undef DECLARE_SCRIPT
#undef END_SCRIPT

class AIScripts {
private:
	BladeRunnerEngine *_vm;
	int                _inScriptCounter;
	int                _actorCount;
	AIScriptBase     **_AIScripts;
	bool              *_actorUpdating;
public:
	AIScripts(BladeRunnerEngine *vm, int actorCount);
	~AIScripts();

	void initialize(int actor);
	void update(int actor);
	void timerExpired(int actor, int timer);
	void completedMovementTrack(int actor);
	void receivedClue(int actor, int clueId, int fromActorId);
	void clickedByPlayer(int actor);
	void enteredSet(int actor, int setId);
	void otherAgentEnteredThisSet(int actor, int otherActorId);
	void otherAgentExitedThisSet(int actor, int otherActorId);
	void otherAgentEnteredCombatMode(int actorId, int otherActorId, int combatMode);
	void shotAtAndMissed(int actorId);
	bool shotAtAndHit(int actorId);
	void retired(int actor, int retiredByActorId);
	void goalChanged(int actor, int currentGoalNumber, int newGoalNumber);
	bool reachedMovementTrackWaypoint(int actor, int waypointId);
	void updateAnimation(int actor, int *animation, int *frame);
	void changeAnimationMode(int actor, int mode);
	void queryAnimationState(int actor, int *animationState, int *animationFrame, int *animationStateNext, int *animationNext);
	void setAnimationState(int actor, int animationState, int animationFrame, int animationStateNext, int animationNext);
	void fledCombat(int actor);

	bool isInsideScript() const { return _inScriptCounter > 0; }

	void callChangeAnimationMode(int actor, int mode) { _AIScripts[actor]->ChangeAnimationMode(mode); }
	int callGetFriendlinessModifierIfGetsClue(int actor, int otherActorId, int clueId) {
		return _AIScripts[actor]->GetFriendlinessModifierIfGetsClue(otherActorId, clueId);
	}
};

} // End of namespace BladeRunner

#endif
