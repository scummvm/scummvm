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
//#include "common/debug.h"

namespace BladeRunner {

AIScriptFreeSlotB::AIScriptFreeSlotB(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 0;
	_var2 = 1;
}

void AIScriptFreeSlotB::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_var2 = 1;

	World_Waypoint_Set(527, kSetKP02,  -468.46f, -616.58f, 2840.60f);
	// TODO: A bug? world waypoints 528, 529 are unused
	World_Waypoint_Set(528, kSetKP02, -1024.46f, -615.49f, 2928.60f);
	World_Waypoint_Set(529, kSetKP02, -1024.46f, -615.49f, 2788.60f);
}

bool AIScriptFreeSlotB::Update() {
	if (Global_Variable_Query(kVariableChapter) > 5) {
		return false;
	}

	if (Global_Variable_Query(kVariableChapter) == 4) {
		switch (Actor_Query_Goal_Number(kActorFreeSlotB)) {
		case kGoalFreeSlotBAct4Default:
			Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct4WalkAround);
			Actor_Set_Targetable(kActorFreeSlotB, true);
			break;

		case kGoalFreeSlotBAct4WalkAround:
#if BLADERUNNER_ORIGINAL_BUGS
			if (Actor_Query_Which_Set_In(kActorFreeSlotB) == Player_Query_Current_Set()
			    && Actor_Query_Inch_Distance_From_Actor(kActorFreeSlotB, kActorMcCoy) <= 48
			) {
				Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct4AttackMcCoy);
			}
#else
			if (Actor_Query_Which_Set_In(kActorFreeSlotB) == Player_Query_Current_Set()
			    && Actor_Query_Inch_Distance_From_Actor(kActorFreeSlotB, kActorMcCoy) <= 54
			) {
				Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct4AttackMcCoy);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			break;

		case kGoalFreeSlotBAct4AttackMcCoy:
			if (Actor_Query_Which_Set_In(kActorFreeSlotB) != Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct4WalkAround);
			}
			break;

		case kGoalFreeSlotBGone:
			if (Actor_Query_Which_Set_In(kActorFreeSlotB) != Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct4Default);
			}
			break;

		default:
			Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct4Default);
			break;
		}

		return false;
	}
	// if in Act 5
	if (Actor_Query_Goal_Number(kActorFreeSlotB) < kGoalFreeSlotBAct5Default) {
		AI_Movement_Track_Flush(kActorFreeSlotB);
		Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct5Default);
		return true;
	} else {
		if (Actor_Query_Goal_Number(kActorFreeSlotB) != kGoalFreeSlotBAct5Prepare
		    || Actor_Query_Which_Set_In(kActorMcCoy) != kSetKP02
		) {
			if (Actor_Query_Goal_Number(kActorFreeSlotB) == kGoalFreeSlotBGone) {
				if (Actor_Query_Which_Set_In(kActorFreeSlotB) != Player_Query_Current_Set()) {
					Non_Player_Actor_Combat_Mode_Off(kActorFreeSlotB);
					Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct5Default);
					return true;
				}
			}
			return false;
		}
		Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct5KP02Attack);
		Actor_Set_Targetable(kActorFreeSlotB, true);
		return true;
	}
}

void AIScriptFreeSlotB::TimerExpired(int timer) {
	//return false;
}

void AIScriptFreeSlotB::CompletedMovementTrack() {
//	debug("Rat B completed move with Goal: %d", Actor_Query_Goal_Number(kActorFreeSlotB));
	switch (Actor_Query_Goal_Number(kActorFreeSlotB)) {
	case kGoalFreeSlotBAct4Default:
		Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct4WalkAround);
		break;

	case kGoalFreeSlotBAct4WalkAround:
		Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct4Default);
		break;

	case kGoalFreeSlotBAct5Default:
		Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct5Prepare);
		break;

	case kGoalFreeSlotBAct5KP02Attack:
		Non_Player_Actor_Combat_Mode_On(kActorFreeSlotB, kActorCombatStateIdle, false, kActorMcCoy, 8, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 5, 300, false);
		break;

	default:
		return; //false;
	}

	return; //true;
}

void AIScriptFreeSlotB::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptFreeSlotB::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorFreeSlotB) != kGoalFreeSlotBGone)
		return; //false;

	Actor_Face_Actor(kActorMcCoy, kActorFreeSlotB, true);
	if (_vm->_cutContent && !Game_Flag_Query(kFlagMcCoyCommentsOnHoodooRats)) {
		Game_Flag_Set(kFlagMcCoyCommentsOnHoodooRats);
		Actor_Voice_Over(1060, kActorVoiceOver);  // Hoodoo rats
		// Note: Quote 1070 is *boop* in ENG version.
		// However, it is similar to 1060 quote in FRA, DEU, ESP and ITA versions
		//          with the only difference being not mentioning the "Hoodoo Rats" name.
		//          It uses a generic "rats" in its place.
		Actor_Voice_Over(1080, kActorVoiceOver);
		Actor_Voice_Over(1090, kActorVoiceOver);
	} else {
		if (Random_Query(1, 2) == 1) {
			Actor_Says(kActorMcCoy, 8655, 16);
		} else {
			Actor_Says(kActorMcCoy, 8665, 16);
		}
	}
}

void AIScriptFreeSlotB::EnteredSet(int setId) {
	// return false;
}

void AIScriptFreeSlotB::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptFreeSlotB::OtherAgentExitedThisSet(int otherActorId) {
#if !BLADERUNNER_ORIGINAL_BUGS
	if (otherActorId == kActorMcCoy && Actor_Query_Goal_Number(kActorFreeSlotB) == kGoalFreeSlotBGone) {
		if (Global_Variable_Query(kVariableChapter) == 4) {
			Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct4Default);
		} else if (Global_Variable_Query(kVariableChapter) == 5) {
			Non_Player_Actor_Combat_Mode_Off(kActorFreeSlotB);
			Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBAct5Default);
		}
	}

#endif // BLADERUNNER_ORIGINAL_BUGS
	// return false;
}

void AIScriptFreeSlotB::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptFreeSlotB::ShotAtAndMissed() {
	// return false;
}

bool AIScriptFreeSlotB::ShotAtAndHit() {
	return false;
}

void AIScriptFreeSlotB::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorFreeSlotB, kGoalFreeSlotBGone);
}

int AIScriptFreeSlotB::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptFreeSlotB::GoalChanged(int currentGoalNumber, int newGoalNumber) {
//	debug("Rat B goal changed from %d to: %d", currentGoalNumber, newGoalNumber);
	switch (newGoalNumber) {
	case kGoalFreeSlotBAct4Default:
		// kSetFreeSlotG
		AI_Movement_Track_Flush(kActorFreeSlotB);
		AI_Movement_Track_Append(kActorFreeSlotB, 39, 2);
		AI_Movement_Track_Repeat(kActorFreeSlotB);
		break;

	case kGoalFreeSlotBAct4WalkAround:
		AI_Movement_Track_Flush(kActorFreeSlotB);
		goToRandomUGxx();
		AI_Movement_Track_Repeat(kActorFreeSlotB);
		break;

	case kGoalFreeSlotBAct4AttackMcCoy:
		Actor_Set_Targetable(kActorFreeSlotB, true);
		Non_Player_Actor_Combat_Mode_On(kActorFreeSlotB, kActorCombatStateIdle, false, kActorMcCoy, 8, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 25, 0, 75, 5, 300, false);
		break;

	case kGoalFreeSlotBAct5Default:
		// kSetFreeSlotG
		AI_Movement_Track_Append(kActorFreeSlotB, 39, 0);
		AI_Movement_Track_Repeat(kActorFreeSlotB);
		break;

	case kGoalFreeSlotBAct5KP02Attack:
		// kSetKP02
		AI_Movement_Track_Flush(kActorFreeSlotB);
		AI_Movement_Track_Append(kActorFreeSlotB, 527, 0);
		AI_Movement_Track_Repeat(kActorFreeSlotB);
		break;

	case kGoalFreeSlotBGone:
		Actor_Set_Health(kActorFreeSlotB, 20, 20);
		break;

	default:
		return false;
	}

	return true;
}

bool AIScriptFreeSlotB::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationRatIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatIdle)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = kModelAnimationRatSlowWalk;
		if (_var1) {
			--_var1;
		} else {
			_animationFrame += _var2;
			if (_animationFrame < 8) {
				_var2 = 1;
			} else {
				if (_animationFrame > 8) {
					_var2 = -1;
				} else if (Random_Query(0, 4)) {
					_var2 = -_var2;
				}
			}
			if (_animationFrame >= 7 && _animationFrame <= 9) {
				_var1 = Random_Query(0, 1);
			}
		}
		break;

	case 2:
		*animation = kModelAnimationRatSlowWalk;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatSlowWalk) - 1) {
			*animation = kModelAnimationRatIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 3:
		*animation = kModelAnimationRatRunning;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatRunning)) {
			_animationFrame = 0;
		}
		break;

	case 4:
		*animation = kModelAnimationRatJumpAttack;
		++_animationFrame;
		if (_animationFrame == 3) {
			int snd;
			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(kActorFreeSlotB, snd, 75, 0, 99);
		}
		if (_animationFrame == 3) {
			Actor_Combat_AI_Hit_Attempt(kActorFreeSlotB);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = kModelAnimationRatIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 5:
		// This is an animation for Maggie (exploding) but is also used for generic death states (rats, generic walkers)
		// probably for debug purposes
		*animation = kModelAnimationMaggieExploding;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieExploding) - 1) {
			_animationState = 8;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieExploding) - 1;
		}
		break;

	case 6:
		if (_animationFrame == 1) {
			Ambient_Sounds_Play_Sound(kSfxRATTY3, 99, 0, 0, 20);
		}
		*animation = kModelAnimationRatHurt;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatHurt)) {
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorFreeSlotB, kAnimationModeIdle);
		}
		break;

	case 7:
		*animation = kModelAnimationRatDying;
		++_animationFrame;
		if (_animationFrame == 1) {
			Ambient_Sounds_Play_Sound(kSfxRATTY5, 99, 0, 0, 25);
		}
#if BLADERUNNER_ORIGINAL_BUGS
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatDying) - 1) {
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatDying) - 1;
		}
		_animationState = 8;
#else
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatDying) - 1) {
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatDying) - 1;
			_animationState = 8;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		break;

	case 8:
		*animation = kModelAnimationRatDying;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatDying) - 1;
		break;

	default:
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptFreeSlotB::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if ((unsigned int)(_animationState - 1) > 1) {
			_animationState = 0;
			_animationFrame = 0;
		} else if (_animationState == 1) {
			_animationState = 2;
		}
		break;

	case 1:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 4:
		if ((unsigned int)(_animationState - 1) > 1) {
			_animationState = 0;
			_animationFrame = 0;
		} else if (_animationState == 1) {
			_animationState = 2;
		}
		break;

	case 6:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case 7:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 8:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 21:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case 43:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 7;
		_animationFrame = 0;
		break;
	}

	return true;
}

void AIScriptFreeSlotB::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptFreeSlotB::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptFreeSlotB::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptFreeSlotB::FledCombat() {
	// return false;
}

void AIScriptFreeSlotB::goToRandomUGxx() {
	switch (Random_Query(1, 14)) {
	case 1:
		// kSetUG01
#if BLADERUNNER_ORIGINAL_BUGS
		AI_Movement_Track_Append(kActorFreeSlotB, 450, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 451, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 450, 0);
		break;
#else
		// Don't put rats in UG01 when Lucy is also here
		if (!Actor_Query_In_Set(kActorLucy, kSetUG01)) {
			AI_Movement_Track_Append(kActorFreeSlotB, 450, 1);
			AI_Movement_Track_Append(kActorFreeSlotB, 451, 5);
			AI_Movement_Track_Append(kActorFreeSlotB, 450, 0);
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 2:
		// kSetUG01
#if BLADERUNNER_ORIGINAL_BUGS
		World_Waypoint_Set(466, kSetUG01, 144.98f, -50.13f, -175.75f);
		World_Waypoint_Set(547, kSetUG01, 105.6f,  -50.13f, -578.46f);
		World_Waypoint_Set(548, kSetUG01,  62.0f,  -50.13f, -574.0f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 548, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 5);
		break;
#else
		// Don't put rats in UG01 when Lucy is also here
		if (!Actor_Query_In_Set(kActorLucy, kSetUG01)) {
			World_Waypoint_Set(466, kSetUG01, 144.98f, -50.13f, -175.75f);
			World_Waypoint_Set(547, kSetUG01, 105.6f,  -50.13f, -578.46f);
			World_Waypoint_Set(548, kSetUG01,  62.0f,  -50.13f, -574.0f);
			AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
			AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
			AI_Movement_Track_Append(kActorFreeSlotB, 548, 5);
			AI_Movement_Track_Append(kActorFreeSlotB, 466, 5);
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 3:
		// kSetUG04
		AI_Movement_Track_Append(kActorFreeSlotB, 446, 15);
		AI_Movement_Track_Append(kActorFreeSlotB, 447,  1);
		AI_Movement_Track_Append(kActorFreeSlotB, 449,  1);
		AI_Movement_Track_Append(kActorFreeSlotB, 448,  2);
		AI_Movement_Track_Append(kActorFreeSlotB, 449,  0);
		break;

	case 4:
		// kSetUG04
		World_Waypoint_Set(466, kSetUG04, -22.70f,  6.39f,    33.12f);
		World_Waypoint_Set(547, kSetUG04,  -6.70f, -1.74f,  -362.88f);
		World_Waypoint_Set(548, kSetUG04, 164.0f,  11.87f, -1013.0f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 2);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 0);
		AI_Movement_Track_Append(kActorFreeSlotB, 548, 0);
		break;

	case 5:
		// kSetUG05
		AI_Movement_Track_Append(kActorFreeSlotB, 457, 15);
		AI_Movement_Track_Append(kActorFreeSlotB, 458,  0);
		AI_Movement_Track_Append(kActorFreeSlotB, 459, 15);
		break;

	case 6:
		// kSetUG06
		AI_Movement_Track_Append(kActorFreeSlotB, 460, 15);
		AI_Movement_Track_Append(kActorFreeSlotB, 461,  5);
		AI_Movement_Track_Append(kActorFreeSlotB, 460, 15);
		break;

	case 7:
		// kSetUG07 or kSetFreeSlotG
#if BLADERUNNER_ORIGINAL_BUGS
		if (Actor_Query_In_Set(kActorClovis, kSetUG07)) {
			// this check is not very effective since Clovis
			// will spawn in the set when McCoy goes to the downwards exit
			// at which point a rat may already be in the set and that could
			// make them collide with Clovis' path
			AI_Movement_Track_Append(kActorFreeSlotB, 39, 10);
		} else {
			World_Waypoint_Set(466, kSetUG07,  -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(547, kSetUG07,  250.0f,  -12.21f, -342.0f);
			World_Waypoint_Set(548, kSetUG07, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotB, 466, 5);
			AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
			// TODO: A bug? the waypoint 548 is created but is unused
		}
		break;
#else
		// Don't put rats in UG07 after the UG18 Guzza scene
		// since Clovis may be there too and that does not work well
		if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
			World_Waypoint_Set(466, kSetUG07,  -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(547, kSetUG07,  250.0f,  -12.21f, -342.0f);
			World_Waypoint_Set(548, kSetUG07, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotB, 466, 5);
			AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
			// TODO: A bug? the waypoint 548 is created but is unused
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 8:
		// kSetUG07
#if BLADERUNNER_ORIGINAL_BUGS
		World_Waypoint_Set(466, kSetUG07,  -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(547, kSetUG07,  250.0f,  -12.21f, -342.0f);
		World_Waypoint_Set(548, kSetUG07, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		// TODO: A bug? the waypoint 548 is created but is unused
		break;
#else
		// Don't put rats in UG07 after the UG18 Guzza scene
		// since Clovis may be there too and that does not work well
		if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
			World_Waypoint_Set(466, kSetUG07,  -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(547, kSetUG07,  250.0f,  -12.21f, -342.0f);
			World_Waypoint_Set(548, kSetUG07, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotB, 547, 5);
			AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
			// TODO: A bug? the waypoint 548 is created but is unused
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 9:
		// kSetUG07
#if BLADERUNNER_ORIGINAL_BUGS
		World_Waypoint_Set(466, kSetUG07,  -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(547, kSetUG07,  250.0f,  -12.21f, -342.0f);
		World_Waypoint_Set(548, kSetUG07, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 548, 1);
		// TODO: A bug? the waypoint 466 is created but is unused
		break;
#else
		// Don't put rats in UG07 after the UG18 Guzza scene
		// since Clovis may be there too and that does not work well
		if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
			World_Waypoint_Set(466, kSetUG07,  -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(547, kSetUG07,  250.0f,  -12.21f, -342.0f);
			World_Waypoint_Set(548, kSetUG07, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotB, 547, 5);
			AI_Movement_Track_Append(kActorFreeSlotB, 548, 1);
			// TODO: A bug? the waypoint 466 is created but is unused
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 10:
		// kSetUG07
#if BLADERUNNER_ORIGINAL_BUGS
		World_Waypoint_Set(466, kSetUG07,  -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(547, kSetUG07,  250.0f,  -12.21f, -342.0f);
		World_Waypoint_Set(548, kSetUG07, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotB, 548, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
		// TODO: A bug? the waypoint 466 is created but is unused
		break;
#else
		// Don't put rats in UG07 after the UG18 Guzza scene
		// since Clovis may be there too and that does not work well
		if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
			World_Waypoint_Set(466, kSetUG07,  -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(547, kSetUG07,  250.0f,  -12.21f, -342.0f);
			World_Waypoint_Set(548, kSetUG07, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotB, 548, 5);
			AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
			// TODO: A bug? the waypoint 466 is created but is unused
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 11:
		// kSetUG09
		World_Waypoint_Set(466, kSetUG09,   91.0f, 156.94f, -498.0f);
		World_Waypoint_Set(547, kSetUG09, -149.0f, 156.94f, -498.0f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
		break;

	case 12:
		// kSetUG09
		World_Waypoint_Set(466, kSetUG09,   91.0f, 156.94f, -498.0f);
		World_Waypoint_Set(547, kSetUG09, -149.0f, 156.94f, -498.0f);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		break;

	case 13:
		// kSetUG09
#if BLADERUNNER_ORIGINAL_BUGS
		// this makes the rat appear on the pipe (top left)
		// but this is buggy since it will appear floating there
		World_Waypoint_Set(466, kSetUG09, -152.51f, 277.31f, 311.98f);
		World_Waypoint_Set(547, kSetUG09, -124.51f, 275.08f, 319.98f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 8);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
#else
		// replacing with something more normal
		World_Waypoint_Set(466, kSetUG09, -149.0f,  156.94f, -498.0f);
		World_Waypoint_Set(547, kSetUG09,  -32.60f, 156.94f, -498.0f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 2);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
#endif // BLADERUNNER_ORIGINAL_BUGS
		break;

	case 14:
		// kSetUG12
		World_Waypoint_Set(466, kSetUG12, -360.67f, 21.39f,   517.55f);
		World_Waypoint_Set(547, kSetUG12, -250.67f, 21.39f,   477.55f);
		World_Waypoint_Set(548, kSetUG12, -248.67f, 21.39f, -1454.45f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 8);
		AI_Movement_Track_Append(kActorFreeSlotB, 548, 1);
		break;

	default:
		// kSetFreeSlotG
		AI_Movement_Track_Append(kActorFreeSlotB, 39, Random_Query(1, 10));
		break;
	}
}

} // End of namespace BladeRunner
