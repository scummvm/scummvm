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
	Actor_Set_Goal_Number(kActorGenwalkerA, kGoalGenwalkerDefault);
}

bool AIScriptGenericWalkerA::Update() {
#if !BLADERUNNER_ORIGINAL_BUGS
	// extra check for possible fix of Bullet Bob's gun missing
	if (Player_Query_Current_Set() == kSetRC04
	    && Actor_Query_Goal_Number(kActorGenwalkerA) != kGoalGenwalkerABulletBobsTrackGun
	) {
		Actor_Set_Goal_Number(kActorGenwalkerA, kGoalGenwalkerABulletBobsTrackGun);
	}
#endif // !BLADERUNNER_ORIGINAL_BUGS
	switch (Actor_Query_Goal_Number(kActorGenwalkerA)) {
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
		case kGoalGenwalkerABulletBobsTrackGun: // Automatic gun at Bullet Bob
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
#if !BLADERUNNER_ORIGINAL_BUGS
	// extra check for possible fix of Bullet Bob's gun missing
	if (Player_Query_Current_Set() == kSetRC04
	    && Actor_Query_Goal_Number(kActorGenwalkerA) != kGoalGenwalkerABulletBobsTrackGun
	) {
		Actor_Set_Goal_Number(kActorGenwalkerA, kGoalGenwalkerABulletBobsTrackGun);
		return;
	}
#endif // !BLADERUNNER_ORIGINAL_BUGS

	if (Actor_Query_Goal_Number(kActorGenwalkerA) > kGoalGenwalkerDefault) {
		Actor_Set_Goal_Number(kActorGenwalkerA, kGoalGenwalkerDefault);
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
	if (Actor_Query_Goal_Number(kActorGenwalkerA) == kGoalGenwalkerABulletBobsTrackGun) {
		Actor_Says(kActorMcCoy, 5290, 18);   // kActorGenwalkerA here is actually the tracking gun in Bullet Bob's
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
			Actor_Says(kActorMcCoy, 365, 3); // Re-used line, same as case 1
			break;
		case 10:
			Actor_Says(kActorMcCoy, 7415, 3);
			break;
		}
	}
	//return false;
}

void AIScriptGenericWalkerA::EnteredSet(int setId) {
	//return false;
}

void AIScriptGenericWalkerA::OtherAgentEnteredThisSet(int otherActorId) {
	//return false;
}

void AIScriptGenericWalkerA::OtherAgentExitedThisSet(int otherActorId) {
	if (Actor_Query_Goal_Number(kActorGenwalkerA) > kGoalGenwalkerDefault && otherActorId == kActorMcCoy) {
		Actor_Set_Goal_Number(kActorGenwalkerA, kGoalGenwalkerDefault);
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
	if (Actor_Query_Goal_Number(kActorGenwalkerA) > kGoalGenwalkerDefault) {
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
	if (newGoalNumber == kGoalGenwalkerDefault) {
		AI_Movement_Track_Flush(kActorGenwalkerA);
		Actor_Put_In_Set(kActorGenwalkerA, kSetFreeSlotH);
		Global_Variable_Set(kVariableGenericWalkerAModel, -1);
		return false;
	} else if (newGoalNumber == kGoalGenwalkerMoving) {
		return true;
	} else if (newGoalNumber == kGoalGenwalkerABulletBobsTrackGun) {
		// Bullet Bob's tracking gun
#if !BLADERUNNER_ORIGINAL_BUGS
		// Possible bug fix for disappearing gun - don't allow track complete events to interfere with Gun state
		AI_Movement_Track_Flush(kActorGenwalkerA);
#endif
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
			*animation = 426; // Hatted Person with umbrella still
			break;
		case 1:
			*animation = 430; // Hooded person with umbrella still
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
			*animation = 431; // Person with glasses and beard still
			break;
		case 4:
			*animation = 427; // Hatted Person without umbrella still
			break;
		case 5:
			*animation = 433; // Punk person with glasses still
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
		    || (Global_Variable_Query(kVariableGenericWalkerAModel) < 6 && Global_Variable_Query(kVariableGenericWalkerAModel) != 2)
		) {
			_animationFrame = 0;
		}
		break;
	case kGenericWalkerAStatesWalk:
		switch (Global_Variable_Query(kVariableGenericWalkerAModel)) {
		case 0:
			*animation = 424; // Hatted person with umbrella walking
			break;
		case 1:
			*animation = 428; // Hooded person with umbrella walking
			break;
		case 2:
			*animation = 436; // Hatted person with wooden umbrella walking
			break;
		case 3:
			*animation = 429; // Person with glasses and beard walking
			break;
		case 4:
			*animation = 425; // Hatted Person without umbrella - walking small steps
			break;
		case 5:
			*animation = 432; // Punk person with glasses walking
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
	case kGenericWalkerAStatesDie:
		// This is an animation for Maggie (exploding) but is also used for generic death states (rats, generic walkers)
		// probably for debug purposes
		*animation = 874;
		++_animationFrame;
		if (++_animationFrame >= Slice_Animation_Query_Number_Of_Frames(874))
		{
			_animationFrame = 0;
			Actor_Set_Goal_Number(kActorGenwalkerA, kGoalGenwalkerDefault);
			_animationState = kGenericWalkerAStatesIdle;
			deltaX = 0.0f;
			deltaZ = 0.0f;
		}
		break;
	case kGenericWalkerAStatesGun:
		*animation = kModelAnimationBulletBobsTrackingGun;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobsTrackingGun)) {
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
		if (!_vm->_cutContent
		    || (Global_Variable_Query(kVariableGenericWalkerAModel) < 6 && Global_Variable_Query(kVariableGenericWalkerAModel) != 2)
		) {
			_animationFrame = 0;
		}
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
			model = Random_Query(3, 5); // 0, 1, 2 models have umbrellas so they should be in outdoors locations
		} else {
			if (_vm->_cutContent) {
				model = Random_Query(0, 9);
			} else {
				model = Random_Query(0, 5);
			}
		}
		// this while loop ensures choosing a different model for Walker A than the Walker B or Walker C
	} while (model == Global_Variable_Query(kVariableGenericWalkerBModel) || model == Global_Variable_Query(kVariableGenericWalkerCModel));

	Global_Variable_Set(kVariableGenericWalkerAModel, model);
	Game_Flag_Set(kFlagGenericWalkerWaiting);
	AI_Countdown_Timer_Reset(kActorGenwalkerA, kActorTimerAIScriptCustomTask2);
	AI_Countdown_Timer_Start(kActorGenwalkerA, kActorTimerAIScriptCustomTask2, Random_Query(4, 12));
	Actor_Set_Goal_Number(kActorGenwalkerA, kGoalGenwalkerMoving);
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
