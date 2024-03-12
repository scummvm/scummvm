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
#include "bladerunner/vector.h"
#include "bladerunner/actor.h"

namespace BladeRunner {

enum kMaggieStates {
	kMaggieStateIdle         = 0,
	kMaggieStateWalking      = 1,
	kMaggieStateJumping      = 2,
	kMaggieStateHappyA       = 3,
	kMaggieStateHappyB       = 4,
	kMaggieStateLyingDown    = 5,
	kMaggieStateLyingIdle    = 6,
	kMaggieStateStandingUp   = 7,
	kMaggieStateGoingToSleep = 8,
	kMaggieStateSleeping     = 9,
	kMaggieStateWakingUp     = 10,
	kMaggieStateBombIdle     = 11,
	kMaggieStateBombWalk     = 12,
	kMaggieStateBombJumping  = 13,
	kMaggieStateExploding    = 14,
	kMaggieStateDeadExploded = 15,
	kMaggieStateDead         = 16
};

AIScriptMaggie::AIScriptMaggie(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_varTimesToLoopWhenHappyB = 0;
	_varTimesToBarkWhenHappyA = 0;
	_varMaggieSoundPan = 0;
	_varMaggieClickResponse = 0;
	var_45F408 = 0;
}

void AIScriptMaggie::Initialize() {
	_animationState = kMaggieStateIdle;
	_animationFrame = 0;
	_animationStateNext = 0;
	_animationNext = 0;
	_varTimesToLoopWhenHappyB = 0;
	_varTimesToBarkWhenHappyA = 0;
	_varMaggieSoundPan = 0;
	_varMaggieClickResponse = 0;
	var_45F408 = 0; // only assigned to 0. Never checked. Unused.
	Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Default);
}

bool AIScriptMaggie::Update() {
	if (Actor_Query_Which_Set_In(kActorMaggie) == kSetMA02_MA04
	 && Global_Variable_Query(kVariableChapter) == 4
	) {
		Actor_Put_In_Set(kActorMaggie, kSetFreeSlotG);
		Actor_Set_At_Waypoint(kActorMaggie, 39, 0);
	}

	switch (Actor_Query_Goal_Number(kActorMaggie)) {
	case kGoalMaggieKP05WillExplode:
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05Explode);
		break;

	case kGoalMaggieKP05WalkToMcCoy:
		if (Actor_Query_Inch_Distance_From_Actor(kActorMcCoy, kActorMaggie) < 60) {
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05Explode);
		}
		break;
	}

	if (Global_Variable_Query(kVariableChapter) == 5) {
		if (Actor_Query_Goal_Number(kActorMaggie) < kGoalMaggieAct5Default) {
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieAct5Default);
		}
		return true;
	}
	return false;
}

void AIScriptMaggie::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask0) {
		switch (Actor_Query_Goal_Number(kActorMaggie)) {
		case kGoalMaggieMA02Wait:
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			if (Random_Query(0, 4)) {
				AI_Movement_Track_Flush(kActorMaggie);
				AI_Movement_Track_Append(kActorMaggie, randomWaypointMA02(), 0);
				AI_Movement_Track_Repeat(kActorMaggie);
			} else {
				if (_vm->_cutContent) {
					// In the Restored Content Mode:
					// With a Random Chance Maggie will either:
					// walk to another (random) waypoint and then sleep (kGoalMaggieMA02GoingToSleep)
					// or sleep here (again kGoalMaggieMA02GoingToSleep)
					// or lie down awake (and maybe sleep or get up again)
					if (Random_Query(0, 3) == 0) {
						Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02GoingToSleep);
					} else {
						// At the end of the lying down animation, we will check for kGoalMaggieMA02Wait goal
						// and act accordingly. See: AIScriptMaggie::UpdateAnimation()
						Actor_Change_Animation_Mode(kActorMaggie, 54); // Go to lying (awake) pose
					}
				} else {
					// ORIGINAL: By setting the animation mode explicitly here, and not setting the goal,
					// Maggie will lie awake and stay in that pose idling. She will not do anything else
					// until McCoy leaves the room.
					// NEW: At the end of the lying down animation, we will check for kGoalMaggieMA02Wait goal
					//      and act accordingly. See: AIScriptMaggie::UpdateAnimation()
					Actor_Change_Animation_Mode(kActorMaggie, 54); // Go to lying (awake) pose
				}
			}
			break; // return true

		case kGoalMaggieMA02SitDownToSleep:
			// Untriggered in the original.
			// Will put Maggie to sleep, if already lying down.
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			// Changing animation mode to 55 (sleeping) is effective only if state is at kMaggieStateLyingIdle.
			Actor_Change_Animation_Mode(kActorMaggie, 55); // Go to sleeping pose
			break; // return true

		case kGoalMaggieMA02GoingToSleep:
			// New case (end of lying idle - time to sleep)
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			// At the end of the lying down animation, we will check the goal
			// and act accordingly. See: AIScriptMaggie::UpdateAnimation()
			Actor_Change_Animation_Mode(kActorMaggie, 54); // Go to lying (awake) pose
			break;

		case kGoalMaggieMA02SitDownToGetUp:
			// New case (end of lying idle - time to get up)
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			// At the end of the stand up animation, we will check the goal
			// and act accordingly. See: AIScriptMaggie::UpdateAnimation()
			Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle); // Go to stand up pose
			break;

		case kGoalMaggieMA02Sleeping:
			// New case (end of sleeping session)
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			// At the end of the wake-up animation, we will check the goal
			// and act accordingly (set new goal, restart timer)
			Actor_Change_Animation_Mode(kActorMaggie, 54); // Go to lying (awake) pose
			break;
		}
	}
	return; //false
}

void AIScriptMaggie::CompletedMovementTrack() {
	// Note, CompletedMovementTrack() is triggered *after* the delay at the last waypoint of the track has expired
	switch (Actor_Query_Goal_Number(kActorMaggie)) {
	case kGoalMaggieMA02WalkToEntrance:
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02WalkToMcCoy);
		break; // return true

	case kGoalMaggieMA02Wait:
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
		AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(1, 5));
		break; // return true

	case kGoalMaggieMA02GoingToSleep:
		// (In the original engine) when Maggie is moving to a random waypoint
		// --due to having her goal changed to kGoalMaggieMA02GoingToSleep--
		// the delay when reaching that waypoint is too long (486 seconds).
		// So Maggie would stand at the target waypoint, waiting for the delay to pass
		// without turning to face McCoy or sitting down.
		// However, this goal is UNTRIGGERED in the original engine, so the bug was not visible.
		// We changed the behavior to have no delay when reaching this waypoint,
		// and start a timer with a random delay from 5 seconds to 486 seconds for Maggie to by lying awake.
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorMaggie, 54); // Go to lying (awake) pose
#if !BLADERUNNER_ORIGINAL_BUGS
		AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(5, 486));
#endif // !BLADERUNNER_ORIGINAL_BUGS
		break; // return true

	case kGoalMaggieKP05WalkToMcCoy:
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05WillExplode);
		break; // return true

	case kGoalMaggieMA02Default:
		//fall through
	default:
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
		break;  // return true
	}
}

void AIScriptMaggie::ReceivedClue(int clueId, int fromActorId) {
}

void AIScriptMaggie::ClickedByPlayer() {
	if (!Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
	 &&  Global_Variable_Query(kVariableChapter) == 5
	) {
		if (Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieKP05WalkToMcCoy) {
			Actor_Set_Targetable(kActorMaggie, true);
			AI_Movement_Track_Flush(kActorMaggie);
			Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
			Actor_Says(kActorMcCoy, 2400, kAnimationModeFeeding);
		}
		return; // true
	}

	if (_animationState == kMaggieStateDead) {
		return; // false
	}

#if !BLADERUNNER_ORIGINAL_BUGS
	if (Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02WalkToMcCoy
	 || Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02GetFed) {
		// Don't do anything if Maggie's goal is already kGoalMaggieMA02WalkToMcCoy or kGoalMaggieMA02GetFed
		// This is for the small time-gap when Maggie has to wake up and get up,
		// during which the player/McCoy still has control
		return; // false;
	}
#endif // !BLADERUNNER_ORIGINAL_BUGS

	Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);

	float mccoy_x, mccoy_y, mccoy_z;
	Actor_Query_XYZ(kActorMcCoy, &mccoy_x, &mccoy_y, &mccoy_z);
	if (distanceToActor(kActorMaggie, mccoy_x, mccoy_y, mccoy_z) > 60.0f) {
		if (_vm->_cutContent && Random_Query(0, 1)) {
			Actor_Says(kActorMcCoy, 2395, 18);
		} else {
			Actor_Says(kActorMcCoy, 2430, 18);
		}
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02WalkToMcCoy);
		return; // true
	}

	// 1: McCoy told Maggie to sit down, 2: McCoy asks Maggie if hungry, 3: McCoy sits down in front of Maggie
	_varMaggieClickResponse = 0;
	int randMcCoyCueToMaggie = Random_Query(0, 4);
	if (_vm->_cutContent) {
		// enhance somewhat the probability of getting a cut cue and interaction
		if (Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02SitDownToSleep
			 || Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02SitDownToGetUp) {
			int randFavorOfCutCue = Random_Query(1, 10);
			if (randFavorOfCutCue < 4) {
				randMcCoyCueToMaggie = 2;
			} else if (randFavorOfCutCue > 6) {
				randMcCoyCueToMaggie = 4;
			}
			// otherwise keep original randMcCoyCueToMaggie (0 to 4)
		} else if (_animationState == kMaggieStateIdle) {
			int randFavorOfCutCue = Random_Query(1, 5);
			if (randFavorOfCutCue < 2) {
				randMcCoyCueToMaggie = 3;
			}
			// otherwise keep original randMcCoyCueToMaggie (0 to 4)
		}
	}

	switch (randMcCoyCueToMaggie) {
	case 0:
		// Good doggy.
		Actor_Says(kActorMcCoy, 2435, 13);
		break;

	case 1:
		// Who's the best dog...?
		Actor_Says(kActorMcCoy, 2440, 18);
		break;

	case 2:
		if (_vm->_cutContent) {
			if (Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02Sleeping
			 || Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02SitDownToSleep
			 || Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02SitDownToGetUp) {
				// Hey, Maggie (if sitting or sleeping)
				Actor_Says(kActorMcCoy, 2395, 18);
			}
		}
		break;

	case 3:
		if (_vm->_cutContent) {
			// Down Maggie, get down! That's my girl. (if standing still)
			if (_animationState == kMaggieStateIdle) {
				Actor_Start_Speech_Sample(kActorMcCoy, 2415);
				_varMaggieClickResponse = 1;
			}
		}
		break;

	case 4:
		if (_vm->_cutContent) {
			// Are you hungry Mags? (if lying awake)
			if (Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02SitDownToSleep
			 || Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02SitDownToGetUp) {
				// Set a var to trigger appropriate response
				Actor_Says(kActorMcCoy, 2425, 18);
				_varMaggieClickResponse = 2;
			}
		}
		break;

	default:
		break;
	}

	switch (Actor_Query_Goal_Number(kActorMaggie)) {
	case kGoalMaggieMA02Wait:
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
		if (_animationState == kMaggieStateIdle && _varMaggieClickResponse == 1) {
			// (Cut Content; _varMaggieClickResponse only gets values > 0 in Cut Content)
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			Delay(500);
			Actor_Change_Animation_Mode(kActorMaggie, 54); // Go to lying (awake) pose
		} else {
			if (Random_Query(0, 1)) {
				Actor_Change_Animation_Mode(kActorMaggie, 57); // HappyB - Not Barking - Tail Wagging
				if (_vm->_cutContent && Random_Query(1, 5) < 4) {
					Player_Loses_Control();
					Actor_Change_Animation_Mode(kActorMcCoy, 85); // McCoy sits down in front of Maggie
					_varMaggieClickResponse = 3;
				}
			} else {
				Actor_Change_Animation_Mode(kActorMaggie, 56); // HappyA - Barking
			}
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			if (_varMaggieClickResponse == 3) {
				AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(6, 9));
			} else {
				AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(3, 9));
			}
		}
		break; // return true

	case kGoalMaggieMA02SitDownToSleep:
		// fall through
	case kGoalMaggieMA02SitDownToGetUp:
		// Maggie just stands up
		Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle);
		if (_varMaggieClickResponse == 2) {
			// Maggie makes "Need" sounds 
			// (Cut Content; _varMaggieClickResponse only gets values > 0 in Cut Content)
			_varMaggieSoundPan = _vm->_actors[kActorMaggie]->soundPan(75);
			Sound_Play(Random_Query(kSfxDOGNEED1, kSfxDOGNEED2), 50, _varMaggieSoundPan, _varMaggieSoundPan, 50);
			_varMaggieClickResponse = 0;
		}
		break; // return true

	case kGoalMaggieMA02Sleeping:
		// Go to lying awake pose from sleeping (onclick)
		Actor_Change_Animation_Mode(kActorMaggie, 54);
		break; // return true

	default:
		// This effects basically in restarting the custom timer for Maggie
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
		break; // return true
	}
}

void AIScriptMaggie::EnteredSet(int setId) {
}

void AIScriptMaggie::OtherAgentEnteredThisSet(int otherActorId) {
	// this is executed *after* the scene's script (eg. SceneScriptMA02::PlayerWalkedIn())
	if (_vm->_cutContent
	 && otherActorId == kActorMcCoy
	 && Actor_Query_Which_Set_In(kActorMaggie) == kSetMA02_MA04
	 && Global_Variable_Query(kVariableChapter) < 4
	 && Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02Default) {
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
	}
}

void AIScriptMaggie::OtherAgentExitedThisSet(int otherActorId) {
	if (otherActorId == kActorMcCoy
	 && Actor_Query_Which_Set_In(kActorMaggie) == kSetMA02_MA04
	 && Global_Variable_Query(kVariableChapter) < 4
	) {
		AI_Movement_Track_Flush(kActorMaggie);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Default);
	}
}

void AIScriptMaggie::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
}

void AIScriptMaggie::ShotAtAndMissed() {
}

bool AIScriptMaggie::ShotAtAndHit() {
	AI_Movement_Track_Flush(kActorMaggie);
	Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05WillExplode);
	Actor_Set_Targetable(kActorMaggie, false);
	return false;
}

void AIScriptMaggie::Retired(int byActorId) {
}

int AIScriptMaggie::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMaggie::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (currentGoalNumber == kGoalMaggieDead) {
		return true;
	}

	switch (newGoalNumber) {
	case kGoalMaggieMA02Default:
		Actor_Put_In_Set(kActorMaggie, kSetMA02_MA04);
		Actor_Set_At_Waypoint(kActorMaggie, 265, 780);
		return true;

	case kGoalMaggieMA02WalkToEntrance:
		Actor_Put_In_Set(kActorMaggie, kSetMA02_MA04);
		Actor_Set_At_Waypoint(kActorMaggie, randomWaypointMA02(), 512);
		AI_Movement_Track_Flush(kActorMaggie);
		AI_Movement_Track_Append(kActorMaggie, 264, 0);
		AI_Movement_Track_Repeat(kActorMaggie);
		return true;

	case kGoalMaggieMA02GetFed:
#if BLADERUNNER_ORIGINAL_BUGS
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorMaggie);
		Loop_Actor_Walk_To_Actor(kActorMaggie, kActorMcCoy, 48, false, false);
		Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, false);
		Actor_Says(kActorMcCoy, 2400, kAnimationModeFeeding);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
		Player_Gains_Control();
#else
		if (_animationState != kMaggieStateSleeping
		 && _animationState != kMaggieStateWakingUp
		 && _animationState != kMaggieStateStandingUp
		 && _animationState	!= kMaggieStateLyingDown
		 && _animationState != kMaggieStateLyingIdle) {
			// For specific animationStates we have to ignore this goal change,
			// and go through animation chain until Maggie is standing up
			// at which point (end of animation of standing up), the goal will change again to kGoalMaggieMA02GetFed
			// to trigger this case
			Player_Loses_Control();
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			AI_Movement_Track_Flush(kActorMaggie);
			// Allows McCoy to perform both animated turns (first towards the BAR-MAIN and then towards Maggie)
			// when Maggie is already too close
			// original bug: When Maggie is close McCoy would alternate between
			// - turning to Maggie and throw food at her
			// - only performing the turn toward the BAR-MAIN and "throw" food to wrong direction
			if (Actor_Query_Inch_Distance_From_Actor(kActorMaggie, kActorMcCoy) <= 85) {
				Delay(500);
			}
			Loop_Actor_Walk_To_Actor(kActorMaggie, kActorMcCoy, 48, false, false);
			Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			Actor_Says(kActorMcCoy, 2400, kAnimationModeFeeding);
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
			Player_Gains_Control();
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		return true;

	case kGoalMaggieMA02WalkToMcCoy:
		AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
#if BLADERUNNER_ORIGINAL_BUGS
		AI_Movement_Track_Flush(kActorMaggie);
		Loop_Actor_Walk_To_Actor(kActorMaggie, kActorMcCoy, 30, false, false);
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorMaggie, 56);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
#else
		if (_animationState != kMaggieStateSleeping
		 && _animationState != kMaggieStateWakingUp
		 && _animationState != kMaggieStateStandingUp
		 && _animationState	!= kMaggieStateLyingDown
		 && _animationState != kMaggieStateLyingIdle) {
			// For specific animationStates we have to ignore this goal change,
			// and go through animation chain until Maggie is standing up
			// at which point (end of animation of standing up), the goal will change again to kGoalMaggieMA02WalkToMcCoy
			// to trigger this case
			AI_Movement_Track_Flush(kActorMaggie);
			// When an actor other than McCoy does a loopWalk, their walk is non interruptible
			// (we set it to false explicitly here, but this is also taken care of inside the Actor::loopWalk())
			// This means McCoy loses control until the other Actor reaches the target waypoint
			// Also Loop_Actor_Walk_To_Actor() is blocking. The commands following it will be executed
			// after Maggie completes her loopWalk ie. reaches her target (McCoy).
			Loop_Actor_Walk_To_Actor(kActorMaggie, kActorMcCoy, 30, false, false);
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
			Actor_Change_Animation_Mode(kActorMaggie, 56);
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
		}
#endif
		return true;

	case kGoalMaggieMA02Wait:
		AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(3, 9));
		return true;

	case kGoalMaggieMA02GoingToSleep:
		// Maggie will (randomly) either:
		// - sit and go to sleep at her current position,
		// - or go to a random waypoint and then sit and go to sleep after a while.
		//
		// Note: Original checks for <= 0, but that is basically same as checking for == 0 here.
		if (Random_Query(0, 2) == 0) {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			Actor_Change_Animation_Mode(kActorMaggie, 54); // Go to lying (awake) pose
			// At the end of the animation, if the mode change is not ignored,
			// the goal will be set to kGoalMaggieMA02SitDownToSleep
			// (specifically for the case of kGoalMaggieMA02GoingToSleep being the current goal)
			// and the state to kMaggieStateLyingIdle
		} else {
			AI_Movement_Track_Flush(kActorMaggie);
			if (Actor_Query_Which_Set_In(kActorMaggie) == kSetMA02_MA04) {
#if BLADERUNNER_ORIGINAL_BUGS
				// Maggie stays for 486 seconds at the target waypoint (8.1 minutes)
				// This is a bug since CompletedMovementTrack() won't trigger until after this huge delay.
				AI_Movement_Track_Append(kActorMaggie, randomWaypointMA02(), 486);
#else
				AI_Movement_Track_Append(kActorMaggie, randomWaypointMA02(), 0);
#endif // BLADERUNNER_ORIGINAL_BUGS
			}
			AI_Movement_Track_Repeat(kActorMaggie);
		}
		return true;

	case kGoalMaggieMA02SitDownToSleep:
		// fall through
	case kGoalMaggieMA02SitDownToGetUp:
		Actor_Change_Animation_Mode(kActorMaggie, 54); // Go to lying (awake) pose
		// By setting _animationState and frame explicitly here,
		// the lying-down-idling pose is enforced, and the transition animation (from standing to sitting) will not play
		_animationState = kMaggieStateLyingIdle;
		_animationFrame = 0;
		AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(2, 9));
		return true;

	case kGoalMaggieMA02Sleeping:
		// When setting Maggie's *goal* to "sleeping", we expect it to be enforced
		// However, Actor_Change_Animation_Mode is not enforceable and could be ignored.
		// The goal change here is *NOT* done in order to play the animation.
		// It is to set the animation State, and by explicitly setting it, it overrides playing the animation transition.
		// Actor_Change_Animation_Mode() is called to store the _animationMode on Maggie's actor object (see: Actor::changeAnimationMode())
		Actor_Change_Animation_Mode(kActorMaggie, 55); // Go to sleeping pose
		_animationState = kMaggieStateSleeping;
#if BLADERUNNER_ORIGINAL_BUGS
		_animationFrame = 0;
#else
		// We actually need the final frame here to avoid Maggie glitching here
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieToggleSleepingWakeUp) - 1;
		AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
		// Sleep for 20 - 390 seconds, unless McCoy wakes Maggie up.
		AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(20, 390));
#endif // BLADERUNNER_ORIGINAL_BUGS
		return true;

	case kGoalMaggieAct5Default:
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieAct5Start);
		break;

	case kGoalMaggieKP05Wait:
		AI_Movement_Track_Flush(kActorMaggie);
		Game_Flag_Set(kFlagMaggieHasBomb);
		Actor_Put_In_Set(kActorMaggie, kSetKP05_KP06);
		Actor_Set_At_XYZ(kActorMaggie, -672.0, 0.0, -428.0, 653);
		Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle);
		break;

	case kGoalMaggieKP05McCoyEntered:
		Scene_Exits_Disable();
		Loop_Actor_Walk_To_XYZ(kActorMaggie, -734.0, 0.0, -432.0, 0, false, false, false);
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorMaggie, 56);
		Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
		Actor_Says(kActorMcCoy, 2225, kAnimationModeTalk);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05WalkToMcCoy);
		break;

	case kGoalMaggieKP05WalkToMcCoy:
		if (Actor_Query_Is_In_Current_Set(kActorSteele)) {
			Actor_Says(kActorSteele, 3270, 59);
		}
		AI_Movement_Track_Flush(kActorMaggie);
		AI_Movement_Track_Append(kActorMaggie, 540, 0);
		AI_Movement_Track_Repeat(kActorMaggie);
		break;

	case kGoalMaggieKP05Explode:
		AI_Movement_Track_Flush(kActorMaggie);
		Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
#if BLADERUNNER_ORIGINAL_BUGS
		Sound_Play(kSfxDOGEXPL1, 50, 0, 0, 100);
#else
		_varMaggieSoundPan = _vm->_actors[kActorMaggie]->soundPan(75);
		Sound_Play(kSfxDOGEXPL1, 50, _varMaggieSoundPan, _varMaggieSoundPan, 100);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieDead);
		Actor_Change_Animation_Mode(kActorMaggie, 51);
		if (Actor_Query_Inch_Distance_From_Actor(kActorMcCoy, kActorMaggie) < 144) {
			Player_Loses_Control();
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
			Actor_Retired_Here(kActorMcCoy, 6, 6, 1, -1);
		} else {
			Delay(3000);
			Scene_Exits_Disable();
			Actor_Says(kActorMcCoy, 2235, 12);
			if (Actor_Query_Is_In_Current_Set(kActorSteele)) {
				Actor_Says(kActorSteele, 1530, 58);
				Actor_Set_Goal_Number(kActorSteele, kGoalSteeleKP05Leave);
			}
			Delay(2000);
			Actor_Says(kActorMcCoy, 2390, 13);
			if (Actor_Query_Goal_Number(kActorSadik) == 411) {
				Actor_Set_Goal_Number(kActorSadik, 412);
			} else { // there is no way how Maggie can explode and Sadik's goal is not 411
				Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP06TalkToMcCoy);
			}
		}
		break;

	}
	return false;
}

bool AIScriptMaggie::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case kMaggieStateDead:
		*animation = kModelAnimationMaggieLyingDead;
		_animationFrame = 0;
		break;

	case kMaggieStateDeadExploded:
		*animation = kModelAnimationMaggieExploding;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieExploding) - 1;
		break;

	case kMaggieStateExploding:
		*animation = kModelAnimationMaggieExploding;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieExploding) - 1) {
			_animationState = kMaggieStateDeadExploded;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			Actor_Put_In_Set(kActorMaggie, kSetFreeSlotI);
			Actor_Set_At_Waypoint(kActorMaggie, 41, 0);
		}
		break;

	case kMaggieStateBombJumping:
		*animation = kModelAnimationMaggieStandingOnTwoFeetTrapped;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieStandingOnTwoFeetTrapped)) {
			_animationState = kMaggieStateBombIdle;
			_animationFrame = 0;
			*animation = kModelAnimationMaggieStandingIdleTrapped;
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05WillExplode);
		}
		break;

	case kMaggieStateBombWalk:
		*animation = kModelAnimationMaggieWalkingTrapped;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieWalkingTrapped)) {
			_animationFrame = 0;
		}
		break;

	case kMaggieStateBombIdle:
		*animation = kModelAnimationMaggieStandingIdleTrapped;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieStandingIdleTrapped)) {
			_animationFrame = 0;
		}
		break;

	case kMaggieStateWakingUp:
		*animation = kModelAnimationMaggieToggleSleepingWakeUp;
		--_animationFrame;
		if (_animationFrame > 0) {
			break;
		}
		// At the end of the wake-up animation
		_animationState = kMaggieStateLyingIdle;
		_animationFrame = 0;
		*animation = kModelAnimationMaggieLyingIdleTailWagging;
		switch (Actor_Query_Goal_Number(kActorMaggie)) {
		case kGoalMaggieMA02GetFed:
			_animationState = kMaggieStateStandingUp;
			_animationFrame = 0;
			*animation = kModelAnimationMaggieLyingStandingUp;
			break;

		case kGoalMaggieMA02WalkToMcCoy:
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02SitDownToSleep);
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02WalkToMcCoy);
#else
			// Stand up, similar to GetFed case above
			_animationState = kMaggieStateLyingIdle;
			_animationFrame = 0;
			*animation = kModelAnimationMaggieLyingStandingUp;
#endif
			break;

		case kGoalMaggieMA02Sleeping:
			// New behavior:
			// Decide randomly whether Maggie will:
			// - Stand up or
			// - Go back to sleep (which is the default case too)
			// Both animations will happen after a small delay period,
			// which is set via timer, restarted in the GoalChanged() code
			if (Random_Query(0, 1)) {
				// sleep after a while
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02SitDownToSleep);
			} else {
				// get up after a while
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02SitDownToGetUp);
			}
			break;

		default:
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02SitDownToSleep);
			break;
		}
		break;

	case kMaggieStateSleeping:
		*animation = kModelAnimationMaggieToggleSleepingWakeUp;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieToggleSleepingWakeUp) - 1;
#if !BLADERUNNER_ORIGINAL_BUGS
		switch (Actor_Query_Goal_Number(kActorMaggie)) {
		case kGoalMaggieMA02GetFed:
			// fall through
		case kGoalMaggieMA02WalkToMcCoy:
			// _animationFrame and model do not change here, just the state
			_animationState = kMaggieStateWakingUp;
			break;

		default:
			break;
		}
#endif // !BLADERUNNER_ORIGINAL_BUGS
		break;

	case kMaggieStateGoingToSleep:
		*animation = kModelAnimationMaggieToggleSleepingWakeUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieToggleSleepingWakeUp) - 1) {
			_animationState = kMaggieStateSleeping;
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Sleeping);
		}
		break;

	case kMaggieStateStandingUp:
		*animation = kModelAnimationMaggieLyingStandingUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieLyingStandingUp)) {
			*animation = kModelAnimationMaggieStandingIdle;
			_animationState = kMaggieStateIdle;
			_animationFrame = 0;
			switch (Actor_Query_Goal_Number(kActorMaggie)) {
			case kGoalMaggieMA02SitDownToGetUp: // new
				// fall through
			case kGoalMaggieMA02SitDownToSleep:
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
				break;

			case kGoalMaggieMA02WalkToMcCoy:
				// kGoalMaggieMA02Intermediate12 is a dummy goal, used only
				// to trigger a GoalChanged case, when we set back to kGoalMaggieMA02WalkToMcCoy below
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Intermediate12);
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02WalkToMcCoy);
				break;

#if !BLADERUNNER_ORIGINAL_BUGS
			case kGoalMaggieMA02GetFed:
				// kGoalMaggieMA02Intermediate12 is a dummy goal, used only
				// to trigger a GoalChanged case, when we set back to kGoalMaggieMA02GetFed below
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Intermediate12);
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02GetFed);
				break;
#endif // !BLADERUNNER_ORIGINAL_BUGS

			default:
				break;
			}
		}
		break;

	case kMaggieStateLyingIdle:
		*animation = kModelAnimationMaggieLyingIdleTailWagging;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieLyingIdleTailWagging)) {
			_animationFrame = 0;
#if !BLADERUNNER_ORIGINAL_BUGS
			switch (Actor_Query_Goal_Number(kActorMaggie)) {
			case kGoalMaggieMA02GetFed:
				// fall through
			case kGoalMaggieMA02WalkToMcCoy:
				_animationState = kMaggieStateStandingUp;
				*animation = kModelAnimationMaggieLyingStandingUp;
				break;

			default:
				break;
			}
#endif // !BLADERUNNER_ORIGINAL_BUGS
		}
		break;

	case kMaggieStateLyingDown:
		*animation = kModelAnimationMaggieLyingDown;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieLyingDown)) {
			_animationState = kMaggieStateLyingIdle;
			_animationFrame = 0;
			*animation = kModelAnimationMaggieLyingIdleTailWagging;
			switch (Actor_Query_Goal_Number(kActorMaggie)) {
			case kGoalMaggieMA02GoingToSleep:
				// sleep after a while
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02SitDownToSleep);
				break;

#if !BLADERUNNER_ORIGINAL_BUGS
			case kGoalMaggieMA02Wait:
				// This case was missing from the original engine
				// but is required in order to set Maggie's goal properly,
				// so that when she get's up she will go through the proper standing up animation.
				// This could be guarded with a _cutContent clause check, 
				// since this fix restores untriggered cases,
				// but since it's fixing a missing animation glitch,
				// it will be part of vanilla mode, as well.
				if (Random_Query(0, 1) && _varMaggieClickResponse != 1) {
					// sleep after a while
					Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02SitDownToSleep);
				} else {
					// get up after a while
					Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02SitDownToGetUp);
					if (_varMaggieClickResponse == 1) {
						_varMaggieClickResponse = 0;
					}
				}
				break;

			case kGoalMaggieMA02GetFed:
				// fall through
			case kGoalMaggieMA02WalkToMcCoy:
				_animationState = kMaggieStateStandingUp;
				*animation = kModelAnimationMaggieLyingStandingUp;
				break;
#endif // !BLADERUNNER_ORIGINAL_BUGS

			default:
				break;
			}
		}
		break;

	case kMaggieStateHappyB:
		// Not actually barking in this case, but tail wagging (with breathing sounds)
		*animation = kModelAnimationMaggieBarking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieBarking)) {
			_animationFrame = 0;
			--_varTimesToLoopWhenHappyB;
			if (_varTimesToLoopWhenHappyB <= 0) {
				Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle);
				*animation = kModelAnimationMaggieStandingIdle;
				if (_varMaggieClickResponse == 3) {
					// (Cut Content; _varMaggieClickResponse only gets values > 0 in Cut Content)
					Actor_Start_Speech_Sample(kActorMcCoy, 2705);
					Actor_Change_Animation_Mode(kActorMcCoy, 29); // Get up
					Player_Gains_Control();
					_varMaggieClickResponse = 0;
				}
			}
		}
		break;

	case kMaggieStateHappyA:
		// Barking in this case
		*animation = kModelAnimationMaggieBarkingOrHeadUp;
		if (_animationFrame == 1) {
			// one of kSfxDOGBARK1, kSfxDOGBARK3
#if BLADERUNNER_ORIGINAL_BUGS
			Sound_Play(Random_Query(kSfxDOGBARK1, kSfxDOGBARK3), 50, 0, 0, 50);
#else
			_varMaggieSoundPan = _vm->_actors[kActorMaggie]->soundPan(75);
			Sound_Play(Random_Query(kSfxDOGBARK1, kSfxDOGBARK3), 50, _varMaggieSoundPan, _varMaggieSoundPan, 50);
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			--_varTimesToBarkWhenHappyA;
			if (_varTimesToBarkWhenHappyA <= 0) {
				Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle);
				*animation = kModelAnimationMaggieStandingIdle;
				_animationState = kMaggieStateIdle;
			}
			_animationFrame = 0;
		}
		break;

	case kMaggieStateJumping:
		*animation = kModelAnimationMaggieStandingOnTwoFeet;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieStandingOnTwoFeet)) {
			Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle);
			*animation = kModelAnimationMaggieStandingIdle;
			_animationState = kMaggieStateIdle;
			_animationFrame = 0;
		}
		break;

	case kMaggieStateWalking:
		*animation = kModelAnimationMaggieWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieWalking)) {
			_animationFrame = 0;
		}
		break;

	case kMaggieStateIdle:
		*animation = kModelAnimationMaggieStandingIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieStandingIdle)) {
			_animationFrame = 0;
		}
		break;

	default:
		debugC(6, kDebugAnimation, "AIScriptMaggie::UpdateAnimation() - Current _animationState (%d) is not supported", _animationState);
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptMaggie::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (Game_Flag_Query(kFlagMaggieHasBomb)) {
			_animationState = kMaggieStateBombIdle;
			_animationFrame = 0;
		} else {
			switch (_animationState) {
			case kMaggieStateGoingToSleep:
				// fall through
			case kMaggieStateSleeping:
				_animationState = kMaggieStateWakingUp;
				break;

			case kMaggieStateLyingIdle:
				_animationState = kMaggieStateStandingUp;
				_animationFrame = 0;
				break;

			case kMaggieStateLyingDown:
				_animationState = kMaggieStateStandingUp;
				_animationFrame = 0;
				break;

			case kMaggieStateJumping:
				// fall through
			case kMaggieStateStandingUp:
				// fall through
			case kMaggieStateWakingUp:
				break;

			default:
				_animationState = kMaggieStateIdle;
				_animationFrame = 0;
				break;
			}
		}
		break;

	case kAnimationModeWalk:
		if (Game_Flag_Query(kFlagMaggieHasBomb)) {
			_animationState = kMaggieStateBombWalk;
			_animationFrame = 0;
		} else {
			_animationState = kMaggieStateWalking;
			_animationFrame = 0;
		}
		break;

	case 51:
		_animationState = kMaggieStateExploding;
		_animationFrame = 0;
#if BLADERUNNER_ORIGINAL_BUGS
		Sound_Play(kSfxDOGHURT1, 50, 0, 0, 50);
#else
		_varMaggieSoundPan = _vm->_actors[kActorMaggie]->soundPan(75);
		Sound_Play(kSfxDOGHURT1, 50, _varMaggieSoundPan, _varMaggieSoundPan, 50);
#endif // BLADERUNNER_ORIGINAL_BUGS
		break;

	case kAnimationModeFeeding:
		if (Game_Flag_Query(kFlagMaggieHasBomb)) {
			_animationState = kMaggieStateBombJumping;
			_animationFrame = 0;
		} else {
			_animationState = kMaggieStateJumping;
			_animationFrame = 0;
		}
		break;

	case 54:
		if (_animationState <= kMaggieStateSleeping) {
			if (_animationState > kMaggieStateIdle) {
				if (_animationState == kMaggieStateSleeping) {
					_animationState = kMaggieStateWakingUp;
#if BLADERUNNER_ORIGINAL_BUGS
					// Don't start from frame 0 here, since the animation has to play backwards,
					// and being on kMaggieStateSleeping state means we are already at the proper (end) frame
					_animationFrame = 0;
#endif // BLADERUNNER_ORIGINAL_BUGS
				}
			} else {
				_animationState = kMaggieStateLyingDown;
				_animationFrame = 0;
			}
		}
		// otherwise, the request for animation 54 (ie. assume lying awake pose) is ignored
		break;

	case 55:
		if (_animationState == kMaggieStateLyingIdle) {
			_animationState = kMaggieStateGoingToSleep;
			_animationFrame = 0;
		}
		// otherwise, the request for animation 55 (ie. assume sleeping pose) is ignored
		break;

	case 56:
		if (_animationState != kMaggieStateHappyA) {
			_animationFrame = 0;
			_animationState = kMaggieStateHappyA;
		}
		_varTimesToBarkWhenHappyA = Random_Query(2, 6);
		break;

	case 57:
		if (_animationState != kMaggieStateHappyB) {
			_animationFrame = 0;
			_animationState = kMaggieStateHappyB;
		}

		if (_varMaggieClickResponse == 3) {
			// (Cut Content; _varMaggieClickResponse only gets values > 0 in Cut Content)
			// Allow for time for McCoy to sit down
			_varTimesToLoopWhenHappyB = Random_Query(4, 6);
		} else {
			_varTimesToLoopWhenHappyB = Random_Query(2, 6);
		}
		_varMaggieSoundPan = _vm->_actors[kActorMaggie]->soundPan(75);
		if (_vm->_cutContent) {
			Sound_Play(Random_Query(kSfxDOGTAIL1, kSfxDOGTAIL2), 50, _varMaggieSoundPan, _varMaggieSoundPan, 50);
		} else {
#if BLADERUNNER_ORIGINAL_BUGS
			Sound_Play(kSfxDOGTAIL1, 50, 0, 0, 50);
#else
			Sound_Play(kSfxDOGTAIL1, 50, _varMaggieSoundPan, _varMaggieSoundPan, 50);
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		break;

	case 88:
		_animationState = kMaggieStateDead;
		_animationFrame = 0;
		break;

	default:
		debugC(6, kDebugAnimation, "AIScriptMaggie::ChangeAnimationMode(%d) - Target mode is not supported", mode);
		break;
	}
	return true;
}

void AIScriptMaggie::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptMaggie::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptMaggie::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptMaggie::FledCombat() {
}

int AIScriptMaggie::randomWaypointMA02() {
	switch (Random_Query(0, 3)) {
	case 0:
		// near counter and apartment door (eg. when greeting McCoy)
		return 264;

	case 1:
		// bottom right of screen
		return 265;

	case 2:
		// near bedroom door (left)
		return 266;

	default:
		// in front of windows (near counter)
		return 267;
	}
}

float AIScriptMaggie::distanceToActor(int actorId, float x, float y, float z) {
	float actorX, actorY, actorZ;
	Actor_Query_XYZ(actorId, &actorX, &actorY, &actorZ);
	return sqrt(static_cast<float>((z - actorZ) * (z - actorZ) + (y - actorY) * (y - actorY) + (x - actorX) * (x - actorX)));
}

} // End of namespace BladeRunner
