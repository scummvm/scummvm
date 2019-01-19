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

AIScriptMoraji::AIScriptMoraji(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 1;
	_var2 = 0;
}

void AIScriptMoraji::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 1;
	_var2 = 0;

	Actor_Set_Goal_Number(kActorMoraji, 0);
}

bool AIScriptMoraji::Update() {
	if ( Actor_Query_Goal_Number(kActorMoraji) == 0
	 &&  Player_Query_Current_Scene() == kSceneDR05
	 && !Game_Flag_Query(kFlagDR05BombActivated)
	) {
		AI_Countdown_Timer_Reset(kActorMoraji, 2);
		AI_Countdown_Timer_Start(kActorMoraji, 2, 30);
		Game_Flag_Set(kFlagDR05BombActivated);
		return true;
	}

	if (Actor_Query_Goal_Number(kActorMoraji) == 19) {
		Actor_Says(kActorMoraji, 80, 13);
		_animationState = 9;
		_animationFrame = -1;
		Actor_Set_Goal_Number(kActorMoraji, 18);
	}
	return false;
}

void AIScriptMoraji::TimerExpired(int timer) {
	if (timer == 2) {
		AI_Countdown_Timer_Reset(kActorMoraji, 2);
		if (Actor_Query_Goal_Number(kActorMoraji) != 20
		 && Actor_Query_Goal_Number(kActorMoraji) != 21
		 && Actor_Query_Goal_Number(kActorMoraji) != 99
		) {
			Game_Flag_Set(kFlagDR05BombWillExplode);
		}
		return; //true;
	}

	return; //false;
}

void AIScriptMoraji::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorMoraji) == 11) {
		AI_Countdown_Timer_Reset(kActorMoraji, 2);
		Game_Flag_Set(kFlagDR05BombWillExplode);
		_animationState = 3;

		return; //true;
	}
	return; //false
}

void AIScriptMoraji::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptMoraji::ClickedByPlayer() {
	//return false;
}

void AIScriptMoraji::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptMoraji::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptMoraji::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptMoraji::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptMoraji::ShotAtAndMissed() {
	// return false;
}

bool AIScriptMoraji::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorMoraji)) {
		if (Actor_Query_Goal_Number(kActorMoraji) == 21) {
			Game_Flag_Set(713);
			Actor_Set_Goal_Number(kActorMoraji, 22);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 101);

			return true;
		} else {
			return false;
		}
	} else {
		Actor_Set_Goal_Number(kActorMoraji, 5);
		return true;
	}
}

void AIScriptMoraji::Retired(int byActorId) {
	// return false;
}

int AIScriptMoraji::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMoraji::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 0:
		Actor_Put_In_Set(kActorMoraji, kSetDR05);
		Actor_Set_At_XYZ(kActorMoraji, 50.0f, 0.30f, 35.0f, 414);
		Actor_Set_Targetable(kActorMoraji, 1);
		return false;
	case 5:
		Actor_Set_Targetable(kActorMoraji, 0);
		Sound_Play(4, 100, 0, 0, 50);
		_animationState = 10;
		_animationFrame = 0;
		Actor_Retired_Here(kActorMoraji, 60, 16, 1, -1);
		return true;
	case 10:
		Actor_Set_Targetable(kActorMoraji, 0);
		_animationState = 8;
		_animationFrame = 1;
		return true;
	case 11:
		_animationState = 3;
		_animationFrame = 0;
		AI_Movement_Track_Flush(kActorMoraji);
		AI_Movement_Track_Append_Run(kActorMoraji, 95, 0);
		AI_Movement_Track_Append_Run(kActorMoraji, 96, 0);
		AI_Movement_Track_Append_Run(kActorMoraji, 97, 0);
		AI_Movement_Track_Append_Run(kActorMoraji, 98, 0);
		AI_Movement_Track_Repeat(kActorMoraji);
		return true;
	case 20:
		_animationState = 11;
		return true;
	case 21:
		Actor_Retired_Here(kActorMoraji, 60, 16, 0, -1);
		Actor_Set_Targetable(kActorMoraji, 1);
		return true;
	case 22:
		_animationFrame = -1;
		_animationState = 13;
		return true;
	case 23:
		Actor_Set_Targetable(kActorMoraji, 0);
		_animationState = 14;
		Actor_Retired_Here(kActorMoraji, 60, 16, 1, -1);
		return true;
		break;
	case 30:
		if (Player_Query_Current_Scene() == kSceneDR05) {
			Game_Flag_Set(kFlagDR05BombExplosionView);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR04);
		} else {
			if (Actor_Query_In_Set(kActorMoraji, kSetDR05)) {
				Actor_Set_Goal_Number(kActorMoraji, 99);
			} else {
				Actor_Set_Goal_Number(kActorMoraji, 20);
			}
			Game_Flag_Set(kFlagMorajiExploded);
		}
		return true;
	case 99:
		AI_Movement_Track_Flush(kActorMoraji);
		AI_Movement_Track_Append(kActorMoraji, 41, 0);
		AI_Movement_Track_Repeat(kActorMoraji);
		return true;
	default:
		break;
	}

	return false;
}

bool AIScriptMoraji::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 733;
		if (_var2) {
			_var2--;
		} else {
			_animationFrame += _var1;
			if (!Random_Query(0, 10)) {
				_var1 = -_var1;
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			}
			_var2 = Random_Query(0, 1);
		}
		break;

	case 1:
		_animationFrame = 0;
		*animation = _animationNext;
		_animationState = _animationStateNext;
		break;

	case 2:
		*animation = 290;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(290))
			_animationFrame = 0;
		break;

	case 3:
		*animation = 732;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(732))
			_animationFrame = 0;
		break;

	case 4:
		break;

	case 5:
		*animation = 734;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(734)) {
			_animationFrame = 0;
			_animationState = Random_Query(0, 2) + 5;
		}
		break;

	case 6:
		*animation = 735;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(735)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = 735;
		}
		break;

	case 7:
		*animation = 736;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(736)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = 736;
		}
		break;

	case 8:
		*animation = 737;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(737)) {
			_animationFrame = 0;
			_animationState = 0;
			*animation = 733;
			Actor_Set_Goal_Number(kActorMoraji, 19);
		}
		if (_animationFrame == 6) {
			Ambient_Sounds_Play_Sound(488, 69, 0, 0, 20);
		}
		break;

	case 9:
		*animation = 742;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(742)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 732;
			Actor_Set_Goal_Number(kActorMoraji, 11);
		}
		break;

	case 10:
		*animation = 738;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(738) - 1) {
			++_animationFrame;
		}
		break;

	case 11:
		*animation = 739;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(739)) {
			Actor_Set_Goal_Number(kActorMoraji, 21);
			_animationFrame = 0;
			_animationState = 12;
			*animation = 740;
		}
		break;

	case 12:
		*animation = 740;
		if (_var2) {
			_var2--;
		} else {
			_animationFrame += _var1;
			if (!Random_Query(0, 5)) {
				_var1 = -_var1;
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			}
			_var2 = Random_Query(0, 2);
		}
		break;

	case 13:
		*animation = 741;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(741) - 1) {
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			Actor_Set_Goal_Number(kActorMoraji, 23);
		}
		break;

	case 14:
		*animation = 741;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(741) - 1;
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptMoraji::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState != 12 && Actor_Query_Goal_Number(kActorMoraji) != 11) {
			_animationState = 0;
			_animationFrame = 0;
		}
		return 1;

	case 1:
		_animationState = 2;
		_animationFrame = 0;
		return 1;

	case 2:
		if (_animationState) {
			if (_animationState != 3 || _animationState > 3) {
				_animationState = 3;
				_animationFrame = 0;
			}
		} else {
			_animationState = 1;
			_animationStateNext = 3;
			_animationNext = 732;
		}
		break;

	case 3:
		if (_animationState != 12) {
			if (_animationState) {
				_animationState = 5;
				_animationFrame = 0;
			} else {
				_animationState = 1;
				_animationStateNext = 5;
				_animationNext = 734;
			}
		}
		break;

	case 12:
		if (_animationState) {
			_animationState = 6;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 6;
			_animationNext = 735;
		}
		break;

	case 13:
		if (_animationState) {
			_animationState = 7;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 7;
			_animationNext = 736;
		}
		break;

	case 48:
		_animationState = 13;
		_animationFrame = -1;
		break;
	}

	return true;
}

void AIScriptMoraji::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptMoraji::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptMoraji::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 96)
		AI_Countdown_Timer_Reset(kActorMoraji, 2);

	return true;
}

void AIScriptMoraji::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
