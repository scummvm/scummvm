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

void ScriptMA04::InitializeScene() {
	if (Game_Flag_Query(63)) {
		Setup_Scene_Information(-7199.0f, 953.97f, 1579.0f, 502);
		if (Global_Variable_Query(1) != 2 && Global_Variable_Query(1) != 3) {
			Scene_Loop_Start_Special(0, 0, 0);
		}
	} else if (Game_Flag_Query(35)) {
		Setup_Scene_Information(-7099.0f, 954.0f, 1866.0f, 502);
	} else if (Game_Flag_Query(647)) {
		Setup_Scene_Information(-7107.0f, 954.0f, 1742.0f, 502);
		Scene_Loop_Start_Special(0, 4, 0);
	} else {
		Setup_Scene_Information(-7143.0f, 954.0f, 1868.0f, 733);
	}
	Scene_Exit_Add_2D_Exit(0, 496, 0, 639, 354, 1);
	Scene_Exit_Add_2D_Exit(1, 33, 63, 113, 258, 0);
	Scene_Exit_Add_2D_Exit(2, 248, 98, 314, 284, 1);
	Scene_2D_Region_Add(0, 343, 97, 353, 190);
	Scene_2D_Region_Add(1, 0, 340, 116, 479);
	Ambient_Sounds_Add_Looping_Sound(408, 30, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(103, 30, -80, 1);
	Ambient_Sounds_Add_Looping_Sound(104, 12, 0, 1);
	Ambient_Sounds_Add_Sound(72, 5, 30, 11, 11, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(73, 5, 30, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 30, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 10, 60, 20, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 10, 60, 20, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(87, 10, 60, 16, 16, -100, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(68, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	if (sub_402758()) {
		Ambient_Sounds_Add_Sound(403, 3, 3, 100, 100, 0, 0, 0, 0, 99, 0);
	}
	Scene_Loop_Set_Default(1);
}

void ScriptMA04::SceneLoaded() {
	Obstacle_Object("BED-DOG DISH", true);
	Unobstacle_Object("BEDDog BONE", true);
	Unobstacle_Object("BED-BOOK1", true);
	Clickable_Object("BED-SHEETS");
	if (Game_Flag_Query(711)) {
		Unclickable_Object("BED-TV-1");
		Unclickable_Object("BED-TV-2");
	} else {
		Clickable_Object("BED-TV-1");
		Clickable_Object("BED-TV-2");
	}
}

bool ScriptMA04::MouseClick(int x, int y) {
	return false;
}

bool ScriptMA04::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("BED-SHEETS", objectName)) {
		sub_403DA8();
		return false;
	}
	if (Object_Query_Click("BED-TV-1", objectName) || Object_Query_Click("BED-TV-2", objectName)) {
		if (!Loop_Actor_Walk_To_Scene_Object(0, "BED-TV-2", 24, 1, false)) {
			Game_Flag_Set(711);
			Unclickable_Object("BED-TV-1");
			Unclickable_Object("BED-TV-2");
			Sound_Play(132, 100, 0, 0, 50);
			sub_403864();
			return false;
		}
		return true;
	}
	return false;
}

bool ScriptMA04::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptMA04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptMA04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -7099.0f, 954.0f, 1866.0f, 0, 1, false, 0)) {
			Game_Flag_Set(36);
			Set_Enter(10, 49);
		}
		return true;
	}
	if (exitId == 1) {
		float x, y, z;
		Actor_Query_XYZ(0, &x, &y, &z);
		if (z <= 1677.0f || !Loop_Actor_Walk_To_XYZ(0, -7199.0f, 955.0f, 1675.0f, 0, 1, false, 0)) {
			if (sub_402888()) {
				Overlay_Remove("MA04OVER");
			}
			Loop_Actor_Walk_To_XYZ(0, -7199.0f, 955.0f, 1675.0f, 0, 0, false, 1);
			Game_Flag_Set(62);
			if (Global_Variable_Query(1) != 2 && Global_Variable_Query(1) != 3) {
				Async_Actor_Walk_To_XYZ(0, -7199.0f, 956.17f, 1568.0f, 0, false);
			}
			Set_Enter(51, 51);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, -7115.0f, 954.0f, 1742.0f, 0, 1, false, 0)) {
			int sounds[] = {252, 405, 404, 407, 406};
			Ambient_Sounds_Play_Sound(sounds[Random_Query(0, 4)], 50, 0, 0, 0);
			Delay(3000);
			Loop_Actor_Walk_To_XYZ(0, -7139.0f, 954.0f, 1746.0f, 0, 1, false, 1);
		}
	}
	return false;
}

bool ScriptMA04::ClickedOn2DRegion(int region) {
	if (Player_Query_Combat_Mode()) {
		return false;
	}
	if (region == 1) {
		sub_403DA8();
		return true;
	}
	if (region == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -7176.0f, 954.0f, 1806.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 256, false);
			if (sub_402758()) {
				Actor_Says(0, 2680, 0);
				Ambient_Sounds_Remove_Sound(403, true);
				Sound_Play(123, 100, 0, 0, 50);
				Overlay_Remove("MA04OVER");
				Delay(500);
				if (Game_Flag_Query(653)) {
					if (Global_Variable_Query(45) == 2) {
						sub_4028A8();
					} else if (Global_Variable_Query(45) == 3) {
						sub_402F2C();
					} else {
						sub_4034D8();
					}
				} else {
					sub_4032A0();
				}
				Music_Play(2, 52, 0, 3, -1, 0, 0);
				return false;
			}
			if (Actor_Clue_Query(5, 222) && !Game_Flag_Query(649)) {
				Sound_Play(123, 100, 0, 0, 50);
				Overlay_Remove("MA04OVER");
				Delay(500);
				Actor_Says(5, 310, 3);
				Actor_Says(5, 320, 3);
				if (!Game_Flag_Query(378) && Global_Variable_Query(1) < 3) {
					Actor_Voice_Over(1300, 99);
					Actor_Voice_Over(1310, 99);
					Actor_Voice_Over(1320, 99);
				}
				Actor_Says(0, 2445, 13);
				Sound_Play(123, 100, 0, 0, 50);
				Game_Flag_Set(649);
				return true;
			}
			if (Actor_Clue_Query(6, 215) && !Game_Flag_Query(650)) {
				Sound_Play(123, 100, 0, 0, 50);
				Overlay_Remove("MA04OVER");
				Delay(500);
				Actor_Says(6, 500, 3);
				Actor_Says(6, 510, 3);
				if (!Game_Flag_Query(378) && Global_Variable_Query(1) < 3) {
					Actor_Voice_Over(1330, 99);
					Actor_Voice_Over(1340, 99);
					Actor_Voice_Over(1350, 99);
				}
				Actor_Says(0, 2445, 13);
				Sound_Play(123, 100, 0, 0, 50);
				Game_Flag_Set(650);
				return true;
			}
			Actor_Says(0, 2670, 13);
			if (!Game_Flag_Query(378)) {
				Actor_Says(0, 2675, 17);
			}
		}
		return true;
	}
	return false;
}

void ScriptMA04::SceneFrameAdvanced(int frame) {
	Set_Fade_Color(0, 0, 0);
	if (frame >= 91 && frame < 121) {
		Set_Fade_Density((frame - 91) / 30.0f);
	} else if (frame >= 121 && frame < 151) {
		Set_Fade_Density((151 - frame) / 30.0f);
	} else {
		Set_Fade_Density(0.0f);
	}
	if (frame == 121 && (Game_Flag_Query(40) == 1 || Game_Flag_Query(41) == 1) && !Game_Flag_Query(159)) {
		Sound_Play(403, 50, 0, 0, 50);
	}
}

void ScriptMA04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptMA04::PlayerWalkedIn() {
	if (Game_Flag_Query(647)) {
		Player_Gains_Control();
	}
	if (sub_402820() || sub_402758()) {
		Overlay_Play("MA04OVER", 0, 1, 0, 0);
	}
	if (Game_Flag_Query(647)) {
		Loop_Actor_Walk_To_XYZ(0, -7139.0f, 954.0f, 1746.0f, 0, 1, false, 0);
	} else if (Game_Flag_Query(35)) {
		Loop_Actor_Walk_To_XYZ(0, -7143.0f, 954.0f, 1868.0f, 0, 1, false, 0);
	}
	Game_Flag_Reset(35);
	Game_Flag_Reset(63);
	Game_Flag_Reset(647);
	if (Game_Flag_Query(61)) {
		if (Global_Variable_Query(1) == 2 && !Actor_Clue_Query(0, 43)) {
			Sound_Play(403, 100, 0, 0, 50);
			Loop_Actor_Walk_To_XYZ(0, -7176.0f, 954.0f, 1806.0f, 0, 0, false, 0);
			Actor_Face_Heading(0, 256, true);
			Actor_Says(0, 2680, 0);
			Sound_Play(123, 100, 0, 0, 50);
			Delay(500);
			Actor_Says(4, 0, 3);
			Actor_Says(0, 2685, 13);
			Actor_Says(4, 10, 3);
			Actor_Says(0, 2690, 17);
			Actor_Says(4, 30, 3);
			Actor_Says(0, 2695, 12);
			Actor_Says(4, 40, 3);
			Actor_Says(4, 50, 3);
			Actor_Says(0, 2700, 3);
			Actor_Says(4, 60, 3);
			Actor_Says(4, 70, 3);
			Sound_Play(123, 100, 0, 0, 50);
			Actor_Clue_Acquire(0, 43, 1, 4);
			Spinner_Set_Selectable_Destination_Flag(5, 1);
			Game_Flag_Set(186);
			if (!Game_Flag_Query(163)) {
				Game_Flag_Set(163);
				Item_Remove_From_World(66);
			}
			Actor_Set_Goal_Number(23, 99);
			Actor_Put_In_Set(23, 93);
			Actor_Set_At_Waypoint(23, 35, 0);
			Autosave_Game(0);
		}
		//return false;
		return;
	}
	if ((Game_Flag_Query(40) || Game_Flag_Query(41)) && !Game_Flag_Query(146)) {
		Music_Play(2, 52, 0, 2, -1, 0, 0);
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(0, -7199.0f, 955.0f, 1677.0f, 0, 1, false, 0);
		if (sub_402820() || sub_402758()) {
			Overlay_Remove("MA04OVER");
		}
		Loop_Actor_Walk_To_XYZ(0, -7199.0f, 955.0f, 1675.0f, 0, 1, false, 0);
		Game_Flag_Set(146);
		Async_Actor_Walk_To_XYZ(0, -7204.0f, 956.17f, 1568.0f, 0, false);
		Set_Enter(51, 51);
	}
}

void ScriptMA04::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (Game_Flag_Query(678)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(1, 0, -1);
		Game_Flag_Reset(678);
	}
}

void ScriptMA04::DialogueQueueFlushed(int a1) {
	Overlay_Remove("MA04OVR2");
}

bool ScriptMA04::sub_402758() {
	return Global_Variable_Query(1) == 5 && !Actor_Clue_Query(0, 143) && !Actor_Clue_Query(0, 144) && !Actor_Clue_Query(0, 139) && !Actor_Clue_Query(0, 140) && !Actor_Clue_Query(0, 141) && !Actor_Clue_Query(0, 142);
}

bool ScriptMA04::sub_402820() {
	return (Actor_Clue_Query(5, 222) && !Game_Flag_Query(649)) || (Actor_Clue_Query(6, 215) && !Game_Flag_Query(650));
}

bool ScriptMA04::sub_402888() {
	return sub_402820() || sub_402758();
}

void ScriptMA04::sub_4028A8() {
	int answer;
	Actor_Says(3, 220, 3);
	Actor_Says(0, 2460, 0);
	Actor_Says(3, 230, 3);
	Actor_Says(3, 240, 3);
	Actor_Says(0, 2465, 0);
	Actor_Says(3, 250, 3);
	Actor_Says_With_Pause(0, 2470, 1.5f, 17);
	Actor_Says(3, 260, 3);
	Actor_Says(0, 2475, 15);
	Actor_Says(3, 270, 3);
	Actor_Says(0, 2480, 0);
	Actor_Says(3, 280, 3);
	Actor_Says(3, 290, 3);
	Actor_Says(0, 2485, 19);
	Actor_Says(3, 300, 3);
	Actor_Says(3, 310, 3);
	Actor_Says(0, 2490, 0);
	Actor_Says(3, 330, 3);
	Actor_Says(0, 2495, 0);
	Actor_Says(3, 340, 3);
	Actor_Says(3, 350, 3);
	if (Game_Flag_Query(165) || Actor_Query_Goal_Number(9) == 2) {
		answer = 1170;
	} else {
		Dialogue_Menu_Clear_List();
		DM_Add_To_List_Never_Repeat_Once_Selected(1160, 1, 1, 2);
		DM_Add_To_List_Never_Repeat_Once_Selected(1170, 2, 1, 1);
		Dialogue_Menu_Appear(320, 240);
		answer = Dialogue_Menu_Query_Input();
		Dialogue_Menu_Disappear();
	}
	if (answer == 1160) {
		Actor_Says(0, 2500, 19);
		Actor_Says(3, 360, 3);
		Actor_Says(0, 2510, 0);
		Actor_Says(3, 370, 3);
		Actor_Says(3, 380, 3);
		Actor_Says(0, 2515, 12);
		Actor_Says(3, 390, 3);
		Actor_Says(0, 2520, 13);
		Actor_Says(3, 400, 3);
		Actor_Says(3, 410, 3);
		Actor_Says(0, 2525, 15);
		Actor_Says(3, 420, 3);
		Sound_Play(123, 100, 0, 0, 50);
		Actor_Clue_Acquire(0, 139, 1, -1);
	} else {
		Actor_Says_With_Pause(0, 2505, 0.5f, 19);
		Actor_Says(3, 430, 3);
		Actor_Says(3, 440, 3);
		Actor_Says(0, 2530, 0);
		Actor_Says(3, 450, 3);
		Actor_Says(0, 2535, 12);
		Actor_Says(3, 460, 3);
		Actor_Says_With_Pause(3, 470, 1.0f, 3);
		Actor_Says(3, 480, 3);
		Actor_Says(3, 490, 3);
		Sound_Play(123, 100, 0, 0, 50);
		Actor_Says(0, 2540, 15);
		Actor_Clue_Acquire(0, 140, 1, -1);
	}
}

void ScriptMA04::sub_402F2C() {
	Actor_Says(6, 530, 3);
	Actor_Says(0, 2545, 19);
	Actor_Says(6, 540, 3);
	Actor_Says(6, 550, 3);
	Actor_Says(0, 2550, 13);
	Actor_Says(6, 560, 3);
	Actor_Says(0, 2555, 19);
	Actor_Says(6, 570, 3);
	Actor_Says(0, 2560, 17);
	Actor_Says(6, 580, 3);
	if (Game_Flag_Query(165) || Actor_Query_Goal_Number(9) == 2) {
		Actor_Says(6, 630, 3);
		Actor_Says_With_Pause(0, 2575, 0.0f, 15);
		if (!Game_Flag_Query(378)) {
			Actor_Says(6, 640, 3);
		}
		Actor_Clue_Acquire(0, 142, 1, -1);
	} else {
		Actor_Says(6, 590, 3);
		Actor_Says(0, 2565, 12);
		Actor_Says(6, 600, 3);
		Actor_Says(6, 610, 3);
		Actor_Says(6, 620, 3);
		Actor_Says(0, 2570, 13);
		Actor_Says_With_Pause(6, 630, 0.0f, 3);
		Actor_Says_With_Pause(0, 2575, 0.0f, 15);
		if (!Game_Flag_Query(378)) {
			Actor_Says(6, 640, 3);
		}
		Actor_Clue_Acquire(0, 141, 1, -1);
	}
	Sound_Play(123, 100, 0, 0, 50);
}

void ScriptMA04::sub_4032A0() {
	Actor_Says(1, 680, 3);
	Actor_Says(0, 2630, 17);
	Actor_Says(1, 690, 3);
	Actor_Says(0, 2635, 18);
	Actor_Says(1, 700, 3);
	Actor_Says(0, 2640, 14);
	Actor_Says(1, 710, 3);
	Actor_Says(1, 720, 3);
	Actor_Says(0, 2645, 13);
	Actor_Says(1, 740, 3);
	Actor_Says(1, 750, 3);
	Actor_Says(0, 2650, 12);
	Actor_Says(1, 760, 3);
	Actor_Says(0, 2665, 13);
	Actor_Says(1, 810, 3);
	Actor_Says(1, 820, 3);
	Sound_Play(123, 100, 0, 0, 50);
	Actor_Clue_Acquire(0, 144, 1, -1);
}

void ScriptMA04::sub_4034D8() {
	Actor_Says(5, 330, 3);
	Actor_Says(0, 2580, 14);
	Actor_Says(5, 340, 3);
	Actor_Says(0, 2585, 19);
	Actor_Says(5, 350, 3);
	Actor_Says(5, 360, 3);
	Actor_Says(0, 2590, 18);
	Actor_Says(5, 370, 3);
	Actor_Says(0, 2595, 15);
	Actor_Says(5, 390, 3);
	Actor_Says(5, 400, 3);
	Actor_Says(5, 410, 3);
	Actor_Says(0, 2600, 15);
	Actor_Says_With_Pause(5, 420, 1.5f, 3);
	Actor_Says(0, 2605, 17);
	Actor_Says(5, 430, 3);
	Actor_Says(5, 440, 3);
	Actor_Says(0, 2610, 3);
	Actor_Says(5, 450, 3);
	Actor_Says(5, 460, 3);
	Actor_Says(5, 470, 3);
	Actor_Says(5, 480, 3);
	Actor_Says(5, 490, 3);
	Actor_Says(0, 2615, 17);
	Actor_Says(5, 500, 3);
	Actor_Says(5, 530, 3);
	Actor_Says(5, 540, 3);
	Sound_Play(123, 100, 0, 0, 50);
	Actor_Clue_Acquire(0, 143, 1, -1);
}

void ScriptMA04::sub_403864() {
	Overlay_Play("MA04OVR2", 0, 1, 0, 0);
	switch (Global_Variable_Query(52)) {
	case 4:
		ADQ_Add(61, 230, 3);
		ADQ_Add(61, 240, 3);
		break;
	case 3:
		ADQ_Add(61, 170, 3);
		ADQ_Add(61, 180, 3);
		ADQ_Add(61, 190, 3);
		ADQ_Add(61, 200, 3);
		ADQ_Add(61, 210, 3);
		ADQ_Add(61, 220, 3);
		ADQ_Add(41, 80, 3);
		ADQ_Add(41, 90, 3);
		ADQ_Add(41, 100, 3);
		ADQ_Add(41, 110, 3);
		ADQ_Add(41, 120, 3);
		ADQ_Add(41, 130, 3);
		break;
	case 2:
		if (Actor_Query_Friendliness_To_Other(5, 0) <= Actor_Query_Friendliness_To_Other(1, 0)) {
			ADQ_Add(61, 90, 3);
			ADQ_Add(61, 100, 3);
			ADQ_Add(61, 110, 3);
			ADQ_Add(4, 1540, 3);
			ADQ_Add(4, 1550, 3);
			ADQ_Add(4, 1560, 3);
		} else {
			ADQ_Add(61, 120, 3);
			ADQ_Add(61, 130, 3);
			ADQ_Add(61, 140, 3);
			ADQ_Add(61, 150, 3);
			ADQ_Add(4, 1570, 3);
			ADQ_Add(4, 1580, 3);
			ADQ_Add(4, 1590, 3);
		}
		break;
	case 1:
		ADQ_Add(61, 40, 3);
		ADQ_Add(61, 50, 3);
		ADQ_Add(61, 60, 3);
		ADQ_Add(61, 70, 3);
		ADQ_Add(61, 80, 3);
		break;
	case 0:
		ADQ_Add(61, 0, 3);
		ADQ_Add(61, 10, 3);
		ADQ_Add(61, 20, 3);
		ADQ_Add(61, 30, 3);
		ADQ_Add(51, 430, 3);
		ADQ_Add(51, 440, 3);
		ADQ_Add(51, 450, 3);
		ADQ_Add(51, 460, 3);
		break;
	}
}

void ScriptMA04::sub_403DA8() {
	if (!Loop_Actor_Walk_To_Scene_Object(0, "BED-SHEETS", 12, 1, false)) {
		Actor_Says(0, 8530, 12);
		Music_Stop(4);
		if (sub_402820() || sub_402758()) {
			Overlay_Remove("MA04OVER");
		}
		Player_Loses_Control();
		Game_Flag_Set(647);
		if ((Game_Flag_Query(40) || Game_Flag_Query(41)) && Global_Variable_Query(1) == 1) {
			if (Actor_Query_Goal_Number(19) == 599) {
				Actor_Put_In_Set(19, 91);
				Actor_Set_At_Waypoint(19, 33, 0);
			}
			Game_Flag_Set(678);
			Global_Variable_Set(1, 2);
			Chapter_Enter(2, 10, 50);
			if (Query_Difficulty_Level() != 0) {
				if (!Game_Flag_Query(723)) {
					Global_Variable_Increment(2, 200);
				}
			}
		} else {
			Set_Enter(10, 50);
		}
		Scene_Loop_Start_Special(1, 3, 0);
	}
}

} // End of namespace BladeRunner
