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

AIScriptHasan::AIScriptHasan(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 6;
	_var2 = 1;
	_var3 = 0;
	_var4 = 0;
	_var5 = 0;
	_var6 = 0;
}

void AIScriptHasan::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 6;
	_var2 = 1;
	_var3 = 0;
	_var4 = 0;
	_var5 = 0;
	_var6 = 0;

	Actor_Put_In_Set(kActorHasan, kSetAR01_AR02);
	Actor_Set_At_XYZ(kActorHasan, -214.0f, 0.0f, -1379.0f, 371);
	Actor_Set_Goal_Number(kActorHasan, kGoalHasanDefault);
}

bool AIScriptHasan::Update() {
	if (_vm->_cutContent) {
		if (Global_Variable_Query(kVariableChapter) == 1
		    && Actor_Query_Goal_Number(kActorHasan) != kGoalHasanIsAway)
		{
			// use this goal to put Hasan in a FreeSlot set for Act 1
			Actor_Set_Goal_Number(kActorHasan, kGoalHasanIsAway);
			return true;
		} else if (Global_Variable_Query(kVariableChapter) == 2
		           && Actor_Query_Goal_Number(kActorHasan) != kGoalHasanDefault
		) {
			Actor_Set_Goal_Number(kActorHasan, kGoalHasanDefault);
			return true;
		} else if (Global_Variable_Query(kVariableChapter) > 2
		           && Actor_Query_Goal_Number(kActorHasan) < kGoalHasanIsWalkingAroundIsAtAR02) {
			Actor_Set_Goal_Number(kActorHasan, kGoalHasanIsWalkingAroundIsAtAR02);
			return true;
		}
		return false;
	} else {
		// original behavior results in Hasan going away when chapter 3 begins
		if (Global_Variable_Query(kVariableChapter) != 3 || Actor_Query_Goal_Number(kActorHasan) >= kGoalHasanIsAway)
			return false;

		Actor_Set_Goal_Number(kActorHasan, kGoalHasanIsAway);
		return true;
	}
}

void AIScriptHasan::TimerExpired(int timer) {
	//return false;
}

void AIScriptHasan::CompletedMovementTrack() {
	if (_vm->_cutContent) {
		//
		// *after Act 2* Hasan will be leaving Animoid Row for sometime
		if (Global_Variable_Query(kVariableChapter) > 2) {
			int maxHasanLeaveProb = 4;
			// if Bullet Bob is dead or McCoy reprimanded Bob about Hasan,
			// then Hasan is less likely to leave at Animoid Row, *after Act 2*
			if (Game_Flag_Query(kFlagMcCoyTalkedToBulletBobAboutHasan)
			    || Actor_Query_Goal_Number(kGoalBulletBobDead)
			    || Actor_Query_Goal_Number(kGoalBulletBobGone)
			) {
				maxHasanLeaveProb = 10;
			}

			if (Actor_Query_Goal_Number(kActorHasan) == kGoalHasanIsWalkingAroundIsAtAR02) {
				if (Random_Query(1, maxHasanLeaveProb) == 1
				    && Player_Query_Current_Scene() != kSceneAR01
					&& Player_Query_Current_Scene() != kSceneAR02
				) {
					// Hasan leaves Animoid Row
					Actor_Set_Goal_Number(kActorHasan, kGoalHasanIsWalkingAroundIsAway);
					return; // true;
				} else {
					// stay (change goal to a temp one to trigger GoalChanged() (so that we can reset the "track")
					Actor_Set_Goal_Number(kActorHasan, kGoalHasanIsWalkingAroundStaysAtAR02);
					return; // true;
				}
			}

			if (Actor_Query_Goal_Number(kActorHasan) == kGoalHasanIsWalkingAroundIsAway) {
				if (Random_Query(1, 2) == 1
				    && Player_Query_Current_Scene() != kSceneAR01
					&& Player_Query_Current_Scene() != kSceneAR02
				) {
					// Hasan returns tp Animoid Row
					Actor_Set_Goal_Number(kActorHasan, kGoalHasanIsWalkingAroundIsAtAR02);
					return; // true;
				} else {
					// stay away (change goal to a temp one to trigger GoalChanged() (so that we can reset the "track")
					Actor_Set_Goal_Number(kActorHasan, kGoalHasanIsWalkingAroundStayAwayFromAR02);
					return; // true;
				}
			}
		}
	}
	//return false;
}

void AIScriptHasan::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHasan::ClickedByPlayer() {
	//return false;
}

void AIScriptHasan::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptHasan::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptHasan::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptHasan::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptHasan::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHasan::ShotAtAndHit() {
	return false;
}

void AIScriptHasan::Retired(int byActorId) {
	// return false;
}

int AIScriptHasan::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHasan::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (_vm->_cutContent) {
		switch (newGoalNumber) {
		case kGoalHasanDefault:
			Actor_Put_In_Set(kActorHasan, kSetAR01_AR02);
			Actor_Set_At_XYZ(kActorHasan, -214.0f, 0.0f, -1379.0f, 371);
			break;
		case kGoalHasanIsAway:
			Actor_Put_In_Set(kActorHasan, kSetFreeSlotH);
			Actor_Set_At_Waypoint(kActorHasan, 40, 0); // kSetFreeSlotH
			break;
		case kGoalHasanIsWalkingAroundIsAtAR02:
			AI_Movement_Track_Flush(kActorHasan);
			World_Waypoint_Set(554, kSetAR01_AR02, -214.0f, 0.0f, -1379.0f);
			AI_Movement_Track_Append(kActorHasan, 554, Random_Query(60, 180)); // kSetAR01_AR02
			Actor_Face_Heading(kActorHasan, 371, false);
			AI_Movement_Track_Repeat(kActorHasan);
			break;
		case kGoalHasanIsWalkingAroundIsAway:
			AI_Movement_Track_Flush(kActorHasan);
			AI_Movement_Track_Append(kActorHasan, 40, Random_Query(10, 40)); // kSetFreeSlotH
			AI_Movement_Track_Repeat(kActorHasan);
			break;
		case kGoalHasanIsWalkingAroundStayAwayFromAR02:
			Actor_Set_Goal_Number(kActorHasan, kGoalHasanIsWalkingAroundIsAway);
			break;
		case kGoalHasanIsWalkingAroundStaysAtAR02:
			Actor_Set_Goal_Number(kActorHasan, kGoalHasanIsWalkingAroundIsAtAR02);
			break;
		}
	} else {
		// original behavior - Hasan leaves on Act 3
		if (newGoalNumber == kGoalHasanIsAway) {
			Actor_Put_In_Set(kActorHasan, kSetFreeSlotH);
			Actor_Set_At_Waypoint(kActorHasan, 40, 0); // kSetFreeSlotH
		}
	}
	return false;
}

bool AIScriptHasan::UpdateAnimation(int *animation, int *frame) {
	if (_var4) {
		_var4--;
	}
	if (_var5) {
		_var5--;
	}

	switch (_animationState) {
	case 0:
		if (_var6 == 1) {
			*animation = kModelAnimationHasanTalkSuggest;
			if (_var3) {
				_var3--;
			} else {
				_animationFrame++;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHasanTalkSuggest)) {
					_animationFrame = 0;
					_var6 = 0;
					*animation = kModelAnimationHasanIdleSlightMovement;
					_var1 = Random_Query(6, 14);
					_var2 = 2 * Random_Query(0, 1) - 1;
					_var4 = Random_Query(40, 60);
				}
				if (_animationFrame >= 10 && _animationFrame <= 14) {
					_var3 = Random_Query(0, 1);
				}
			}
		} else if (_var6 == 2) {
			*animation = kModelAnimationHasanTakingABiteSnakeMove;
			if (_var3) {
				_var3--;
			} else {
				_animationFrame++;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHasanTakingABiteSnakeMove)) {
					_animationFrame = 0;
					_var6 = 0;
					*animation = kModelAnimationHasanIdleSlightMovement;
					_var1 = Random_Query(6, 14);
					_var2 = 2 * Random_Query(0, 1) - 1;
					_var5 = Random_Query(40, 60);
				}
				if (_animationFrame == 14) {
					_var3 = Random_Query(3, 10);
				}
				if (_animationFrame == 23) {
					_var3 = Random_Query(0, 4);
				}
			}
		} else if (_var6 == 0) {
			*animation = kModelAnimationHasanIdleSlightMovement;
			if (_var3) {
				_var3--;
			} else {
				_animationFrame += _var2;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHasanIdleSlightMovement)) {
					_animationFrame = 0;
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationHasanIdleSlightMovement) - 1;
				}
				if (!--_var1) {
					_var2 = 2 * Random_Query(0, 1) - 1;
					_var1 = Random_Query(6, 14);
					_var3 = Random_Query(0, 4);
				}
				if (!_animationFrame) {
					_var6 = Random_Query(0, 2);
				}
				if (_var6 == 1 && _var4) {
					_var6 = 0;
				}
				if (_var6 == 2 && _var5) {
					_var6 = 0;
				}
			}
		}
		break;

	case 1:
		*animation = kModelAnimationHasanTalkMovingBothHands;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHasanTalkMovingBothHands)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationHasanTalkLeftRightLeftGesture;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHasanTalkLeftRightLeftGesture)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationHasanTalkMovingBothHands;
		}
		break;

	case 3:
		*animation = kModelAnimationHasanTalkRaiseHandSnakeMove;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHasanTalkRaiseHandSnakeMove)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationHasanTalkMovingBothHands;
		}
		break;

	case 4:
		*animation = kModelAnimationHasanTalkMovingBothHandsAndNod;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHasanTalkMovingBothHandsAndNod)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationHasanTalkMovingBothHands;
		}
		break;

	case 5:
		*animation = kModelAnimationHasanTalkWipeFaceLeftHand02;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHasanTalkWipeFaceLeftHand02)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationHasanTalkMovingBothHands;
		}
		break;

	case 6:
		*animation = kModelAnimationHasanTalkUpset;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHasanTalkUpset)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationHasanTalkMovingBothHands;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptHasan::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		_animationState = 0;
		_var6 = 0;
		_animationFrame = 0;
		break;

	case 3:
		_animationState = 1;
		_var6 = 0;
		_animationFrame = 0;
		break;

	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		_animationState = 6;
		_var6 = 0;
		_animationFrame = 0;
		break;

	default:
		break;
	}
	return true;
}

void AIScriptHasan::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHasan::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHasan::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptHasan::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
