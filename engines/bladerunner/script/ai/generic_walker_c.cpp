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
enum kGenericWalkerCStates {
	kGenericWalkerCStatesIdle = 0,
	kGenericWalkerCStatesWalk = 1,
	kGenericWalkerCStatesDie  = 2,
	kGenericWalkerCStatesGun  = 3
};

AIScriptGenericWalkerC::AIScriptGenericWalkerC(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	isInside = false;
	deltaX = 0.0f;
	deltaZ = 0.0f;
}

void AIScriptGenericWalkerC::Initialize() {
	_animationState = kGenericWalkerCStatesIdle;
	_animationFrame = 0;
	_animationStateNext = 0;
	isInside = false;
	deltaX = 0.0f;
	deltaZ = 0.0f;
	Actor_Set_Goal_Number(kActorGenwalkerC, kGoalGenwalkerDefault);
}

bool AIScriptGenericWalkerC::Update() {
	switch (Actor_Query_Goal_Number(kActorGenwalkerC)) {
		case kGoalGenwalkerDefault:
			if (prepareWalker()) {
				return true;
			}
			break;
		case kGoalGenwalkerMoving:
			if (deltaX != 0.0f || deltaZ != 0.0f) {
				movingUpdate();
			}
			break;
	}
	return false;
}

void AIScriptGenericWalkerC::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask2) {
		AI_Countdown_Timer_Reset(kActorGenwalkerC, kActorTimerAIScriptCustomTask2);
		Game_Flag_Reset(kFlagGenericWalkerWaiting);
		return;// true;
	}
	//return false;
}

void AIScriptGenericWalkerC::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorGenwalkerC) > kGoalGenwalkerDefault) {
		Actor_Set_Goal_Number(kActorGenwalkerC, kGoalGenwalkerDefault);
		if (!Game_Flag_Query(kFlagGenericWalkerWaiting)) {
			Game_Flag_Set(kFlagGenericWalkerWaiting);
			AI_Countdown_Timer_Reset(kActorGenwalkerC, kActorTimerAIScriptCustomTask2);
			AI_Countdown_Timer_Start(kActorGenwalkerC, kActorTimerAIScriptCustomTask2, Random_Query(6, 10));
		}
		// return true;
	}
	// return false;
}

void AIScriptGenericWalkerC::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptGenericWalkerC::ClickedByPlayer() {
	Actor_Face_Actor(kActorMcCoy, kActorGenwalkerC, true);
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
		Actor_Says(kActorMcCoy, 365, 3); // Re-used line, same as case 1
		break;
	case 10:
		Actor_Says(kActorMcCoy, 7415, 3);
		break;
	}
	//return false;
}

void AIScriptGenericWalkerC::EnteredSet(int setId) {
	//return false;
}

void AIScriptGenericWalkerC::OtherAgentEnteredThisSet(int otherActorId) {
	//return false;
}

void AIScriptGenericWalkerC::OtherAgentExitedThisSet(int otherActorId) {
	if (Actor_Query_Goal_Number(kActorGenwalkerC) > kGoalGenwalkerDefault && otherActorId == kActorMcCoy) {
		Actor_Set_Goal_Number(kActorGenwalkerC, kGoalGenwalkerDefault);
	}
	//return false;
}

void AIScriptGenericWalkerC::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	//return false;
}

void AIScriptGenericWalkerC::ShotAtAndMissed() {
	//return false;
}

bool AIScriptGenericWalkerC::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorGenwalkerC) > kGoalGenwalkerDefault) {
		AI_Movement_Track_Flush(kActorGenwalkerC);
		_animationState = kGenericWalkerCStatesDie;
		_animationFrame = 0;
		Sound_Play(kSfxPOTSPL5, 100, 0, 0, 50);
		movingStart();
		return true;
	}
	return false;
}

void AIScriptGenericWalkerC::Retired(int byActorId) {
	//return false;
}

int AIScriptGenericWalkerC::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptGenericWalkerC::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == kGoalGenwalkerDefault) {
		AI_Movement_Track_Flush(kActorGenwalkerC);
		Actor_Put_In_Set(kActorGenwalkerC, kSetFreeSlotH);
		Global_Variable_Set(kVariableGenericWalkerCModel, -1);
		return false;
	} else if (newGoalNumber == kGoalGenwalkerMoving) {
		return true;
	}
	return false;
}

bool AIScriptGenericWalkerC::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case kGenericWalkerCStatesIdle:
		switch (Global_Variable_Query(kVariableGenericWalkerCModel)) {
		case 0:
			*animation = 426;
			break;
		case 1:
			*animation = 430;
			break;
		case 2:
#if BLADERUNNER_ORIGINAL_BUGS
			// Hatted lady with wooden umbrella still (different from 436 model!)
			*animation = 437;
#else
			// use model 436 and animation frame 4
			*animation = 436;
			_animationFrame = 4;
#endif // BLADERUNNER_ORIGINAL_BUGS
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
		case 6:
			*animation = 434; // Hatted child walking // frame 11 could be used for still
			_animationFrame = 11;
			break;
		case 7:
			*animation = 435; // Child walking // frame 5 or 0 could be used for still
			_animationFrame = 0;
			break;
		case 8:
			*animation = 422; // Hatted person walking fast // frame 1 could be used for still
			_animationFrame = 1;
			break;
		case 9:
			*animation = 423; // Hatted person walking lowered face // frame 6 could be used for still
			_animationFrame = 6;
			break;
		}
		if (!_vm->_cutContent
		    || (Global_Variable_Query(kVariableGenericWalkerCModel) < 6 && Global_Variable_Query(kVariableGenericWalkerCModel) != 2)
		) {
			_animationFrame = 0;
		}
		break;
	case kGenericWalkerCStatesWalk:
		switch (Global_Variable_Query(kVariableGenericWalkerCModel)) {
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
		case 6:
			*animation = 434; // Hatted child walking
			break;
		case 7:
			*animation = 435; // Child walking
			break;
		case 8:
			*animation = 422; // Hatted person walking fast
			break;
		case 9:
			*animation = 423; // Hatted person walking lowered face
			break;
		}
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;
	case kGenericWalkerCStatesDie:
		// This is an animation for Maggie (exploding) but is also used for generic death states (rats, generic walkers)
		// probably for debug purposes
		*animation = 874;
		++_animationFrame;
		if (++_animationFrame >= Slice_Animation_Query_Number_Of_Frames(874))
		{
			_animationFrame = 0;
			Actor_Set_Goal_Number(kActorGenwalkerC, kGoalGenwalkerDefault);
			_animationState = kGenericWalkerCStatesIdle;
			deltaX = 0.0f;
			deltaZ = 0.0f;
		}
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptGenericWalkerC::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		_animationState = kGenericWalkerCStatesIdle;
		if (!_vm->_cutContent
		    || (Global_Variable_Query(kVariableGenericWalkerCModel) < 6 && Global_Variable_Query(kVariableGenericWalkerCModel) != 2)
		) {
			_animationFrame = 0;
		}
		break;
	case kAnimationModeWalk:
		_animationState = kGenericWalkerCStatesWalk;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptGenericWalkerC::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptGenericWalkerC::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptGenericWalkerC::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptGenericWalkerC::FledCombat() {
	//return false;
}

void AIScriptGenericWalkerC::movingStart() {
	float mccoyX, mccoyY, mccoyZ;
	float walkerX, walkerY, walkerZ;

	Actor_Query_XYZ(kActorMcCoy, &mccoyX, &mccoyY, &mccoyZ);
	Actor_Query_XYZ(kActorGenwalkerC, &walkerX, &walkerY, &walkerZ);

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

void AIScriptGenericWalkerC::movingUpdate() {
	float walkerX, walkerY, walkerZ;

	Actor_Query_XYZ(kActorGenwalkerC, &walkerX, &walkerY, &walkerZ);
	int facing = Actor_Query_Facing_1024(kActorGenwalkerC);

	walkerX += deltaX;
	walkerZ += deltaZ;

	deltaX = deltaX * 0.97f;
	deltaZ = deltaZ * 0.97f;

	Actor_Set_At_XYZ(kActorGenwalkerC, walkerX, walkerY, walkerZ, facing);
}

bool AIScriptGenericWalkerC::prepareWalker() {
	if (Game_Flag_Query(kFlagGenericWalkerWaiting) || Global_Variable_Query(kVariableGenericWalkerConfig) < 2 || !preparePath()) {
		return false;
	}

	int model = 0;
	do {
		if (isInside) {
			model = Random_Query(3, 5); // 0, 1, 2 models have umbrellas so they should be in outdoors locations
		} else {
			if (_vm->_cutContent) {
				model = Random_Query(0, 9);
			} else {
				model = Random_Query(0, 5);
			}
		}
		// this while loop ensures choosing a different model for Walker C than the Walker A or Walker B
	} while (model == Global_Variable_Query(kVariableGenericWalkerAModel) || model == Global_Variable_Query(kVariableGenericWalkerBModel));


	Global_Variable_Set(kVariableGenericWalkerCModel, model);
	Game_Flag_Set(kFlagGenericWalkerWaiting);
	AI_Countdown_Timer_Reset(kActorGenwalkerC, kActorTimerAIScriptCustomTask2);
	AI_Countdown_Timer_Start(kActorGenwalkerC, kActorTimerAIScriptCustomTask2, Random_Query(4, 12));
	Actor_Set_Goal_Number(kActorGenwalkerC, kGoalGenwalkerMoving);
	return true;
}

bool AIScriptGenericWalkerC::preparePath() {
	AI_Movement_Track_Flush(kActorGenwalkerC);
	int set = Player_Query_Current_Set();

	if (set == kSetAR01_AR02) {
		isInside = false;
		int waypointStart = Random_Query(155, 158);
		int waypointEnd = 0;
		AI_Movement_Track_Append(kActorGenwalkerC, waypointStart, 0);
		do {
			waypointEnd = Random_Query(155, 158);
		} while (waypointEnd == waypointStart);
		if ((waypointStart == 155 || waypointStart == 156) && (waypointEnd == 157 || waypointEnd == 158)) {
			AI_Movement_Track_Append(kActorGenwalkerC, 159, 0);
			AI_Movement_Track_Append(kActorGenwalkerC, 160, 0);
			if (Random_Query(0, 3) == 0) {
				AI_Movement_Track_Append_With_Facing(kActorGenwalkerC, 161, Random_Query(15, 30), 904);
			}
		} else if ((waypointEnd == 155 || waypointEnd == 156) && (waypointStart == 157 || waypointStart == 158)) {
			if (Random_Query(0, 3) == 0) {
				AI_Movement_Track_Append_With_Facing(kActorGenwalkerC, 161, Random_Query(15, 30), 904);
			}
			AI_Movement_Track_Append(kActorGenwalkerC, 160, 0);
			AI_Movement_Track_Append(kActorGenwalkerC, 159, 0);
		} else if ((waypointStart == 155 && waypointEnd == 156) || (waypointStart == 156 && waypointEnd == 155)) {
			AI_Movement_Track_Append(kActorGenwalkerC, 159, 0);
		}
		AI_Movement_Track_Append(kActorGenwalkerC, waypointEnd, 0);
		AI_Movement_Track_Repeat(kActorGenwalkerC);
		return true;
	}

	if (set == kSetCT01_CT12) {
		isInside = false;
		if (Random_Query(0, 1)) {
			AI_Movement_Track_Append(kActorGenwalkerC, 54, 1);
			if (Random_Query(1, 3) == 1) {
				AI_Movement_Track_Append(kActorGenwalkerC, 56, 0);
				AI_Movement_Track_Append(kActorGenwalkerC, 43, 1);
			} else {
				AI_Movement_Track_Append(kActorGenwalkerC, 53, 1);
			}
			AI_Movement_Track_Append(kActorGenwalkerC, 40, 1);
			AI_Movement_Track_Repeat(kActorGenwalkerC);
		} else {
			AI_Movement_Track_Append(kActorGenwalkerC, 53, 1);
			if (Random_Query(1, 3) == 1) {
				AI_Movement_Track_Append(kActorGenwalkerC, 43, 1);
			} else {
				AI_Movement_Track_Append(kActorGenwalkerC, 54, 1);
			}
			AI_Movement_Track_Append(kActorGenwalkerC, 40, 1);
			AI_Movement_Track_Repeat(kActorGenwalkerC);
		}
		return true;
	}

	if (set == kSetHC01_HC02_HC03_HC04) {
		isInside = true;
		if (Random_Query(0, 1)) {
			AI_Movement_Track_Append(kActorGenwalkerC, 164, 0);
			if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorGenwalkerC, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerC, 162, 0);
			} else if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorGenwalkerC, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerC, 162, 0);
			} else {
				AI_Movement_Track_Append(kActorGenwalkerC, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerC, 162, 0);
			}
		} else {
			AI_Movement_Track_Append(kActorGenwalkerC, 162, 0);
			if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorGenwalkerC, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerC, 164, 0);
			} else {
				if (Random_Query(0, 1)) {
					AI_Movement_Track_Append_With_Facing(kActorGenwalkerC, 166, 0, 30);
				}
				AI_Movement_Track_Append(kActorGenwalkerC, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerC, 164, 0);
			}
		}
		AI_Movement_Track_Repeat(kActorGenwalkerC);
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
		AI_Movement_Track_Append(kActorGenwalkerC, waypointStart, 0);
		if (waypointStart == 170) {
			AI_Movement_Track_Append(kActorGenwalkerC, 169, 0);
			AI_Movement_Track_Append(kActorGenwalkerC, 168, 0);
		} else if (waypointEnd == 170) {
			AI_Movement_Track_Append(kActorGenwalkerC, 168, 0);
			AI_Movement_Track_Append(kActorGenwalkerC, 169, 0);
		}
		AI_Movement_Track_Append(kActorGenwalkerC, waypointEnd, 0);
		AI_Movement_Track_Repeat(kActorGenwalkerC);
		return true;
	}

	return false;
}

} // End of namespace BladeRunner
