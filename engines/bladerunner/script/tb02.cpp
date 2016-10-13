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

#include "bladerunner/script/script.h"

namespace BladeRunner {

void ScriptTB02::InitializeScene() {
	if (Game_Flag_Query(155)) {
		Setup_Scene_Information(-152.0f, 0.0f, 1774.0f, 999);
	} else if (Game_Flag_Query(95)) {
		Setup_Scene_Information(-32.0f, 0.0f, 1578.0f, 639);
	} else if (Game_Flag_Query(608)) {
		Setup_Scene_Information(-32.0f, 0.0f, 1578.0f, 639);
	} else {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(27, 0, -1);
		Setup_Scene_Information(-304.0f, -81.46f, 1434.0f, 250);
	}
	if (Global_Variable_Query(1) > 3) {
		Scene_Exit_Add_2D_Exit(0, 0, 455, 639, 479, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(211, 20, 0, 1);
	Ambient_Sounds_Add_Sound(212, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(213, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(214, 2, 20, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(215, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(216, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	if (Global_Variable_Query(1) <= 3) {
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
	if (Game_Flag_Query(307) && Global_Variable_Query(1) < 4) {
		Scene_Exit_Add_2D_Exit(2, 67, 0, 233, 362, 3);
	}
	if (Game_Flag_Query(155)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(1);
	}
	Actor_Put_In_Set(17, 17);
	Actor_Set_At_XYZ(17, -38.53f, 2.93f, 1475.97f, 673);
	if (Global_Variable_Query(1) == 4) {
		if (Actor_Query_Goal_Number(17) < 300) {
			Actor_Set_Goal_Number(17, 300);
		}
		Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
	}
}

void ScriptTB02::SceneLoaded() {
	Obstacle_Object("SPHERE02", true);
	Unobstacle_Object("BOX36", true);
}

bool ScriptTB02::MouseClick(int x, int y) {
	return Region_Check(600, 300, 639, 479);
}

bool ScriptTB02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptTB02::ClickedOnActor(int actorId) {
	if (actorId == 17) {
		if (!Loop_Actor_Walk_To_XYZ(0, -76.35f, 0.15f, 1564.2f, 0, 1, false, 0)) {
			Actor_Face_Actor(0, 17, true);
			int v1 = Global_Variable_Query(1);
			if (v1 == 2) {
				if (Game_Flag_Query(450) && !Game_Flag_Query(451)) {
					Actor_Says(0, 5150, 18);
					Actor_Says(17, 60, 12);
					Actor_Says(17, 70, 13);
					Actor_Says(0, 5155, 13);
					Actor_Modify_Friendliness_To_Other(17, 0, -1);
					return true;
				}
				if (!Game_Flag_Query(450) && !Game_Flag_Query(451)) {
					Game_Flag_Set(450);
					Actor_Says(0, 5160, 18);
					Actor_Says(17, 80, 14);
					Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
					return true;
				}
				if (Game_Flag_Query(451)) {
					sub_402644();
				} else {
					Actor_Face_Actor(17, 0, true);
					Actor_Says(0, 5150, 18);
					Actor_Says(17, 60, 13);
					Actor_Says(17, 70, 12);
					Actor_Says(0, 5155, 13);
					Actor_Modify_Friendliness_To_Other(17, 0, -1);
					Actor_Face_Heading(17, 788, false);
				}
				return true;
			}
			if (v1 == 3) {
				Actor_Says(0, 5235, 18);
				Actor_Says(17, 280, 13);
				Actor_Says(17, 290, 12);
				Actor_Says(0, 5240, 18);
				Actor_Says(17, 300, 12);
				return false;
			}
			if (v1 == 4) {
				if (Actor_Query_Goal_Number(17) == 300) {
					Actor_Set_Goal_Number(17, 301);
				}
			}
		}
	}
	return false;
}

bool ScriptTB02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptTB02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -152.0f, 0.0f, 1774.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(154);
			Game_Flag_Reset(450);
			Set_Enter(17, 83);
			Async_Actor_Walk_To_XYZ(0, -152.0f, 0.0f, 1890.0f, 0, false);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -32.0f, 0.0f, 1578.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			if (Global_Variable_Query(1) < 4) {
				Game_Flag_Set(451);
				Game_Flag_Set(96);
				Set_Enter(72, 84);
			} else {
				Set_Enter(18, 108);
			}
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, -192.0f, 0.0f, 1430.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 800, false);
			Loop_Actor_Travel_Stairs(0, 9, 0, 0);
			if (Actor_Query_Goal_Number(17) == 300) {
				Actor_Set_Goal_Number(17, 301);
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
					Game_Flag_Reset(307);
					Game_Flag_Set(256);
					Set_Enter(37, 34);
					break;
				case 8:
					Game_Flag_Set(181);
					Game_Flag_Reset(307);
					Game_Flag_Set(255);
					Set_Enter(54, 54);
					break;
				case 7:
					Game_Flag_Set(258);
					Game_Flag_Reset(307);
					Game_Flag_Set(254);
					Set_Enter(20, 2);
					break;
				case 6:
					Game_Flag_Set(177);
					Game_Flag_Reset(307);
					Game_Flag_Set(253);
					Set_Enter(7, 25);
					break;
				case 4:
					Game_Flag_Set(180);
					Game_Flag_Reset(307);
					Game_Flag_Set(252);
					Set_Enter(0, 0);
					break;
				case 3:
					Game_Flag_Set(176);
					Game_Flag_Reset(307);
					Game_Flag_Set(248);
					Set_Enter(4, 13);
					break;
				case 2:
					Game_Flag_Set(182);
					Game_Flag_Reset(307);
					Game_Flag_Set(249);
					Set_Enter(69, 78);
					break;
				case 1:
					Game_Flag_Set(179);
					Game_Flag_Reset(307);
					Game_Flag_Set(250);
					Set_Enter(49, 48);
					break;
				case 0:
					Game_Flag_Set(178);
					Game_Flag_Reset(307);
					Game_Flag_Set(251);
					Set_Enter(61, 65);
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

bool ScriptTB02::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptTB02::SceneFrameAdvanced(int frame) {
}

void ScriptTB02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptTB02::PlayerWalkedIn() {
	if (Game_Flag_Query(625) && ((Game_Flag_Reset(625) , Global_Variable_Query(1) == 2) || Global_Variable_Query(1) == 3)) {
		Set_Enter(18, 108);
		//return true;
		return;
	}
	if (Game_Flag_Query(155)) {
		Async_Actor_Walk_To_XYZ(0, -152.0f, 0.0f, 1702.0f, 0, false);
		Game_Flag_Reset(155);
	} else if (Game_Flag_Query(95)) {
		Game_Flag_Reset(95);
	} else if (Game_Flag_Query(608)) {
		Game_Flag_Reset(608);
		if (Actor_Query_Goal_Number(17) == 300) {
			Actor_Set_Goal_Number(17, 302);
		}
		Music_Play(1, 50, 0, 2, -1, 0, 0);
	} else {
		Loop_Actor_Travel_Stairs(0, 9, 1, 0);
		Loop_Actor_Walk_To_XYZ(0, -140.0f, 0.79f, 1470.0f, 0, 0, false, 0);
	}
	int v0 = Global_Variable_Query(1);
	if (v0 > 4) {
		//return false;
		return;
	}
	if (v0 == 2) {
		if (!Game_Flag_Query(453)) {
			Player_Loses_Control();
			Actor_Says(0, 5125, 18);
			Actor_Says(17, 0, 50);
			Actor_Says(0, 5130, 13);
			Actor_Says(17, 10, 15);
			Item_Pickup_Spin_Effect(975, 351, 315);
			Actor_Says(17, 20, 23);
			Actor_Says(0, 5140, 17);
			Actor_Says(17, 30, 14);
			Actor_Says(17, 40, 13);
			Loop_Actor_Walk_To_XYZ(0, -140.0f, 0.0f, 1586.0f, 12, 0, false, 0);
			Loop_Actor_Walk_To_XYZ(0, -112.0f, 0.0f, 1586.0f, 12, 0, false, 0);
			Actor_Face_Actor(0, 17, true);
			Actor_Face_Actor(17, 0, true);
			Actor_Says(0, 5145, 13);
			Actor_Says(17, 50, 15);
			Actor_Face_Heading(17, 788, false);
			Actor_Clue_Acquire(0, 45, 1, -1);
			Game_Flag_Set(453);
			Game_Flag_Set(450);
			Player_Gains_Control();
			Loop_Actor_Walk_To_XYZ(0, -138.17f, 0.15f, 1578.32f, 0, 1, false, 0);
		}
		if (Game_Flag_Query(450)) {
			Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
		}
		if (Game_Flag_Query(451) && !Game_Flag_Query(450)) {
			Actor_Says(17, 90, 18);
			Game_Flag_Set(450);
			Scene_Exit_Add_2D_Exit(1, 430, 235, 487, 396, 0);
		}
		if (Game_Flag_Query(451) && !Game_Flag_Query(456)) {
			Loop_Actor_Walk_To_Actor(1, 0, 36, 1, false);
			Actor_Says(1, 2220, 14);
			Actor_Says(0, 5245, 13);
			Actor_Says(1, 2230, 12);
			Actor_Says(1, 2240, 13);
			sub_402B50();
			//return true;
		}
		//return false;
		return;
	}
	if (v0 == 3 && !Game_Flag_Query(455)) {
		Loop_Actor_Walk_To_XYZ(0, -131.28f, 0.79f, 1448.25f, 12, 1, false, 0);
		Actor_Says(17, 260, 15);
		Actor_Says(0, 5225, 16);
		Actor_Says(17, 270, 14);
		Game_Flag_Set(455);
		Actor_Modify_Friendliness_To_Other(17, 0, -1);
	}
	//return false;
}

void ScriptTB02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptTB02::DialogueQueueFlushed(int a1) {
}

void ScriptTB02::sub_402644() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(700, 4, 5, 6);
	if (Actor_Clue_Query(0, 44)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(710, 5, 5, 4);
	}
	if (Actor_Clue_Query(0, 50) || Actor_Clue_Query(0, 51)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(720, 3, 5, 5);
	}
	if (Actor_Clue_Query(0, 51)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(730, 3, 4, 8);
	}
	Dialogue_Menu_Add_DONE_To_List(100);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 100:
		Actor_Says(0, 5145, 13);
		Actor_Says(17, 50, 15);
		break;
	case 730:
		Actor_Says(0, 5180, 16);
		Actor_Says(17, 240, 12);
		Actor_Says(0, 5215, 18);
		Actor_Says(17, 250, 13);
		Actor_Says(0, 5220, 16);
		break;
	case 720:
		Actor_Says(0, 5175, 12);
		Actor_Says(17, 210, 14);
		Actor_Says(0, 5200, 13);
		Actor_Says(17, 220, 13);
		Actor_Says(0, 5205, 15);
		Actor_Says(17, 230, 12);
		Actor_Says(0, 5210, 12);
		break;
	case 710:
		Actor_Says(0, 5170, 12);
		Actor_Says(17, 180, 12);
		Actor_Says(17, 190, 14);
		if (Game_Flag_Query(102)) {
			Actor_Says(0, 5195, 13);
			Actor_Says(17, 200, 13);
		}
		break;
	case 700:
		Actor_Says(0, 5165, 11);
		Actor_Says(17, 100, 13);
		Actor_Says(17, 110, 12);
		Actor_Says(0, 5185, 15);
		Actor_Says(17, 120, 12);
		Actor_Says(17, 130, 14);
		Actor_Says(0, 5190, 16);
		Actor_Says(17, 140, 13);
		Actor_Says(17, 150, 14);
		Actor_Says(17, 170, 12);
		Actor_Clue_Acquire(0, 50, 1, 17);
		break;
	}
}

void ScriptTB02::sub_402B50() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(740, 4, 5, 6);
	DM_Add_To_List_Never_Repeat_Once_Selected(750, 3, 5, 5);
	Dialogue_Menu_Add_DONE_To_List(100);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 740:
		Actor_Says(0, 5250, 15);
		if (Game_Flag_Query(48)) {
			Actor_Says(1, 2250, 12);
			Actor_Says(1, 2260, 13);
			Actor_Says(0, 5265, 12);
			Actor_Says(1, 2270, 16);
			Actor_Says(1, 2280, 13);
			Actor_Says(0, 5270, 16);
			Actor_Says(1, 2290, 14);
			Actor_Clue_Acquire(0, 52, 1, 1);
			Actor_Modify_Friendliness_To_Other(1, 0, 1);
			Game_Flag_Set(456);
		} else {
			Actor_Says(1, 2300, 12);
			Actor_Says(1, 2310, 15);
			Actor_Says(0, 5275, 14);
			Actor_Says(1, 2320, 12);
			Actor_Says(0, 5280, 13);
			Actor_Modify_Friendliness_To_Other(1, 0, 1);
			Game_Flag_Set(456);
		}
		break;
	case 750:
		Actor_Says(0, 5255, 11);
		Actor_Says(1, 2330, 13);
		Actor_Says(1, 2340, 14);
		Game_Flag_Set(456);
		break;
	case 100:
		Actor_Says(1, 2350, 13);
		Actor_Modify_Friendliness_To_Other(1, 0, -5);
		Game_Flag_Set(456);
		break;
	}
}

} // End of namespace BladeRunner
