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

AIScriptSadik::AIScriptSadik(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_resumeIdleAfterFramesetCompletesFlag = false;
	_nextSoundId = -1; // changed from original (0) to be more clear that this is an invalid sfx id
	// _varChooseIdleAnimation can have valid values: 0, 1
	_varChooseIdleAnimation = 0;
	_varNumOfTimesToHoldCurrentFrame = 0;
	_var4 = 1;
}

void AIScriptSadik::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_resumeIdleAfterFramesetCompletesFlag = false;
	_nextSoundId = -1; // changed from original (0) to be more clear that this is an invalid sfx id
	_varChooseIdleAnimation = 0;
	_varNumOfTimesToHoldCurrentFrame = 0;
	_var4 = 1;

	Actor_Put_In_Set(kActorSadik, kSetFreeSlotA);
	Actor_Set_At_Waypoint(kActorSadik, 33, 0);
	Actor_Set_Goal_Number(kActorSadik, kGoalSadikDefault);
}

bool AIScriptSadik::Update() {
	if ( Global_Variable_Query(kVariableChapter) == 2
	 &&  Player_Query_Current_Scene() == kSceneBB09
	 && !Game_Flag_Query(kFlagBB09SadikRun)
	) {
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikRunFromBB09);
		Actor_Set_Targetable(kActorSadik, true);
		Game_Flag_Set(kFlagBB09SadikRun);
		Game_Flag_Set(kFlagUnused406);
		return true;
	}

	if (_nextSoundId != -1) { // changed from original (0) to be more clear that this is an invalid sfx id
		Sound_Play(_nextSoundId, 100, 0, 0, 50);
		_nextSoundId = -1;   // changed from original (0) to be more clear that this is an invalid sfx id
	}

	if (Global_Variable_Query(kVariableChapter) == 3
	 && Actor_Query_Goal_Number(kActorSadik) < 200
	) {
		Actor_Set_Goal_Number(kActorSadik, 200);
	}

	if (Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorSadik) < 400
	) {
		Actor_Set_Goal_Number(kActorSadik, 400);
	}

	if (Actor_Query_Goal_Number(kActorSadik) == 411) {
		if (Game_Flag_Query(kFlagNotUsed657)) {
			Actor_Set_Goal_Number(kActorSadik, 412);
		}
	}
	return false;
}

void AIScriptSadik::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask0) {
		AI_Countdown_Timer_Reset(kActorSadik, kActorTimerAIScriptCustomTask0);

		// goals 303, 304 and 305 are never set,  cut out part of game?
		switch (Actor_Query_Goal_Number(kActorSadik)) {
		case kGoalSadikUG18Decide:
			Actor_Set_Goal_Number(kActorSadik, 305);
			break;

		case 303:
			Actor_Set_Goal_Number(kActorSadik, 305);
			break;

		case kGoalSadikUG18PrepareShootMcCoy:
			Actor_Set_Goal_Number(kActorSadik, kGoalSadikUG18ShootMcCoy);
			break;
		}
	}
}

void AIScriptSadik::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorSadik)) {
	case kGoalSadikRunFromBB09:
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikBB11Wait);
		break;

	case kGoalSadikBB11CatchMcCoy:
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikBB11KnockOutMcCoy);
		break;

	case kGoalSadikBB11KnockOutMcCoy:
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikBB11KickMcCoy);
		break;

	case kGoalSadikUG18Move:
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikUG18Decide);
		break;

	default:
		return; //false;
	}

	return; //true;
}

void AIScriptSadik::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptSadik::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikGone) {
		Actor_Face_Actor(kActorMcCoy, kActorSadik, true);
		Actor_Says(kActorMcCoy, 8580, 16);
	}
}

void AIScriptSadik::EnteredSet(int setId) {
	// return false;
}

void AIScriptSadik::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptSadik::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptSadik::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptSadik::ShotAtAndMissed() {
	if (Actor_Query_Goal_Number(kActorSadik) == 414
	 || Actor_Query_Goal_Number(kActorSadik) == kGoalSadikKP06NeedsReactorCoreFromMcCoy
	) {
		Game_Flag_Set(kFlagMcCoyAttackedReplicants);
		if (Actor_Query_Which_Set_In(kActorSadik) != kSetKP07) {
			Actor_Set_Goal_Number(kActorSadik, 418);
			Scene_Exits_Disable();
		}
	}
}

bool AIScriptSadik::ShotAtAndHit() {
#if BLADERUNNER_ORIGINAL_BUGS // Sadik killed in BB09 dead end bug fix
#else
	if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikRunFromBB09) {
		// Like Izo's ShotAtAndHit() and the code below for kGoalSadikUG18Move
		// this will keep resetting Sadik's health to avoid killing him
		// Still, this is lowering Sadik's health from 80 (when Replicant) and 50 (when Human)
		if (Game_Flag_Query(kFlagSadikIsReplicant)) {
			Actor_Set_Health(kActorSadik, 60, 60);
		} else {
			Actor_Set_Health(kActorSadik, 40, 40);
		}
		return true;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
	if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikUG18Move) {
		// this lowers Sadik's original health but makes it impossible to kill him here (UG18)
		if (Game_Flag_Query(kFlagSadikIsReplicant)) {
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Health(kActorSadik, 60, 60);
#else
			// Sadik killed in BB09 dead end bug fix
			if (Actor_Query_Current_HP(kActorSadik) == 60) { // shot also at Bradbury, so lower his health further
				Actor_Set_Health(kActorSadik, 50, 50);
			} else {
				Actor_Set_Health(kActorSadik, 60, 60);
			}
#endif
		} else {
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Health(kActorSadik, 40, 40);
#else
			// Sadik killed in BB09 dead end bug fix
			if (Actor_Query_Current_HP(kActorSadik) == 40) { // shot also at Bradbury, so lower his health further
				Actor_Set_Health(kActorSadik, 30, 30);
			} else {
				Actor_Set_Health(kActorSadik, 40, 40);
			}
#endif
		}
		return true;
	}

	if (Actor_Query_Goal_Number(kActorSadik) == 414
	 || Actor_Query_Goal_Number(kActorSadik) == kGoalSadikKP06NeedsReactorCoreFromMcCoy
	) {
		Game_Flag_Set(kFlagMcCoyAttackedReplicants);
		if (Actor_Query_Which_Set_In(kActorSadik) != kSetKP07) {
			Actor_Set_Goal_Number(kActorSadik, 418);
			Scene_Exits_Disable();
		}
	}
	return false;
}

void AIScriptSadik::Retired(int byActorId) {
	if ((Actor_Query_Goal_Number(kActorSadik) == 418
	  || Actor_Query_Goal_Number(kActorSadik) == 450
	 )
	 && Actor_Query_Which_Set_In(kActorSadik) != kSetKP07
	) {
		Scene_Exits_Enable();
	}

	if (Actor_Query_In_Set(kActorSadik, kSetKP07)) {
		Global_Variable_Decrement(kVariableReplicantsSurvivorsAtMoonbus, 1); // can't Sadik still be human (Rep-sympathiser here? A bug?
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikGone);

		if (Global_Variable_Query(kVariableReplicantsSurvivorsAtMoonbus) == 0) {
			Player_Loses_Control();
			Delay(2000);
			Player_Set_Combat_Mode(false);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -41.58f, 72.0f, 0, true, false, false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagKP07toKP06);
			Game_Flag_Reset(kFlagMcCoyIsHelpingReplicants);
			Set_Enter(kSetKP05_KP06, kSceneKP06);
			return; //true;
		}
	}

	Actor_Set_Goal_Number(kActorSadik, kGoalSadikGone);

	return; //false;
}

int AIScriptSadik::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptSadik::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalSadikDefault:
		AI_Movement_Track_Flush(kActorSadik);
		AI_Movement_Track_Append(kActorSadik, 33, 0);
		AI_Movement_Track_Repeat(kActorSadik);
		return true;

	case kGoalSadikRunFromBB09:
		AI_Movement_Track_Flush(kActorSadik);
		AI_Movement_Track_Append_Run(kActorSadik, 131, 0);
		AI_Movement_Track_Append_Run(kActorSadik, 132, 0);
		AI_Movement_Track_Append_Run(kActorSadik, 133, 0);
		AI_Movement_Track_Repeat(kActorSadik);
		return true;

	case kGoalSadikBB11Wait:
		AI_Movement_Track_Flush(kActorSadik);
		AI_Movement_Track_Append(kActorSadik, 313, 0);
		AI_Movement_Track_Repeat(kActorSadik);
		Game_Flag_Set(kFlagBB11SadikFight);
		return true;

	case kGoalSadikBB11ThrowMcCoy:
		Actor_Set_Immunity_To_Obstacles(kActorSadik, true);
		Actor_Face_Heading(kActorSadik, kActorMcCoy, kActorMcCoy);
		_animationState = 32;
		_animationFrame = -1;
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		return true;

	case kGoalSadikBB11CatchMcCoy:
		Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyBB11GetUp);
		AI_Movement_Track_Flush(kActorSadik);
		AI_Movement_Track_Append(kActorSadik, 314, 0);
		AI_Movement_Track_Append_Run(kActorSadik, 317, 0);
		AI_Movement_Track_Repeat(kActorSadik);
		return true;

	case kGoalSadikBB11KnockOutMcCoy:
		Actor_Change_Animation_Mode(kActorSadik, 62);
		return true;

	case kGoalSadikBB11KickMcCoy:
		Actor_Face_Heading(kActorSadik, 100, false);
		Actor_Change_Animation_Mode(kActorSadik, 63);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisBB11WalkToMcCoy);
		Actor_Set_Immunity_To_Obstacles(kActorSadik, false);
		return true;

	case kGoalSadikBB11TalkWithClovis:
		_nextSoundId = -1; // changed from original (0) to be more clear that this is an invalid sfx id
		return false;

	case 200:
		Actor_Put_In_Set(kActorSadik, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorSadik, 33, 0);
		Actor_Set_Goal_Number(kActorMcCoy, 199);
		return true;

	case kGoalSadikUG18Wait:
		Actor_Put_In_Set(kActorSadik, kSetUG18);
		Actor_Set_At_XYZ(kActorSadik, 111.89f, 0.0f, 408.42f, 0);
		Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatIdle);
		return true;

	case kGoalSadikUG18Move:
		Actor_Set_Targetable(kActorSadik, true);
		World_Waypoint_Set(436, kSetUG18, -356.11f, 0.0f, 652.42f);
		AI_Movement_Track_Flush(kActorSadik);
		AI_Movement_Track_Append_Run(kActorSadik, 436, 0);
		AI_Movement_Track_Repeat(kActorSadik);
		return true;

	case kGoalSadikUG18Decide:
		// This is called first and then the scene script SceneScriptUG18::ActorChangedGoal
		Actor_Set_Targetable(kActorSadik, false);
		return true;

	// goals 303, 304 and 305 are never set,  cut out part of game?
	case 303:
		AI_Countdown_Timer_Reset(kActorSadik, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorSadik, kActorTimerAIScriptCustomTask0, 5);
		return true;

	case 304:
		Actor_Set_Targetable(kActorSadik, false);
		AI_Countdown_Timer_Reset(kActorSadik, kActorTimerAIScriptCustomTask0);
		return true;

	case 305:
		// fall through
	case kGoalSadikUG18WillShootMcCoy:
		// fall through
	case kGoalSadikUG18Leave:
		return true;

	case kGoalSadikUG18PrepareShootMcCoy:
		Sound_Play(kSfxLGCAL1, 100, 0, 0, 50);
		AI_Countdown_Timer_Start(kActorSadik, kActorTimerAIScriptCustomTask0, 2);
		return true;

	case kGoalSadikUG18ShootMcCoy:
		if (Player_Query_Current_Scene() == kSceneUG18) {
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Force_Stop_Walking(kActorMcCoy);
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatAttack);
			Sound_Play(kSfxLGCAL1, 100, 0, 0, 50);
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
			Actor_Retired_Here(kActorMcCoy, 6, 6, true, -1);
#else
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatAttack);
			Sound_Play(kSfxLGCAL1, 100, 0, 0, 50);
			Player_Loses_Control();
			Actor_Force_Stop_Walking(kActorMcCoy);
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
			Actor_Retired_Here(kActorMcCoy, 6, 6, true, kActorSadik);
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		return true;

	case 309:
		AI_Countdown_Timer_Reset(kActorSadik, kActorTimerAIScriptCustomTask0);
		return true;

	case 400:
		Actor_Set_Goal_Number(kActorSadik, 410);
		return true;

	case 410:
		if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants) == 1) {
			Actor_Set_Goal_Number(kActorSadik, 414);
		} else {
			Actor_Set_Goal_Number(kActorSadik, 411);
		}
		return true;

	case 411:
		Actor_Put_In_Set(kActorSadik, kSetKP05_KP06);
		Actor_Set_At_XYZ(kActorSadik, -1134.0f, 0.0f, 73.45f, 398);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP07Wait);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05Wait);
		return true;

	case 412:
		Actor_Says(kActorSadik, 60, 3);
		Actor_Says(kActorMcCoy, 2240, 3);
		Actor_Says(kActorSadik, 70, 3);
		Actor_Says(kActorSadik, 80, 3);
		Actor_Says(kActorMcCoy, 2245, 3);
		Actor_Says(kActorSadik, 90, 3);
		Actor_Says(kActorSadik, 100, 3);
		Actor_Says(kActorMcCoy, 2250, 3);
		Actor_Set_Goal_Number(kActorSadik, 413);
		return true;

	case 413:
		Loop_Actor_Walk_To_XYZ(kActorSadik, -1062.0f, 0.0f, 219.0f, 0, false, true, false);
		Actor_Set_Targetable(kActorSadik, true);
		Non_Player_Actor_Combat_Mode_On(kActorSadik, kActorCombatStateIdle, true, kActorMcCoy, 9, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, -1, -1, 15, 300, false);
		Actor_Set_Goal_Number(kActorSadik, 450);
		return true;

	case 414:
		Actor_Put_In_Set(kActorSadik, kSetKP05_KP06);
		Actor_Set_At_XYZ(kActorSadik, -961.0f, 0.0f, -778.0f, 150);
		Actor_Set_Targetable(kActorSadik, true);
		return true;

	case 415:
		Actor_Says(kActorSadik, 110, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 2290, kAnimationModeTalk);
		Actor_Says(kActorSadik, 310, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 2300, kAnimationModeTalk);
		if (Game_Flag_Query(kFlagSadikIsReplicant)) {
			Actor_Says(kActorSadik, 180, kAnimationModeTalk);
			Actor_Says(kActorSadik, 190, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 2310, kAnimationModeTalk);
			Actor_Says(kActorSadik, 200, kAnimationModeTalk);
		} else {
			Actor_Says(kActorSadik, 140, kAnimationModeTalk);
			Actor_Says(kActorSadik, 150, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 2305, kAnimationModeTalk);
			Actor_Says(kActorSadik, 160, kAnimationModeTalk);
			Actor_Says(kActorSadik, 170, kAnimationModeTalk);
		}
		Actor_Says(kActorMcCoy, 2315, kAnimationModeTalk);
		Actor_Says(kActorSadik, 210, kAnimationModeTalk);
		Actor_Says(kActorSadik, 220, kAnimationModeTalk);
		Actor_Says(kActorSadik, 230, kAnimationModeTalk);
		Actor_Says(kActorSadik, 240, kAnimationModeTalk);
		Actor_Says(kActorSadik, 250, kAnimationModeTalk);
		Actor_Says(kActorSadik, 260, kAnimationModeTalk);
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikKP06NeedsReactorCoreFromMcCoy);
		return true;

	case kGoalSadikKP06NeedsReactorCoreFromMcCoy:
		Loop_Actor_Walk_To_XYZ(kActorSadik, -961.0f, 0.0f, -778.0f, 0, false, false, false);
		Actor_Face_Heading(kActorSadik, 150, false);
		return true;

	case 417:
		Actor_Face_Actor(kActorSadik, kActorMcCoy, true);
		Actor_Says(kActorSadik, 320, kAnimationModeTalk);
		Loop_Actor_Walk_To_XYZ(kActorSadik, -857.0f, 0.0f, -703.0f, 0, false, true, false);
		Actor_Says(kActorMcCoy, 2330, kAnimationModeTalk);
		Actor_Says(kActorSadik, 330, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 2335, kAnimationModeTalk);
		Actor_Says(kActorSadik, 340, kAnimationModeTalk);
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikKP06NeedsReactorCoreFromMcCoy);
		return true;

	case 418:
		Game_Flag_Reset(kFlagMcCoyIsHelpingReplicants);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP07LayDown);
		Non_Player_Actor_Combat_Mode_On(kActorSadik, kActorCombatStateIdle, true, kActorMcCoy, 9, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, -1, -1, 15, 300, false);
		return true;

	case 419:
		Actor_Put_In_Set(kActorSadik, kSetKP07);
		Actor_Set_At_XYZ(kActorSadik, -12.0f, -41.58f, 72.0f, 0);
		return true;

	case 420:
		// fall through
	case 450:
		return true;
	}
	return false;
}

bool AIScriptSadik::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationSadikShiftsShoulders;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikShiftsShoulders)) {
				*animation = kModelAnimationSadikIdle;
				_animationFrame = 0;
				_varChooseIdleAnimation = 0;
			}
		} else if (_varChooseIdleAnimation == 0) {
			*animation = kModelAnimationSadikIdle;
			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;
				if (!Random_Query(0, 6)) {
					_var4 = -_var4;
				}
			} else {
				_animationFrame += _var4;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikIdle)) {
					_animationFrame = 0;
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikIdle) - 1;
				}
				if (!Random_Query(0, 4)) {
					_varNumOfTimesToHoldCurrentFrame = 1;
				}
				if (_animationFrame == 0 || _animationFrame == 8) {
					_varNumOfTimesToHoldCurrentFrame = Random_Query(2, 8);
				}
				if (!Random_Query(0, 2)) {
					if (_animationFrame == 0) {
						_varChooseIdleAnimation = 1;
						_varNumOfTimesToHoldCurrentFrame = 0;
						*animation = kModelAnimationSadikShiftsShoulders;
					}
				}
			}
		}
		break;

	case 1:
		// fall through
	case 2:
		// fall through
	case 3:
		// fall through
	case 4:
		// fall through
	case 5:
		// fall through
	case 6:
		switch (_animationState) {
		case 1:
			*animation = kModelAnimationSadikWalking;
			break;

		case 2:
			*animation = kModelAnimationSadikRunning;
			break;

		case 3:
			*animation = kModelAnimationSadikCombatWalking;
			break;

		case 4:
			*animation = kModelAnimationSadikCombatRunning;
			break;

		case 6:
			*animation = kModelAnimationSadikClimbLadderUp;
			break;

		case 5:
			*animation = kModelAnimationSadikClimbLadderDown;
			break;
		}
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 7:
		*animation = kModelAnimationSadikCombatIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikCombatIdle)) {
			_animationFrame = 0;
		}
		break;

	case 8:
		*animation = kModelAnimationSadikCombatTurnRight;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikCombatTurnRight)) {
			_animationFrame = 0;
			_animationState = 7;
			*animation = kModelAnimationSadikCombatIdle;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatIdle);
		}
		break;

	case 9:
		*animation = kModelAnimationSadikCombatTurnLeft;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikCombatTurnLeft)) {
			_animationFrame = 0;
			_animationState = 7;
			*animation = kModelAnimationSadikCombatIdle;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatIdle);
		}
		break;

	case 10:
		*animation = kModelAnimationSadikGotHitFront;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikGotHitFront)) {
			*animation = kModelAnimationSadikIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeIdle);
		}
		break;

	case 11:
		*animation = kModelAnimationSadikGotHitMore;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikGotHitMore)) {
			*animation = kModelAnimationSadikIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeIdle);
		}
		break;

	case 12:
		*animation = kModelAnimationSadikCombatGotHitFront;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikCombatGotHitFront)) {
			_animationFrame = 0;
			_animationState = 7;
			*animation = kModelAnimationSadikCombatIdle;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatIdle);
		}
		break;

	case 13:
		*animation = kModelAnimationSadikCombatGotHitMore;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikCombatGotHitMore)) {
			_animationFrame = 0;
			_animationState = 7;
			*animation = kModelAnimationSadikCombatIdle;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatIdle);
		}
		break;

	case 14:
		*animation = kModelAnimationSadikShotDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikShotDead) - 1) {
			++_animationFrame;
		}
		break;

	case 15:
		// A bug? This is identical to case 14. Maybe make case 14 fall through?
		*animation = kModelAnimationSadikShotDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikShotDead) - 1) {
			++_animationFrame;
		}
		break;

	case 16:
		*animation = kModelAnimationSadikCombatUnholsterGun;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikCombatUnholsterGun)) {
			_animationFrame = 0;
			_animationState = 7;
			*animation = kModelAnimationSadikCombatIdle;
		}
		break;

	case 17:
		*animation = kModelAnimationSadikCombatHolsterGun;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikCombatHolsterGun)) {
			*animation = kModelAnimationSadikIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 18:
		*animation = kModelAnimationSadikCombatFireGunAndReturnToPoseIdle;
		++_animationFrame;
		if (_animationFrame == 5) {
			int snd;
			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(kActorSadik, snd, 75, 0, 99);
		}
		if (_animationFrame == 7) {
			Actor_Combat_AI_Hit_Attempt(kActorSadik);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikCombatFireGunAndReturnToPoseIdle)) {
			_animationFrame = 0;
			_animationState = 7;
			*animation = kModelAnimationSadikCombatIdle;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatIdle);
		}
		break;

	case 19:
		*animation = kModelAnimationSadikCalmTalk;
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationSadikIdle;
			_animationState = 0;
			_resumeIdleAfterFramesetCompletesFlag = false;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikCalmTalk)) {
				_animationFrame = 0;
			}
		}
		break;

	case 20:
		// fall through
	case 21:
		// fall through
	case 22:
		// fall through
	case 23:
		// fall through
	case 24:
		// fall through
	case 25:
		// fall through
	case 26:
		switch (_animationState) {
		case 20:
			*animation = kModelAnimationSadikMoreCalmTalk;
			break;

		case 21:
			*animation = kModelAnimationSadikSuggestTalk;
			break;

		case 22:
			*animation = kModelAnimationSadikUrgeTalk;
			break;

		case 23:
			*animation = kModelAnimationSadikAccuseTalk;
			break;

		case 24:
			*animation = kModelAnimationSadikProtestTalk;
			break;

		case 25:
			*animation = kModelAnimationSadikMockTalk;
			break;

		default:
			*animation = kModelAnimationSadikThisAndThatTalk;
			break;
		}
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 19;
			*animation = kModelAnimationSadikCalmTalk;
		}
		break;

	case 27:
		*animation = kModelAnimationSadikGesturePointOrGive;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikGesturePointOrGive)) {
			*animation = kModelAnimationSadikIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeIdle);
		}
		break;

	case 28:
		*animation = kModelAnimationSadikJumpAcross;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikJumpAcross)) {
			*animation = kModelAnimationSadikIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 29:
		*animation = kModelAnimationSadikHangingDropsDown;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikHangingDropsDown)) {
			*animation = kModelAnimationSadikIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 30:
		*animation = kModelAnimationSadikKicksSomeoneWhoIsDown;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikKicksSomeoneWhoIsDown)) {
			*animation = kModelAnimationSadikIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeIdle);
		}
		break;

	case 31:
		*animation = kModelAnimationSadikHoldsSomeoneAndPunches;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationSadikHoldsSomeoneAndPunches)) {
			*animation = kModelAnimationSadikIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeIdle);
		}
		break;

	case 32:
		*animation = kModelAnimationSadikPicksUpAndThrowsMcCoy;
		++_animationFrame;
		if (_animationFrame == 23) {
			_nextSoundId = kSfxMTLDOOR2;
		}
		if (_animationFrame >= 25) {
			// TODO last frames 27-30 are empty
			//      but maybe we could still accept frames 25 and 26!
			_animationFrame = 0;
			_animationState = 0;
			*animation = kModelAnimationSadikIdle;
			Actor_Set_Goal_Number(kActorSadik, kGoalSadikBB11CatchMcCoy);
		}
		break;

	case 33:
		*animation = kModelAnimationSadikHoldsSomeoneAndPunches;
		++_animationFrame;
		if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikBB11KnockOutMcCoy) {
			if (_animationFrame == 4) {
				_nextSoundId = kSfxPUNCH1;
			}
			if (_animationFrame == 6) {
				Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeHit);
			}
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = kModelAnimationSadikIdle;
			_animationFrame = 0;
			_animationState = 0;

			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeIdle);
			if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikBB11KnockOutMcCoy) {
				Actor_Change_Animation_Mode(kActorSadik, 63);
			}
		}
		break;

	case 34:
		*animation = kModelAnimationSadikKicksSomeoneWhoIsDown;
		++_animationFrame;
		if (_animationFrame == 4) {
			if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikBB11KnockOutMcCoy) {
				Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
				_nextSoundId = kSfxKICK1;
			} else {
				Actor_Change_Animation_Mode(kActorMcCoy, 68);
				_nextSoundId = kSfxKICK2;
			}
		}

		if (_animationFrame >= 15) {
			*animation = kModelAnimationSadikIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeIdle);
			if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikBB11KnockOutMcCoy) {
				AI_Movement_Track_Flush(kActorSadik);
				AI_Movement_Track_Append(kActorSadik, 318, 0);
				AI_Movement_Track_Repeat(kActorSadik);
			} else {
				if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikBB11KickMcCoy) {
					Actor_Change_Animation_Mode(kActorSadik, 63);
				}
			}
		}
		break;

	default:
		// Dummy placeholder, kModelAnimationZubenIdle (406) is a Zuben animation
		*animation = kModelAnimationZubenIdle;
		_animationFrame = 0;
		break;
	}

	*frame = _animationFrame;

	return true;
}

bool AIScriptSadik::ChangeAnimationMode(int mode) {
	Actor_Set_Frame_Rate_FPS(kActorSadik, -2);

	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 19:
			// fall through
		case 20:
			// fall through
		case 21:
			// fall through
		case 22:
			// fall through
		case 23:
			// fall through
		case 24:
			// fall through
		case 25:
			// fall through
		case 26:
			_resumeIdleAfterFramesetCompletesFlag = true;
			break;

		case 30:
			// fall through
		case 31:
			return 1;

		default:
			_animationState = 0;
			_animationFrame = 0;
			_varNumOfTimesToHoldCurrentFrame = 0;
			break;
		}
		break;

	case kAnimationModeWalk:
		_animationFrame = 0;
		_animationState = 1;
		break;

	case kAnimationModeRun:
		_animationFrame = 0;
		_animationState = 2;
		break;

	case kAnimationModeTalk:
		_animationState = 20;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatIdle:
		switch (_animationState) {
		case 0:
			_animationFrame = 0;
			_animationState = 16;
			break;

		case 3:
			// fall through
		case 4:
			_animationState = 7;
			_animationFrame = 0;
			break;

		case 7:
			// fall through
		case 16:
			// fall through
		case 18:
			return true;

		case 17:
			_animationFrame = 0;
			_animationState = 7;
			break;
		default:
			_animationFrame = 0;
			_animationState = 16;
			break;
		}
		break;

	case 5:
		// fall through
	case 9:
		// fall through
	case 10:
		// fall through
	case 11:
		// fall through
	case 19:
		// fall through
	case 20:
		return true;

	case kAnimationModeCombatAttack:
		_animationFrame = 0;
		_animationState = 18;
		break;

	case kAnimationModeCombatWalk:
		_animationFrame = 0;
		_animationState = 3;
		break;

	case kAnimationModeCombatRun:
		_animationFrame = 0;
		_animationState = 4;
		break;

	case 12:
		_animationState = 20;
		_animationFrame = 0;
		break;

	case 13:
		_animationState = 21;
		_animationFrame = 0;
		break;

	case 14:
		_animationState = 22;
		_animationFrame = 0;
		break;

	case 15:
		_animationState = 23;
		_animationFrame = 0;
		break;

	case 16:
		_animationState = 24;
		_animationFrame = 0;
		break;

	case 17:
		_animationState = 25;
		_animationFrame = 0;
		break;

	case 18:
		_animationState = 26;
		_animationFrame = 0;
		break;

	case kAnimationModeHit:
		switch (_animationState) {
		case 7:
			// fall through
		case 8:
			// fall through
		case 9:
			// fall through
		case 16:
			// fall through
		case 17:
			// fall through
		case 18:
			if (Random_Query(0, 1)) {
				_animationState = 13;
			} else {
				_animationState = 12;
			}
			break;

		case 10:
			// fall through
		case 11:
			// fall through
		case 12:
			// fall through
		case 13:
			// fall through
		case 14:
			// fall through
		case 15:
			if (Random_Query(0, 1)) {
				_animationState = 11;
			} else {
				_animationState = 10;
			}
			break;
		}
		_animationFrame = 0;
		break;

	case kAnimationModeCombatHit:
		if (Random_Query(0, 1)) {
			_animationState = 12;
		} else {
			_animationState = 13;
		}
		_animationFrame = 0;
		break;

	case 23:
		_animationState = 27;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 14;
		_animationFrame = 0;
		break;

	case 62:
		if (Actor_Query_Goal_Number(kActorSadik) != kGoalSadikBB11KnockOutMcCoy
		 && Actor_Query_Goal_Number(kActorSadik) != kGoalSadikBB11KickMcCoy
		) {
			_animationState = 31;
			_animationFrame = 0;
		} else {
			_animationState = 33;
			_animationFrame = 0;
		}
		break;

	case 63:
		if (Actor_Query_Goal_Number(kActorSadik) != kGoalSadikBB11KnockOutMcCoy
		 && Actor_Query_Goal_Number(kActorSadik) != kGoalSadikBB11KickMcCoy
		) {
			_animationState = 30;
			_animationFrame = 2;
		} else {
			_animationState = 34;
			_animationFrame = 2;
		}
		break;
	}
	return true;
}

void AIScriptSadik::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptSadik::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptSadik::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptSadik::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
