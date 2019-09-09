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

AIScriptSebastian::AIScriptSebastian(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = false;
}

void AIScriptSebastian::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = false;

	Actor_Set_Goal_Number(kActorSebastian, 0);
}

bool AIScriptSebastian::Update() {
	if (Actor_Query_Goal_Number(kActorSebastian) < 200
	 && Global_Variable_Query(kVariableChapter) == 3
	) {
		Actor_Set_Goal_Number(kActorSebastian, 200);
	}

	return false;
}

void AIScriptSebastian::TimerExpired(int timer) {
	//return false;
}

void AIScriptSebastian::CompletedMovementTrack() {
	//return false;
}

void AIScriptSebastian::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptSebastian::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorSebastian) == 205) {
		AI_Movement_Track_Pause(kActorSebastian);
		Actor_Face_Actor(kActorSebastian, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorSebastian, true);
		if (Actor_Clue_Query(kActorSebastian, kClueMcCoyIsABladeRunner)) {
			Actor_Says(kActorMcCoy, 6985, 16);
			Actor_Says(kActorSebastian, 610, 14);
		} else {
			dialogue();
		}

		AI_Movement_Track_Unpause(kActorSebastian);
		return; //true;
	}
	return; //false;
}

void AIScriptSebastian::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptSebastian::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptSebastian::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptSebastian::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (otherActorId == kActorMcCoy
	 && combatMode
	) {
		Global_Variable_Increment(kVariableGunPulledInFrontOfSebastian, 1);
		Actor_Modify_Friendliness_To_Other(kActorSebastian, kActorMcCoy, -5);
		AI_Movement_Track_Pause(kActorSebastian);
		Actor_Face_Actor(kActorSebastian, kActorMcCoy, true);

		if (Global_Variable_Query(kVariableGunPulledInFrontOfSebastian) == 1) {
			Actor_Says(kActorSebastian, 680, 12);
			Actor_Face_Actor(kActorMcCoy, kActorSebastian, true);
			Actor_Says_With_Pause(kActorMcCoy, 7265, 0.0f, kAnimationModeCombatIdle);
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatIdle);
			Delay(500);
			Actor_Says(kActorSebastian, 690, 16);
		} else {
			Actor_Says(kActorSebastian, 700, 15);
			Actor_Says_With_Pause(kActorMcCoy, 7270, 0.0f, kAnimationModeCombatIdle);
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatIdle);
		}
		AI_Movement_Track_Unpause(kActorSebastian);

		return; //true;
	}
}

void AIScriptSebastian::ShotAtAndMissed() {
	// return false;
}

bool AIScriptSebastian::ShotAtAndHit() {
	return false;
}

void AIScriptSebastian::Retired(int byActorId) {
	// return false;
}

int AIScriptSebastian::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptSebastian::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 200) {
		Actor_Put_In_Set(kActorSebastian, kSetBB05);
		Actor_Set_At_XYZ(kActorSebastian, -13.08f, -60.31f, 100.88f, 470);
	}

	return false;
}

bool AIScriptSebastian::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 811;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(811) - 1) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = 809;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(809) - 1) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = 810;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(810) - 1) {
			Actor_Change_Animation_Mode(kActorSebastian, kAnimationModeIdle);
			*animation = 811;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 3:
		*animation = 821;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(821) - 1) {
			_animationFrame++;
		}
		break;

	case 4:
		if (!_animationFrame && _flag) {
			Actor_Change_Animation_Mode(kActorSebastian, kAnimationModeIdle);
			*animation = 811;
			_animationState = 0;
			_animationFrame = 0;
		} else {
			*animation = 813;
			_animationFrame++;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(813) - 1) {
				_animationFrame = 0;
			}
		}
		break;

	case 5:
		*animation = 814;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(814) - 1) {
			_animationFrame = 0;
			_animationState = 4;
			*animation = 813;
		}
		break;

	case 6:
		*animation = 815;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(815) - 1) {
			_animationFrame = 0;
			_animationState = 4;
			*animation = 813;
		}
		break;

	case 7:
		*animation = 816;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(816) - 1) {
			_animationFrame = 0;
			_animationState = 4;
			*animation = 813;
		}
		break;

	case 8:
		*animation = 817;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(817) - 1) {
			_animationFrame = 0;
			_animationState = 4;
			*animation = 813;
		}
		break;

	case 9:
		*animation = 818;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(818) - 1) {
			_animationFrame = 0;
			_animationState = 4;
			*animation = 813;
		}
		break;

	case 10:
		*animation = 819;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(819) - 1) {
			_animationFrame = 0;
			_animationState = 4;
			*animation = 813;
		}
		break;

	case 11:
		*animation = 820;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(820) - 1) {
			_animationFrame = 0;
			_animationState = 4;
			*animation = 813;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptSebastian::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (_animationState > 11) {
			_animationState = 0;
			_animationFrame = 0;
		} else {
			_flag = true;
		}
		break;

	case kAnimationModeWalk:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case kAnimationModeTalk:
		_animationState = 4;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 12:
		_animationState = 5;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 13:
		_animationState = 6;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 14:
		_animationState = 7;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 15:
		_animationState = 8;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 16:
		_animationState = 9;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 17:
		_animationState = 10;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 18:
		_animationState = 11;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 20:
		_animationState = 2;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 3;
		_animationFrame = 0;
		break;
	}

	return true;
}

void AIScriptSebastian::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptSebastian::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptSebastian::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptSebastian::FledCombat() {
	// return false;
}

void AIScriptSebastian::dialogue() {
	Dialogue_Menu_Clear_List();

	if (Actor_Query_Friendliness_To_Other(kActorSebastian, kActorMcCoy) >= 45) {
		DM_Add_To_List_Never_Repeat_Once_Selected(930, 5, 5, 5); // MORAJI AND CHEW
		DM_Add_To_List_Never_Repeat_Once_Selected(940, -1, 5, 6); // EISENDULLER
		DM_Add_To_List_Never_Repeat_Once_Selected(950, 5, 5, 5); // TYRELL
	}

	if (Actor_Clue_Query(kActorMcCoy, kClueAnsweringMachineMessage)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(960, 3, -1, 5); // TWINS
	}

	if (Actor_Clue_Query(kActorMcCoy, kClueAnsweringMachineMessage)
	 && Actor_Clue_Query(kActorMcCoy, kClueEnvelope)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(970, -1, 4, -1); // RUNCITER
	}

	DM_Add_To_List_Never_Repeat_Once_Selected(980, -1, -1, 7); // ROBBERS
	DM_Add_To_List_Never_Repeat_Once_Selected(990, 7, 3, -1); // NEXUS-6

	if (Dialogue_Menu_Query_List_Size()) {
		// This condition clause for non-empty dialogue menu options before adding the DONE option
		// only occurs in Sebastian's AI script.
		// Probably because, selecting "DONE" here, McCoy has nothing to say
		// so there's no point to add it as a "auto-selected" last option
		// if no other options exist in the list
		Dialogue_Menu_Add_DONE_To_List(1000); // DONE
		Dialogue_Menu_Appear(320, 240);
		int answer = Dialogue_Menu_Query_Input();
		Dialogue_Menu_Disappear();

		switch (answer) {
		case 930: // MORAJI AND CHEW
			Actor_Says(kActorMcCoy, 7075, 13);
			Actor_Says(kActorSebastian, 290, 12);
			Actor_Says(kActorSebastian, 300, 13);
			break;

		case 940: // EISENDULLER
			Actor_Says(kActorMcCoy, 7080, 15);
			Actor_Says(kActorSebastian, 310, 13);
			Actor_Says(kActorSebastian, 320, 16);
			Actor_Says(kActorSebastian, 340, 12);
			Actor_Says(kActorMcCoy, 7120, 14);
			Actor_Says(kActorSebastian, 350, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 7125, 13);
			Actor_Says(kActorSebastian, 360, 17);
			Actor_Says_With_Pause(kActorMcCoy, 7130, 1.0f, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 7135, 18);
			break;

		case 950: // TYRELL
			Actor_Says(kActorMcCoy, 7085, 15);
			Actor_Says_With_Pause(kActorSebastian, 370, 0.30f, 13);
			Actor_Says_With_Pause(kActorSebastian, 380, 0.70f, 17);
			Actor_Says(kActorSebastian, 390, 14);
			if (Actor_Clue_Query(kActorMcCoy, kClueChessTable)) {
				Actor_Says(kActorMcCoy, 7140, kAnimationModeTalk);
				Actor_Says(kActorSebastian, 400, 12);
				Actor_Says(kActorMcCoy, 7145, 16);
				Actor_Says(kActorSebastian, 410, 13);
				Actor_Says(kActorMcCoy, 7150, 17);
				Actor_Says(kActorSebastian, 420, 13);
				Actor_Says(kActorSebastian, 430, 14);
			}
			break;

		case 960: // TWINS
			Actor_Says(kActorMcCoy, 7090, 17);
			Actor_Says(kActorSebastian, 440, 14);
			Actor_Says(kActorSebastian, 450, 13);
			Actor_Says(kActorMcCoy, 7155, 13);
			Actor_Says(kActorSebastian, 460, 17);
			Actor_Says(kActorSebastian, 470, 12);
			Actor_Says(kActorSebastian, 480, 13);
			Actor_Says(kActorMcCoy, 7160, 18);
			Actor_Says(kActorSebastian, 490, 14);
			Actor_Says(kActorMcCoy, 7165, 14);
			setMcCoyIsABladeRunner();
			break;

		case 970: // RUNCITER
			Actor_Says(kActorMcCoy, 7095, 13);
			Actor_Says(kActorSebastian, 500, 15);
			Actor_Says(kActorMcCoy, 7170, 17);
			Actor_Says(kActorSebastian, 510, 12);
			Actor_Says(kActorMcCoy, 7175, 18);
			Actor_Says(kActorSebastian, 520, 14);
			Actor_Says(kActorMcCoy, 7180, 12);
			Actor_Says(kActorSebastian, 530, 13);
			Actor_Says(kActorMcCoy, 7185, 12);
			setMcCoyIsABladeRunner();
			break;

		case 980: // ROBBERS
			Actor_Says(kActorMcCoy, 7100, 12);
			Actor_Says(kActorSebastian, 540, 16);
			Actor_Says(kActorMcCoy, 7195, 18);
			Actor_Says(kActorSebastian, 720, 12);
			break;

		case 990: // NEXUS-6
			Actor_Says(kActorMcCoy, 7105, 18);
			setMcCoyIsABladeRunner();
			break;

		default:
			break;
		}
	} else if (Actor_Query_Friendliness_To_Other(kActorSebastian, kActorMcCoy) >= 45) {
		Actor_Says(kActorMcCoy, 7115, 13);
		Actor_Says(kActorSebastian, 280, 14);
	} else {
		Actor_Says(kActorMcCoy, 7110, 15);
		Actor_Says(kActorSebastian, 270, 16);
	}
}

void AIScriptSebastian::setMcCoyIsABladeRunner() {
	Actor_Clue_Acquire(kActorSebastian, kClueMcCoyIsABladeRunner, true, kActorMcCoy);
	Actor_Modify_Friendliness_To_Other(kActorSebastian, kActorMcCoy, -5);
	Actor_Says(kActorSebastian, 560, 15);
	Actor_Says(kActorMcCoy, 7200, 14);
	Actor_Says(kActorSebastian, 570, 16);
	Actor_Says(kActorMcCoy, 7205, 17);
	Actor_Says(kActorSebastian, 580, 13);
	Actor_Says_With_Pause(kActorMcCoy, 7210, 1.0f, 16);
	Actor_Says(kActorSebastian, 590, 12);
	Actor_Says(kActorMcCoy, 7215, 19);
	Actor_Says(kActorSebastian, 600, 14);
	Actor_Says(kActorMcCoy, 7220, 13);
	Actor_Says_With_Pause(kActorMcCoy, 7225, 0.80f, 14);
	Actor_Says(kActorSebastian, 610, 15);
}

} // End of namespace BladeRunner
