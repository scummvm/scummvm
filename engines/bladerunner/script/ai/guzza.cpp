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

AIScriptGuzza::AIScriptGuzza(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_frameDelta = 0;
	_counter = 0;
	_state = 0;
	_flag = false;
}

void AIScriptGuzza::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;
	_frameDelta = 1;
	_counter = 0;
	_state = 0;
	_flag = false;
#if BLADERUNNER_ORIGINAL_BUGS
	// Guzza begins with -1 as a goal number in the original, it is unset until Act 2
#else
	Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaDefault);
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool AIScriptGuzza::Update() {
	if (Global_Variable_Query(kVariableChapter) == 2) {
		if (!Game_Flag_Query(kFlagGuzzaIsMovingAround)) {
			Game_Flag_Set(kFlagGuzzaIsMovingAround);
			Actor_Put_In_Set(kActorGuzza, kSetFreeSlotC);
			Actor_Set_At_Waypoint(kActorGuzza, 35, 0);
			Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaLeftOffice);
			return true;
		}

		if ( Actor_Query_Goal_Number(kActorGuzza) != kGoalGuzzaGoToHawkersCircle1
		 && !Game_Flag_Query(kFlagHC01GuzzaWalk)
		 &&  Game_Flag_Query(kFlagHC01GuzzaPrepare)
		) {
			Game_Flag_Set(kFlagHC01GuzzaWalk);
			Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaGoToHawkersCircle2);
			return true;
		}
	}
	return false;
}

void AIScriptGuzza::TimerExpired(int timer) {
	//return false;
}

void AIScriptGuzza::CompletedMovementTrack() {
	// For Guzza, his movement tracks and goals are used to make him move around
	// If McCoy enters his office (PS04) his movement is paused (and unpaused when McCoy exits),
	// so ,while McCoy is there, Guzza won't blink in or out of the office.
	// Guzza starts moving around from Act 2. In Act 1 he has no movement tracks and stays in his office -- and in original his goal is -1 (undefined).
	//
	// In Act 2, he may appear at HC01 when McCoy enters HC01 from AR01 (goal  (if he hasn't been there already
	// After that he can be there by 50% after he leaves the office (if his goal is set to kGoalGuzzaGoToHawkersCircle1)
	//
	// In Acts 2, 3: if McCoy enters the Police Elevator from ground floor, Guzza's goal is reset to "kGoalGuzzaLeftOffice"
	// so Guzza can't get "stuck" away from his office forever during those Acts
	//
	// TODO Check if in Act 4: is it possible (albeit highly unlikely) that he will be at Hawker's Circle (but hidden at final waypoint of kGoalGuzzaGoToHawkersCircle1) (before UG18 meeting)?
	//
	// In Act 4, after his scene in UG18, he goes to kSetFreeSlotI and stays there
	switch (Actor_Query_Goal_Number(kActorGuzza)) {
	case kGoalGuzzaLeftOffice:
		// This puts Guzza back to his office, when his time away (track) is complete
		// Guzza stays in his office for 600 seconds (10 minutes)
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaAtOffice);
		// return true;
		break;

	case kGoalGuzzaAtOffice:
		// after his time in the office is complete:
		if (Random_Query(1, 2) == 1) {
			// Guzza goes to Hawker's Circle
			// (and stays at final way point awaiting a goal change)
			Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaGoToHawkersCircle1);
		} else {
			// Guzza goes "away" for 60 seconds (1 minute)
			// (and stays at final way point awaiting a goal change)
			Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaGoToFreeSlotB);
		}
		// return true;
		break;

	case kGoalGuzzaGoToHawkersCircle2:
		// After the short walk in Hawker's Circle:
		// Guzza will "leave his office", stay for 90 seconds in kSetFreeSlotC
		// (after that he'll be back in his office)
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaLeftOffice);
		// return true;
		break;

	case kGoalGuzzaGoToFreeSlotG: // bug? when does this happen?
		// Guzza will "leave his office", stay for 90 seconds in kSetFreeSlotC
		// (after that he'll be back in his office)
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaLeftOffice);
		// return true;
		break;

	}
	// return false;
}

void AIScriptGuzza::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptGuzza::ClickedByPlayer() {
	if (Global_Variable_Query(kVariableChapter) == 2
	 && Game_Flag_Query(kFlagGuzzaIsMovingAround)
	) {
		Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);

#if BLADERUNNER_ORIGINAL_BUGS
		if (Actor_Query_Friendliness_To_Other(kActorGordo, kActorMcCoy) < 48) { // a bug? shouldn't this be Gordo?
			Actor_Says(kActorMcCoy, 3970, 13);
			Actor_Says(kActorGuzza, 780, -1);
		}
		// TODO: test this, looks like a bug in game
		// At the very least Random_Query(1, 4) should only be calculated once
		// and clicking on Guzza should probably always produce a quote?
		if (Random_Query(1, 4) == 1) {
			AI_Movement_Track_Pause(kActorGuzza);
			Actor_Says(kActorMcCoy, 4005, 15);
			Actor_Says(kActorGuzza, 780, -1);
			AI_Movement_Track_Unpause(kActorGuzza);
		} else if (Random_Query(1, 4) == 2) {
			AI_Movement_Track_Pause(kActorGuzza);
			Actor_Says(kActorMcCoy, 3970, 14);
			Actor_Says(kActorGuzza, 780, -1);
			AI_Movement_Track_Unpause(kActorGuzza);
		} else if (Random_Query(1, 4) == 3) {
			Actor_Says(kActorMcCoy, 3970, 16);
		} else if (Random_Query(1, 4) == 4) {
			Actor_Says(kActorMcCoy, 3970, 13);
		}
#else
		if (Actor_Query_Friendliness_To_Other(kActorGuzza, kActorMcCoy) < 48) {
			Actor_Says(kActorMcCoy, 3970, 13);	// Hey
			Actor_Says(kActorGuzza, 780, -1);   // Get lost
		} else {
			// At the very least Random_Query(1, 4) should only be calculated once
			switch (Random_Query(1, 4)) {
			case 1:
				AI_Movement_Track_Pause(kActorGuzza);
				Actor_Says(kActorMcCoy, 4005, 15);
				Actor_Says(kActorGuzza, 780, -1);
				AI_Movement_Track_Unpause(kActorGuzza);
				break;
			case 2:
				AI_Movement_Track_Pause(kActorGuzza);
				Actor_Says(kActorMcCoy, 3970, 14);
				Actor_Says(kActorGuzza, 780, -1);
				AI_Movement_Track_Unpause(kActorGuzza);
				break;
			case 3:
				Actor_Says(kActorMcCoy, 3970, 16);
				break;
			case 4:
				// fall through
			default:
				Actor_Says(kActorMcCoy, 3970, 13);
				break;
			}
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	// return false;
}

void AIScriptGuzza::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptGuzza::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptGuzza::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptGuzza::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptGuzza::ShotAtAndMissed() {
	if (Actor_Query_Goal_Number(kActorGuzza) == kGoalGuzzaUG18Target) {
		Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatHit);
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaUG18MissedByMcCoy);
	}
	// return false;
}

bool AIScriptGuzza::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorGuzza) == kGoalGuzzaUG18Target) {
		Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatHit);
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaUG18HitByMcCoy);
	}
	return false;
}

void AIScriptGuzza::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaGone);
	// return false;
}

int AIScriptGuzza::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptGuzza::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalGuzzaLeftOffice:
		// Guzza stays for a few seconds in his office (waypoint 263) then goes to kSetFreeSlotC (waypoint 35) for 90 seconds
		AI_Movement_Track_Flush(kActorGuzza);
		AI_Movement_Track_Append_With_Facing(kActorGuzza, 263, 0, 150);
		AI_Movement_Track_Append_With_Facing(kActorGuzza, 263, 5, 150);
		AI_Movement_Track_Append(kActorGuzza, 35, 90);
		AI_Movement_Track_Repeat(kActorGuzza);
		return true;

	case kGoalGuzzaGoToHawkersCircle1:
		// walk around in kSetHC01_HC02_HC03_HC04 for a short while
		AI_Movement_Track_Flush(kActorGuzza);
		AI_Movement_Track_Append(kActorGuzza, 258, 0);
		AI_Movement_Track_Append(kActorGuzza, 260, 8);
		AI_Movement_Track_Append(kActorGuzza, 261, 5);
		AI_Movement_Track_Append(kActorGuzza, 262, 0);
		AI_Movement_Track_Repeat(kActorGuzza);
		return true;

	case kGoalGuzzaAtOffice:
		// stay for 600 seconds in office
		AI_Movement_Track_Flush(kActorGuzza);
		AI_Movement_Track_Flush(kActorGuzza); // a bug? is this needed twice?
		AI_Movement_Track_Append_With_Facing(kActorGuzza, 263, 600, 150);
		AI_Movement_Track_Repeat(kActorGuzza);
		return true;

	case kGoalGuzzaGoToHawkersCircle2:
		// walk around in kSetHC01_HC02_HC03_HC04 for few seconds
		AI_Movement_Track_Flush(kActorGuzza);
		AI_Movement_Track_Append(kActorGuzza, 258, 0);
		AI_Movement_Track_Append(kActorGuzza, 259, 1);
		AI_Movement_Track_Append(kActorGuzza, 258, 0);
		AI_Movement_Track_Repeat(kActorGuzza);
		return true;

	case kGoalGuzzaGoToFreeSlotB:
		// stay in kSetFreeSlotB for 60 seconds
		AI_Movement_Track_Flush(kActorGuzza);
		AI_Movement_Track_Append(kActorGuzza, 34, 60);
		AI_Movement_Track_Repeat(kActorGuzza);
		return true;

	case kGoalGuzzaGoToFreeSlotG:
		// stay in kSetFreeSlotG for 39 seconds // a bug? this goal is never set
		AI_Movement_Track_Flush(kActorGuzza);
		AI_Movement_Track_Append(kActorGuzza, 39, 120);
		AI_Movement_Track_Repeat(kActorGuzza);
		return true;

	case kGoalGuzzaSitAtNR03:
		Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeSit);
		_animationState = 1;
		_animationFrame = 0;
		Actor_Put_In_Set(kActorGuzza, kSetNR03);
		Actor_Set_At_XYZ(kActorGuzza, -229.0f, -70.19f, -469.0f, 400);
		return true;

	case kGoalGuzzaUG18Wait:
		Actor_Put_In_Set(kActorGuzza, kSetUG18);
		Actor_Set_At_XYZ(kActorGuzza, 10.79f, 0.0f, -354.17f, 400);
		Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeIdle);
		return true;

	case kGoalGuzzaUG18Target:
		Actor_Set_Targetable(kActorGuzza, true);
		return true;

	case kGoalGuzzaUG18WillGetShotBySadik:
	case kGoalGuzzaUG18HitByMcCoy:
	case kGoalGuzzaUG18MissedByMcCoy:
		Actor_Set_Targetable(kActorGuzza, false);
		return true;

	case kGoalGuzzaUG18ShotByMcCoy:
	case kGoalGuzzaUG18ShootMcCoy:
	case kGoalGuzzaUG18FallDown:
	case kGoalGuzzaUG18ShotBySadik:
		return true;
	}
	return false;
}

bool AIScriptGuzza::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		switch (_state) {
		case 0:
			*animation = 197;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(197)) {
				_animationFrame = 0;
				if (Random_Query(0, 5) == 0) {
					_state = Random_Query(1, 2);
				}
			}
			break;
		case 1:
			*animation = 198;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(198)) {
				*animation = 197;
				_animationFrame = 0;
				_state = 0;
			}
			break;
		case 2:
			*animation = 199;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(199)) {
				*animation = 197;
				_animationFrame = 0;
				_state = 0;
			}
			break;
		}
		break;

	case 1:
		switch (_state) {
		case 0:
			*animation = 189;
			if (_counter) {
				_counter--;
				if (Random_Query(0, 6) == 0) {
					_frameDelta = -_frameDelta;
				}
			} else {
				_animationFrame += _frameDelta;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
					_animationFrame = 0;
					if (Random_Query(0, 2) == 0) {
						_state = 2 * Random_Query(0, 1);
					}
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
				}
				_counter = Random_Query(0, 1);
				if (_animationFrame == 2 || _animationFrame == 15) {
					_counter = Random_Query(5, 12);
				}
				if (_animationFrame == 8) {
					_counter = Random_Query(3, 7);
				}
			}
			break;
		case 1:
			*animation = 190;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(190)) {
				*animation = 189;
				_animationFrame = 0;
				_state = 0;
			}
			break;
		case 2:
			*animation = 191;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(191)) {
				if (Random_Query(0, 2) == 0) {
					*animation = 189;
					_animationFrame = 0;
					_state = 0;
				} else {
					*animation = 190;
					_animationFrame = 0;
					_state = 1;
				}
			}
			break;
		}
		break;

	case 2:
		if (_state == 0) {
			*animation = _animationNext;
			_animationFrame = 0;
			_animationState = _animationStateNext;
		} else {
			if (_state == 1) {
				*animation = 198;
			} else if (_state == 2) {
				*animation = 199;
			}
			_animationFrame += 2;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				*animation = _animationNext;
				_animationFrame = 0;
				_animationState = _animationStateNext;
			}
		}
		break;

	case 3:
		switch (_state) {
		case 0:
			*animation = 189;
			break;
		case 1:
			*animation = 190;
			break;
		case 2:
			*animation = 191;
			break;
		}
		_animationFrame += 4;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = _animationNext;
			_animationFrame = 0;
			_animationState = _animationStateNext;
		}
		break;

	case 4:
		*animation = 185;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(185)) {
			_animationFrame = 0;
		}
		break;

	case 5:
		*animation = 186;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(186)) {
			_animationFrame = 0;
		}
		break;

	case 6:
		*animation = 176;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(176)) {
			_animationFrame = 0;
		}
		break;

	case 7:
		*animation = 177;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(177)) {
			_animationFrame = 0;
		}
		break;

	case 8:
		*animation = 181;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(181)) {
			_animationFrame = 0;
		}
		break;

	case 9:
		*animation = 187;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(187)) {
			_animationFrame = 0;
		}
		break;

	case 10:
		*animation = 188;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(188)) {
			_animationFrame = 0;
		}
		break;

	case 11:
		if (_animationFrame == 0 && _flag) {
			*animation = 197;
			_animationState = 0;
			_flag = false;
			_state = 0;
			_counter = 0;
			_frameDelta = 1;
		} else {
			*animation = 201;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(201)) {
				_animationFrame = 0;
			}
		}
		break;

	case 12:
		*animation = 202;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(202)) {
			*animation = 201;
			_animationFrame = 0;
			_animationState = 11;
		}
		break;

	case 13:
		*animation = 203;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(203)) {
			*animation = 201;
			_animationFrame = 0;
			_animationState = 11;
		}
		break;

	case 14:
		*animation = 204;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(204)) {
			*animation = 201;
			_animationFrame = 0;
			_animationState = 11;
		}
		break;

	case 15:
		*animation = 205;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(205)) {
			*animation = 201;
			_animationFrame = 0;
			_animationState = 11;
		}
		break;

	case 16:
		*animation = 206;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(206)) {
			*animation = 201;
			_animationFrame = 0;
			_animationState = 11;
		}
		break;

	case 17:
		if (_animationFrame == 0 && _flag) {
			*animation = 189;
			_animationState = 1;
			_flag = false;
			Actor_Change_Animation_Mode(kActorGuzza, 53);
			_state = 0;
			_counter = 0;
			_frameDelta = 1;
		} else {
			*animation = 192;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(192)) {
				_animationFrame = 0;
			}
		}
		break;

	case 18:
		*animation = 193;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(193)) {
			*animation = 192;
			_animationFrame = 0;
			_animationState = 17;
		}
		break;

	case 19:
		*animation = 194;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(194)) {
			*animation = 192;
			_animationState = 17;
			_animationFrame = 0;
		}
		break;

	case 20:
		*animation = 195;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(195)) {
			*animation = 192;
			_animationFrame = 0;
			_animationState = 17;
		}
		break;

	case 21:
		*animation = 196;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(196)) {
			*animation = 192;
			_animationFrame = 0;
			_animationState = 17;
		}
		break;

	case 22:
		if (_animationFrame == 0 && _flag) {
			*animation = 172;
			_animationState = 24;
			_flag = false;
			Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatIdle);
			_state = 0;
			_counter = 0;
			_frameDelta = 1;
		} else {
			*animation = 179;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(179)) {
				_animationFrame = 0;
			}
		}
		break;

	case 23:
		if (_animationFrame == 0 && _flag) {
			*animation = 172;
			_animationState = 24;
			_flag = false;
			Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatIdle);
			_state = 0;
			_counter = 0;
			_frameDelta = 1;
		} else {
			*animation = 180;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(180)) {
				_animationFrame = 0;
			}
		}
		break;

	case 24:
		*animation = 172;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(172)) {
			_animationFrame = 0;
		}
		break;

	case 25:
		*animation = 173;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(173)) {
			_animationFrame = 0;
		}
		break;

	case 26:
		*animation = 174;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(174)) {
			*animation = 172;
			_animationFrame = 0;
			_animationState = 24;
			Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatIdle);
		}
		break;

	case 27:
		*animation = 175;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(175)) {
			*animation = 172;
			_animationFrame = 0;
			_animationState = 24;
			Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatIdle);
		}
		break;

	case 29:
		*animation = 182;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(182)) {
			*animation = 172;
			_animationFrame = 0;
			_animationState = 24;
		}
		break;

	case 30:
		*animation = 183;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(183)) {
			*animation = 197;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 31:
		*animation = 184;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(184)) {
			*animation = 172;
			_animationFrame = 0;
			_animationState = 24;
			Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatIdle);
		}
		break;

	case 32:
		*animation = 200;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(200)) {
			*animation = 197;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeIdle);
		}
		break;

	case 33:
		*animation = 207;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(207)) {
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			float x, y, z;
			Actor_Query_XYZ(kActorGuzza, &x, &y, &z);
			// TODO: test
			if (-180.0f <= y) {
				y -= 15.0f;
				x += 6.0f;
				z -= 12.0f;
				Actor_Set_At_XYZ(kActorGuzza, x, y, z, 729);
			} else {
				_animationState = 34;
			}
		}
		break;

	case 34:
		*animation = 207;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(207) - 1;
		break;

	default:
		*animation = 399;
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptGuzza::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 0:
		case 30:
		case 32:
		case 33:
			break;
		case 6:
		case 7:
			_animationState = 24;
			_animationFrame = 0;
			break;
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			_flag = true;
			break;
		case 24:
		case 31:
			_animationState = 30;
			_animationFrame = 0;
			break;
		default:
			_animationState = 0;
			_animationFrame = 0;
			_state = 0;
			_counter = 0;
			_frameDelta = 1;
			break;
		}
		break;

	case kAnimationModeWalk:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case kAnimationModeRun:
		_animationState = 5;
		_animationFrame = 0;
		break;

	case kAnimationModeTalk:
		if (_animationState) {
			_animationState = 11;
			_animationFrame = 0;
			_flag = false;
		} else {
			_animationState = 2;
			_animationFrame = 0;
			_animationStateNext = 11;
			_animationNext = 201;
		}
		break;

	case kAnimationModeCombatIdle:
		if (_animationState == 0) {
			_animationState = 29;
			_animationFrame = 0;
		} else if (_animationState != 24
		        && _animationState != 29
		) {
			_animationState = 24;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeCombatAttack:
		_animationState = 31;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalk:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatRun:
		_animationState = 7;
		_animationFrame = 0;
		break;

	case 12:
		if (_animationState) {
			_animationState = 12;
			_animationFrame = 0;
			_flag = false;
		} else {
			_animationState = 2;
			_animationFrame = 0;
			_animationStateNext = 12;
			_animationNext = 202;
		}
		break;

	case 13:
		if (_animationState) {
			_animationState = 13;
			_animationFrame = 0;
			_flag = false;
		} else {
			_animationState = 2;
			_animationFrame = 0;
			_animationStateNext = 13;
			_animationNext = 203;
		}
		break;

	case 14:
		if (_animationState) {
			_animationState = 14;
			_animationFrame = 0;
			_flag = false;
		} else {
			_animationState = 2;
			_animationFrame = 0;
			_animationStateNext = 14;
			_animationNext = 204;
		}
		break;

	case 15:
		if (_animationState) {
			_animationState = 15;
			_animationFrame = 0;
			_flag = false;
		} else {
			_animationState = 2;
			_animationFrame = 0;
			_animationStateNext = 15;
			_animationNext = 205;
		}
		break;

	case 16:
		if (_animationState) {
			_animationState = 16;
			_animationFrame = 0;
			_flag = false;
		} else {
			_animationState = 2;
			_animationFrame = 0;
			_animationStateNext = 16;
			_animationNext = 206;
		}
		break;

	case kAnimationModeCombatHit:
		if (Random_Query(0, 1)) {
			_animationState = 26;
		} else {
			_animationState = 27;
		}
		_animationFrame = 0;
		break;

	case 23:
		_animationState = 32;
		_animationFrame = 0;
		break;

	case 30:
		if (_animationState == 1) {
			_animationState = 3;
			_animationStateNext = 17;
			_animationNext = 192;
		} else {
			_animationState = 17;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case 31:
		if (_animationState == 1) {
			_animationState = 3;
			_animationStateNext = 18;
			_animationNext = 193;
		} else {
			_animationState = 18;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case 32:
		if (_animationState == 1) {
			_animationState = 3;
			_animationStateNext = 19;
			_animationNext = 194;
		} else {
			_animationState = 19;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case 33:
		if (_animationState == 1) {
			_animationState = 3;
			_animationStateNext = 20;
			_animationNext = 195;
		} else {
			_animationState = 20;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case 34:
		if (_animationState == 1) {
			_animationState = 3;
			_animationStateNext = 21;
			_animationNext = 196;
		} else {
			_animationState = 21;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case kAnimationModeWalkUp:
		_animationState = 9;
		_animationFrame = 0;
		break;

	case kAnimationModeWalkDown:
		_animationState = 10;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 28;
		_animationFrame = 0;
		break;

	case 53:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case 58:
		_animationState = 22;
		_animationFrame = 0;
		_flag = false;
		break;

	case 59:
		_animationState = 23;
		_animationFrame = 0;
		_flag = false;
		break;

	case 61:
		_animationState = 33;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptGuzza::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptGuzza::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptGuzza::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 263) {
		_animationFrame = 0;
		_animationState = 1;
		return false;
	}
	return true;
}

void AIScriptGuzza::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
