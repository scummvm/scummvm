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

void ScriptRC03::InitializeScene() {
	if (Game_Flag_Query(115) ) {
		Setup_Scene_Information(298.0f, -4.0f, 405.0f, 800);
		Game_Flag_Reset(115);
	} else if (Game_Flag_Query(117) ) {
		Setup_Scene_Information(-469.0f, -4.0f, 279.0f, 250);
	} else if (Game_Flag_Query(119) ) {
		Setup_Scene_Information(147.51f, -4.0f, 166.48f, 500);
		if (!Game_Flag_Query(151)) {
			Game_Flag_Set(151);
		}
	} else if (Game_Flag_Query(107) ) {
		Setup_Scene_Information(-487.0f, 1.0f, 116.0f, 400);
	} else if (Game_Flag_Query(121) ) {
		Setup_Scene_Information(-22.0f, 1.0f, -63.0f, 400);
	} else {
		Setup_Scene_Information(0.0f, 0.0f, 0.0f, 0);
	}
	Scene_Exit_Add_2D_Exit(0, 610, 0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
	if (Game_Flag_Query(151) ) {
		Scene_Exit_Add_2D_Exit(2, 524, 350, 573, 359, 2);
	}
	Scene_Exit_Add_2D_Exit(3, 85, 255, 112, 315, 0);
	Scene_Exit_Add_2D_Exit(4, 428, 260, 453, 324, 0);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 0, 1);
	Ambient_Sounds_Add_Sound(82, 5, 30, 40, 70, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(83, 5, 30, 40, 75, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(84, 5, 30, 40, 70, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(68, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
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
	if (Game_Flag_Query(107)  && Actor_Query_Goal_Number(7) != 102) {
		Scene_Loop_Start_Special(0, 0, 0);
	}
	Scene_Loop_Set_Default(1);
}

void ScriptRC03::SceneLoaded() {
	Obstacle_Object("Box-Streetlight01", true);
	Obstacle_Object("Box-Streetlight02", true);
	Obstacle_Object("Parking Meter 01", true);
	Obstacle_Object("Parking Meter 02", true);
	Obstacle_Object("Parking Meter 03", true);
	Obstacle_Object("Trash can with fire", true);
	Obstacle_Object("Baricade01", true);
	Obstacle_Object("Foreground Junk01", true);
	Obstacle_Object("Steam01", true);
	Obstacle_Object("Steam02", true);
	Obstacle_Object("Box-BBcolumn01", true);
	Obstacle_Object("Box-BBcolumn02", true);
	Obstacle_Object("Box-BBcolumn03", true);
	Obstacle_Object("Box-BBcolumn04", true);
	Obstacle_Object("Box-BBbuilding01", true);
	Obstacle_Object("Box-BBbuilding02", true);
	Obstacle_Object("Box-BBbuilding03", true);
	Obstacle_Object("Box-BBbuilding04", true);
	Unclickable_Object("BOX-BBBUILDING01");
	Unclickable_Object("BOX-BBBUILDING02");
	Unclickable_Object("BOX-BBBUILDING03");
	Unclickable_Object("BOX-BBBUILDING04");
	Unclickable_Object("BOX-STREETLIGHT01");
	Unclickable_Object("BOX-STREETLIGHT02");
	Unclickable_Object("BOX-BBCOLUMN01");
	Unclickable_Object("BOX-BBCOLUMN02");
	Unclickable_Object("BOX-BBCOLUMN03");
	Unclickable_Object("BOX-BBCOLUMN04");
	Unclickable_Object("PARKING METER 02");
	Unclickable_Object("PARKING METER 03");
	Unclickable_Object("TRASH CAN WITH FIRE");
	Unclickable_Object("BARICADE01");
	Unclickable_Object("FOREGROUND JUNK01");
}

bool ScriptRC03::MouseClick(int x, int y) {
	return false;
}

bool ScriptRC03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptRC03::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptRC03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptRC03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 298.0f, -4.0f, 405.0f, 0, 1, false, 0)) {
			if (Game_Flag_Query(289)) {
				Game_Flag_Set(702);
			}
			Game_Flag_Set(114);
			Set_Enter(69, 78);
			Actor_Set_Goal_Number(3, 100);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -469.0f, -4.0f, 279.0f, 0, 1, false, 0)) {
			if (Game_Flag_Query(289)) {
				Game_Flag_Set(702);
			}
			Game_Flag_Set(116);
			Game_Flag_Reset(182);
			Game_Flag_Set(180);
			Set_Enter(0, 1);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 147.51f, -4.0f, 166.48f, 0, 1, false, 0)) {
			Game_Flag_Set(118);
			Game_Flag_Reset(182);
			Game_Flag_Set(259);
			if (Game_Flag_Query(289)) {
				Game_Flag_Set(702);
			}
			Set_Enter(74, 86);
			Actor_Set_Goal_Number(3, 100);
		}
		return true;
	}
	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(0, -487.0f, 1.0f, 116.0f, 0, 1, false, 0)) {
			Game_Flag_Set(108);
			Game_Flag_Reset(182);
			Game_Flag_Set(479);
			if (Game_Flag_Query(289)) {
				Game_Flag_Set(702);
			}
			Set_Enter(8, 106);
			Actor_Set_Goal_Number(3, 100);
		}
		return true;
	}
	if (exitId == 4) {
		if (!Loop_Actor_Walk_To_XYZ(0, -22.0f, 1.0f, -63.0f, 0, 1, false, 0)) {
			if (Global_Variable_Query(1) == 3 || Global_Variable_Query(1) == 5 || Game_Flag_Query(702)) {
				Actor_Says(0, 8522, 14);
			} else {
				Game_Flag_Set(120);
				Set_Enter(71, 81);
			}
		}
		return true;
	}
	return false;
}

bool ScriptRC03::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptRC03::SceneFrameAdvanced(int frame) {
	if (frame == 1) {
		Sound_Play(286, Random_Query(33, 33), 100, -100, 50);
	}
	if (frame == 15) {
		Sound_Play(287, Random_Query(50, 50), -100, 100, 50);
	}
}

void ScriptRC03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptRC03::sub_402834() {
	Actor_Face_Actor(1, 0, true);
	Actor_Says(1, 1820, 3);
	Actor_Face_Actor(0, 1, true);
	Actor_Says(0, 4815, 14);
	Actor_Says(1, 1830, 3);
	Actor_Says(1, 1840, 3);
	Actor_Says(0, 4820, 12);
	Actor_Says(1, 1850, 3);
	Actor_Says(1, 1950, 3);
	Actor_Says(0, 4835, 14);
	Actor_Says(1, 1960, 3);
	Actor_Says(1, 1980, 3);
	Actor_Says(0, 4840, 15);
	Actor_Says(1, 1990, 3);
	Actor_Says(1, 2000, 3);
}

void ScriptRC03::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(7) == 102) {
		Scene_Exits_Disable();
		if (Game_Flag_Query(119) ) {
			Player_Set_Combat_Mode(false);
			Player_Loses_Control();
			Actor_Set_At_XYZ(0, 147.51f, -4.0f, 166.48f, 500);
			Actor_Put_In_Set(7, 70);
			Actor_Set_At_XYZ(7, 196.0f, -4.0f, 184.0f, 775);
			Actor_Face_Actor(7, 0, true);
			Actor_Face_Actor(0, 7, true);
			Actor_Change_Animation_Mode(7, 4);
			Actor_Says_With_Pause(7, 630, 0, -1);
			Actor_Says_With_Pause(7, 640, 0, -1);
			Actor_Says_With_Pause(7, 650, 0, -1);
			if (Game_Flag_Query(44) ) {
				Actor_Set_Goal_Number(1, 100);
			}
			Actor_Change_Animation_Mode(0, 20);
			Loop_Actor_Walk_To_XYZ(7, 180.0f, -4.0f, 184.0f, 0, 0, false, 0);
			Actor_Change_Animation_Mode(7, 6);
			if (!Game_Flag_Query(44)) {
				Actor_Set_Goal_Number(1, 100);
			}
			Player_Gains_Control();
		} else {
			Actor_Put_In_Set(7, 70);
			Actor_Set_At_XYZ(7, -226.0f, 1.72f, 86.0f, 0);
			Actor_Set_Targetable(7, true);
			Actor_Set_Goal_Number(7, 110);
		}
	}
	if (Actor_Query_Goal_Number(7) == 103) {
		Player_Loses_Control();
		Actor_Set_Goal_Number(1, 200);
		Actor_Put_In_Set(1, 70);
		if (Game_Flag_Query(119)  || Game_Flag_Query(121) ) {
			Actor_Set_At_Waypoint(1, 175, 0);
		} else {
			Actor_Set_At_Waypoint(1, 203, 0);
		}
		sub_402834();
		Async_Actor_Walk_To_Waypoint(1, 174, 0, 0);
		Actor_Set_Goal_Number(7, 200);
		Player_Gains_Control();
	}
	Game_Flag_Reset(119);
	Game_Flag_Reset(117);
	Game_Flag_Reset(107);
	Game_Flag_Reset(121);
	if (Global_Variable_Query(1) == 1 || Global_Variable_Query(1) == 2) {
		Actor_Set_Goal_Number(3, 103);
	}
}

void ScriptRC03::PlayerWalkedOut() {
	if (Actor_Query_Goal_Number(7) == 199) {
		Actor_Set_Goal_Number(7, 198);
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptRC03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
