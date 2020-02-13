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

enum kCT01Loops {
	kCT01LoopInshotFromCT12WithSpinner = 0, //   0 -  14
	kCT01LoopInshot                    = 1, //  15 - 194
	kCT01LoopMainLoop                  = 2, // 195 - 255
	kCT01LoopDoorAnim                  = 4, // 256 - 315
	kCT01LoopOutshot                   = 5, // 316 - 435
	kCT01LoopInshotFromCT12NoSpinner   = 6, // 436 - 450
	kCT01LoopMainLoopNoSpinner         = 7  // 451 - 511
};

enum kCT01Exits {
	kCT01ExitCT02    = 0,
	kCT01ExitCT03    = 1,
	kCT01ExitCT12    = 2,
	kCT01ExitSpinner = 3
};

void SceneScriptCT01::InitializeScene() {
	Music_Play(kMusicKyoto, 28, 0, 2, -1, 1, 0);
	Game_Flag_Reset(kFlagArrivedFromSpinner1);
	if (Game_Flag_Query(kFlagCT02toCT01)) {
		Game_Flag_Reset(kFlagCT02toCT01);
		Setup_Scene_Information(-35.2f, -6.5f, 352.28f, 603);
	} else if (Game_Flag_Query(kFlagCT03toCT01)) {
		Game_Flag_Reset(kFlagCT03toCT01);
		Setup_Scene_Information(-311.0f, -6.5f, 710.0f, 878);
	} else if (Game_Flag_Query(kFlagCT12toCT01)) {
		Game_Flag_Reset(kFlagCT12toCT01);
		Setup_Scene_Information(-419.0f, -6.5f, 696.0f, 28);
		if (Global_Variable_Query(kVariableChapter) != 2
		 && Global_Variable_Query(kVariableChapter) != 3
		) {
			if (Game_Flag_Query(kFlagSpinnerAtCT01)) {
				Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kCT01LoopInshotFromCT12WithSpinner, false);
			} else {
				Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kCT01LoopInshotFromCT12NoSpinner, false);
			}
		}
	} else if (Game_Flag_Query(kFlagSpinnerAtCT01)) {
		if (_vm->_cutContent) {
			// 0. This scene is not available in chapters 4 and 5
			// 1. Add open/close spinner door animation and sound
			// 2. Keep walkers from messing about with the scene (popping up or overlapping with landing) until spinner has landed
			// Note: kFlagSpinnerAtCT01 reset (original) is not handled the same was as in NR01 but it still works
			// Note 2: Gordo sitting at the diner overlaps with the counter bar in front of him
			//         so the loop will be prevented from playing when he is there.
			if ( Global_Variable_Query(kVariableChapter) < 4
			    && Actor_Query_Which_Set_In(kActorGordo) != kSetCT01_CT12
			    && Random_Query(1, 2) == 1
			) {
				// enhancement: don't always play
				Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kCT01LoopInshot, false);
			}
			// Pause generic walkers outside special loop
			// so that they're always paused when McCoy enters (less chance to collide with him)
			// We use the previously unused kVariableGenericWalkerConfig
			// The flag kFlagGenericWalkerWaiting will not do, because it can be reset
			// if a walker is already moving (goal == 1).
			// There's also another flag called kFlagUnpauseGenWalkers
			// but the usage of that flag seems more obscure and dubious for this purpose
			// Furthermore, kFlagUnpauseGenWalkers seems to be a code remnant, since the
			// walkers tracks are never pause in that occasion (or any other)
			Actor_Set_Goal_Number(kActorGenwalkerA, kGoalGenwalkerDefault);
			Actor_Set_Goal_Number(kActorGenwalkerB, kGoalGenwalkerDefault);
			Actor_Set_Goal_Number(kActorGenwalkerC, kGoalGenwalkerDefault);
			Global_Variable_Set(kVariableGenericWalkerConfig, -1);
		}
		Setup_Scene_Information(-530.0f, -6.5f, 241.0f, 506);
		Game_Flag_Set(kFlagArrivedFromSpinner1);
	} else {
		Setup_Scene_Information(-397.0f, -6.5f, 471.0f, 250);
	}
	Scene_Exit_Add_2D_Exit(kCT01ExitCT02, 290, 256, 360, 304, 1);
	if (Actor_Clue_Query(kActorMcCoy, kClueZubenRunsAway)) {
		Scene_Exit_Add_2D_Exit(kCT01ExitCT03, 571, 233, 639, 367, 1);
	}
	if (Game_Flag_Query(kFlagCT05WarehouseOpen)) {
		Scene_Exit_Add_2D_Exit(kCT01ExitCT12, 506, 400, 639, 479, 2);
	}
	if (Game_Flag_Query(kFlagSpinnerAtCT01)) {
		Scene_Exit_Add_2D_Exit(kCT01ExitSpinner, 0, 286, 158, 350, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1, 50,    1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTAMBL1, 40, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTAMBR1, 40,  100, 1);
	Ambient_Sounds_Add_Sound(kSfxDISH1,   10, 30, 16, 20,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxDISH2,   10, 30, 16, 20,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxDISH3,   10, 30, 16, 20,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxDISH4,   10, 30, 16, 20,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  10, 40, 33, 50,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  10, 40, 33, 50,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 20, 40, 33, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 20, 40, 33, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 20, 40, 33, 50, -100, 100, -101, -101, 0, 0);

#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (!Actor_Query_In_Set(kActorHowieLee, kSetCT01_CT12)
	    && Global_Variable_Query(kVariableChapter) < 4) {
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 67, 0); // in kSetCT01_CT12
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (Game_Flag_Query(kFlagSpinnerAtCT01)) {
		Scene_Loop_Set_Default(kCT01LoopMainLoop);
	} else {
		Scene_Loop_Set_Default(kCT01LoopMainLoopNoSpinner);
	}
}

void SceneScriptCT01::SceneLoaded() {
	Obstacle_Object("HYDRANT02", true);
	Obstacle_Object("HOWWALLRT", true);
	Obstacle_Object("HOW-CHAIR1", true);
	Obstacle_Object("HOWWALLLFT", true);
	Obstacle_Object("HOWDOOR01", true);
	Unobstacle_Object("SPINNER BODY", true);
	Unobstacle_Object("HOWFLOOR", true);
	Unobstacle_Object("PAPER11", true);
	Unobstacle_Object("PAPER16", true);
	Unclickable_Object("HYDRANT02");
	Unclickable_Object("TURBINE");
	Unclickable_Object("SPINNER BODY");
	Unclickable_Object("OBJECT04");
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Unclickable_Object("OBJECT03");
	Unclickable_Object("OBJECT05");
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptCT01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptCT01::ClickedOn3DObject(const char *objectName, bool a2) {
//	if ("ASIANSITTINGANDEATI" == objectName) { //bug?
	if (Object_Query_Click("ASIANSITTINGANDEATI", objectName)) {
		Actor_Face_Object(kActorMcCoy, "ASIANSITTINGANDEATI", true);
		Actor_Says(kActorMcCoy, 365, 13);    // Excuse me, pal!
		Actor_Says(kActorHowieLee, 160, 13); // I take care of you soon, McCoy. Real busy tonight.
		return true;
	}
	return false;
}

bool SceneScriptCT01::ClickedOnActor(int actorId) {
	if (actorId == kActorHowieLee) {
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeStopMoving);
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -335.23f, -6.5f, 578.97f, 12, true, false, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorHowieLee, true);
			Actor_Face_Actor(kActorHowieLee, kActorMcCoy, true);
			if (!Game_Flag_Query(kFlagCT01McCoyTalkedToHowieLee)) {
				Actor_Says(kActorMcCoy, 260, 18);
				Actor_Says(kActorHowieLee, 0, 14);
				Game_Flag_Set(kFlagCT01McCoyTalkedToHowieLee);
				Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
				return true;
			}

			if (_vm->_cutContent) {
				// Howie begins with friendliness of 60
				if (!Game_Flag_Query(kFlagCT01TalkToHowieAfterZubenMissing)) {
					dialogueWithHowieLee();
				} else {
					if (Game_Flag_Query(kFlagCT01ZubenGone) && !Game_Flag_Query(kFlagCT01TalkToHowieAboutDeadZuben)) {
						Game_Flag_Set(kFlagCT01TalkToHowieAboutDeadZuben);
						Actor_Says(kActorMcCoy, 330, 17);
						Actor_Says(kActorHowieLee, 130, 13);
						Actor_Says(kActorHowieLee, 140, 14);
						if (_vm->_cutContent) {
							Actor_Says(kActorMcCoy, 315, 16);
						}
						Actor_Modify_Friendliness_To_Other(kActorHowieLee, kActorMcCoy, -10);
					} else if (Actor_Query_Friendliness_To_Other(kActorHowieLee, kActorMcCoy) < 50) {
						Actor_Says(kActorMcCoy, 310, 11);    // keeping out of trouble...?
						Actor_Says(kActorHowieLee, 190, 13); // I look like I got time for chit-er chat-er?
					} else if (Actor_Query_Friendliness_To_Other(kActorHowieLee, kActorMcCoy) <= 55) {
						Actor_Says(kActorMcCoy, 330, 13);
						Actor_Says(kActorHowieLee, 160, 15);  // real busy tonight
					} else { // friendly > 55
						Actor_Says(kActorMcCoy, 310, 11);
						Actor_Says(kActorHowieLee, 10, 16);
					}
				}
			} else {
				// Original: Howie begins with friendliness of 60, max can be 65, lowest is 52
				if (!Game_Flag_Query(kFlagCT01TalkToHowieAfterZubenMissing)
				 && Actor_Query_Friendliness_To_Other(kActorHowieLee, kActorMcCoy) >= 40
				) {
					dialogueWithHowieLee();
					Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
					return true;
				}
				// In the original Howie's friendliness to McCoy can never go below 52
				// and the flag kFlagCT01TalkToHowieAfterZubenMissing is never set
				// so the code below was un-triggered
				if (Game_Flag_Query(kFlagCT01ZubenGone)) {
					Actor_Says(kActorMcCoy, 330, 17);
					Actor_Says(kActorHowieLee, 130, 13);
					Actor_Says(kActorHowieLee, 140, 14);
				} else if (Actor_Query_Friendliness_To_Other(kActorHowieLee, kActorMcCoy) < 50) {
					Actor_Says(kActorMcCoy, 330, 13);
					Actor_Says(kActorHowieLee, 160, 15);
				} else { // friendly >= 50
					Actor_Says(kActorMcCoy, 310, 11);
					Actor_Says(kActorHowieLee, 10, 16);
				}
			}
			Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
			return true;
		}
	}

	if (actorId == kActorZuben) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -335.23f, -6.5f, 578.97f, 12, true, false, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorZuben, true);
			Actor_Says(kActorMcCoy, 355, 18);
			if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenDefault) {
				Actor_Says(kActorZuben, 10, 16);
				Actor_Face_Actor(kActorHowieLee, kActorMcCoy, true);
				Actor_Says(kActorHowieLee, 150, kAnimationModeTalk);
				Actor_Face_Actor(kActorMcCoy, kActorHowieLee, true);
				Actor_Says(kActorMcCoy, 360, 13);
				Actor_Modify_Friendliness_To_Other(kActorHowieLee, kActorMcCoy, -5);
				Actor_Modify_Friendliness_To_Other(kActorZuben, kActorMcCoy, -4);
			}
			return true;
		}
	}

	if (actorId == kActorGordo) {
		if (Actor_Query_Goal_Number(kActorGordo) == kGoalGordoDefault) {
			if (Loop_Actor_Walk_To_XYZ(kActorMcCoy, -338.1f, -6.5f, 419.65f, 6, true, false, false)) {
				return false;
			}
		}
		Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
		if (!Game_Flag_Query(kFlagCT01GordoTalk)) {
			Actor_Says(kActorMcCoy, 335, 18);
			Actor_Says(kActorGordo, 20, 30);
			Game_Flag_Set(kFlagCT01GordoTalk);
			Actor_Clue_Acquire(kActorGordo, kClueMcCoysDescription, true, kActorMcCoy);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Clue_Acquire(kActorMcCoy, kClueMcCoyIsABladeRunner, true, kActorMcCoy); // a bug?
#else
			Actor_Clue_Acquire(kActorGordo, kClueMcCoyIsABladeRunner, true, kActorMcCoy);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Modify_Friendliness_To_Other(kActorGordo, kActorMcCoy, -1);
		} else {
			if (Actor_Query_Goal_Number(kActorGordo) == kGoalGordoDefault) {
				Actor_Says(kActorMcCoy, 340, 13);
				Actor_Says(kActorMcCoy, 345, 11);
				Actor_Says(kActorGordo, 30, 30);
				Actor_Says(kActorMcCoy, 350, 13);
				Actor_Says(kActorGordo, 40, 30);
				Actor_Modify_Friendliness_To_Other(kActorGordo, kActorMcCoy, -5);
				Player_Loses_Control();
			} else {
				Actor_Says(kActorMcCoy, 365, 14);
			}
		}
		if (Actor_Query_Is_In_Current_Set(kActorZuben)) {
			Actor_Modify_Friendliness_To_Other(kActorZuben, kActorMcCoy, -2);
		}
		return true;
	}

	return false;
}

bool SceneScriptCT01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptCT01::ClickedOnExit(int exitId) {
	if (exitId == kCT01ExitCT02) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -327.5f, -6.5f, 352.28f, 0, true, false, false)) {
			Player_Loses_Control();
			Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 106, 0, false, false);
			Player_Gains_Control();
			Game_Flag_Reset(kFlagArrivedFromSpinner1);
			Set_Enter(kSetCT02, kSceneCT02);
		}
		return true;
	}

	if (exitId == kCT01ExitCT03) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -259.0f, -6.5f, 710.0f, 0, true, false, false)) {
			Game_Flag_Reset(kFlagArrivedFromSpinner1);
			Set_Enter(kSetCT03_CT04, kSceneCT03);
		}
		return true;
	}

	if (exitId == kCT01ExitCT12) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -419.0f, -6.5f, 696.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagCT01toCT12);
			Game_Flag_Reset(kFlagArrivedFromSpinner1);
			Set_Enter(kSetCT01_CT12, kSceneCT12);
		}
		return true;
	}

	if (exitId == kCT01ExitSpinner) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -314.0f, -6.5f, 326.0f, 0, true, false, false)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -330.0f, -6.5f, 221.0f, 0, false, true, false);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -530.0f, -6.5f, 241.0f, 0, false, true, false);
			Game_Flag_Reset(kFlagMcCoyInChinaTown);
			Game_Flag_Reset(kFlagMcCoyInRunciters);
			Game_Flag_Reset(kFlagMcCoyInMcCoyApartment);
			Game_Flag_Reset(kFlagMcCoyInPoliceStation);
			Game_Flag_Reset(kFlagMcCoyInAnimoidRow);
			Game_Flag_Reset(kFlagMcCoyInTyrellBuilding);
			Game_Flag_Reset(kFlagMcCoyInDNARow);
			Game_Flag_Reset(kFlagMcCoyInBradburyBuilding);
//if (_vm->_cutContent) {
//			// Restored spinner door opens/ closes, so we disable this for now
//			// NOTE: Reverted this cut content since this might be annoying
//                   as it slows down the pacing...
//			int spinnerDest = Spinner_Interface_Choose_Dest(kCT01LoopDoorAnim, false);
//} else {
			int spinnerDest = Spinner_Interface_Choose_Dest(-1, false);
//}

			switch (spinnerDest) {
			case kSpinnerDestinationPoliceStation:
				Game_Flag_Set(kFlagMcCoyInPoliceStation);
				Game_Flag_Reset(kFlagArrivedFromSpinner1);
				Game_Flag_Reset(kFlagSpinnerAtCT01);
				Game_Flag_Set(kFlagSpinnerAtPS01);
				Set_Enter(kSetPS01, kScenePS01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kCT01LoopOutshot, true);
				break;
			case kSpinnerDestinationMcCoysApartment:
				Game_Flag_Set(kFlagMcCoyInMcCoyApartment);
				Game_Flag_Reset(kFlagArrivedFromSpinner1);
				Game_Flag_Reset(kFlagSpinnerAtCT01);
				Game_Flag_Set(kFlagSpinnerAtMA01);
				Set_Enter(kSetMA01, kSceneMA01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kCT01LoopOutshot, true);
				break;
			case kSpinnerDestinationRuncitersAnimals:
				Game_Flag_Set(kFlagMcCoyInRunciters);
				Game_Flag_Reset(kFlagArrivedFromSpinner1);
				Game_Flag_Reset(kFlagSpinnerAtCT01);
				Game_Flag_Set(kFlagSpinnerAtRC01);
				Set_Enter(kSetRC01, kSceneRC01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kCT01LoopOutshot, true);
				break;
			case kSpinnerDestinationAnimoidRow:
				Game_Flag_Set(kFlagMcCoyInAnimoidRow);
				Game_Flag_Reset(kFlagArrivedFromSpinner1);
				Game_Flag_Reset(kFlagSpinnerAtCT01);
				Game_Flag_Set(kFlagSpinnerAtAR01);
				Set_Enter(kSetAR01_AR02, kSceneAR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kCT01LoopOutshot, true);
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(kFlagMcCoyInTyrellBuilding);
				Game_Flag_Reset(kFlagSpinnerAtCT01);
				Game_Flag_Reset(kFlagArrivedFromSpinner1);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kCT01LoopOutshot, true);
				break;
			case kSpinnerDestinationDNARow:
				Game_Flag_Set(kFlagMcCoyInDNARow);
				Game_Flag_Reset(kFlagArrivedFromSpinner1);
				Game_Flag_Reset(kFlagSpinnerAtCT01);
				Game_Flag_Set(kFlagSpinnerAtDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kCT01LoopOutshot, true);
				break;
			case kSpinnerDestinationBradburyBuilding:
				Game_Flag_Set(kFlagMcCoyInBradburyBuilding);
				Game_Flag_Reset(kFlagArrivedFromSpinner1);
				Game_Flag_Reset(kFlagSpinnerAtCT01);
				Game_Flag_Set(kFlagSpinnerAtBB01);
				Set_Enter(kSetBB01, kSceneBB01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kCT01LoopOutshot, true);
				break;
			case kSpinnerDestinationNightclubRow:
				Game_Flag_Set(kFlagMcCoyInNightclubRow);
				Game_Flag_Reset(kFlagArrivedFromSpinner1);
				Game_Flag_Reset(kFlagSpinnerAtCT01);
				Game_Flag_Set(kFlagSpinnerAtNR01);
				Set_Enter(kSetNR01, kSceneNR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kCT01LoopOutshot, true);
				break;
			case kSpinnerDestinationHysteriaHall:
				Game_Flag_Set(kFlagMcCoyInHysteriaHall);
				Game_Flag_Reset(kFlagArrivedFromSpinner1);
				Game_Flag_Reset(kFlagSpinnerAtCT01);
				Game_Flag_Set(kFlagSpinnerAtHF01);
				Set_Enter(kSetHF01, kSceneHF01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kCT01LoopOutshot, true);
				break;
			default:
				Game_Flag_Set(kFlagMcCoyInChinaTown);
				Player_Loses_Control();
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -530.0f, -6.5f, 241.0f, 0, false, true, false);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -330.0f, -6.5f, 221.0f, 0, false, true, false);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -314.0f, -6.5f, 326.0f, 0, false, false, false);
				Player_Gains_Control();
				break;
			}
		}
		return true;
	}

	return false;
}

bool SceneScriptCT01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptCT01::SceneFrameAdvanced(int frame) {
	if ((frame < 316
	  || frame > 435
	 )
	 && ((frame - 1) % 10) == 0
	) {
		Ambient_Sounds_Play_Sound(Random_Query(kSfxNEON5, kSfxNEON6), 25, 30, 30, 0);
	}

	if (frame == 23) {
		Ambient_Sounds_Play_Sound(kSfxCARDOWN3, 40,  99,   0,  0);
	}

	if (_vm->_cutContent) {
		if (frame == 136 || frame == 258) {
			Sound_Play(kSfxSPINOPN4, 100, 80, 80, 50);
		}

		if (frame == 183 || frame == 303) {
			Sound_Play(kSfxSPINCLS1, 100, 80, 80, 50);
		}
	}

	if (frame == 316) {
		Ambient_Sounds_Play_Sound(kSfxCARUP3B,  50, -50, 100, 99);
	}

	if (frame == 196
	 || frame == 452
	) {
		int v3 = Random_Query(0, 6);
		if (v3 == 0) {
			Overlay_Play("ct01spnr", 0, false, true, 0);
			if (Random_Query(0, 1)) {
				Ambient_Sounds_Play_Sound(kSfxSPIN2B, Random_Query(33, 50), 0, 0, 0);
			} else {
				Ambient_Sounds_Play_Sound(kSfxSPIN2A, Random_Query(33, 50), 0, 0, 0);
			}
		} else if (v3 == 1) {
			Overlay_Play("ct01spnr", 1, false, true, 0);
			if (Random_Query(0, 1)) {
				Ambient_Sounds_Play_Sound(kSfxSPIN3A, Random_Query(33, 50), 0, 0, 0);
			} else {
				Ambient_Sounds_Play_Sound(kSfxSPIN1A, Random_Query(33, 50), 0, 0, 0);
			}
		}
	}
}

void SceneScriptCT01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT01::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagCT02toCT01walk)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -327.2f, -6.5f, 352.28f, 0, false, false, false);
		Game_Flag_Reset(kFlagCT02toCT01walk);
	} else {
		if (!Game_Flag_Query(kFlagArrivedFromSpinner1)) {
			Game_Flag_Reset(kFlagArrivedFromSpinner1); // a bug? why reset a flag that is already cleared?
			return;
		}
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -330.0f, -6.5f, 221.0f, 0, false, false, false);
		if (_vm->_cutContent) {
			// unpause generic walkers here, less chance to collide with McCOy while he enters the scene
			if (Game_Flag_Query(kFlagArrivedFromSpinner1)
				&& Global_Variable_Query(kVariableGenericWalkerConfig) < 0
			) {
				Global_Variable_Set(kVariableGenericWalkerConfig, 2);
			}
		}
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -314.0f, -6.5f, 326.0f, 0, false, false, false);

		if (!Game_Flag_Query(kFlagCT01Visited)) {
			Game_Flag_Set(kFlagCT01Visited);
			if (!Game_Flag_Query(kFlagDirectorsCut)) {
				Actor_Voice_Over(200, kActorVoiceOver);
				Actor_Voice_Over(210, kActorVoiceOver);
				Actor_Voice_Over(220, kActorVoiceOver);
			}
		}
	}
}

void SceneScriptCT01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	if (Game_Flag_Query(kFlagCT01toCT12)) {
		Ambient_Sounds_Remove_Looping_Sound(kSfxCTAMBL1, 1);
		Ambient_Sounds_Remove_Looping_Sound(kSfxCTAMBR1, 1);
	} else {
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
	}
	Music_Stop(5);
#if BLADERUNNER_ORIGINAL_BUGS
	if (!Game_Flag_Query(kFlagMcCoyInChinaTown) && Global_Variable_Query(kVariableChapter) == 1) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeTowards3, true, -1);
	}
#else
	// Acts 2, 3 - should still use a spinner fly-through transition
	// also removed the redundant Ambient_Sounds_Remove_All_Non_Looping_Sounds
	if (!Game_Flag_Query(kFlagMcCoyInChinaTown)) {
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		if (!Game_Flag_Query(kFlagMcCoyInTyrellBuilding)) {
			// don't play this outtake when going to Tyrell Building
			Outtake_Play(kOuttakeTowards3, true, -1);   // available in Acts 1, 2, 3
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptCT01::DialogueQueueFlushed(int a1) {
}

void SceneScriptCT01::dialogueWithHowieLee() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(kActorMcCoy, kClueLucy)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(40, 4, 5, 6); // LUCY PHOTO
	}

	if (
	 (   Actor_Clue_Query(kActorMcCoy, kClueChopstickWrapper)
	  || Actor_Clue_Query(kActorMcCoy, kClueSushiMenu)
	 )
	 && !Game_Flag_Query(kFlagCT01Evidence1Linked)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(50, 5, 5, 4); // RUNCITER CLUES
	}

	if ( Actor_Clue_Query(kActorMcCoy, kClueChopstickWrapper)
	 &&  Actor_Clue_Query(kActorMcCoy, kClueSushiMenu)
	 &&  Game_Flag_Query(kFlagCT01Evidence1Linked)
	 && !Game_Flag_Query(kFlagCT01Evidence2Linked)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(60, 3, 5, 5); // MORE RUNCITER CLUES
	}

	if (Game_Flag_Query(kFlagCT02PotTipped)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(80, 9, 9, 9); // EMPLOYEE
	} else if (Game_Flag_Query(kFlagCT01ZubenLeft)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(80, 3, 4, 8); // EMPLOYEE
	}

	if ((Actor_Clue_Query(kActorMcCoy, kClueCarColorAndMake)
	    || (_vm->_cutContent && Actor_Clue_Query(kActorMcCoy, kClueLabPaintTransfer)))
	    && Actor_Clue_Query(kActorMcCoy, kClueDispatchHitAndRun) // this clue is now acquired in restored Cut Content
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(90, 5, 4, 5); // HIT AND RUN
	}

	DM_Add_To_List_Never_Repeat_Once_Selected(70, 7, 3, -1); // SMALL TALK
	Dialogue_Menu_Add_DONE_To_List(100); // DONE

	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	switch (answer) {
	case 40: // LUCY PHOTO
		Actor_Says(kActorMcCoy, 265, 11);
		Actor_Says(kActorHowieLee, 20, 14);
		if (Actor_Query_Is_In_Current_Set(kActorZuben)) {
			if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenDefault) {
				Actor_Face_Actor(kActorHowieLee, kActorZuben, true);
				Actor_Says(kActorHowieLee, 120, 14);
				Actor_Face_Actor(kActorZuben, kActorHowieLee, true);
				Actor_Says(kActorZuben, 40, 18);
				Actor_Face_Heading(kActorZuben, 103, false);
				Actor_Face_Actor(kActorHowieLee, kActorMcCoy, true);
				Actor_Modify_Friendliness_To_Other(kActorZuben, kActorMcCoy, -2);
				if (Actor_Query_Is_In_Current_Set(kActorGordo)) {
					Actor_Modify_Friendliness_To_Other(kActorGordo, kActorMcCoy, -3);
					Actor_Clue_Acquire(kActorGordo, kClueMcCoysDescription, true, kActorMcCoy);
				}
			}
		}
		break;

	case 50: // RUNCITER CLUES
		if (Actor_Clue_Query(kActorMcCoy, kClueChopstickWrapper)) {
			Actor_Says(kActorMcCoy, 270, 11);
			Actor_Says(kActorHowieLee, 30, 16);
		} else {
			Actor_Says(kActorMcCoy, 280, 11);
			Actor_Says(kActorHowieLee, 40, 14);
		}
		Game_Flag_Set(kFlagCT01Evidence1Linked);
		break;

	case 60: // MORE RUNCITER CLUES
		if (Actor_Clue_Query(kActorMcCoy, kClueSushiMenu)) {
			Actor_Says(kActorMcCoy, 270, 11);
			Actor_Says(kActorHowieLee, 40, 15); // You do Howie a favor? Distribute...
			// TODO Possible YES/NO option for McCoy? -> and friendliness adjustment accordingly
		} else {
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorMcCoy, 270, 11);
#else
			Actor_Says(kActorMcCoy, 280, 11);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorHowieLee, 30, 14);
		}
		Actor_Modify_Friendliness_To_Other(kActorHowieLee, kActorMcCoy, 5);
		Game_Flag_Set(kFlagCT01Evidence2Linked);
		break;

	case 70: // SMALL TALK
		Actor_Says(kActorMcCoy, 290, 13); // what's real fresh tonight
		if (((!_vm->_cutContent && Actor_Query_Friendliness_To_Other(kActorHowieLee, kActorMcCoy) > 49)
		     || Actor_Query_Friendliness_To_Other(kActorHowieLee, kActorMcCoy) > 59)
		    && (Global_Variable_Query(kVariableChinyen) > 10
		     || Query_Difficulty_Level() == kGameDifficultyEasy)
		) {
			Actor_Says(kActorHowieLee, 50, kAnimationModeTalk);
			Actor_Says(kActorHowieLee, 60, kAnimationModeTalk);
			if (_vm->_cutContent) {
				Actor_Says(kActorMcCoy, 320, 13);
			}
			Actor_Face_Actor(kActorHowieLee, kActorMcCoy, true);
			Actor_Says(kActorHowieLee, 70, 16);
			Actor_Says(kActorMcCoy, 325, 13);
			if (Query_Difficulty_Level() != kGameDifficultyEasy) {
				Global_Variable_Decrement(kVariableChinyen, 10);
			}
			Game_Flag_Set(kFlagCT01BoughtHowieLeeFood);
		} else {
			Actor_Says(kActorHowieLee, 130, 15); // nothing now
			if (_vm->_cutContent) {
				Actor_Says(kActorMcCoy, 8565, 14); // really?
				Actor_Says(kActorHowieLee, 80, 16); // No, sir. Any luck...
			}
		}
		break;

	case 80: // EMPLOYEE
		Actor_Says(kActorMcCoy, 295, 11);
		Actor_Says(kActorHowieLee, 90, 14);
		Actor_Says(kActorHowieLee, 100, 13);
		Actor_Clue_Acquire(kActorMcCoy, kClueHowieLeeInterview, true, kActorHowieLee);
		Actor_Modify_Friendliness_To_Other(kActorHowieLee, kActorMcCoy, -3);
		break;

	case 90: // HIT AND RUN
		Actor_Says(kActorMcCoy, 300, 13);
		Actor_Says(kActorHowieLee, 110, 16);
		break;

	case 100: // DONE
		Actor_Says(kActorMcCoy, 305, 18);
		break;
	}
}

} // End of namespace BladeRunner
