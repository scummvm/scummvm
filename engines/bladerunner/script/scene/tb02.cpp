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

void SceneScriptTB02::InitializeScene() {
	if (Game_Flag_Query(kFlagTB03toTB02)) {
		Setup_Scene_Information(-152.0f, 0.0f, 1774.0f, 999);
	} else if (Game_Flag_Query(kFlagTB05toTB02)) {
		Setup_Scene_Information(-32.0f, 0.0f, 1578.0f, 639);
	} else if (Game_Flag_Query(kFlagTB07toTB02)) {
		Setup_Scene_Information(-32.0f, 0.0f, 1578.0f, 639);
	} else {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeTyrellBuildingFly, false, -1); // TB_FLY_#.VQA Note: this is the only fly-through scene that is localized!
		Setup_Scene_Information(-304.0f, -81.46f, 1434.0f, 250);
	}
	if (Global_Variable_Query(kVariableChapter) > 3) {
		Scene_Exit_Add_2D_Exit(0, 0, 455, 639, 479, 2);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxFOUNTAIN, 20, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBELLY1, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY2, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY3, 2, 20, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY4, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY5, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	if (Global_Variable_Query(kVariableChapter) <= 3) {
		Ambient_Sounds_Add_Looping_Sound(kSfxPSAMB6, 35, 0, 1);
		Ambient_Sounds_Add_Sound(kSfx67_0470R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0500R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0960R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1100R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1140R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	}

	if (Game_Flag_Query(kFlagSpinnerAtTB02)
	 && Global_Variable_Query(kVariableChapter) < 4
	) {
		Scene_Exit_Add_2D_Exit(2, 67, 0, 233, 362, 3);
	}

	if (Game_Flag_Query(kFlagTB03toTB02)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(1);
	}

	Actor_Put_In_Set(kActorTyrellGuard, kSetTB02_TB03);
	Actor_Set_At_XYZ(kActorTyrellGuard, -38.53f, 2.93f, 1475.97f, 673);
	if (Global_Variable_Query(kVariableChapter) == 4) {
		if (Actor_Query_Goal_Number(kActorTyrellGuard) < kGoalTyrellGuardSleeping) {
			Actor_Set_Goal_Number(kActorTyrellGuard, kGoalTyrellGuardSleeping);
		}
		Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
	}
}

void SceneScriptTB02::SceneLoaded() {
	Obstacle_Object("SPHERE02", true);
	Unobstacle_Object("BOX36", true);
}

bool SceneScriptTB02::MouseClick(int x, int y) {
	return Region_Check(600, 300, 639, 479);
}

bool SceneScriptTB02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptTB02::ClickedOnActor(int actorId) {
	if (actorId == kActorTyrellGuard) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -76.35f, 0.15f, 1564.2f, 0, true, false, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorTyrellGuard, true);
			int chapter = Global_Variable_Query(kVariableChapter);
			if (chapter == 2) {
				if ( Game_Flag_Query(kFlagTB02ElevatorToTB05)
				 && !Game_Flag_Query(kFlagTB05Entered)
				) {
					Actor_Says(kActorMcCoy, 5150, 18);
					Actor_Says(kActorTyrellGuard, 60, 12);
					Actor_Says(kActorTyrellGuard, 70, 13);
					Actor_Says(kActorMcCoy, 5155, 13);
					Actor_Modify_Friendliness_To_Other(kActorTyrellGuard, kActorMcCoy, -1);
					return true;
				}

				if (!Game_Flag_Query(kFlagTB02ElevatorToTB05)
				 && !Game_Flag_Query(kFlagTB05Entered)
				) {
					Game_Flag_Set(kFlagTB02ElevatorToTB05);
					Actor_Says(kActorMcCoy, 5160, 18);
					Actor_Says(kActorTyrellGuard, 80, 14);
					Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
					return true;
				}

				if (Game_Flag_Query(kFlagTB05Entered)) {
					dialogueWithTyrellGuard();
				} else {
					Actor_Face_Actor(kActorTyrellGuard, kActorMcCoy, true);
					Actor_Says(kActorMcCoy, 5150, 18);
					Actor_Says(kActorTyrellGuard, 60, 13);
					Actor_Says(kActorTyrellGuard, 70, 12);
					Actor_Says(kActorMcCoy, 5155, 13);
					Actor_Modify_Friendliness_To_Other(kActorTyrellGuard, kActorMcCoy, -1);
					Actor_Face_Heading(kActorTyrellGuard, 788, false);
				}
				return true;
			}

			if (chapter == 3) {
				Actor_Says(kActorMcCoy, 5235, 18);
				Actor_Says(kActorTyrellGuard, 280, 13);
				Actor_Says(kActorTyrellGuard, 290, 12);
				Actor_Says(kActorMcCoy, 5240, 18);
				Actor_Says(kActorTyrellGuard, 300, 12);
				return false;
			}

			if (chapter == 4) {
				if (Actor_Query_Goal_Number(kActorTyrellGuard) == kGoalTyrellGuardSleeping) {
					Actor_Set_Goal_Number(kActorTyrellGuard, kGoalTyrellGuardWakeUpAndArrestMcCoy);
				}
			}
		}
	}
	return false;
}

bool SceneScriptTB02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptTB02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -152.0f, 0.0f, 1774.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagTB02toTB03);
			Game_Flag_Reset(kFlagTB02ElevatorToTB05);
			Set_Enter(kSetTB02_TB03, kSceneTB03);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -152.0f, 0.0f, 1890.0f, 0, false);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -32.0f, 0.0f, 1578.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			if (Global_Variable_Query(kVariableChapter) < 4) {
				Game_Flag_Set(kFlagTB05Entered);
				Game_Flag_Set(kFlagTB02toTB05);
				Set_Enter(kSetTB05, kSceneTB05);
			} else {
				Set_Enter(kSetTB07, kSceneTB07);
			}
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -192.0f, 0.0f, 1430.0f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 800, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 9, false, kAnimationModeIdle);
			if (Actor_Query_Goal_Number(kActorTyrellGuard) == kGoalTyrellGuardSleeping) {
				Actor_Set_Goal_Number(kActorTyrellGuard, kGoalTyrellGuardWakeUpAndArrestMcCoy);
			} else {
				Game_Flag_Reset(kFlagMcCoyInChinaTown);
				Game_Flag_Reset(kFlagMcCoyInRunciters);
				Game_Flag_Reset(kFlagMcCoyInMcCoyApartment);
				Game_Flag_Reset(kFlagMcCoyInPoliceStation);
				Game_Flag_Reset(kFlagMcCoyInBradburyBuilding);
				Game_Flag_Reset(kFlagMcCoyInHysteriaHall);
				Game_Flag_Reset(kFlagMcCoyInTyrellBuilding);
				Game_Flag_Reset(kFlagTB02ElevatorToTB05);
				switch (Spinner_Interface_Choose_Dest(-1, false)) {
				case kSpinnerDestinationPoliceStation:
					Game_Flag_Set(kFlagMcCoyInPoliceStation);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtPS01);
					Set_Enter(kSetPS01, kScenePS01);
					break;
				case kSpinnerDestinationMcCoysApartment:
					Game_Flag_Set(kFlagMcCoyInMcCoyApartment);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtMA01);
					Set_Enter(kSetMA01, kSceneMA01);
					break;
				case kSpinnerDestinationRuncitersAnimals:
					Game_Flag_Set(kFlagMcCoyInRunciters);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtRC01);
					Set_Enter(kSetRC01, kSceneRC01);
					break;
				case kSpinnerDestinationChinatown:
					Game_Flag_Set(kFlagMcCoyInChinaTown);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtCT01);
					Set_Enter(kSetCT01_CT12, kSceneCT01);
					break;
				case kSpinnerDestinationAnimoidRow:
					Game_Flag_Set(kFlagMcCoyInAnimoidRow);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtAR01);
					Set_Enter(kSetAR01_AR02, kSceneAR01);
					break;
				case kSpinnerDestinationDNARow:
					Game_Flag_Set(kFlagMcCoyInDNARow);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtDR01);
					Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
					break;
				case kSpinnerDestinationBradburyBuilding:
					Game_Flag_Set(kFlagMcCoyInBradburyBuilding);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtBB01);
					Set_Enter(kSetBB01, kSceneBB01);
					break;
				case kSpinnerDestinationNightclubRow:
					Game_Flag_Set(kFlagMcCoyInNightclubRow);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtNR01);
					Set_Enter(kSetNR01, kSceneNR01);
					break;
				case kSpinnerDestinationHysteriaHall:
					Game_Flag_Set(kFlagMcCoyInHysteriaHall);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtHF01);
					Set_Enter(kSetHF01, kSceneHF01);
					break;
				default:
					Game_Flag_Set(kFlagMcCoyInTyrellBuilding);
					break;
				}
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptTB02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptTB02::SceneFrameAdvanced(int frame) {
}

void SceneScriptTB02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptTB02::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagTB07TyrellMeeting)) {
		Game_Flag_Reset(kFlagTB07TyrellMeeting);
		if (Global_Variable_Query(kVariableChapter) == 2
		 || Global_Variable_Query(kVariableChapter) == 3
		) {
			Set_Enter(kSetTB07, kSceneTB07);
			return; // true;
		}
	}
	if (Game_Flag_Query(kFlagTB03toTB02)) {
		Async_Actor_Walk_To_XYZ(kActorMcCoy, -152.0f, 0.0f, 1702.0f, 0, false);
		Game_Flag_Reset(kFlagTB03toTB02);
	} else if (Game_Flag_Query(kFlagTB05toTB02)) {
		Game_Flag_Reset(kFlagTB05toTB02);
	} else if (Game_Flag_Query(kFlagTB07toTB02)) {
		Game_Flag_Reset(kFlagTB07toTB02);
		if (Actor_Query_Goal_Number(kActorTyrellGuard) == kGoalTyrellGuardSleeping) {
			Actor_Set_Goal_Number(kActorTyrellGuard, kGoalTyrellGuardWakeUp);
		}
		Music_Play(kMusicBatl226M, 50, 0, 2, -1, 0, 0);
	} else {
		Loop_Actor_Travel_Stairs(kActorMcCoy, 9, true, kAnimationModeIdle);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -140.0f, 0.79f, 1470.0f, 0, false, false, false);
	}
	int chapter = Global_Variable_Query(kVariableChapter);
	if (chapter > 4) {
		//return false;
		return;
	}
	if (chapter == 2) {
		if (!Game_Flag_Query(kFlagTB02GuardTalk1)) {
			Player_Loses_Control();
			Actor_Says(kActorMcCoy, 5125, 18);
			Actor_Says(kActorTyrellGuard, 0, 50);
			Actor_Says(kActorMcCoy, 5130, 13);
			Actor_Says(kActorTyrellGuard, 10, 15);
			Item_Pickup_Spin_Effect(kModelAnimationVideoDisc, 351, 315);
			Actor_Says(kActorTyrellGuard, 20, 23);
			Actor_Says(kActorMcCoy, 5140, 17);
			Actor_Says(kActorTyrellGuard, 30, 14);
			Actor_Says(kActorTyrellGuard, 40, 13);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -140.0f, 0.0f, 1586.0f, 12, false, false, false);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -112.0f, 0.0f, 1586.0f, 12, false, false, false);
			Actor_Face_Actor(kActorMcCoy, kActorTyrellGuard, true);
			Actor_Face_Actor(kActorTyrellGuard, kActorMcCoy, true);
			Actor_Says(kActorMcCoy, 5145, 13);
			Actor_Says(kActorTyrellGuard, 50, 15);
			Actor_Face_Heading(kActorTyrellGuard, 788, false);
			Actor_Clue_Acquire(kActorMcCoy, kClueTyrellSecurity, true, -1);
			Game_Flag_Set(kFlagTB02GuardTalk1);
			Game_Flag_Set(kFlagTB02ElevatorToTB05);
			Player_Gains_Control();
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -138.17f, 0.15f, 1578.32f, 0, true, false, false);
		}

		if (Game_Flag_Query(kFlagTB02ElevatorToTB05)) {
			Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
		}

		if ( Game_Flag_Query(kFlagTB05Entered)
		 && !Game_Flag_Query(kFlagTB02ElevatorToTB05)
		) {
			Actor_Says(kActorTyrellGuard, 90, 18);
			Game_Flag_Set(kFlagTB02ElevatorToTB05);
			Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
		}

		if ( Game_Flag_Query(kFlagTB05Entered)
		 && !Game_Flag_Query(kFlagTB02SteeleTalk)
		) {
			Loop_Actor_Walk_To_Actor(kActorSteele, kActorMcCoy, 36, true, false);
			Actor_Says(kActorSteele, 2220, 14);
			Actor_Says(kActorMcCoy, 5245, 13);
			Actor_Says(kActorSteele, 2230, 12);
			Actor_Says(kActorSteele, 2240, 13);
			dialogueWithSteele();
			//return true;
		}
		//return false;
		return;
	}

	if ( chapter == 3
	 && !Game_Flag_Query(kFlagTB02GuardTalk2)
	) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -131.28f, 0.79f, 1448.25f, 12, true, false, false);
		Actor_Says(kActorTyrellGuard, 260, 15);
		Actor_Says(kActorMcCoy, 5225, 16);
		Actor_Says(kActorTyrellGuard, 270, 14);
		Game_Flag_Set(kFlagTB02GuardTalk2);
		Actor_Modify_Friendliness_To_Other(kActorTyrellGuard, kActorMcCoy, -1);
	}
	//return false;
}

void SceneScriptTB02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (Global_Variable_Query(kVariableChapter) < 4
	    && !Game_Flag_Query(kFlagMcCoyInTyrellBuilding)
	) {
		// Acts 2, 3 - use a spinner fly-through transition
		Outtake_Play(kOuttakeTowards3, true, -1); // available in Acts 1, 2, 3
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptTB02::DialogueQueueFlushed(int a1) {
}

void SceneScriptTB02::dialogueWithTyrellGuard() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(700, 4, 5, 6); // VICTIM

	if (Actor_Clue_Query(kActorMcCoy, kClueDragonflyEarring)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(710, 5, 5, 4); // EARRING
	}

	if (Actor_Clue_Query(kActorMcCoy, kClueVictimInformation)
	 || Actor_Clue_Query(kActorMcCoy, kClueAttemptedFileAccess)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(720, 3, 5, 5); // TYRELL
	}

	if (Actor_Clue_Query(kActorMcCoy, kClueAttemptedFileAccess)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(730, 3, 4, 8); // SECURITY
	}

	Dialogue_Menu_Add_DONE_To_List(100); // DONE

	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	switch (answer) {
	case 100: // DONE
		Actor_Says(kActorMcCoy, 5145, 13);
		Actor_Says(kActorTyrellGuard, 50, 15);
		break;

	case 700: // VICTIM
		Actor_Says(kActorMcCoy, 5165, 11);
		Actor_Says(kActorTyrellGuard, 100, 13);
		Actor_Says(kActorTyrellGuard, 110, 12);
		Actor_Says(kActorMcCoy, 5185, 15);
		Actor_Says(kActorTyrellGuard, 120, 12);
		Actor_Says(kActorTyrellGuard, 130, 14);
		Actor_Says(kActorMcCoy, 5190, 16);
		Actor_Says(kActorTyrellGuard, 140, 13);
		Actor_Says(kActorTyrellGuard, 150, 14);
		Actor_Says(kActorTyrellGuard, 170, 12);
		Actor_Clue_Acquire(kActorMcCoy, kClueVictimInformation, true, kActorTyrellGuard);
		break;

	case 710: // EARRING
		Actor_Says(kActorMcCoy, 5170, 12);
		Actor_Says(kActorTyrellGuard, 180, 12);
		Actor_Says(kActorTyrellGuard, 190, 14);
		if (Game_Flag_Query(kFlagTB06Visited)) {
			Actor_Says(kActorMcCoy, 5195, 13);
			Actor_Says(kActorTyrellGuard, 200, 13);
		}
		break;

	case 720: // TYRELL
		Actor_Says(kActorMcCoy, 5175, 12);
		Actor_Says(kActorTyrellGuard, 210, 14);
		Actor_Says(kActorMcCoy, 5200, 13);
		Actor_Says(kActorTyrellGuard, 220, 13);
		Actor_Says(kActorMcCoy, 5205, 15);
		Actor_Says(kActorTyrellGuard, 230, 12);
		Actor_Says(kActorMcCoy, 5210, 12);
		break;

	case 730: // SECURITY
		Actor_Says(kActorMcCoy, 5180, 16);
		Actor_Says(kActorTyrellGuard, 240, 12);
		Actor_Says(kActorMcCoy, 5215, 18);
		Actor_Says(kActorTyrellGuard, 250, 13);
		Actor_Says(kActorMcCoy, 5220, 16);
		break;
	}
}

void SceneScriptTB02::dialogueWithSteele() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(740, 4, 5, 6); // HER CASE
	DM_Add_To_List_Never_Repeat_Once_Selected(750, 3, 5, 5); // MURDER
	Dialogue_Menu_Add_DONE_To_List(100); // DONE

	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	switch (answer) {
	case 100: // DONE
		Actor_Says(kActorSteele, 2350, 13);
		Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, -5);
		Game_Flag_Set(kFlagTB02SteeleTalk);
		break;

	case 740: // HER CASE
		Actor_Says(kActorMcCoy, 5250, 15);
		if (Game_Flag_Query(kFlagSadikIsReplicant)) {
			Actor_Says(kActorSteele, 2250, 12);
			Actor_Says(kActorSteele, 2260, 13);
			Actor_Says(kActorMcCoy, 5265, 12);
			Actor_Says(kActorSteele, 2270, 16);
			Actor_Says(kActorSteele, 2280, 13);
			Actor_Says(kActorMcCoy, 5270, 16);
			Actor_Says(kActorSteele, 2290, 14);
			Actor_Clue_Acquire(kActorMcCoy, kClueCrystalsCase, true, kActorSteele);
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 1);
			Game_Flag_Set(kFlagTB02SteeleTalk);
		} else {
			Actor_Says(kActorSteele, 2300, 12);
			Actor_Says(kActorSteele, 2310, 15);
			Actor_Says(kActorMcCoy, 5275, 14);
			Actor_Says(kActorSteele, 2320, 12);
			Actor_Says(kActorMcCoy, 5280, 13);
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 1);
			Game_Flag_Set(kFlagTB02SteeleTalk);
		}
		break;

	case 750: // MURDER
		Actor_Says(kActorMcCoy, 5255, 11);
		Actor_Says(kActorSteele, 2330, 13);
		Actor_Says(kActorSteele, 2340, 14);
		Game_Flag_Set(kFlagTB02SteeleTalk);
		break;
	}
}

} // End of namespace BladeRunner
