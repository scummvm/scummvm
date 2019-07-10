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

#include "bladerunner/script/scene_script.h"

namespace BladeRunner {

enum kMA01Loops {
	kMA01LoopInshotRoof   = 0,
	kMA01LoopMainLoop = 1,
	kMA01LoopOutDoorAnim  = 3,
	kMA01LoopOutshotRoof  = 4
};

enum kMA01Exits {
	kMA01ExitMA06    = 0,
	kMA01ExitSpinner = 1
};

void SceneScriptMA01::InitializeScene() {
	Setup_Scene_Information(381.0f, 0.0f, 54.0f, 992);
	if (Game_Flag_Query(kFlagSpinnerAtMA01)) {
		Setup_Scene_Information( 381.0f, 0.0f,   54.0f, 992);
	}
	if (Game_Flag_Query(kFlagMA06toMA01)) {
		Setup_Scene_Information(1446.0f, 0.0f, -725.0f, 660);
	}

	Scene_Exit_Add_2D_Exit(kMA01ExitMA06, 328, 132, 426, 190, 0);
	if (Game_Flag_Query(kFlagSpinnerAtMA01)) {
		Scene_Exit_Add_2D_Exit(kMA01ExitSpinner, 234, 240, 398, 328, 2);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxROOFRAN1, 90,    0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxROOFAIR1, 40, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxROOFRMB1, 40,  100, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  10, 100, 25,  50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  10, 100, 25,  50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 10,  70, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 10,  70, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 10,  70, 50, 100, 0, 0, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagMA06toMA01)) {
		Scene_Loop_Set_Default(kMA01LoopMainLoop);
		Game_Flag_Reset(kFlagMA06toMA01);
	} else {
		Actor_Set_Invisible(kActorMcCoy, true);
		Game_Flag_Set(kFlagArrivedFromSpinner2);
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kMA01LoopInshotRoof, false);
		Scene_Loop_Set_Default(kMA01LoopMainLoop);
	}

	if (Game_Flag_Query(kFlagMA01GaffApproachMcCoy)) {
		Actor_Set_Goal_Number(kActorGaff, kGoalGaffMA01ApproachMcCoy);
		Game_Flag_Reset(kFlagMA01GaffApproachMcCoy);
	}
}

void SceneScriptMA01::SceneLoaded() {
	Obstacle_Object("WRENCH", true);
	Unobstacle_Object("OBSTICLEBOX01", true);
	Clickable_Object("WRENCH");
	Unclickable_Object("Y2 PADRIM 01");
	Unclickable_Object("Y2 PADRIM 02");
	Unclickable_Object("NGON01");
}

bool SceneScriptMA01::MouseClick(int x, int y) {
	return Region_Check(286, 326, 348, 384);
}

bool SceneScriptMA01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptMA01::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptMA01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptMA01::ClickedOnExit(int exitId) {
	if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenMA01AttackMcCoy) {
		return true;
	}

	if (exitId == kMA01ExitMA06) {
		if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenFled) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1446.0f, 0.0f, -725.0f, 72, true, false, false)) {
				Actor_Set_Goal_Number(kActorZuben, kGoalZubenMA01AttackMcCoy);
				Scene_Exits_Disable();
			}
		} else if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1446.0f, 0.0f, -725.0f, 12, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagMA01toMA06);
			Set_Enter(kSetMA06, kSceneMA06);
		}
		return true;
	}

	if (exitId == kMA01ExitSpinner) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 381.0f, 0.0f, 54.0f, 0, true, false, false)) {
			Player_Loses_Control();
			Actor_Face_Heading(kActorMcCoy, 736, false);
			Game_Flag_Reset(kFlagMcCoyInChinaTown);
			Game_Flag_Reset(kFlagMcCoyInRunciters);
			Game_Flag_Reset(kFlagMcCoyInMcCoyApartment);
			Game_Flag_Reset(kFlagMcCoyInAnimoidRow);
			Game_Flag_Reset(kFlagMcCoyInTyrellBuilding);
			Game_Flag_Reset(kFlagMcCoyInDNARow);
			Game_Flag_Reset(kFlagMcCoyInBradburyBuilding);
			Game_Flag_Reset(kFlagMcCoyInPoliceStation);
			int spinnerDest = Spinner_Interface_Choose_Dest(kMA01LoopOutDoorAnim, false);
			switch (spinnerDest) {
			case kSpinnerDestinationPoliceStation:
				Game_Flag_Set(kFlagMcCoyInPoliceStation);
				Game_Flag_Reset(kFlagSpinnerAtMA01);
				Game_Flag_Set(kFlagSpinnerAtPS01);
				Set_Enter(kSetPS01, kScenePS01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationRuncitersAnimals:
				Game_Flag_Set(kFlagMcCoyInRunciters);
				Game_Flag_Reset(kFlagSpinnerAtMA01);
				Game_Flag_Set(kFlagSpinnerAtRC01);
				Set_Enter(kSetRC01, kSceneRC01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationChinatown:
				Game_Flag_Set(kFlagMcCoyInChinaTown);
				Game_Flag_Reset(kFlagSpinnerAtMA01);
				Game_Flag_Set(kFlagSpinnerAtCT01);
				Set_Enter(kSetCT01_CT12, kSceneCT01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(kFlagMcCoyInTyrellBuilding);
				Game_Flag_Reset(kFlagSpinnerAtMA01);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationAnimoidRow:
				Game_Flag_Set(kFlagMcCoyInAnimoidRow);
				Game_Flag_Reset(kFlagSpinnerAtMA01);
				Game_Flag_Set(kFlagSpinnerAtAR01);
				Set_Enter(kSetAR01_AR02, kSceneAR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationDNARow:
				Game_Flag_Set(kFlagMcCoyInDNARow);
				Game_Flag_Reset(kFlagSpinnerAtMA01);
				Game_Flag_Set(kFlagSpinnerAtDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationBradburyBuilding:
				Game_Flag_Set(kFlagMcCoyInBradburyBuilding);
				Game_Flag_Reset(kFlagSpinnerAtMA01);
				Game_Flag_Set(kFlagSpinnerAtBB01);
				Set_Enter(kSetBB01, kSceneBB01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationNightclubRow:
				Game_Flag_Set(kFlagMcCoyInNightclubRow);
				Game_Flag_Reset(kFlagSpinnerAtMA01);
				Game_Flag_Set(kFlagSpinnerAtNR01);
				Set_Enter(kSetNR01, kSceneNR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationHysteriaHall:
				Game_Flag_Set(kFlagMcCoyInHysteriaHall);
				Game_Flag_Reset(kFlagSpinnerAtMA01);
				Game_Flag_Set(kFlagSpinnerAtHF01);
				Set_Enter(kSetHF01, kSceneHF01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			default:
				Actor_Set_Invisible(kActorMcCoy, false);
				Actor_Face_Heading(kActorMcCoy, 736, false);
				Game_Flag_Set(kFlagMcCoyInMcCoyApartment);
				break;
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptMA01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptMA01::SceneFrameAdvanced(int frame) {
	if (frame == 15) {
		Ambient_Sounds_Play_Sound(kSfxROOFLIT1,  70, -100, 100,  0);
	}

	if (frame == 61
	 || frame == 183
	) {
		Ambient_Sounds_Play_Sound(kSfxSPINOPN4, 100,   40,   0, 99);
	}

	if (frame == 107
	 || frame == 227
	) {
		Ambient_Sounds_Play_Sound(kSfxSPINCLS1, 100,   40,   0, 99);
	}

	if (frame == 1) {
		Ambient_Sounds_Play_Sound(kSfxCARDOWN3,  40,  -60,  20, 99);
	}

	if (frame == 241) {
		Ambient_Sounds_Play_Sound(kSfxCARUP3,    40,    0,   0, 99);
	}

	if (frame == 58) {
		Sound_Play(kSfxMTLHIT1, 17, 20, 20, 50);
	}

	if ((frame == 75
	  || frame == 196
	 )
	 && Game_Flag_Query(kFlagArrivedFromSpinner2)
	) {
		Actor_Face_Heading(kActorMcCoy, 736, false);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeSpinnerGetOut);
		Game_Flag_Reset(kFlagArrivedFromSpinner2);
	} else {
		if ( frame == 196
		 && !Game_Flag_Query(kFlagArrivedFromSpinner2)
		) {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeSpinnerGetIn);
			//return true;
			return;
		}
		if (frame == 240) {
			Player_Gains_Control();
		}
	}
	//return true;
}

void SceneScriptMA01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptMA01::PlayerWalkedIn() {
	if (_vm->_cutContent) {
		if (!Game_Flag_Query(kFlagCT01Visited) && !Actor_Clue_Query(kActorMcCoy, kClueDispatchHitAndRun)) {
			Actor_Clue_Acquire(kActorMcCoy, kClueDispatchHitAndRun, false, kActorDispatcher);
			ADQ_Add(kActorDispatcher, 80, kAnimationModeTalk);
			if (Game_Flag_Query(kFlagRC01PoliceDone)) {
				ADQ_Add(kActorOfficerLeary, 340, kAnimationModeTalk);  // sector 3 - go ahead
			} else {
				ADQ_Add(kActorOfficerGrayford, 360, kAnimationModeTalk);  // sector 3 - go ahead
			}
			ADQ_Add(kActorDispatcher, 90, kAnimationModeTalk);
			ADQ_Add(kActorDispatcher, 100, kAnimationModeTalk);
			ADQ_Add(kActorDispatcher, 110, kAnimationModeTalk);
			if (Game_Flag_Query(kFlagRC01PoliceDone)) {
				ADQ_Add(kActorOfficerLeary, 350, kAnimationModeTalk);  // sector 3 - responding code 3
			} else {
				ADQ_Add(kActorOfficerGrayford, 370, kAnimationModeTalk);  // sector 3 - responding code 3
			}
			ADQ_Add_Pause(1000);
			ADQ_Add(kActorDispatcher, 120, kAnimationModeTalk);
			ADQ_Add(kActorDispatcher, 130, kAnimationModeTalk);
			ADQ_Add(kActorDispatcher, 140, kAnimationModeTalk);
			ADQ_Add(kActorDispatcher, 150, kAnimationModeTalk);
		}
	}
}

void SceneScriptMA01::PlayerWalkedOut() {
	Actor_Set_Invisible(kActorMcCoy, false);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);

	if (_vm->_cutContent) {
		ADQ_Flush(); // for dispatcher queue cleanup
	}

	if (!Game_Flag_Query(kFlagMA01toMA06)) {
		if (Global_Variable_Query(kVariableChapter) == 1) {
			Outtake_Play(kOuttakeTowards2, true, -1);
			Outtake_Play(kOuttakeInside1,  true, -1);
//			// Commented out - Has no sound - TODO can we use external SFX for it?
//			if (_vm->_cutContent) {
//				Outtake_Play(kOuttakeFlyThrough,  true, -1);
//			}
			Outtake_Play(kOuttakeTowards1, true, -1);
		}
#if BLADERUNNER_ORIGINAL_BUGS
#else
		else {
			// Acts 2, 3 - should still use a spinner fly-through transition
			if (!Game_Flag_Query(kFlagMcCoyInTyrellBuilding)) {
				// don't play an extra outtake when going to Tyrell Building
				Outtake_Play(kOuttakeAway1,    true, -1); // available in Acts 2, 3
			}
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
}

void SceneScriptMA01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
