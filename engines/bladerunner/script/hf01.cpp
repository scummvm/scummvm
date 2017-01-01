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

void ScriptHF01::InitializeScene() {
	if (Game_Flag_Query(617)) {
		Setup_Scene_Information(243.94f, 8.0f, -341.9f, 342);
	} else if (Game_Flag_Query(313)) {
		Setup_Scene_Information(-202.0f, 0.0f, -619.0f, 407);
	} else if (Game_Flag_Query(311)) {
		Setup_Scene_Information(124.0f, 8.0f, -880.0f, 455);
	} else if (Game_Flag_Query(309)) {
		Setup_Scene_Information(406.0f, 8.0f, -813.0f, 455);
	} else {
		Setup_Scene_Information(100.0f, 0.0f, -260.0f, 0);
	}
	Scene_Exit_Add_2D_Exit(0, 81, 226, 169, 321, 0);
	if (!Game_Flag_Query(663)) {
		Scene_Exit_Add_2D_Exit(1, 304, 239, 492, 339, 0);
		Scene_Exit_Add_2D_Exit(2, 560, 231, 639, 360, 0);
		if (Game_Flag_Query(256)) {
			Scene_Exit_Add_2D_Exit(3, 0, 311, 66, 417, 2);
		}
	}
	Ambient_Sounds_Add_Looping_Sound(54, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(340, 25, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(81, 60, 100, 1);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(68, 10, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(256)) {
		if (!Game_Flag_Query(309) && !Game_Flag_Query(311) && !Game_Flag_Query(313)) {
			Scene_Loop_Start_Special(0, 0, 0);
		}
		Scene_Loop_Set_Default(1);
	} else if (Game_Flag_Query(663)) {
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(5);
	}
}

void ScriptHF01::SceneLoaded() {
	Obstacle_Object("LOFT41", true);
	if (!Game_Flag_Query(256)) {
		Unobstacle_Object("OBSTACLE BOX15", true);
	}
}

bool ScriptHF01::MouseClick(int x, int y) {
	return false;
}

bool ScriptHF01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptHF01::ClickedOnActor(int actorId) {
	int v1;
	if (Global_Variable_Query(45) == 2) {
		v1 = 3;
	} else if (Global_Variable_Query(45) == 3) {
		v1 = 6;
	} else {
		v1 = -1;
	}
	if (actorId == 22 || actorId == 31) {
		if (!Loop_Actor_Walk_To_XYZ(0, 504.04f, 8.0f, -242.17f, 12, 1, false, 0)) {
			ADQ_Flush();
			Actor_Face_Actor(0, 31, true);
			if (Game_Flag_Query(382)) {
				sub_4026B4();
			} else {
				Actor_Says(0, 1455, 15);
				Actor_Says(31, 40, 13);
				Actor_Says(0, 1460, 13);
				Actor_Says(31, 50, 12);
				Item_Pickup_Spin_Effect(951, 396, 359);
				Actor_Face_Heading(31, 271, false);
				Actor_Says(31, 60, 12);
				Actor_Says(0, 1465, 15);
				Actor_Face_Actor(31, 0, true);
				Actor_Says(31, 70, 13);
				Actor_Says(0, 1470, 14);
				Actor_Says(22, 30, 3);
				Actor_Says(31, 80, 15);
				Actor_Says(22, 50, 3);
				Actor_Says(31, 110, 14);
				Game_Flag_Set(382);
			}
			return true;
		}
		return false;
	}
	if (actorId == v1) {
		if (!Loop_Actor_Walk_To_Actor(0, actorId, 28, 1, false)) {
			if (Actor_Query_Goal_Number(v1) == 599) {
				Actor_Says(0, 8630, 13);
				return true;
			}
			if (Global_Variable_Query(1) == 5 && (Actor_Clue_Query(0, 139) || Actor_Clue_Query(0, 141)) && !Game_Flag_Query(165) && Actor_Query_Goal_Number(9) != 2 && Game_Flag_Query(653) && !Game_Flag_Query(662)) {
				Game_Flag_Set(662);
				Actor_Face_Actor(0, v1, true);
				Actor_Face_Actor(v1, 0, true);
				if (v1 == 3) {
					sub_4032DC();
				} else if (v1 == 6) {
					sub_403484();
				}
				Async_Actor_Walk_To_XYZ(v1, -175.0f, 8.0f, -617.0f, 0, false);
				Loop_Actor_Walk_To_XYZ(0, -137.0f, 8.0f, -577.0f, 0, 0, false, 1);
				Game_Flag_Set(312);
				Set_Enter(41, 38);
			}
		}
	}
	return false;
}

bool ScriptHF01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptHF01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -202.0f, 8.0f, -619.0f, 0, 1, false, 0)) {
			Game_Flag_Set(312);
			Set_Enter(41, 38);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 124.0f, 8.0f, -724.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(0, 124.0f, 8.0f, -880.0f, 0, 0, false, 0);
			Game_Flag_Set(310);
			Set_Enter(39, 36);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 406.0f, 8.0f, -717.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(0, 406.0f, 8.0f, -813.0f, 0, 0, false, 0);
			Game_Flag_Set(308);
			Set_Enter(38, 35);
		}
		return true;
	}
	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(0, 100.0f, 0.0f, -260.0f, 0, 1, false, 0)) {
			Game_Flag_Reset(176);
			Game_Flag_Reset(182);
			Game_Flag_Reset(179);
			Game_Flag_Reset(178);
			Game_Flag_Reset(258);
			Game_Flag_Reset(257);
			int spinnerDest = Spinner_Interface_Choose_Dest(3, 0);
			switch (spinnerDest) {
			case 0:
				Game_Flag_Set(178);
				Game_Flag_Reset(256);
				Game_Flag_Set(251);
				Set_Enter(61, 65);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 1:
				Game_Flag_Set(179);
				Game_Flag_Reset(256);
				Game_Flag_Set(250);
				Set_Enter(49, 48);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 2:
				Game_Flag_Set(182);
				Game_Flag_Reset(256);
				Game_Flag_Set(249);
				Set_Enter(69, 78);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 3:
				Game_Flag_Set(176);
				Game_Flag_Reset(256);
				Game_Flag_Set(248);
				Set_Enter(4, 13);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 5:
				Game_Flag_Set(261);
				Game_Flag_Reset(256);
				Game_Flag_Set(307);
				Set_Enter(17, 82);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 4:
				Game_Flag_Set(180);
				Game_Flag_Reset(256);
				Game_Flag_Set(252);
				Set_Enter(0, 0);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 7:
				Game_Flag_Set(258);
				Game_Flag_Reset(256);
				Game_Flag_Set(254);
				Set_Enter(20, 2);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 6:
				Game_Flag_Set(177);
				Game_Flag_Reset(256);
				Game_Flag_Set(253);
				Set_Enter(7, 25);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 8:
				Game_Flag_Set(181);
				Game_Flag_Reset(256);
				Game_Flag_Set(255);
				Set_Enter(54, 54);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			default:
				Game_Flag_Set(257);
				Loop_Actor_Walk_To_XYZ(0, 100.0f, 0.0f, -300.0f, 0, 1, false, 0);
				break;
			}
		}
		return true;
	}
	return false;
}

bool ScriptHF01::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptHF01::SceneFrameAdvanced(int frame) {
	if (frame == 10) {
		Sound_Play(118, 40, 0, 0, 50);
	}
	if (frame == 72 || frame == 193) {
		Sound_Play(116, 100, -50, -50, 50);
	}
	if (frame == 88 || frame == 214) {
		Sound_Play(119, 100, -50, -50, 50);
	}
	if (frame == 242) {
		Sound_Play(117, 40, -50, 80, 50);
	}
	if (Actor_Query_Goal_Number(31) == 1) {
		Actor_Set_Goal_Number(31, 0);
	}
}

void ScriptHF01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptHF01::PlayerWalkedIn() {
	if (Game_Flag_Query(663)) {
		ADQ_Flush();
		ADQ_Add(24, 280, 3);
		Actor_Put_In_Set(23, 37);
		Actor_Set_At_XYZ(23, 8.2f, 8.0f, -346.67f, 1021);
		Actor_Put_In_Set(24, 37);
		Actor_Set_At_XYZ(24, 51.21f, 8.0f, -540.78f, 796);
		Non_Player_Actor_Combat_Mode_On(23, 3, 1, 0, 4, 4, 7, 8, 0, 0, 0, 100, 300, 0);
		Non_Player_Actor_Combat_Mode_On(24, 3, 1, 0, 4, 4, 7, 8, 0, 0, 0, 100, 300, 0);
	}
	if (!Game_Flag_Query(165) && Actor_Query_Goal_Number(9) != 2) {
		if (Actor_Clue_Query(0, 141) && Global_Variable_Query(45) == 3 && Actor_Query_Goal_Number(6) != 599) {
			Actor_Put_In_Set(6, 37);
			Actor_Set_At_XYZ(6, -5.0f, 8.0f, -622.0f, 419);
			Actor_Set_Targetable(6, true);
		} else if (Actor_Clue_Query(0, 139) && Global_Variable_Query(45) == 2 && Actor_Query_Goal_Number(3) != 599) {
			Actor_Put_In_Set(3, 37);
			Actor_Set_At_XYZ(3, -5.0f, 8.0f, -622.0f, 419);
			Actor_Set_Targetable(3, true);
		}
	}
	if (Game_Flag_Query(617)) {
		Actor_Set_Goal_Number(1, 280);
		Game_Flag_Reset(617);
		//return true;
		return;
	}
	if (Game_Flag_Query(652)) {
		Game_Flag_Reset(652);
		Actor_Voice_Over(950, 99);
		Actor_Voice_Over(960, 99);
		Actor_Voice_Over(970, 99);
		Actor_Voice_Over(980, 99);
	} else if (!Game_Flag_Query(377) && Global_Variable_Query(1) < 4) {
		ADQ_Flush();
		ADQ_Add(31, 0, 14);
		ADQ_Add(31, 10, 3);
		ADQ_Add(22, 0, 3);
		Actor_Face_Actor(31, 22, true);
		ADQ_Add(31, 20, 13);
		ADQ_Add(22, 10, 3);
		ADQ_Add(31, 30, 3);
		ADQ_Add(22, 20, 3);
		Actor_Face_Heading(31, 271, false);
		Game_Flag_Set(377);
	}
	if (Game_Flag_Query(311)) {
		Loop_Actor_Walk_To_XYZ(0, 124.0f, 8.0f, -724.0f, 0, 1, false, 0);
	} else if (Game_Flag_Query(309)) {
		Loop_Actor_Walk_To_XYZ(0, 406.0f, 8.0f, -717.0f, 0, 1, false, 0);
	} else if (!Game_Flag_Query(313)) {
		Loop_Actor_Walk_To_XYZ(0, 100.0f, 0.0f, -300.0f, 0, 1, false, 0);
	}
	Game_Flag_Reset(311);
	Game_Flag_Reset(309);
	Game_Flag_Reset(313);
	//return false;
}

void ScriptHF01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(312) && !Game_Flag_Query(308) && !Game_Flag_Query(310) && !Game_Flag_Query(722)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(35, 1, -1);
		Outtake_Play(38, 1, -1);
	}
	Game_Flag_Reset(722);
	if (Actor_Query_Goal_Number(6) == 450) {
		Actor_Put_In_Set(6, 97);
		Actor_Set_At_Waypoint(6, 39, 0);
		Actor_Set_Goal_Number(6, 599);
	}
	if (Actor_Query_Goal_Number(3) == 450) {
		Actor_Put_In_Set(3, 97);
		Actor_Set_At_Waypoint(3, 39, 0);
		Actor_Set_Goal_Number(3, 599);
	}
}

void ScriptHF01::DialogueQueueFlushed(int a1) {
}

void ScriptHF01::sub_4026B4() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 13) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(440, 8, 6, 3);
	}
	if (!Actor_Clue_Query(0, 13) && Actor_Clue_Query(0, 22) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(450, 7, 6, 3);
	}
	if (Actor_Clue_Query(0, 87) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(460, 3, 5, 6);
	}
	if (Actor_Clue_Query(0, 118) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(470, -1, 3, 8);
	}
	Dialogue_Menu_Add_DONE_To_List(480);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 440:
		Actor_Says(0, 1480, 15);
		Actor_Says(22, 70, 3);
		Actor_Says(31, 120, 3);
		Actor_Says(0, 1505, 14);
		Actor_Says(22, 80, 3);
		Actor_Says(31, 130, 3);
		Actor_Says(22, 90, 3);
		Actor_Says(0, 1510, 12);
		break;
	case 450:
		Actor_Says(0, 1485, 16);
		Actor_Says(22, 100, 3);
		Actor_Says(31, 140, 12);
		Actor_Says(22, 110, 3);
		Actor_Says(22, 120, 3);
		Actor_Says(31, 150, 14);
		break;
	case 460:
		Actor_Says(0, 1490, 13);
		Actor_Says(31, 160, 15);
		Actor_Says(22, 130, 13);
		Actor_Says(31, 170, 12);
		Actor_Says(31, 180, 13);
		Actor_Says(31, 190, 14);
		Actor_Says(0, 1515, 15);
		Actor_Says(31, 200, 3);
		Actor_Says(0, 1520, 15);
		Actor_Says(31, 210, 13);
		Actor_Says(31, 220, 13);
		Actor_Says(22, 140, 12);
		Actor_Says(31, 230, 13);
		Actor_Clue_Acquire(0, 101, 0, 31);
		break;
	case 470:
		Actor_Says(0, 1495, 14);
		Actor_Face_Actor(31, 0, true);
		Actor_Says(31, 240, 13);
		Actor_Face_Actor(31, 22, true);
		break;
	case 480:
		Actor_Says(0, 1500, 16);
		break;
	}
}

void ScriptHF01::sub_4032DC() {
	Actor_Says(3, 0, 3);
	Actor_Says(0, 1400, 3);
	Actor_Says(3, 10, 3);
	Actor_Says(0, 1405, 3);
	Actor_Says(3, 20, 3);
	Actor_Says(0, 1410, 3);
	Actor_Says(3, 30, 3);
	Actor_Says(0, 1415, 3);
	Actor_Says(3, 40, 3);
	Actor_Says(3, 50, 3);
	Actor_Says(3, 60, 3);
	Actor_Says(3, 70, 3);
	Actor_Says(0, 1420, 3);
	Actor_Says(3, 80, 3);
}

void ScriptHF01::sub_403484() {
	Actor_Says(6, 0, 3);
	Actor_Says(0, 1425, 3);
	Actor_Says(6, 10, 3);
	Actor_Says(6, 20, 3);
	Actor_Says(0, 1430, 3);
	Actor_Says(6, 30, 3);
	Actor_Says(0, 1435, 3);
	Actor_Says(6, 40, 3);
	Actor_Says(6, 50, 3);
	Actor_Says(0, 1440, 3);
	Actor_Says(6, 60, 3);
	Actor_Says(6, 70, 3);
	Actor_Says(0, 1445, 3);
	Actor_Says(6, 80, 3);
	Actor_Says(6, 3030, 3);
}

} // End of namespace BladeRunner
