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
	} else if (Game_Flag_Query(608)) {
		Setup_Scene_Information(-32.0f, 0.0f, 1578.0f, 639);
	} else {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeTyrellBuildingFly, false, -1);
		Setup_Scene_Information(-304.0f, -81.46f, 1434.0f, 250);
	}
	if (Global_Variable_Query(kVariableChapter) > 3) {
		Scene_Exit_Add_2D_Exit(0, 0, 455, 639, 479, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(211, 20, 0, 1);
	Ambient_Sounds_Add_Sound(212, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(213, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(214, 2, 20, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(215, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(216, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	if (Global_Variable_Query(kVariableChapter) <= 3) {
		Ambient_Sounds_Add_Looping_Sound(45, 35, 0, 1);
		Ambient_Sounds_Add_Sound(181, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(183, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(190, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(193, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(194, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	}
	if (Game_Flag_Query(kFlagSpinnerAtTB02) && Global_Variable_Query(kVariableChapter) < 4) {
		Scene_Exit_Add_2D_Exit(2, 67, 0, 233, 362, 3);
	}
	if (Game_Flag_Query(kFlagTB03toTB02)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(1);
	}
	Actor_Put_In_Set(kActorTyrellGuard, 17);
	Actor_Set_At_XYZ(kActorTyrellGuard, -38.53f, 2.93f, 1475.97f, 673);
	if (Global_Variable_Query(kVariableChapter) == 4) {
		if (Actor_Query_Goal_Number(kActorTyrellGuard) < 300) {
			Actor_Set_Goal_Number(kActorTyrellGuard, 300);
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
	if (actorId == 17) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -76.35f, 0.15f, 1564.2f, 0, 1, false, 0)) {
			Actor_Face_Actor(kActorMcCoy, kActorTyrellGuard, true);
			int v1 = Global_Variable_Query(kVariableChapter);
			if (v1 == 2) {
				if (Game_Flag_Query(450) && !Game_Flag_Query(451)) {
					Actor_Says(kActorMcCoy, 5150, 18);
					Actor_Says(kActorTyrellGuard, 60, 12);
					Actor_Says(kActorTyrellGuard, 70, 13);
					Actor_Says(kActorMcCoy, 5155, 13);
					Actor_Modify_Friendliness_To_Other(kActorTyrellGuard, kActorMcCoy, -1);
					return true;
				}
				if (!Game_Flag_Query(450) && !Game_Flag_Query(451)) {
					Game_Flag_Set(450);
					Actor_Says(kActorMcCoy, 5160, 18);
					Actor_Says(kActorTyrellGuard, 80, 14);
					Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
					return true;
				}
				if (Game_Flag_Query(451)) {
					sub_402644();
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
			if (v1 == 3) {
				Actor_Says(kActorMcCoy, 5235, 18);
				Actor_Says(kActorTyrellGuard, 280, 13);
				Actor_Says(kActorTyrellGuard, 290, 12);
				Actor_Says(kActorMcCoy, 5240, 18);
				Actor_Says(kActorTyrellGuard, 300, 12);
				return false;
			}
			if (v1 == 4) {
				if (Actor_Query_Goal_Number(kActorTyrellGuard) == 300) {
					Actor_Set_Goal_Number(kActorTyrellGuard, 301);
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
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -152.0f, 0.0f, 1774.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagTB02toTB03);
			Game_Flag_Reset(450);
			Set_Enter(kSetTB02_TB03, kSceneTB03);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -152.0f, 0.0f, 1890.0f, 0, false);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -32.0f, 0.0f, 1578.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			if (Global_Variable_Query(kVariableChapter) < 4) {
				Game_Flag_Set(451);
				Game_Flag_Set(kFlagTB02toTB05);
				Set_Enter(kSetTB05, kSceneTB05);
			} else {
				Set_Enter(kSetTB07, kSceneTB07);
			}
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -192.0f, 0.0f, 1430.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 800, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 9, 0, 0);
			if (Actor_Query_Goal_Number(kActorTyrellGuard) == 300) {
				Actor_Set_Goal_Number(kActorTyrellGuard, 301);
			} else {
				Game_Flag_Reset(176);
				Game_Flag_Reset(182);
				Game_Flag_Reset(179);
				Game_Flag_Reset(178);
				Game_Flag_Reset(258);
				Game_Flag_Reset(257);
				Game_Flag_Reset(261);
				Game_Flag_Reset(450);
				switch (Spinner_Interface_Choose_Dest(-1, 0)) {
				case 9:
					Game_Flag_Set(257);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtHF01);
					Set_Enter(37, kSceneHF01);
					break;
				case 8:
					Game_Flag_Set(181);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtNR01);
					Set_Enter(54, kSceneNR01);
					break;
				case 7:
					Game_Flag_Set(258);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtBB01);
					Set_Enter(20, kSceneBB01);
					break;
				case 6:
					Game_Flag_Set(177);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtDR01);
					Set_Enter(7, kSceneDR01);
					break;
				case 4:
					Game_Flag_Set(180);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtAR01);
					Set_Enter(0, kSceneAR01);
					break;
				case 3:
					Game_Flag_Set(176);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtCT01);
					Set_Enter(4, kSceneCT01);
					break;
				case 2:
					Game_Flag_Set(182);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtRC01);
					Set_Enter(69, kSceneRC01);
					break;
				case 1:
					Game_Flag_Set(179);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtMA01);
					Set_Enter(49, kSceneMA01);
					break;
				case 0:
					Game_Flag_Set(178);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtPS01);
					Set_Enter(61, kScenePS01);
					break;
				default:
					Game_Flag_Set(261);
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
	if (Game_Flag_Query(625) && ((Game_Flag_Reset(625) , Global_Variable_Query(kVariableChapter) == 2) || Global_Variable_Query(kVariableChapter) == 3)) {
		Set_Enter(18, kSceneTB07);
		//return true;
		return;
	}
	if (Game_Flag_Query(kFlagTB03toTB02)) {
		Async_Actor_Walk_To_XYZ(kActorMcCoy, -152.0f, 0.0f, 1702.0f, 0, false);
		Game_Flag_Reset(kFlagTB03toTB02);
	} else if (Game_Flag_Query(kFlagTB05toTB02)) {
		Game_Flag_Reset(kFlagTB05toTB02);
	} else if (Game_Flag_Query(608)) {
		Game_Flag_Reset(608);
		if (Actor_Query_Goal_Number(kActorTyrellGuard) == 300) {
			Actor_Set_Goal_Number(kActorTyrellGuard, 302);
		}
		Music_Play(1, 50, 0, 2, -1, 0, 0);
	} else {
		Loop_Actor_Travel_Stairs(kActorMcCoy, 9, 1, 0);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -140.0f, 0.79f, 1470.0f, 0, 0, false, 0);
	}
	int v0 = Global_Variable_Query(kVariableChapter);
	if (v0 > 4) {
		//return false;
		return;
	}
	if (v0 == 2) {
		if (!Game_Flag_Query(453)) {
			Player_Loses_Control();
			Actor_Says(kActorMcCoy, 5125, 18);
			Actor_Says(kActorTyrellGuard, 0, 50);
			Actor_Says(kActorMcCoy, 5130, 13);
			Actor_Says(kActorTyrellGuard, 10, 15);
			Item_Pickup_Spin_Effect(975, 351, 315);
			Actor_Says(kActorTyrellGuard, 20, 23);
			Actor_Says(kActorMcCoy, 5140, 17);
			Actor_Says(kActorTyrellGuard, 30, 14);
			Actor_Says(kActorTyrellGuard, 40, 13);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -140.0f, 0.0f, 1586.0f, 12, 0, false, 0);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -112.0f, 0.0f, 1586.0f, 12, 0, false, 0);
			Actor_Face_Actor(kActorMcCoy, kActorTyrellGuard, true);
			Actor_Face_Actor(kActorTyrellGuard, kActorMcCoy, true);
			Actor_Says(kActorMcCoy, 5145, 13);
			Actor_Says(kActorTyrellGuard, 50, 15);
			Actor_Face_Heading(kActorTyrellGuard, 788, false);
			Actor_Clue_Acquire(kActorMcCoy, kClueTyrellSecurity, 1, -1);
			Game_Flag_Set(453);
			Game_Flag_Set(450);
			Player_Gains_Control();
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -138.17f, 0.15f, 1578.32f, 0, 1, false, 0);
		}
		if (Game_Flag_Query(450)) {
			Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
		}
		if (Game_Flag_Query(451) && !Game_Flag_Query(450)) {
			Actor_Says(kActorTyrellGuard, 90, 18);
			Game_Flag_Set(450);
			Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
		}
		if (Game_Flag_Query(451) && !Game_Flag_Query(456)) {
			Loop_Actor_Walk_To_Actor(kActorSteele, kActorMcCoy, 36, 1, false);
			Actor_Says(kActorSteele, 2220, 14);
			Actor_Says(kActorMcCoy, 5245, 13);
			Actor_Says(kActorSteele, 2230, 12);
			Actor_Says(kActorSteele, 2240, 13);
			sub_402B50();
			//return true;
		}
		//return false;
		return;
	}
	if (v0 == 3 && !Game_Flag_Query(455)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -131.28f, 0.79f, 1448.25f, 12, 1, false, 0);
		Actor_Says(kActorTyrellGuard, 260, 15);
		Actor_Says(kActorMcCoy, 5225, 16);
		Actor_Says(kActorTyrellGuard, 270, 14);
		Game_Flag_Set(455);
		Actor_Modify_Friendliness_To_Other(kActorTyrellGuard, kActorMcCoy, -1);
	}
	//return false;
}

void SceneScriptTB02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptTB02::DialogueQueueFlushed(int a1) {
}

void SceneScriptTB02::sub_402644() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(700, 4, 5, 6);
	if (Actor_Clue_Query(kActorMcCoy, kClueDragonflyEarring)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(710, 5, 5, 4);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueVictimInformation) || Actor_Clue_Query(kActorMcCoy, kClueAttemptedFileAccess)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(720, 3, 5, 5);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueAttemptedFileAccess)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(730, 3, 4, 8);
	}
	Dialogue_Menu_Add_DONE_To_List(100);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 100:
		Actor_Says(kActorMcCoy, 5145, 13);
		Actor_Says(kActorTyrellGuard, 50, 15);
		break;
	case 730:
		Actor_Says(kActorMcCoy, 5180, 16);
		Actor_Says(kActorTyrellGuard, 240, 12);
		Actor_Says(kActorMcCoy, 5215, 18);
		Actor_Says(kActorTyrellGuard, 250, 13);
		Actor_Says(kActorMcCoy, 5220, 16);
		break;
	case 720:
		Actor_Says(kActorMcCoy, 5175, 12);
		Actor_Says(kActorTyrellGuard, 210, 14);
		Actor_Says(kActorMcCoy, 5200, 13);
		Actor_Says(kActorTyrellGuard, 220, 13);
		Actor_Says(kActorMcCoy, 5205, 15);
		Actor_Says(kActorTyrellGuard, 230, 12);
		Actor_Says(kActorMcCoy, 5210, 12);
		break;
	case 710:
		Actor_Says(kActorMcCoy, 5170, 12);
		Actor_Says(kActorTyrellGuard, 180, 12);
		Actor_Says(kActorTyrellGuard, 190, 14);
		if (Game_Flag_Query(102)) {
			Actor_Says(kActorMcCoy, 5195, 13);
			Actor_Says(kActorTyrellGuard, 200, 13);
		}
		break;
	case 700:
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
		Actor_Clue_Acquire(kActorMcCoy, kClueVictimInformation, 1, kActorTyrellGuard);
		break;
	}
}

void SceneScriptTB02::sub_402B50() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(740, 4, 5, 6);
	DM_Add_To_List_Never_Repeat_Once_Selected(750, 3, 5, 5);
	Dialogue_Menu_Add_DONE_To_List(100);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 740:
		Actor_Says(kActorMcCoy, 5250, 15);
		if (Game_Flag_Query(kFlagSadikIsReplicant)) {
			Actor_Says(kActorSteele, 2250, 12);
			Actor_Says(kActorSteele, 2260, 13);
			Actor_Says(kActorMcCoy, 5265, 12);
			Actor_Says(kActorSteele, 2270, 16);
			Actor_Says(kActorSteele, 2280, 13);
			Actor_Says(kActorMcCoy, 5270, 16);
			Actor_Says(kActorSteele, 2290, 14);
			Actor_Clue_Acquire(kActorMcCoy, kClueCrystalsCase, 1, kActorSteele);
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 1);
			Game_Flag_Set(456);
		} else {
			Actor_Says(kActorSteele, 2300, 12);
			Actor_Says(kActorSteele, 2310, 15);
			Actor_Says(kActorMcCoy, 5275, 14);
			Actor_Says(kActorSteele, 2320, 12);
			Actor_Says(kActorMcCoy, 5280, 13);
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 1);
			Game_Flag_Set(456);
		}
		break;
	case 750:
		Actor_Says(kActorMcCoy, 5255, 11);
		Actor_Says(kActorSteele, 2330, 13);
		Actor_Says(kActorSteele, 2340, 14);
		Game_Flag_Set(456);
		break;
	case 100:
		Actor_Says(kActorSteele, 2350, 13);
		Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, -5);
		Game_Flag_Set(456);
		break;
	}
}

} // End of namespace BladeRunner
