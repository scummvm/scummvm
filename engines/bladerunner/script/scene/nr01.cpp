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

void SceneScriptNR01::InitializeScene() {
	if (Game_Flag_Query(617)) {
		Setup_Scene_Information(-153.86f, 23.88f, -570.21f, 402);
	} else if (Game_Flag_Query(632)) {
		Setup_Scene_Information(-416.0f, 31.93f, -841.0f, 200);
		Actor_Set_Invisible(kActorMcCoy, true);
		Preload(167);
	} else if (Game_Flag_Query(534)) {
		Setup_Scene_Information(-416.0f, 31.93f, -841.0f, 200);
	} else if (Game_Flag_Query(342)) {
		Setup_Scene_Information(-270.0f, 4.93f, -1096.0f, 500);
	} else if (Game_Flag_Query(533)) {
		Setup_Scene_Information(312.0f, 31.66f, -901.0f, 700);
	} else if (Game_Flag_Query(545)) {
		Setup_Scene_Information(-170.0f, 24.0f, -574.0f, 768);
	} else {
		Setup_Scene_Information(76.0f, 23.88f, -109.0f, 966);
	}
	Scene_Exit_Add_2D_Exit(0, 31, 270, 97, 373, 3);
	if (Global_Variable_Query(kVariableChapter) > 3) {
		Scene_Exit_Add_2D_Exit(1, 201, 320, 276, 357, 2);
	}
	Scene_Exit_Add_2D_Exit(2, 583, 262, 639, 365, 1);
	if (Game_Flag_Query(kFlagSpinnerAtNR01)) {
		Scene_Exit_Add_2D_Exit(3, 320, 445, 639, 479, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(54, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(362, 22, 55, 1);
	Ambient_Sounds_Add_Sound(361, 10, 10, 20, 20, -70, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(68, 10, 80, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 80, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 10, 80, 33, 33, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 10, 80, 33, 33, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 10, 80, 33, 33, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(643) && Actor_Query_Goal_Number(kActorSteele) == 230) {
		Game_Flag_Reset(kFlagSpinnerAtNR01);
		Game_Flag_Reset(kFlagSpinnerAtHF01);
	}
	if (Game_Flag_Query(kFlagSpinnerAtNR01) && !Game_Flag_Query(kFlagArrivedFromSpinner)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
		Game_Flag_Set(kFlagArrivedFromSpinner);
	} else if (Game_Flag_Query(kFlagSpinnerAtNR01) && Game_Flag_Query(kFlagArrivedFromSpinner)) {
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(4);
	}
}

void SceneScriptNR01::SceneLoaded() {
	Obstacle_Object("LAMPBASE01", true);
	Unclickable_Object("LAMPBASE01");
}

bool SceneScriptNR01::MouseClick(int x, int y) {
	if (Actor_Query_Goal_Number(kActorMcCoy) == 212) {
		Global_Variable_Increment(47, 4);
		return true;
	}
	return false;
}

bool SceneScriptNR01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptNR01::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptNR01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -380.0f, 31.93f, -841.0f, 0, true, false, 0)) {
			if (Global_Variable_Query(kVariableChapter) > 3) {
				Actor_Says(kActorMcCoy, 8522, 12);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(535);
				Set_Enter(kSetNR03, kSceneNR03);
			}
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -281.0f, 31.93f, -1061.0f, 0, true, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 45, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 3, 0, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(343);
			Set_Enter(kSetUG06, kSceneUG06);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 312.0f, 31.66f, -901.0f, 0, true, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(532);
			Set_Enter(kSetNR02, kSceneNR02);
		}
		return true;
	}
	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 108.0f, 23.88f, -93.0f, 0, true, false, 0)) {
			Game_Flag_Reset(kFlagMcCoyAtCTxx);
			Game_Flag_Reset(kFlagMcCoyAtRCxx);
			Game_Flag_Reset(kFlagMcCoyAtMAxx);
			Game_Flag_Reset(kFlagMcCoyAtPSxx);
			Game_Flag_Reset(kFlagMcCoyAtBBxx);
			Game_Flag_Reset(kFlagMcCoyAtHFxx);
			Game_Flag_Reset(kFlagMcCoyAtTBxx);
			Game_Flag_Reset(kFlagMcCoyAtNRxx);
			switch (Spinner_Interface_Choose_Dest(-1, true)) {
			case kSpinnerDestinationPoliceStation:
				Game_Flag_Set(kFlagMcCoyAtPSxx);
				Game_Flag_Reset(kFlagSpinnerAtNR01);
				Game_Flag_Reset(kFlagArrivedFromSpinner);
				Game_Flag_Set(kFlagSpinnerAtPS01);
				Set_Enter(kSetPS01, kScenePS01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
				break;
			case kSpinnerDestinationMcCoysApartment:
				Game_Flag_Set(kFlagMcCoyAtMAxx);
				Game_Flag_Reset(kFlagSpinnerAtNR01);
				Game_Flag_Reset(kFlagArrivedFromSpinner);
				Game_Flag_Set(kFlagSpinnerAtMA01);
				Set_Enter(kSetMA01, kSceneMA01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
				break;
			case kSpinnerDestinationRuncitersAnimals:
				Game_Flag_Set(kFlagMcCoyAtRCxx);
				Game_Flag_Reset(kFlagSpinnerAtNR01);
				Game_Flag_Reset(kFlagArrivedFromSpinner);
				Game_Flag_Set(kFlagSpinnerAtRC01);
				Set_Enter(kSetRC01, kSceneRC01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
				break;
			case kSpinnerDestinationChinatown:
				Game_Flag_Set(kFlagMcCoyAtCTxx);
				Game_Flag_Reset(kFlagSpinnerAtNR01);
				Game_Flag_Reset(kFlagArrivedFromSpinner);
				Game_Flag_Set(kFlagSpinnerAtCT01);
				Set_Enter(kSetCT01_CT12, kSceneCT01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
				break;
			case kSpinnerDestinationAnimoidRow:
				Game_Flag_Set(kFlagMcCoyAtARxx);
				Game_Flag_Reset(kFlagSpinnerAtNR01);
				Game_Flag_Reset(kFlagArrivedFromSpinner);
				Game_Flag_Set(kFlagSpinnerAtAR01);
				Set_Enter(kSetAR01_AR02, kSceneAR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(kFlagMcCoyAtTBxx);
				Game_Flag_Reset(kFlagSpinnerAtNR01);
				Game_Flag_Reset(kFlagArrivedFromSpinner);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
				break;
			case kSpinnerDestinationDNARow:
				Game_Flag_Set(kFlagMcCoyAtDRxx);
				Game_Flag_Reset(kFlagSpinnerAtNR01);
				Game_Flag_Reset(kFlagArrivedFromSpinner);
				Game_Flag_Set(kFlagSpinnerAtDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
				break;
			case kSpinnerDestinationBradburyBuilding:
				Game_Flag_Set(kFlagMcCoyAtBBxx);
				Game_Flag_Reset(kFlagSpinnerAtNR01);
				Game_Flag_Reset(kFlagArrivedFromSpinner);
				Game_Flag_Set(kFlagSpinnerAtBB01);
				Set_Enter(kSetBB01, kSceneBB01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
				break;
			case kSpinnerDestinationHysteriaHall:
				Game_Flag_Set(kFlagMcCoyAtHFxx);
				Game_Flag_Reset(kFlagSpinnerAtNR01);
				Game_Flag_Set(kFlagSpinnerAtHF01);
				Set_Enter(kSetHF01, kSceneHF01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
				break;
			default:
				Player_Loses_Control();
				Game_Flag_Set(kFlagMcCoyAtNRxx);
				Game_Flag_Set(kFlagArrivedFromSpinner);
				Player_Gains_Control();
				break;
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptNR01::ClickedOn2DRegion(int region) {
	if (region == 0 && Player_Query_Combat_Mode()) {
		Sound_Play(517, 100, 0, 0, 50);
		Actor_Set_Goal_Number(kActorSteele, 260);
		Scene_2D_Region_Remove(0);
	}
	if (region == 1 && Player_Query_Combat_Mode()) {
		Sound_Play(517, 100, 0, 0, 50);
		Actor_Set_Goal_Number(kActorGordo, 299);
		Actor_Set_Goal_Number(kActorSteele, 258);
		Scene_2D_Region_Remove(1);
		return true;
	}
	return false;

}

void SceneScriptNR01::SceneFrameAdvanced(int frame) {
	if (frame == 61) {
		Sound_Play(118, 40, 0, 0, 50);
	}
	if (frame == 184) {
		Sound_Play(117, 40, 80, 80, 50);
	}
	//return 0;
}

void SceneScriptNR01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR01::PlayerWalkedIn() {
	if (Game_Flag_Query(617)) {
		Actor_Set_Goal_Number(kActorSteele, 280);
		Game_Flag_Reset(617);
		//return true;
		return;
	}
	if (Actor_Query_Goal_Number(kActorSteele) == 250) {
		Scene_Exits_Disable();
		ADQ_Flush();
		Actor_Set_Goal_Number(kActorSteele, 251);
		Scene_2D_Region_Add(0, 450, 316, 464, 333);
		Scene_2D_Region_Add(1, 233, 321, 240, 362);
		ADQ_Add(kActorGordo, 70, 81);
		ADQ_Add(kActorSteele, 990, 3);
		ADQ_Add(kActorGordo, 80, 82);
		ADQ_Add(kActorGordo, 90, 81);
		ADQ_Add(kActorSteele, 1010, 3);
		ADQ_Add(kActorGordo, 100, 81);
		ADQ_Add(kActorSteele, 1020, 3);
		ADQ_Add(kActorGordo, 110, 82);
		ADQ_Add(kActorSteele, 1030, 3);
		ADQ_Add(kActorSteele, 1040, 3);
		ADQ_Add(kActorGordo, 120, 82);
	}
	if (Game_Flag_Query(604)) {
		if (Game_Flag_Query(622)) {
			ADQ_Add(kActorHanoi, 150, 3);
			Game_Flag_Reset(622);
		}
		Game_Flag_Reset(604);
		Player_Gains_Control();
		//return true;
		return;
	}
	if (Game_Flag_Query(632)) {
		Delay(3500);
		Set_Enter(kSetNR11, kSceneNR11);
		//return true;
		return;
	}
	if (Game_Flag_Query(534)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -380.0f, 31.73f, -841.0f, 0, 0, false, 0);
		Game_Flag_Reset(534);
	} else {
		if (Game_Flag_Query(342)) {
			Loop_Actor_Travel_Stairs(kActorMcCoy, 3, 1, 0);
			Game_Flag_Reset(342);
			if (Actor_Query_Goal_Number(kActorSteele) == 230) {
				Actor_Face_Actor(kActorSteele, kActorMcCoy, true);
				Actor_Says(kActorSteele, 1440, 13);
				Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorSteele, 48, 0, true);
				Actor_Says(kActorMcCoy, 3145, 13);
				if (Global_Variable_Query(40) != 3) {
					Actor_Says(kActorSteele, 1450, 12);
					Actor_Says(kActorSteele, 1460, 13);
				}
				Actor_Says(kActorMcCoy, 3150, 14);
				Actor_Says(kActorSteele, 1470, 12);
				Actor_Says(kActorSteele, 1480, 13);
				Actor_Says(kActorMcCoy, 3155, 15);
				Actor_Says(kActorSteele, 1500, 16);
				Actor_Says(kActorMcCoy, 3160, 12);
				if (Game_Flag_Query(643)) {
					Actor_Says(kActorSteele, 1330, 12);
					Actor_Says(kActorSteele, 1340, 12);
					Actor_Says(kActorSteele, 1350, 12);
					Actor_Says(kActorMcCoy, 3120, 15);
					Actor_Says(kActorSteele, 1360, 12);
					Actor_Says(kActorSteele, 1370, 12);
					Actor_Says(kActorMcCoy, 3125, 15);
					Actor_Says(kActorSteele, 1380, 12);
					Actor_Says(kActorMcCoy, 3130, 15);
					Actor_Says(kActorSteele, 1390, 12);
					Actor_Says(kActorSteele, 1400, 12);
					Actor_Says(kActorSteele, 1410, 12);
					Actor_Says(kActorMcCoy, 3135, 15);
					Actor_Says(kActorSteele, 1420, 12);
					Actor_Says(kActorMcCoy, 3140, 15);
					Actor_Says(kActorSteele, 1430, 12);
					Actor_Set_Goal_Number(kActorSteele, 285);
				} else {
					int v0 = Global_Variable_Query(40) - 1;
					if (!v0) {
						Actor_Says(kActorSteele, 1510, 15);
						Actor_Says(kActorSteele, 1520, 14);
						Actor_Says(kActorSteele, 1530, 13);
						Actor_Says(kActorMcCoy, 3170, 13);
						Actor_Set_Goal_Number(kActorSteele, 231);
					} else if (v0 == 1) {
						Actor_Says(kActorSteele, 1590, 15);
						Actor_Says(kActorMcCoy, 3195, 14);
						Actor_Says(kActorSteele, 1600, 16);
						Actor_Says(kActorMcCoy, 3200, 13);
						Actor_Says(kActorSteele, 1610, 17);
						Actor_Says(kActorSteele, 1620, 15);
						Actor_Says(kActorSteele, 1630, 14);
						Actor_Says(kActorMcCoy, 3205, 12);
						Actor_Set_Goal_Number(kActorSteele, 232);
					} else if (v0 == 2) {
						Actor_Says(kActorSteele, 1540, 15);
						Actor_Says(kActorMcCoy, 3175, 13);
						Actor_Says(kActorSteele, 1550, 13);
						Actor_Says(kActorSteele, 1560, 16);
						Actor_Says(kActorMcCoy, 3180, 15);
						Actor_Says(kActorSteele, 1570, 12);
						Actor_Says(kActorSteele, 1580, 14);
						Actor_Says(kActorMcCoy, 3190, 12);
						Actor_Set_Goal_Number(kActorSteele, 233);
					}
				}
			}
		} else if (Game_Flag_Query(533)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 239.0f, 31.66f, -901.0f, 0, 0, false, 0);
			Game_Flag_Reset(533);
			if (Actor_Query_Goal_Number(kActorGordo) == 230) {
				Scene_Exits_Disable();
				Actor_Set_Goal_Number(kActorGordo, 231);
				Non_Player_Actor_Combat_Mode_On(kActorGordo, kActorCombatStateIdle, true, kActorMcCoy, 3, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 20, 300, false);
			}
		} else if (Game_Flag_Query(545)) {
			Game_Flag_Reset(545);
			Actor_Put_In_Set(kActorHanoi, kSetNR01);
			Actor_Set_At_XYZ(kActorHanoi, -202.0f, 24.0f, -574.0f, 0);
			Actor_Face_Heading(kActorHanoi, 256, false);
			Actor_Set_Goal_Number(kActorHanoi, 204);
			Player_Gains_Control();
		} else {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 48.0f, 23.88f, -189.0f, 0, 0, false, 0);
		}
	}
	if (Game_Flag_Query(652)) {
		Game_Flag_Reset(652);
		Actor_Voice_Over(950, kActorVoiceOver);
		Actor_Voice_Over(960, kActorVoiceOver);
		Actor_Voice_Over(970, kActorVoiceOver);
		Actor_Voice_Over(980, kActorVoiceOver);
	}
	if (Actor_Query_Goal_Number(kActorGordo) == 240) {
		Scene_Exits_Disable();
		Actor_Set_Goal_Number(kActorGordo, 241);
		if (!Player_Query_Combat_Mode()) {
			Player_Set_Combat_Mode(true);
		}
	}
	//return false;
	return;
}

void SceneScriptNR01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(343) && !Game_Flag_Query(532) && !Game_Flag_Query(535) && !Game_Flag_Query(632) && !Game_Flag_Query(722)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeAway1, true, -1);
		Outtake_Play(kOuttakeInside2, true, -1);
	}
	Game_Flag_Reset(722);
}

void SceneScriptNR01::DialogueQueueFlushed(int a1) {
	if (Actor_Query_Goal_Number(kActorSteele) == 251 && Actor_Query_Goal_Number(kActorGordo) != 299 && Actor_Query_Goal_Number(kActorGordo) != 254 && Actor_Query_Goal_Number(kActorGordo) != 255) {
		Actor_Set_Goal_Number(kActorSteele, 252);
	}
}

} // End of namespace BladeRunner
