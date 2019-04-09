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

#include "bladerunner/script/ai_script.h"

namespace BladeRunner {
enum kGenericWalkerAStates {
	kGenericWalkerAStatesIdle = 0,
	kGenericWalkerAStatesWalk = 1,
	kGenericWalkerAStatesDie  = 2,
	kGenericWalkerAStatesGun  = 3
};

AIScriptGenericWalkerA::AIScriptGenericWalkerA(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	isInside = false;
	deltaX = 0.0f;
	deltaZ = 0.0f;
}

void AIScriptGenericWalkerA::Initialize() {
	_animationState = kGenericWalkerAStatesIdle;
	_animationFrame = 0;
	_animationStateNext = 0;
	isInside = false;
	deltaX = 0.0f;
	deltaZ = 0.0f;
	Actor_Set_Goal_Number(kActorGenwalkerA, 0);
}

bool AIScriptGenericWalkerA::Update() {
	switch (Actor_Query_Goal_Number(kActorGenwalkerA)) {
		case 0:
			if (prepareWalker()) {
				return true;
			}
			break;
		case 1:
			if (deltaX != 0.0f || deltaZ != 0.0f) {
				movingUpdate();
			}
			break;
		case 200: // Automatic gun at Bullet Bob
			Actor_Face_Actor(kActorGenwalkerA, kActorMcCoy, true);
			break;
	}
	return false;
}

void AIScriptGenericWalkerA::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask2) {
		AI_Countdown_Timer_Reset(kActorGenwalkerA, kActorTimerAIScriptCustomTask2);
		Game_Flag_Reset(kFlagGenericWalkerWaiting);
		return;// true;
	}
	//return false;
}

void AIScriptGenericWalkerA::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorGenwalkerA) > 0) {
		Actor_Set_Goal_Number(kActorGenwalkerA, 0);
		if (!Game_Flag_Query(kFlagGenericWalkerWaiting)) {
			Game_Flag_Set(kFlagGenericWalkerWaiting);
			AI_Countdown_Timer_Reset(kActorGenwalkerA, kActorTimerAIScriptCustomTask2);
			AI_Countdown_Timer_Start(kActorGenwalkerA, kActorTimerAIScriptCustomTask2, Random_Query(6, 10));
		}
		// return true;
	}
	// return false;
}

void AIScriptGenericWalkerA::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptGenericWalkerA::ClickedByPlayer() {
	Actor_Face_Actor(kActorMcCoy, kActorGenwalkerA, true);
	if (Actor_Query_Goal_Number(kActorGenwalkerA) == 200) {
		Actor_Says(kActorMcCoy, 5290, 18);
	} else {
		switch (Random_Query(1, 10)) {
		case 1:
			Actor_Says(kActorMcCoy, 365, 3);
			break;
		case 2:
			Actor_Says(kActorMcCoy, 755, 3);
			break;
		case 3:
			Actor_Says(kActorMcCoy, 940, 3);
			break;
		case 4:
			Actor_Says(kActorMcCoy, 4560, 3);
			break;
		case 5:
			Actor_Says(kActorMcCoy, 4870, 3);
			break;
		case 6:
			Actor_Says(kActorMcCoy, 5125, 3);
			break;
		case 7:
			Actor_Says(kActorMcCoy, 8450, 3);
			break;
		case 8:
			Actor_Says(kActorMcCoy, 1085, 3);
			break;
		case 9:
			Actor_Says(kActorMcCoy, 365, 3);
			break;
		case 10:
			Actor_Says(kActorMcCoy, 7415, 3);
			break;
		}
	}
	//return false;
}

void AIScriptGenericWalkerA::EnteredScene(int sceneId) {
	//return false;
}

void AIScriptGenericWalkerA::OtherAgentEnteredThisScene(int otherActorId) {
	//return false;
}

void AIScriptGenericWalkerA::OtherAgentExitedThisScene(int otherActorId) {
	if (Actor_Query_Goal_Number(kActorGenwalkerA) && otherActorId == kActorMcCoy) {
		Actor_Set_Goal_Number(kActorGenwalkerA, 0);
	}
	//return false;
}

void AIScriptGenericWalkerA::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	//return false;
}

void AIScriptGenericWalkerA::ShotAtAndMissed() {
	//return false;
}

bool AIScriptGenericWalkerA::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorGenwalkerA)) {
		AI_Movement_Track_Flush(kActorGenwalkerA);
		_animationState = kGenericWalkerAStatesDie;
		_animationFrame = 0;
		Sound_Play(kSfxPOTSPL5, 100, 0, 0, 50);
		movingStart();
		return true;
	}
	return false;
}

void AIScriptGenericWalkerA::Retired(int byActorId) {
	//return false;
}

int AIScriptGenericWalkerA::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptGenericWalkerA::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 0) {
		AI_Movement_Track_Flush(kActorGenwalkerA);
		Actor_Put_In_Set(kActorGenwalkerA, kSetFreeSlotH);
		Global_Variable_Set(kVariableGenericWalkerAModel, -1);
		return false;
	} else if (newGoalNumber == 1) {
		return true;
	} else if (newGoalNumber == 200) {
		Actor_Put_In_Set(kActorGenwalkerA, kSetRC04);
		Actor_Set_At_XYZ(kActorGenwalkerA, 0.0, 36.0, -172.0, 491);
		Actor_Change_Animation_Mode(kActorGenwalkerA, kAnimationModeCombatIdle);
		return true;
	}
	return false;
}

bool AIScriptGenericWalkerA::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case kGenericWalkerAStatesIdle:
		switch (Global_Variable_Query(kVariableGenericWalkerAModel)) {
		case 0:
			*animation = 426;
			break;
		case 1:
			*animation = 430;
			break;
		case 2:
			*animation = 437;
			break;
		case 3:
			*animation = 431;
			break;
		case 4:
			*animation = 427;
			break;
		case 5:
			*animation = 433;
			break;
		}
		_animationFrame = 0;
		break;
	case kGenericWalkerAStatesWalk:
		switch (Global_Variable_Query(kVariableGenericWalkerAModel)){
		case 0:
			*animation = 424;
			break;
		case 1:
			*animation = 428;
			break;
		case 2:
			*animation = 436;
			break;
		case 3:
			*animation = 429;
			break;
		case 4:
			*animation = 425;
			break;
		case 5:
			*animation = 432;
			break;
		}
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;
	case kGenericWalkerAStatesDie:
		*animation = 874;
		++_animationFrame;
		if (++_animationFrame >= Slice_Animation_Query_Number_Of_Frames(874))
		{
			_animationFrame = 0;
			Actor_Set_Goal_Number(kActorGenwalkerA, 0);
			_animationState = kGenericWalkerAStatesIdle;
			deltaX = 0.0f;
			deltaZ = 0.0f;
		}
		break;
	case kGenericWalkerAStatesGun:
		*animation = 440;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(440)) {
			_animationFrame = 0;
		}
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptGenericWalkerA::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		_animationState = kGenericWalkerAStatesIdle;
		_animationFrame = 0;
		break;
	case kAnimationModeWalk:
		_animationState = kGenericWalkerAStatesWalk;
		_animationFrame = 0;
		break;
	case kAnimationModeCombatIdle:
		_animationState = kGenericWalkerAStatesGun;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptGenericWalkerA::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptGenericWalkerA::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptGenericWalkerA::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptGenericWalkerA::FledCombat() {
	//return false;
}

void AIScriptGenericWalkerA::movingStart() {
	float mccoyX, mccoyY, mccoyZ;
	float walkerX, walkerY, walkerZ;

	Actor_Query_XYZ(kActorMcCoy, &mccoyX, &mccoyY, &mccoyZ);
	Actor_Query_XYZ(kActorGenwalkerA, &walkerX, &walkerY, &walkerZ);

	deltaX = walkerX - mccoyX;
	deltaZ = walkerZ - mccoyZ;

	float dist = sqrt(deltaX * deltaX + deltaZ * deltaZ);
	if (dist == 0.0f) {
		deltaZ = 0.0f;
		deltaX = 0.0f;
	} else {
		deltaX *= 10.0f / dist;
		deltaZ *= 10.0f / dist;
	}
}

void AIScriptGenericWalkerA::movingUpdate() {
	float walkerX, walkerY, walkerZ;

	Actor_Query_XYZ(kActorGenwalkerA, &walkerX, &walkerY, &walkerZ);
	int facing = Actor_Query_Facing_1024(kActorGenwalkerA);

	walkerX += deltaX;
	walkerZ += deltaZ;

	deltaX = deltaX * 0.97f;
	deltaZ = deltaZ * 0.97f;

	Actor_Set_At_XYZ(kActorGenwalkerA, walkerX, walkerY, walkerZ, facing);
}

bool AIScriptGenericWalkerA::prepareWalker() {
	if (Game_Flag_Query(kFlagGenericWalkerWaiting) || Global_Variable_Query(kVariableGenericWalkerConfig) < 0 || !preparePath()) {
		return false;
	}

	int model = 0;
	do {
		if (isInside) {
			model = Random_Query(3, 5);
		} else {
			model = Random_Query(0, 5);
		}
	} while (model == Global_Variable_Query(kVariableGenericWalkerBModel) || model == Global_Variable_Query(kVariableGenericWalkerCModel));

	Global_Variable_Set(kVariableGenericWalkerAModel, model);
	Game_Flag_Set(kFlagGenericWalkerWaiting);
	AI_Countdown_Timer_Reset(kActorGenwalkerA, kActorTimerAIScriptCustomTask2);
	AI_Countdown_Timer_Start(kActorGenwalkerA, kActorTimerAIScriptCustomTask2, Random_Query(4, 12));
	Actor_Set_Goal_Number(kActorGenwalkerA, 1);
	return true;
}

bool AIScriptGenericWalkerA::preparePath() {
	AI_Movement_Track_Flush(kActorGenwalkerA);
	int set = Player_Query_Current_Set();

	if (set == kSetAR01_AR02) {
		isInside = false;
		int waypointStart = Random_Query(155, 158);
		int waypointEnd = 0;
		AI_Movement_Track_Append(kActorGenwalkerA, waypointStart, 0);
		do {
			waypointEnd = Random_Query(155, 158);
		} while (waypointEnd == waypointStart);
		if ((waypointStart == 155 || waypointStart == 156) && (waypointEnd == 157 || waypointEnd == 158)) {
			AI_Movement_Track_Append(kActorGenwalkerA, 159, 0);
			AI_Movement_Track_Append(kActorGenwalkerA, 160, 0);
			if (Random_Query(0, 3) == 0) {
				AI_Movement_Track_Append_With_Facing(kActorGenwalkerA, 161, Random_Query(15, 30), 904);
			}
		} else if ((waypointEnd == 155 || waypointEnd == 156) && (waypointStart == 157 || waypointStart == 158)) {
			if (Random_Query(0, 3) == 0) {
				AI_Movement_Track_Append_With_Facing(kActorGenwalkerA, 161, Random_Query(15, 30), 904);
			}
			AI_Movement_Track_Append(kActorGenwalkerA, 160, 0);
			AI_Movement_Track_Append(kActorGenwalkerA, 159, 0);
		} else if ((waypointStart == 155 && waypointEnd == 156) || (waypointStart == 156 && waypointEnd == 155)) {
			AI_Movement_Track_Append(kActorGenwalkerA, 159, 0);
		}
		AI_Movement_Track_Append(kActorGenwalkerA, waypointEnd, 0);
		AI_Movement_Track_Repeat(kActorGenwalkerA);
		return true;
	}

	if (set == kSetCT01_CT12) {
		isInside = false;
		if (Random_Query(0, 1)) {
			AI_Movement_Track_Append(kActorGenwalkerA, 54, 1);
			if (Random_Query(1, 3) == 1) {
				AI_Movement_Track_Append(kActorGenwalkerA, 56, 0);
				AI_Movement_Track_Append(kActorGenwalkerA, 43, 1);
			} else {
				AI_Movement_Track_Append(kActorGenwalkerA, 53, 1);
			}
			AI_Movement_Track_Append(kActorGenwalkerA, 40, 1);
			AI_Movement_Track_Repeat(kActorGenwalkerA);
		} else {
			AI_Movement_Track_Append(kActorGenwalkerA, 53, 1);
			if (Random_Query(1, 3) == 1) {
				AI_Movement_Track_Append(kActorGenwalkerA, 43, 1);
			} else {
				AI_Movement_Track_Append(kActorGenwalkerA, 54, 1);
			}
			AI_Movement_Track_Append(kActorGenwalkerA, 40, 1);
			AI_Movement_Track_Repeat(kActorGenwalkerA);
		}
		return true;
	}

	if (set == kSetHC01_HC02_HC03_HC04) {
		isInside = true;
		if (Random_Query(0, 1)) {
			AI_Movement_Track_Append(kActorGenwalkerA, 164, 0);
			if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorGenwalkerA, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerA, 162, 0);
			} else if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorGenwalkerA, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerA, 162, 0);
			} else {
				AI_Movement_Track_Append(kActorGenwalkerA, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerA, 162, 0);
			}
		} else {
			AI_Movement_Track_Append(kActorGenwalkerA, 162, 0);
			if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorGenwalkerA, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerA, 164, 0);
			} else {
				if (Random_Query(0, 1)) {
					AI_Movement_Track_Append_With_Facing(kActorGenwalkerA, 166, 0, 30);
				}
				AI_Movement_Track_Append(kActorGenwalkerA, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerA, 164, 0);
			}
		}
		AI_Movement_Track_Repeat(kActorGenwalkerA);
		return true;
	}

	if (set == kSetRC03) {
		isInside = false;
		int waypointStart = 0;
		int waypointEnd = 0;
		do {
			waypointStart = Random_Query(167, 171);
		} while (waypointStart == 168 || waypointStart == 169);
		do {
			waypointEnd = Random_Query(167, 171);
		} while (waypointEnd == waypointStart || waypointEnd == 168 || waypointEnd == 169);
		AI_Movement_Track_Append(kActorGenwalkerA, waypointStart, 0);
		if (waypointStart == 170) {
			AI_Movement_Track_Append(kActorGenwalkerA, 169, 0);
			AI_Movement_Track_Append(kActorGenwalkerA, 168, 0);
		} else if (waypointEnd == 170) {
			AI_Movement_Track_Append(kActorGenwalkerA, 168, 0);
			AI_Movement_Track_Append(kActorGenwalkerA, 169, 0);
		}
		AI_Movement_Track_Append(kActorGenwalkerA, waypointEnd, 0);
		AI_Movement_Track_Repeat(kActorGenwalkerA);
		return true;
	}

	return false;
}

} // End of namespace BladeRunner
