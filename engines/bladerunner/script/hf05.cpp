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

void ScriptHF05::InitializeScene() {
	if (Game_Flag_Query(530)) {
		Setup_Scene_Information(257.0f, 40.63f, 402.0f, 1000);
	} else if (Game_Flag_Query(358)) {
		Setup_Scene_Information(330.0f, 40.63f, -107.0f, 603);
	} else {
		Setup_Scene_Information(483.0f, 40.63f, -189.0f, 600);
	}
	Scene_Exit_Add_2D_Exit(0, 443, 270, 515, 350, 0);
	if (Global_Variable_Query(1) > 3) {
		Scene_Exit_Add_2D_Exit(1, 367, 298, 399, 349, 2);
	}
	Scene_Exit_Add_2D_Exit(2, 589, 0, 639, 479, 1);
	Ambient_Sounds_Add_Looping_Sound(103, 40, 1, 1);
	if (Game_Flag_Query(369)) {
		Scene_Loop_Set_Default(5);
		sub_404474();
	} else if (Game_Flag_Query(559)) {
		Scene_Loop_Set_Default(2);
		sub_404474();
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void ScriptHF05::SceneLoaded() {
	Obstacle_Object("MAINBASE", true);
	Unobstacle_Object("BTIRES02", true);
	Unobstacle_Object("LFTIRE02", true);
	if (Game_Flag_Query(369)) {
		Unobstacle_Object("MONTE CARLO DRY", true);
	} else {
		Unobstacle_Object("OBSTACLE_HOLE", true);
	}
	Clickable_Object("TOP CON");
}

bool ScriptHF05::MouseClick(int x, int y) {
	return false;
}

bool ScriptHF05::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("TOP CON", objectName) == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 95.0f, 40.63f, 308.0f, 0, 1, false, 0)) {
			Actor_Face_Object(0, "TOP CON", true);
			if (Actor_Query_In_Set(9, 41) && Actor_Query_Goal_Number(9) != 1 && Actor_Query_Goal_Number(9) != 2) {
				Actor_Face_Actor(9, 0, true);
				Actor_Says(9, 480, 13);
			} else if (!Game_Flag_Query(662) || Game_Flag_Query(369)) {
				Actor_Change_Animation_Mode(0, 23);
				Sound_Play(412, 100, 0, 0, 50);
			} else {
				Player_Loses_Control();
				Actor_Set_Goal_Number(23, 425);
				Game_Flag_Set(369);
				Game_Flag_Set(368);
				Obstacle_Object("OBSTACLE_HOLE", true);
				Unobstacle_Object("MONTE CARLO DRY", true);
				if (sub_4048C0()) {
					Loop_Actor_Walk_To_XYZ(sub_4048C0(), 181.54f, 40.63f, 388.09f, 0, 0, true, 0);
					Actor_Face_Actor(0, sub_4048C0(), true);
					Actor_Face_Actor(sub_4048C0(), 0, true);
					Actor_Says(0, 1785, 3);
					Actor_Says(0, 1790, 3);
				}
				Actor_Face_Heading(0, 0, false);
				Actor_Change_Animation_Mode(0, 23);
				Scene_Loop_Set_Default(5);
				Scene_Loop_Start_Special(2, 4, 1);
				if (sub_4048C0()) {
					if (sub_4048C0() == 3) {
						Actor_Face_Heading(3, 0, false);
						Ambient_Sounds_Play_Sound(147, 50, 99, 0, 0);
						Delay(3000);
						Actor_Face_Heading(3, 0, false);
						Actor_Change_Animation_Mode(3, 23);
					} else {
						Actor_Face_Heading(6, 0, false);
						Ambient_Sounds_Play_Sound(147, 50, 99, 0, 0);
						Delay(3000);
						Actor_Face_Heading(6, 0, false);
						Actor_Change_Animation_Mode(6, 13);
					}
					Actor_Face_Actor(0, sub_4048C0(), true);
					Actor_Says(0, 1805, 3);
				} else {
					ADQ_Flush();
					ADQ_Add(99, 940, -1);
					Ambient_Sounds_Play_Sound(147, 50, 99, 0, 0);
					Delay(1500);
					Loop_Actor_Walk_To_XYZ(0, 181.53999f, 40.630001f, 388.09f, 0, 0, true, 0);
					Actor_Face_Heading(0, 0, false);
					Actor_Change_Animation_Mode(0, 23);
					Actor_Clue_Lose(0, 146);
				}
				Player_Gains_Control();
			}
		}
		return true;
	}
	return false;
}

bool ScriptHF05::ClickedOnActor(int actorId) {
	if (actorId == 9) {
		if (!Loop_Actor_Walk_To_Actor(0, 9, 60, 1, false)) {
			Actor_Face_Actor(0, 9, true);
			Actor_Face_Actor(9, 0, true);
			sub_402AE4();
		}
	}
	return false;
}

bool ScriptHF05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptHF05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 483.0f, 40.63f, -189.0f, 0, 1, false, 0) && !Game_Flag_Query(684)) {
			Game_Flag_Set(313);
			Set_Enter(37, 34);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 330.0f, 40.63f, -85.0f, 0, 1, false, 0) && !Game_Flag_Query(684)) {
			int v2 = sub_404858();
			if (Game_Flag_Query(663) && Game_Flag_Query(368) && v2 != -1) {
				Actor_Face_Actor(0, v2, true);
				Actor_Says(0, 1810, 16);
			}
			Game_Flag_Set(359);
			Set_Enter(43, 40);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 277.0f, 40.631f, 410.0f, 0, 1, false, 0) && !Game_Flag_Query(684)) {
			Game_Flag_Set(529);
			Set_Enter(42, 39);
		}
		return true;
	}
	return false;
}

bool ScriptHF05::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptHF05::SceneFrameAdvanced(int frame) {
	switch (frame) {
	case 126:
		Sound_Play(352, 90, -20, 70, 50);
		break;
	case 152:
		Sound_Play(346, 90, 0, 0, 50);
		break;
	case 156:
		Sound_Play(348, 47, 100, 100, 50);
		break;
	case 161:
		Sound_Play(345, 90, 0, 0, 50);
		break;
	case 176:
		Sound_Play(350, 32, 100, 100, 50);
		break;
	case 178:
		Sound_Play(355, 47, 100, 100, 50);
		break;
	case 179:
		Sound_Play(490, 90, 0, 0, 50);
		Music_Play(1, 50, 0, 2, -1, 0, 0);
		break;
	case 186:
		Sound_Play(343, 32, 100, 100, 50);
		break;
	case 209:
		Sound_Play(353, 90, 100, -20, 50);
		break;
	case 243:
		Sound_Play(349, 40, -20, -20, 50);
		break;
	case 261:
		Sound_Play(344, 47, -20, -20, 50);
		break;
	case 268:
		Sound_Play(351, 58, -20, -20, 50);
		break;
	case 269:
		Sound_Play(354, 43, -20, -20, 50);
		break;
	}
	//return true;
}

void ScriptHF05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == 23 && newGoal == 430) {
		Game_Flag_Set(684);
		sub_4042E4();
	}
	//return false;
}

void ScriptHF05::PlayerWalkedIn() {
	if (Game_Flag_Query(662)) {
		int v0 = sub_404858();
		if (Game_Flag_Query(662) && v0 != -1) {
			Actor_Put_In_Set(v0, 41);
			Actor_Force_Stop_Walking(v0);
			if (Game_Flag_Query(312)) {
				Actor_Set_At_XYZ(v0, 506.81f, 40.63f, -140.92f, 0);
				Async_Actor_Walk_To_Waypoint(v0, 437, 36, 0);
			} else if (Game_Flag_Query(530)) {
				Actor_Set_At_XYZ(v0, 288.0f, 40.63f, 410.0f, 909);
			} else if (Game_Flag_Query(358)) {
				Actor_Set_At_XYZ(v0, 298.0f, 40.63f, -107.0f, 512);
			} else {
				Actor_Set_At_XYZ(v0, 284.0f, 40.63f, 286.0f, 0);
			}
		}
		if (Game_Flag_Query(684)) {
			sub_4042E4();
		} else if (Actor_Clue_Query(0, 265) || Game_Flag_Query(559)) {
			if (Game_Flag_Query(559) && !Game_Flag_Query(663)) {
				Game_Flag_Set(663);
				Music_Play(1, 40, 0, 2, -1, 0, 0);
				Actor_Says(24, 200, 3);
				Actor_Says(24, 210, 3);
				Actor_Set_Goal_Number(23, 420);
				if (sub_4048C0() == 3) {
					sub_403F0C();
				} else if (sub_4048C0() == 6) {
					sub_40410C();
				}
			}
		} else {
			sub_403A34(v0);
		}
	} else if (Game_Flag_Query(312) == 1) {
		Loop_Actor_Walk_To_XYZ(0, 399.0f, 40.63f, -85.0f, 0, 0, false, 0);
	} else if (Game_Flag_Query(358)) {
		Actor_Set_At_XYZ(0, 346.0f, 4.63f, -151.0f, 603);
		Loop_Actor_Travel_Stairs(0, 4, 1, 0);
	}
	if (Actor_Query_In_Set(9, 41)) {
		if (Game_Flag_Query(562)) {
			if (!Game_Flag_Query(563) && Global_Variable_Query(1) == 3) {
				sub_402970();
				Game_Flag_Set(563);
			}
		} else {
			sub_402370();
			Game_Flag_Set(562);
		}
	}
	Game_Flag_Reset(312);
	Game_Flag_Reset(530);
	Game_Flag_Reset(358);

	//return false;
}

void ScriptHF05::PlayerWalkedOut() {
	if (Actor_Query_Goal_Number(9) == 210) {
		Actor_Set_Goal_Number(9, 2);
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptHF05::DialogueQueueFlushed(int a1) {
}

void ScriptHF05::sub_402970() {
	Loop_Actor_Walk_To_Actor(9, 0, 72, 0, false);
	Actor_Face_Actor(9, 0, true);
	Actor_Face_Actor(0, 9, true);
	Actor_Says(9, 370, 3);
	Actor_Says(0, 1855, 3);
	Actor_Says(9, 380, 12);
	Actor_Says(9, 390, 14);
	Actor_Says(9, 400, 15);
	Actor_Says(9, 410, 16);
	Actor_Says(0, 1860, 3);
	Actor_Says(9, 420, 3);
	Actor_Says(0, 1865, 3);
}

void ScriptHF05::sub_402AE4() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 99) == 1 && Global_Variable_Query(1) == 3) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1180, 3, 6, 7);
	}
	if (Actor_Clue_Query(0, 116) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1190, 2, 7, 4);
	}
	if (Actor_Clue_Query(0, 88) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1200, 5, 5, 3);
	}
	if (Actor_Clue_Query(0, 13) == 1 && Actor_Query_Goal_Number(6) != 599) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1210, 4, 6, 2);
	}
	if (Actor_Clue_Query(0, 237) == 1 || (Actor_Clue_Query(0, 99) == 1 && Global_Variable_Query(1) == 3)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1220, -1, 2, 8);
	}
	if (Actor_Clue_Query(0, 113) == 1 || Actor_Clue_Query(0, 115) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1230, 4, 7, -1);
	}
	if (!Dialogue_Menu_Query_List_Size()) {
		Actor_Says(0, 1880, 15);
		Actor_Says(9, 490, 3);
		Actor_Says(0, 1885, 3);
		Actor_Says(9, 500, 16);
		return;
	}
	Dialogue_Menu_Add_DONE_To_List(1240);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 1180:
		Actor_Says(0, 1890, 23);
		Actor_Says(9, 510, 3);
		Actor_Says(0, 1920, 23);
		Actor_Says(0, 1925, 3);
		Actor_Says(9, 530, 12);
		Actor_Says(0, 1930, 18);
		Actor_Says(9, 540, 14);
		Actor_Says(0, 1935, 14);
		Actor_Says(9, 550, 16);
		Actor_Says(0, 1940, 15);
		Actor_Says(0, 1945, -1);
		Actor_Says(9, 560, 15);
		Actor_Says(9, 570, 16);
		Actor_Says(0, 1950, 17);
		sub_403738();
		break;
	case 1190:
		Actor_Says(0, 1895, 0);
		Actor_Says(9, 620, 3);
		Actor_Says(9, 630, 12);
		Actor_Says(0, 2000, 13);
		Actor_Says(9, 640, 14);
		Actor_Says(9, 650, 15);
		Actor_Says(9, 660, 16);
		Actor_Says(0, 2005, 0);
		Actor_Says(0, 2010, 3);
		Actor_Says(9, 670, 3);
		Actor_Says(9, 680, 12);
		Actor_Says(9, 690, 14);
		Actor_Says(0, 2015, 14);
		Actor_Says(9, 700, 15);
		Actor_Says(0, 2020, 18);
		break;
	case 1200:
		Actor_Says(0, 1900, 23);
		Actor_Says(9, 710, 16);
		Actor_Says(0, 2025, 0);
		Actor_Says(9, 720, 3);
		Actor_Says(9, 730, 12);
		break;
	case 1210:
		Actor_Says(0, 1905, 23);
		Actor_Says(9, 740, 14);
		Actor_Says(0, 2030, 13);
		Actor_Says(9, 750, 15);
		Actor_Says(0, 2035, 18);
		Actor_Says(9, 760, 16);
		Actor_Says(9, 770, 3);
		Actor_Says(0, 2040, 0);
		break;
	case 1220:
		Actor_Says(0, 1910, 3);
		Actor_Says(9, 780, 12);
		Actor_Says(0, 2045, 17);
		Actor_Says(0, 2050, 3);
		Actor_Says(9, 790, 14);
		Actor_Says(0, 2055, 19);
		Actor_Says(0, 2060, -1);
		Actor_Says(9, 800, 15);
		Actor_Says(0, 2065, 18);
		Actor_Says(0, 2070, 14);
		Actor_Says(9, 810, 16);
		sub_403738();
		break;
	case 1230:
		Actor_Says(0, 1915, 12);
		if (Actor_Clue_Query(0, 113)) {
			Actor_Says(9, 820, 3);
			Actor_Says(0, 2075, 13);
			Actor_Says(9, 830, 12);
			Actor_Says(9, 840, 14);
			Actor_Says(9, 850, 15);
			Actor_Says(0, 2080, 3);
			Actor_Says(9, 860, 16);
			Actor_Says(9, 870, 3);
		} else if (Actor_Clue_Query(0, 115)) {
			Actor_Says(9, 880, 12);
			Actor_Says(9, 890, 14);
			Actor_Says(0, 2085, 3);
			Actor_Says(9, 900, 15);
			Actor_Says(0, 2090, 19);
			Actor_Says(9, 910, 16);
			Actor_Says(0, 2095, 14);
			Actor_Says(9, 920, 3);
			Actor_Says(0, 2100, 15);
			Actor_Says(9, 930, 12);
			Actor_Says(9, 940, 14);
			Actor_Says(0, 2105, 3);
			Actor_Says(9, 950, 15);
			Actor_Says(0, 2110, 0);
			Actor_Says(9, 960, 16);
		}
		break;
	}
}

void ScriptHF05::sub_403738() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(1250, -1, -1, 10);
	DM_Add_To_List_Never_Repeat_Once_Selected(1260, 10, 5, -1);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	if (answer == 1250) {
		Actor_Says(0, 1955, 17);
		Actor_Says(0, 1960, 23);
		Item_Pickup_Spin_Effect(986, 315, 327);
		Delay(2000);
		Actor_Says(0, 1980, 23);
		Actor_Says(0, 1985, 3);
		Actor_Says(9, 580, 3);
		Actor_Says(9, 590, 15);
		Actor_Says(0, 1990, 17);
		Actor_Says(9, 600, 16);
		Actor_Says(0, 1995, 3);
		Game_Flag_Set(165);
		Actor_Put_In_Set(9, 67);
		Actor_Set_At_XYZ(9, -315.15f, 0.0f, 241.06f, 583);
		Actor_Set_Goal_Number(9, 699);
		Game_Flag_Set(652);
		if (Game_Flag_Query(255)) {
			Set_Enter(54, 54);
		} else {
			Game_Flag_Set(313);
			Set_Enter(37, 34);
		}
	} else if (answer == 1260) {
		Actor_Says(0, 1965, 12);
		Actor_Says(0, 1970, 3);
		Actor_Says(0, 1975, 3);
		Actor_Says(9, 610, 16);
	}
}

int ScriptHF05::sub_404858() {
	if (Global_Variable_Query(45) == 2 && Actor_Query_Goal_Number(3) != 599) {
		return 3;
	}
	if (Global_Variable_Query(45) == 3 && Actor_Query_Goal_Number(6) != 599) {
		return 6;
	}
	return -1;
}

void ScriptHF05::sub_4042E4() {
	Actor_Force_Stop_Walking(0);
	Actor_Put_In_Set(23, 41);
	Actor_Set_At_XYZ(23, 430.39999f, 40.630001f, -258.17999f, 300);
	Actor_Put_In_Set(24, 41);
	Actor_Set_At_XYZ(24, 526.40002f, 37.18f, -138.17999f, 300);
	ADQ_Flush();
	ADQ_Add(24, 260, -1);
	Player_Loses_Control();
	Non_Player_Actor_Combat_Mode_On(23, 3, 1, 0, 4, 4, 7, 8, 0, 0, 100, 100, 1200, 1);
	return Non_Player_Actor_Combat_Mode_On(24, 3, 1, 0, 4, 4, 7, 8, 0, 0, 100, 100, 300, 1);
}

void ScriptHF05::sub_403F0C() {
	Actor_Face_Actor(0, 3, true);
	Actor_Face_Actor(3, 0, true);
	Actor_Says(3, 2660, 12);
	Actor_Says(0, 8990, 3);
	Actor_Says(3, 2670, 13);
	Actor_Says(3, 2680, 17);
	Actor_Says(0, 8995, 14);
	Actor_Says(3, 2690, 15);
	Actor_Says_With_Pause(0, 9000, 1.0f, 16);
	Actor_Says_With_Pause(0, 9005, 1.0f, 19);
	Actor_Says(0, 1765, 17);
	Actor_Says(3, 160, 12);
	Actor_Says(0, 1770, 15);
	Actor_Says(0, 1775, 3);
	Actor_Says(3, 170, 3);
	Actor_Says_With_Pause(0, 1780, 1.0f, 18);
}

void ScriptHF05::sub_40410C() {
	Actor_Face_Actor(0, 6, true);
	Actor_Face_Actor(6, 0, true);
	Actor_Says(6, 400, 16);
	Actor_Says(0, 1750, 14);
	Actor_Says(6, 410, 12);
	Actor_Says(6, 420, 14);
	Actor_Says(0, 1755, 16);
	Actor_Says(6, 430, 18);
	Actor_Says_With_Pause(0, 1760, 1.0f, 15);
	Actor_Says(0, 1765, 17);
	Actor_Says(6, 440, 3);
	Actor_Says(0, 1770, 15);
	Actor_Says(0, 1775, 3);
	Actor_Says(6, 450, 17);
	Actor_Says_With_Pause(0, 1780, 1.0f, 18);
}

void ScriptHF05::sub_403A34(int actorId) {
	if (actorId != -1 && Actor_Query_In_Set(9, 41)) {
		Async_Actor_Walk_To_Waypoint(actorId, 437, 36, 0);
		Loop_Actor_Walk_To_Waypoint(0, 437, 0, 0, false);
		Actor_Face_Actor(9, 0, true);
		Actor_Face_Actor(0, 9, true);
		Actor_Face_Actor(actorId, 9, true);
		Actor_Says(9, 0, 3);
		Actor_Says(9, 10, 12);
		Actor_Says(0, 1715, 19);
		Actor_Says(0, 1720, -1);
		Actor_Says(9, 20, 14);
		Actor_Says(9, 30, 15);
		Actor_Says(0, 1725, 3);
		Actor_Says(9, 40, 16);
		Actor_Says(9, 50, 3);
		Actor_Says(9, 60, 12);
		Actor_Says(9, 70, 13);
		Actor_Says(0, 1730, 3);
		Loop_Actor_Walk_To_Actor(9, 0, 28, 0, false);
		Item_Pickup_Spin_Effect(986, 315, 327);
		Actor_Says(9, 80, 23);
		Actor_Clue_Acquire(0, 265, 1, 9);
		Actor_Says(9, 90, 15);
		Actor_Says(0, 1735, 17);
		Actor_Says(9, 100, 16);
		Actor_Says(9, 110, 3);
		Actor_Face_Actor(actorId, 0, true);
		if (actorId == 3) {
			Actor_Says(3, 90, 3);
		} else {
			Actor_Says(6, 380, 3);
		}
		Actor_Says(0, 1740, 14);
		Actor_Says(9, 120, 12);
		Actor_Set_Goal_Number(9, 2);
		if (actorId == 3) {
			Actor_Says(3, 100, 3);
		} else {
			Actor_Says(6, 390, 3);
		}
		Actor_Face_Actor(0, actorId, true);
		Actor_Says(0, 1745, 3);
		Async_Actor_Walk_To_XYZ(actorId, 309.0f, 40.63f, 402.0f, 0, false);
		Loop_Actor_Walk_To_XYZ(0, 277.0f, 40.63f, 410.0f, 0, 0, false, 0);
		Game_Flag_Set(529);
		Set_Enter(42, 39);
	}
}

void ScriptHF05::sub_402370() {
	Player_Loses_Control();
	if (Global_Variable_Query(1) == 3) {
		ADQ_Flush();
		ADQ_Add(9, 130, 18);
		ADQ_Add(9, 140, 18);
		ADQ_Add(9, 150, 18);
		ADQ_Add(9, 160, 17);
	}
	Loop_Actor_Walk_To_XYZ(0, 307.0f, 40.63f, 184.0f, 0, 0, false, 0);
	Loop_Actor_Walk_To_Actor(9, 0, 72, 0, false);
	Ambient_Sounds_Play_Sound(149, 99, 99, 0, 0);
	Actor_Face_Actor(9, 0, true);
	Actor_Face_Actor(0, 9, true);
	Actor_Says(9, 170, 3);
	Actor_Says(9, 180, 12);
	Actor_Says(9, 190, 14);
	Actor_Says(9, 200, 15);
	Actor_Says(0, 1815, 12);
	Actor_Says(9, 210, 16);
	Actor_Says(0, 1820, -1);
	Actor_Says(9, 220, 3);
	Actor_Says(9, 230, 12);
	Actor_Says(9, 240, 14);
	Actor_Says(0, 1825, 0);
	Actor_Says(9, 250, 15);
	Actor_Face_Object(9, "MONTE CARLO DRY", true);
	Actor_Says(9, 260, 16);
	Actor_Face_Object(0, "MONTE CARLO DRY", true);
	Actor_Says(0, 1830, 0);
	Actor_Face_Actor(9, 0, true);
	Actor_Face_Actor(0, 9, true);
	Actor_Says(9, 270, 3);
	Actor_Says(9, 280, 12);
	Async_Actor_Walk_To_XYZ(9, 276.0f, 40.63f, 182.0f, 12, false);
	Loop_Actor_Walk_To_XYZ(0, 335.0f, 40.63f, 131.0f, 12, 0, false, 0);
	Actor_Face_Object(9, "MONTE CARLO DRY", true);
	Actor_Face_Object(0, "MONTE CARLO DRY", true);
	Actor_Says(9, 290, 14);
	Actor_Says(9, 300, 15);
	Actor_Says(9, 310, 16);
	Actor_Says(0, 1835, 12);
	Actor_Face_Actor(9, 0, true);
	Actor_Says(9, 320, 3);
	Actor_Face_Actor(0, 9, true);
	Actor_Says(9, 330, 12);
	Actor_Says(0, 1840, 3);
	Actor_Says(9, 340, 14);
	Actor_Says(0, 1845, 3);
	Actor_Says(9, 350, 15);
	Actor_Says(9, 360, 16);
	Actor_Says(0, 1850, 3);
	Player_Gains_Control();
}

void ScriptHF05::sub_404474() {
	Ambient_Sounds_Add_Sound(87, 20, 80, 20, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(23, 250, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(23, 330, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(23, 340, 5, 90, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(23, 360, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(24, 380, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(24, 510, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(38, 80, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(38, 160, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(38, 280, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
}

int ScriptHF05::sub_4048C0() {
	if (Actor_Query_In_Set(3, 41) == 1 && Actor_Query_Goal_Number(3) != 599) {
		return 3;
	}
	if (Actor_Query_In_Set(6, 41) == 1 && Actor_Query_Goal_Number(6) != 599) {
		return 6;
	}
	return 0;
}

} // End of namespace BladeRunner
