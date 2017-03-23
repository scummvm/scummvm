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

#include "bladerunner/script/scene.h"

namespace BladeRunner {

void SceneScriptUG18::InitializeScene() {
	Setup_Scene_Information(-684.71f, 0.0f, 171.59f, 0);
	Game_Flag_Reset(434);
	Scene_Exit_Add_2D_Exit(0, 0, 158, 100, 340, 3);
	Ambient_Sounds_Add_Looping_Sound(105, 71, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(95, 45, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 76, 0, 1);
	Ambient_Sounds_Add_Sound(291, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(292, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(294, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(402, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(368, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(397, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(398, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 2, 20, 25, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(1, 5, 50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(57, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(58, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(196, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(197, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(198, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(199, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Scene_Loop_Set_Default(4);
	if (Game_Flag_Query(670) && !Game_Flag_Query(671) && Global_Variable_Query(1) == 4) {
		Actor_Set_Goal_Number(4, 300);
		Actor_Set_Goal_Number(5, 300);
		Actor_Set_Goal_Number(8, 300);
	}
}

void SceneScriptUG18::SceneLoaded() {
	Obstacle_Object("MACHINE_01", true);
	Unobstacle_Object("PLATFM_RAIL 01", true);
	Unobstacle_Object("PLATFM_RAIL 02", true);
	Unobstacle_Object("OBSTACLE1", true);
	Clickable_Object("MACHINE_01");
	Unclickable_Object("MACHINE_01");
	if (Game_Flag_Query(671)) {
		Actor_Put_In_Set(4, 99);
		Actor_Set_At_Waypoint(4, 41, 0);
		if (Actor_Query_Which_Set_In(8) == 89) {
			Actor_Put_In_Set(8, 91);
			Actor_Set_At_Waypoint(8, 33, 0);
		}
	}
	if (Game_Flag_Query(670) && !Game_Flag_Query(671) && Global_Variable_Query(1) == 4) {
		Item_Add_To_World(91, 987, 89, -55.21f, 0.0f, -302.17f, 0, 12, 12, false, true, false, true);
	}

}

bool SceneScriptUG18::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG18::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG18::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG18::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 91) {
		if (a2) {
			Item_Remove_From_World(91);
		} else if (!Loop_Actor_Walk_To_Item(0, 91, 12, 1, false)) {
			Item_Pickup_Spin_Effect(987, 368, 243);
			Item_Remove_From_World(itemId);
			Game_Flag_Set(703);
			Actor_Clue_Acquire(0, 32, 1, 4);
		}
	}
	return false;
}

bool SceneScriptUG18::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -684.712f, 0.0f, 171.59f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(435);
			Set_Enter(85, 97);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG18::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG18::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG18::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == 4) {
		if (newGoal == 303) {
			Game_Flag_Set(607);
			ADQ_Flush();
			Actor_Modify_Friendliness_To_Other(5, 0, 7);
			Actor_Modify_Friendliness_To_Other(8, 0, 10);
			Player_Loses_Control();
			Actor_Face_Actor(4, 0, true);
			ADQ_Add(4, 1220, 58);
			Scene_Exits_Enable();
			Actor_Set_Goal_Number(4, 305);
		} else if (newGoal == 304) {
			ADQ_Flush();
			Actor_Modify_Friendliness_To_Other(5, 0, 7);
			Actor_Modify_Friendliness_To_Other(8, 0, 10);
			Player_Loses_Control();
			Actor_Face_Actor(4, 0, true);
			ADQ_Add(4, 1220, 58);
			Scene_Exits_Enable();
			Actor_Set_Goal_Number(4, 306);
		}
	} else if (actorId == 8) {
		if (newGoal == 302) {
			if (Actor_Query_Friendliness_To_Other(5, 0) > 55 && Game_Flag_Query(607)) {
				sub_403588();
			} else {
				Actor_Set_Goal_Number(8, 307);
				Actor_Set_Goal_Number(5, 310);
			}
		} else if (newGoal == 304) {
			Actor_Modify_Friendliness_To_Other(5, 0, -3);
			ADQ_Add(8, 380, -1);
			Actor_Set_Goal_Number(8, 306);
		} else if (newGoal == 305) {
			Actor_Change_Animation_Mode(8, 6);
			Sound_Play(12, 100, 0, 0, 50);
			Actor_Force_Stop_Walking(0);
			Actor_Change_Animation_Mode(0, 48);
			Player_Loses_Control();
			Actor_Retired_Here(0, 6, 6, 1, 8);
		}
	}
}

void SceneScriptUG18::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(0, -488.71f, 0.0f, 123.59f, 0, 0, false, 0);
	if (Game_Flag_Query(670) && !Game_Flag_Query(671) && Actor_Query_Is_In_Current_Set(4)) {
		Scene_Exits_Disable();
		sub_402734();
		sub_403278();
		Game_Flag_Set(671);
	}
}

void SceneScriptUG18::PlayerWalkedOut() {
}

void SceneScriptUG18::DialogueQueueFlushed(int a1) {
	int v0 = Actor_Query_Goal_Number(4);
	if (v0 == 301) {
		Actor_Set_Goal_Number(4, 302);
		Actor_Change_Animation_Mode(8, 6);
		Sound_Play(14, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(4, 22);
		ADQ_Add(5, 630, 13);
		Actor_Set_Goal_Number(5, 301);
	} else if (v0 == 305) {
		Actor_Change_Animation_Mode(0, 6);
		Sound_Play(13, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(4, 22);
		Delay(900);
		Actor_Change_Animation_Mode(0, 6);
		Sound_Play(14, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(4, 22);
		Delay(1100);
		Actor_Change_Animation_Mode(0, 6);
		Sound_Play(12, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(4, 22);
		Delay(900);
		Actor_Change_Animation_Mode(0, 6);
		Sound_Play(14, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(4, 61);
		Overlay_Play("UG18over", 1, 0, 1, 0);
		Actor_Set_Goal_Number(4, 307);
		Player_Gains_Control();
		ADQ_Add_Pause(2000);
		ADQ_Add(8, 360, -1);
		ADQ_Add_Pause(2000);
		ADQ_Add(5, 650, 14);
		ADQ_Add(8, 370, 14);
		ADQ_Add(5, 1320, 14);
		Actor_Set_Goal_Number(5, 303);
	} else if (v0 == 306) {
		Actor_Change_Animation_Mode(4, 6);
		Sound_Play(13, 100, 0, 0, 50);
		Actor_Force_Stop_Walking(0);
		Actor_Change_Animation_Mode(0, 48);
		Player_Loses_Control();
		Actor_Retired_Here(0, 6, 6, 1, 4);
		Actor_Set_Goal_Number(4, 307);
	}

	int v1 = Actor_Query_Goal_Number(5);
	if (v1 == 301) {
		Actor_Change_Animation_Mode(8, 6);
		Sound_Play(14, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(4, 22);
		ADQ_Add(5, 640, 13);
		ADQ_Add(4, 1210, 13);
		Actor_Set_Goal_Number(5, 302);
	} else if (v1 == 302) {
		Actor_Change_Animation_Mode(8, 6);
		Sound_Play(14, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(4, 61);
		ADQ_Add_Pause(2000);
		ADQ_Add(5, 650, 14);
		ADQ_Add(8, 370, 14);
		ADQ_Add(5, 1320, 14);
		Actor_Set_Goal_Number(4, 390);
		Actor_Retired_Here(4, 72, 32, 1, 8);
		Actor_Set_Goal_Number(5, 303);
		Scene_Exits_Enable();
	} else if (v1 == 303) {
		Actor_Set_Goal_Number(8, 301);
	}
	if (Actor_Query_Goal_Number(8) == 306) {
		Actor_Change_Animation_Mode(8, 48);
		Actor_Set_Goal_Number(8, 307);
		Actor_Set_Goal_Number(5, 310);
	}
}

void SceneScriptUG18::sub_402734() {
	Actor_Face_Actor(0, 4, true);
	Actor_Says(0, 5860, 9);
	Delay(500);
	Actor_Face_Actor(4, 0, true);
	Delay(500);
	Actor_Says(4, 790, 3);
	Actor_Says(0, 5865, 12);
	Actor_Says(4, 800, 3);
	Loop_Actor_Walk_To_XYZ(0, -357.13f, 0.0f, -44.47f, 0, 0, false, 0);
	Actor_Face_Actor(0, 4, true);
	Actor_Says(0, 5870, 14);
	Actor_Face_Actor(4, 0, true);
	Actor_Start_Speech_Sample(4, 810);
	Loop_Actor_Walk_To_XYZ(4, -57.21f, 0.0f, -334.17f, 0, 0, false, 0);
	Actor_Says(0, 5875, 13);
	Actor_Says(4, 830, 3);
	Actor_Says(4, 840, 12);
	Actor_Says(4, 850, 14);
	Actor_Says(4, 860, 13);
	Actor_Says(0, 5880, 15);
	Actor_Says(0, 5885, 9);
	Actor_Says(0, 5890, 13);
	Actor_Says(4, 870, 15);
	Loop_Actor_Walk_To_XYZ(0, -205.13f, 0.0f, -184.47f, 0, 0, false, 0);
	Actor_Face_Actor(0, 4, true);
	Actor_Says(0, 5900, 15);
	Actor_Says(4, 880, 13);
	Actor_Says(0, 5905, 9);
	Actor_Says(0, 5910, 12);
	Actor_Says(0, 5915, 13);
	Actor_Says(4, 890, 16);
	Actor_Says(0, 5920, 14);
	Loop_Actor_Walk_To_XYZ(4, -57.21f, 0.0f, -334.17f, 0, 0, false, 0);
	Actor_Face_Actor(4, 0, true);
	Actor_Says(4, 900, 15);
	Actor_Says(4, 910, 12);
	Actor_Says(4, 920, 16);
	Actor_Says(0, 5925, 14);
	Actor_Says(4, 940, 14);
	Actor_Says(0, 5930, 18);
	Actor_Says(4, 950, 14);
	Actor_Says(4, 960, 13);
	Actor_Says(4, 970, 3);
	if (Game_Flag_Query(607)) {
		Actor_Modify_Friendliness_To_Other(5, 0, 3);
		Actor_Modify_Friendliness_To_Other(8, 0, 5);
		Loop_Actor_Walk_To_XYZ(0, -117.13f, 0.0f, -284.47f, 0, 0, false, 0);
		Actor_Face_Actor(0, 4, true);
		Actor_Says(0, 5960, 9);
		Actor_Says(0, 5965, 14);
		Actor_Says(4, 980, 15);
		Actor_Says(4, 990, 13);
		Actor_Says(0, 5970, 14);
		Actor_Says(4, 1000, 3);
		Actor_Says(0, 5975, 15);
	} else {
		sub_402DE8();
	}
}

void SceneScriptUG18::sub_402DE8() {

	if (Player_Query_Agenda()) {
		if (Global_Variable_Query(45) > 1 || Player_Query_Agenda() == 2) {
			sub_403114();
		} else {
			sub_402F8C();
		}
	} else {
		Actor_Modify_Friendliness_To_Other(5, 0, -1);
		Actor_Modify_Friendliness_To_Other(8, 0, -1);
		Actor_Says(0, 5935, 14);
		Actor_Says(0, 5940, 18);
		Actor_Says(4, 1020, 13);
		Actor_Says(4, 1030, 14);
	}
}

void SceneScriptUG18::sub_402F8C() {
	Loop_Actor_Walk_To_XYZ(0, -117.13f, 0.0f, -284.47f, 0, 0, false, 0);
	Actor_Face_Actor(0, 4, true);
	Actor_Says(0, 5945, 12);
	Actor_Says(4, 1040, 15);
	Actor_Says(0, 5980, 15);
	Actor_Says(4, 1050, 12);
	Actor_Says(4, 1060, 13);
	Actor_Says(4, 1070, 14);
	Actor_Says(0, 5985, 18);
	Actor_Says(4, 1080, 3);
	Actor_Says(4, 1090, 14);
	Actor_Says(4, 1100, 13);
}

void SceneScriptUG18::sub_403114() {
	Actor_Modify_Friendliness_To_Other(5, 0, 20);
	Actor_Modify_Friendliness_To_Other(8, 0, 10);
	Loop_Actor_Walk_To_XYZ(0, -117.13f, 0.0f, -284.47f, 0, 0, false, 0);
	Actor_Face_Actor(0, 4, true);
	Actor_Says(0, 5950, 16);
	Actor_Says(0, 5955, 14);
	Actor_Says(4, 1110, 13);
	Actor_Says(4, 1120, 15);
	Actor_Says(0, 5990, 3);
	Actor_Says(4, 1130, 15);
	Actor_Says(4, 1140, 16);
}

void SceneScriptUG18::sub_403278() {
	ADQ_Flush();
	Actor_Start_Speech_Sample(5, 590);
	Delay(500);
	Loop_Actor_Walk_To_XYZ(4, 126.79f, 0.0f, -362.17f, 0, 0, false, 0);
	Actor_Face_Heading(4, 729, false);
	Actor_Set_Goal_Number(4, 301);
	ADQ_Add(8, 350, 13);
	ADQ_Add_Pause(1500);
	ADQ_Add(4, 1150, 58);
	ADQ_Add(5, 600, 13);
	ADQ_Add_Pause(1000);
	ADQ_Add(4, 1160, 60);
	ADQ_Add_Pause(500);
	ADQ_Add(4, 1170, 59);
	ADQ_Add(4, 1180, 58);
	ADQ_Add(5, 610, 13);
	ADQ_Add(4, 1190, 60);
	ADQ_Add(5, 620, 13);
	ADQ_Add(4, 1200, 59);
}

void SceneScriptUG18::sub_403588() {
	Actor_Says(5, 660, 13);
	Actor_Says(0, 5995, 13);
	Actor_Says(5, 670, 13);
	Actor_Says(0, 6000, 13);
	Actor_Says_With_Pause(5, 680, 2.0f, 13);
	Actor_Says(5, 690, 13);
	Actor_Says(5, 700, 13);
	Actor_Set_Goal_Number(8, 310);
	Actor_Set_Goal_Number(5, 310);
}

} // End of namespace BladeRunner
