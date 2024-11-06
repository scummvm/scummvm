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

#include "bladerunner/script/ai_script.h"

namespace BladeRunner {
enum kGenericWalkerBStates {
	kGenericWalkerBStatesIdle = 0,
	kGenericWalkerBStatesWalk = 1,
	kGenericWalkerBStatesDie  = 2
};

AIScriptGenericWalkerB::AIScriptGenericWalkerB(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	isInside = false;
	deltaX = 0.0f;
	deltaZ = 0.0f;
}

void AIScriptGenericWalkerB::Initialize() {
	_animationState = kGenericWalkerBStatesIdle;
	_animationFrame = 0;
	_animationStateNext = 0;
	isInside = false;
	deltaX = 0.0f;
	deltaZ = 0.0f;
	Actor_Set_Goal_Number(kActorGenwalkerB, kGoalGenwalkerDefault);
}

bool AIScriptGenericWalkerB::Update() {
	switch (Actor_Query_Goal_Number(kActorGenwalkerB)) {
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

void AIScriptGenericWalkerB::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask2) {
		AI_Countdown_Timer_Reset(kActorGenwalkerB, kActorTimerAIScriptCustomTask2);
		Game_Flag_Reset(kFlagGenericWalkerWaiting);
		return;// true;
	}
	//return false;
}

void AIScriptGenericWalkerB::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorGenwalkerB) > kGoalGenwalkerDefault) {
		Actor_Set_Goal_Number(kActorGenwalkerB, kGoalGenwalkerDefault);
		if (!Game_Flag_Query(kFlagGenericWalkerWaiting)) {
			Game_Flag_Set(kFlagGenericWalkerWaiting);
			AI_Countdown_Timer_Reset(kActorGenwalkerB, kActorTimerAIScriptCustomTask2);
			AI_Countdown_Timer_Start(kActorGenwalkerB, kActorTimerAIScriptCustomTask2, Random_Query(6, 10));
		}
		// return true;
	}
	// return false;
}

void AIScriptGenericWalkerB::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptGenericWalkerB::ClickedByPlayer() {
	Actor_Face_Actor(kActorMcCoy, kActorGenwalkerB, true);
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

void AIScriptGenericWalkerB::EnteredSet(int setId) {
	//return false;
}

void AIScriptGenericWalkerB::OtherAgentEnteredThisSet(int otherActorId) {
	//return false;
}

void AIScriptGenericWalkerB::OtherAgentExitedThisSet(int otherActorId) {
	if (Actor_Query_Goal_Number(kActorGenwalkerB) > kGoalGenwalkerDefault && otherActorId == kActorMcCoy) {
		Actor_Set_Goal_Number(kActorGenwalkerB, kGoalGenwalkerDefault);
	}
	//return false;
}

void AIScriptGenericWalkerB::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	//return false;
}

void AIScriptGenericWalkerB::ShotAtAndMissed() {
	//return false;
}

bool AIScriptGenericWalkerB::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorGenwalkerB) > kGoalGenwalkerDefault) {
		AI_Movement_Track_Flush(kActorGenwalkerB);
		_animationState = kGenericWalkerBStatesDie;
		_animationFrame = 0;
		Sound_Play(kSfxPOTSPL5, 100, 0, 0, 50);
		movingStart();
		return true;
	}
	return false;
}

void AIScriptGenericWalkerB::Retired(int byActorId) {
	//return false;
}

int AIScriptGenericWalkerB::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptGenericWalkerB::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == kGoalGenwalkerDefault) {
		AI_Movement_Track_Flush(kActorGenwalkerB);
		Actor_Put_In_Set(kActorGenwalkerB, kSetFreeSlotH);
		Global_Variable_Set(kVariableGenericWalkerBModel, -1);
		return false;
	} else if (newGoalNumber == kGoalGenwalkerMoving) {
		return true;
	}
	return false;
}

bool AIScriptGenericWalkerB::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case kGenericWalkerBStatesIdle:
		switch (Global_Variable_Query(kVariableGenericWalkerBModel)) {
		case 0:
			*animation = kModelGenWalkerHattedPersonWithUmbrellaStandsStill; // Hatted Person with umbrella still
			break;

		case 1:
			*animation = kModelGenWalkerHoodedPersonWithUmbrellaStandsStill; // Hooded person with umbrella still
			break;

		case 2:
#if BLADERUNNER_ORIGINAL_BUGS
			// Hatted lady with wooden umbrella still (different from 436 model!)
			*animation = kModelGenWalkerHattedLadyWithWoodenUmbrellaStandsStill;
#else
			// use model 436 and animation frame 4
			*animation = kModelGenWalkerHattedPersonWithWoodenUmbrella;
			_animationFrame = 4; // frame 4 is used for still
#endif // BLADERUNNER_ORIGINAL_BUGS
			break;

		case 3:
			*animation = kModelGenWalkerPunkPersonWithGlassesAndBeardStandsStill;
			break;

		case 4:
			*animation = kModelGenWalkerHattedPersonNoUmbrellaStandsStill;
			break;

		case 5:
			*animation = kModelGenWalkerPunkPersonWithGlassesStandsStill;
			break;

		case 6:
			*animation = kModelGenWalkerHattedChild; // Hatted child walking
			_animationFrame = 11; // frame 11 used for still
			break;

		case 7:
			*animation = kModelGenWalkerChild; // Child walking
			_animationFrame = 0; // frame 0 used for still (could also use frame 5)
			break;

		case 8:
			*animation = kModelGenWalkerHattedPersonFastPace; // Hatted person walking fast
			_animationFrame = 1; // frame 1 used for still
			break;

		case 9:
			*animation = kModelGenWalkerHattedPersonLoweredFace; // Hatted person walking lowered face
			_animationFrame = 6;  // frame 6 used for still
			break;
		}
		if (!_vm->_cutContent
		    || (Global_Variable_Query(kVariableGenericWalkerBModel) < 6 && Global_Variable_Query(kVariableGenericWalkerBModel) != 2)
		) {
			_animationFrame = 0;
		}
		break;

	case kGenericWalkerBStatesWalk:
		switch (Global_Variable_Query(kVariableGenericWalkerBModel)) {
		case 0:
			*animation = kModelGenWalkerHattedPersonWithUmbrella;
			break;

		case 1:
			*animation = kModelGenWalkerHoodedPersonWithUmbrella;
			break;

		case 2:
			*animation = kModelGenWalkerHattedPersonWithWoodenUmbrella;
			break;

		case 3:
			*animation = kModelGenWalkerPunkPersonWithGlassesAndBeard;
			break;

		case 4:
			*animation = kModelGenWalkerHattedPersonNoUmbrellaSmallSteps;
			break;

		case 5:
			*animation = kModelGenWalkerPunkPersonWithGlasses;
			break;

		case 6:
			*animation = kModelGenWalkerHattedChild; // Hatted child walking
			break;

		case 7:
			*animation = kModelGenWalkerChild; // Child walking
			break;

		case 8:
			*animation = kModelGenWalkerHattedPersonFastPace; // Hatted person walking fast
			break;

		case 9:
			*animation = kModelGenWalkerHattedPersonLoweredFace; // Hatted person walking lowered face
			break;
		}
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case kGenericWalkerBStatesDie:
		// This is an animation for Maggie (exploding) but is also used for generic death states (rats, generic walkers)
		// probably for debug purposes
		*animation = kModelAnimationMaggieExploding;
		++_animationFrame;
		if (++_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieExploding)) {
			_animationFrame = 0;
			Actor_Set_Goal_Number(kActorGenwalkerB, kGoalGenwalkerDefault);
			_animationState = kGenericWalkerBStatesIdle;
			deltaX = 0.0f;
			deltaZ = 0.0f;
		}
		break;

	default:
		debugC(6, kDebugAnimation, "AIScriptGenericWalkerB::UpdateAnimation() - Current _animationState (%d) is not supported", _animationState);
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptGenericWalkerB::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		_animationState = kGenericWalkerBStatesIdle;
		if (!_vm->_cutContent
		    || (Global_Variable_Query(kVariableGenericWalkerBModel) < 6 && Global_Variable_Query(kVariableGenericWalkerBModel) != 2)
		) {
			_animationFrame = 0;
		}
		break;

	case kAnimationModeWalk:
		_animationState = kGenericWalkerBStatesWalk;
		_animationFrame = 0;
		break;

	default:
		debugC(6, kDebugAnimation, "AIScriptGenericWalkerB::ChangeAnimationMode(%d) - Target mode is not supported", mode);
		break;
	}
	return true;
}

void AIScriptGenericWalkerB::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptGenericWalkerB::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptGenericWalkerB::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptGenericWalkerB::FledCombat() {
	//return false;
}

void AIScriptGenericWalkerB::movingStart() {
	float mccoyX, mccoyY, mccoyZ;
	float walkerX, walkerY, walkerZ;

	Actor_Query_XYZ(kActorMcCoy, &mccoyX, &mccoyY, &mccoyZ);
	Actor_Query_XYZ(kActorGenwalkerB, &walkerX, &walkerY, &walkerZ);

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

void AIScriptGenericWalkerB::movingUpdate() {
	float walkerX, walkerY, walkerZ;

	Actor_Query_XYZ(kActorGenwalkerB, &walkerX, &walkerY, &walkerZ);
	int facing = Actor_Query_Facing_1024(kActorGenwalkerB);

	walkerX += deltaX;
	walkerZ += deltaZ;

	deltaX = deltaX * 0.97f;
	deltaZ = deltaZ * 0.97f;

	Actor_Set_At_XYZ(kActorGenwalkerB, walkerX, walkerY, walkerZ, facing);
}

bool AIScriptGenericWalkerB::prepareWalker() {
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
		// this while loop ensures choosing a different model for Walker B than the Walker A or Walker C
	} while (model == Global_Variable_Query(kVariableGenericWalkerAModel) || model == Global_Variable_Query(kVariableGenericWalkerCModel));

	Global_Variable_Set(kVariableGenericWalkerBModel, model);
	Game_Flag_Set(kFlagGenericWalkerWaiting);
	AI_Countdown_Timer_Reset(kActorGenwalkerB, kActorTimerAIScriptCustomTask2);
	AI_Countdown_Timer_Start(kActorGenwalkerB, kActorTimerAIScriptCustomTask2, Random_Query(4, 12));
	Actor_Set_Goal_Number(kActorGenwalkerB, kGoalGenwalkerMoving);
	return true;
}

bool AIScriptGenericWalkerB::preparePath() {
	AI_Movement_Track_Flush(kActorGenwalkerB);
	int set = Player_Query_Current_Set();

	if (set == kSetAR01_AR02) {
		isInside = false;
		int waypointStart = Random_Query(155, 158);
		int waypointEnd = 0;
		AI_Movement_Track_Append(kActorGenwalkerB, waypointStart, 0);
		do {
			waypointEnd = Random_Query(155, 158);
		} while (waypointEnd == waypointStart);
		if ((waypointStart == 155 || waypointStart == 156) && (waypointEnd == 157 || waypointEnd == 158)) {
			AI_Movement_Track_Append(kActorGenwalkerB, 159, 0);
			AI_Movement_Track_Append(kActorGenwalkerB, 160, 0);
			if (Random_Query(0, 3) == 0) {
				AI_Movement_Track_Append_With_Facing(kActorGenwalkerB, 161, Random_Query(15, 30), 904);
			}
		} else if ((waypointEnd == 155 || waypointEnd == 156) && (waypointStart == 157 || waypointStart == 158)) {
			if (Random_Query(0, 3) == 0) {
				AI_Movement_Track_Append_With_Facing(kActorGenwalkerB, 161, Random_Query(15, 30), 904);
			}
			AI_Movement_Track_Append(kActorGenwalkerB, 160, 0);
			AI_Movement_Track_Append(kActorGenwalkerB, 159, 0);
		} else if ((waypointStart == 155 && waypointEnd == 156) || (waypointStart == 156 && waypointEnd == 155)) {
			AI_Movement_Track_Append(kActorGenwalkerB, 159, 0);
		}
		AI_Movement_Track_Append(kActorGenwalkerB, waypointEnd, 0);
		AI_Movement_Track_Repeat(kActorGenwalkerB);
		return true;
	}

	if (set == kSetCT01_CT12) {
		isInside = false;
		if (Random_Query(0, 1)) {
			AI_Movement_Track_Append(kActorGenwalkerB, 54, 1);
			if (Random_Query(1, 3) == 1) {
				AI_Movement_Track_Append(kActorGenwalkerB, 56, 0);
				AI_Movement_Track_Append(kActorGenwalkerB, 43, 1);
			} else {
				AI_Movement_Track_Append(kActorGenwalkerB, 53, 1);
			}
			AI_Movement_Track_Append(kActorGenwalkerB, 40, 1);
			AI_Movement_Track_Repeat(kActorGenwalkerB);
		} else {
			AI_Movement_Track_Append(kActorGenwalkerB, 53, 1);
			if (Random_Query(1, 3) == 1) {
				AI_Movement_Track_Append(kActorGenwalkerB, 43, 1);
			} else {
				AI_Movement_Track_Append(kActorGenwalkerB, 54, 1);
			}
			AI_Movement_Track_Append(kActorGenwalkerB, 40, 1);
			AI_Movement_Track_Repeat(kActorGenwalkerB);
		}
		return true;
	}

	if (set == kSetHC01_HC02_HC03_HC04) {
		isInside = true;
		if (Random_Query(0, 1)) {
			AI_Movement_Track_Append(kActorGenwalkerB, 164, 0);
			// Original code does indeed have duplication of branches here
			// TODO This could possible indicate intent of different movement tracks for the actor
			// based on repeated "coin flips", but as it was the code block for each branch was identical.
#if 0
			if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorGenwalkerB, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerB, 162, 0);
			} else if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorGenwalkerB, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerB, 162, 0);
			} else {
#endif
				AI_Movement_Track_Append(kActorGenwalkerB, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerB, 162, 0);
			//}
		} else {
			AI_Movement_Track_Append(kActorGenwalkerB, 162, 0);
			// Original code does indeed have duplication of branches here (similar to above)
			// TODO This could possible indicate intent of different movement tracks for the actor
			// based on repeated "coin flips", but as it was the code block for each branch was identical.
			// NOTE The code for generic walker A here is slightly different, setting based on a "coin flip"
			// the actor's "facing".
#if 0
			if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorGenwalkerB, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerB, 164, 0);
			} else if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorGenwalkerB, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerB, 164, 0);
			} else {
#endif
				AI_Movement_Track_Append(kActorGenwalkerB, 163, 0);
				AI_Movement_Track_Append(kActorGenwalkerB, 164, 0);
			//}
		}
		AI_Movement_Track_Repeat(kActorGenwalkerB);
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
		AI_Movement_Track_Append(kActorGenwalkerB, waypointStart, 0);
		if (waypointStart == 170) {
			AI_Movement_Track_Append(kActorGenwalkerB, 169, 0);
			AI_Movement_Track_Append(kActorGenwalkerB, 168, 0);
		} else if (waypointEnd == 170) {
			AI_Movement_Track_Append(kActorGenwalkerB, 168, 0);
			AI_Movement_Track_Append(kActorGenwalkerB, 169, 0);
		}
		AI_Movement_Track_Append(kActorGenwalkerB, waypointEnd, 0);
		AI_Movement_Track_Repeat(kActorGenwalkerB);
		return true;
	}

	return false;
}

} // End of namespace BladeRunner
