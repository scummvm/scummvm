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
public:
	AIScriptBase(BladeRunnerEngine *vm) : ScriptBase(vm) {}

	virtual void Initialize() = 0;
	virtual bool Update() = 0;
	virtual void TimerExpired(int timer) = 0;
	virtual void CompletedMovementTrack() = 0;
	virtual void ReceivedClue(int clueId, int fromActorId) = 0;
	virtual void ClickedByPlayer() = 0;
	virtual void EnteredScene(int setId) = 0;
	virtual void OtherAgentEnteredThisScene(int otherActorId) = 0;
	virtual void OtherAgentExitedThisScene(int otherActorId) = 0;
	virtual void OtherAgentEnteredCombatMode(int otherActorId, int combatMode) = 0;
	virtual void ShotAtAndMissed() = 0;
	virtual void ShotAtAndHit() = 0;
	virtual void Retired(int byActorId) = 0;
	virtual int GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) = 0;
	virtual bool GoalChanged(int currentGoalNumber, int newGoalNumber) = 0;
	virtual bool UpdateAnimation(int *animation, int *frame) = 0;
	virtual bool ChangeAnimationMode(int mode) = 0;
	virtual void QueryAnimationState(int *animationState, int *animationFrame, int *a3, int *a4) = 0;
	virtual void SetAnimationState(int animationState, int animationFrame, int a3, int a4) = 0;
	virtual bool ReachedMovementTrackWaypoint(int waypointId) = 0;
	virtual void FledCombat() = 0;
};

#define DECLARE_SCRIPT(name) \
class AIScript##name : public AIScriptBase { \
public: \
	AIScript##name(BladeRunnerEngine *vm) \
		: AIScriptBase(vm) \
	{} \
	void Initialize(); \
	bool Update(); \
	void TimerExpired(int timer); \
	void CompletedMovementTrack(); \
	void ReceivedClue(int clueId, int fromActorId); \
	void ClickedByPlayer(); \
	void EnteredScene(int setId); \
	void OtherAgentEnteredThisScene(int otherActorId); \
	void OtherAgentExitedThisScene(int otherActorId); \
	void OtherAgentEnteredCombatMode(int otherActorId, int combatMode); \
	void ShotAtAndMissed(); \
	void ShotAtAndHit(); \
	void Retired(int byActorId); \
	int GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId); \
	bool GoalChanged(int currentGoalNumber, int newGoalNumber); \
	bool UpdateAnimation(int *animation, int *frame); \
	bool ChangeAnimationMode(int mode); \
	void QueryAnimationState(int *animationState, int *animationFrame, int *a3, int *a4); \
	void SetAnimationState(int animationState, int animationFrame, int a3, int a4); \
	bool ReachedMovementTrackWaypoint(int waypointId); \
	void FledCombat(); \
private:
#define END_SCRIPT };

DECLARE_SCRIPT(McCoy)
	int dword_45A0D0_animation_state;
	int dword_45A0D4_animation_frame;
	int dword_45A0D8;
	int dword_45A0DC;
	int dword_45A0E0;
	int dword_45A0E4;
	float off_45A0EC;
	int dword_45A0E8;
	int dword_45A0F0;
	int dword_45A0F4;
	int dword_45A0F8;
	int dword_45A0FC;
	int dword_462718;
	int dword_46271C;
	float off_45A100;
	float flt_462710;
	float flt_462714;
	
	void sub_4053E0();
	void sub_4054F0();
	void sub_405660();
	void sub_405800();
	void sub_4058B0();
	void sub_405920();
	void sub_405940(float a1);
	void sub_4059D0(float a1);
END_SCRIPT

DECLARE_SCRIPT(Runciter)
	int var_45CD70_animation_state;
	int var_45CD74_animation_frame;
	int var_462800;
	int var_462804;
	int var_45CD78;
	int var_45CD7C;
	int var_45CD80;
	int var_45CD84;
	int var_45CD88;
END_SCRIPT

DECLARE_SCRIPT(OfficerLeary)
	int var_45D5B0_animation_state;
	int var_45D5B4_animation_frame;
	int var_45D5B8;
	int var_45D5BC;
	int var_462880;
	int var_462884;

	bool sub_431408();
	bool sub_431420();
END_SCRIPT

DECLARE_SCRIPT(Leon)
	int var_45EDB0_animation_state;
	int var_45EDB4_animation_frame;
	int var_45EDAC;
	int var_462AF0;
	int var_462AF4;
	float var_45EDA0_z;
	float var_45EDA4_y;
	float var_45EDA8_x;

	float sub_446700(int actorId, float x, float y, float z);
END_SCRIPT

#undef DECLARE_SCRIPT
#undef END_SCRIPT

class AIScripts {
private:
	BladeRunnerEngine *_vm;
	int                _inScriptCounter;
	int                _actorsCount;
	AIScriptBase     **_AIScripts;
	bool              *_actorUpdating;
public:
	AIScripts(BladeRunnerEngine *vm, int actorsCount);
	~AIScripts();

	void Initialize(int actor);
	void Update(int actor);
	void TimerExpired(int actor, int timer);
	void CompletedMovementTrack(int actor);
	void EnteredScene(int actor, int setId);
	void OtherAgentEnteredThisScene(int actor, int otherActorId);
	void OtherAgentExitedThisScene(int actor, int otherActorId);
	void Retired(int actor, int retiredByActorId);
	void GoalChanged(int actor, int currentGoalNumber, int newGoalNumber);
	bool ReachedMovementTrackWaypoint(int actor, int waypointId);
	void UpdateAnimation(int actor, int *animation, int *frame);
	void ChangeAnimationMode(int actor, int mode);

	bool IsInsideScript() const { return _inScriptCounter > 0; }
};

} // End of namespace BladeRunner

#endif
