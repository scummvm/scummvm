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

void ScriptDR04::InitializeScene() {
	if (Game_Flag_Query(515)) {
		Setup_Scene_Information(0.0f, 0.0f, 0.0f, 0);
	} else if (Game_Flag_Query(10)) {
		Setup_Scene_Information(-711.0f, -0.04f, 70.0f, 472);
	} else if (Game_Flag_Query(229)) {
		Setup_Scene_Information(-1067.0f, 7.18f, 421.0f, 125);
	} else if (Game_Flag_Query(231)) {
		Setup_Scene_Information(-897.75f, 134.45f, 569.75f, 512);
	} else {
		Setup_Scene_Information(-810.0f, -0.04f, 242.0f, 125);
	}
	Scene_Exit_Add_2D_Exit(0, 589, 0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 443, 264, 488, 353, 0);
	Scene_Exit_Add_2D_Exit(2, 222, 110, 269, 207, 0);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(288, 55, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(217, 28, -100, 100);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(67, 40, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(66, 40, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(378, 5, 80, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(379, 5, 80, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(380, 5, 80, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(272)) {
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(4);
	}
	if (Game_Flag_Query(10)) {
		if (Game_Flag_Query(272)) {
			Scene_Loop_Start_Special(0, 0, 0);
		} else {
			Scene_Loop_Start_Special(0, 3, 0);
		}
	}
}

void ScriptDR04::SceneLoaded() {
	Obstacle_Object("TRASH CAN WITH FIRE", true);
	Obstacle_Object("V2PYLON02", true);
	Obstacle_Object("V2PYLON04", true);
	Obstacle_Object("U2 CHEWDOOR", true);
	Obstacle_Object("MMTRASHCAN", true);
	Obstacle_Object("PARKMETR02", true);
	Obstacle_Object("TRANSFORMER 01", true);
	Obstacle_Object("TRANSFORMER 02", true);
	Obstacle_Object("PARKMETR01", true);
	Obstacle_Object("Z2ENTRYDR", true);
	Obstacle_Object("Z2DR2", true);
	Unclickable_Object("PARKMETR01");
	Unclickable_Object("Z2ENTRYDR");
	Unclickable_Object("Z2DR2");
}

bool ScriptDR04::MouseClick(int x, int y) {
	return false;
}

bool ScriptDR04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptDR04::ClickedOnActor(int actorId) {
	if (actorId == 35 && !Player_Query_Combat_Mode()) {
		if (Actor_Query_Goal_Number(35) != 21) {
			if (Actor_Query_Goal_Number(35) == 23) {
				if (!Loop_Actor_Walk_To_Actor(0, 35, 36, 1, false)) {
					Actor_Set_Goal_Number(24, 106);
					return true;
				}
			}
			return false;
		}
		if (!Loop_Actor_Walk_To_Waypoint(0, 109, 0, 1, true)) {
			Actor_Face_Actor(0, 35, true);
			Actor_Says(0, 945, 13);
			Actor_Says(35, 0, 3);
			Actor_Says(35, 10, 3);
			Actor_Says(0, 950, 13);
			Actor_Says(35, 20, 3);
			Actor_Says(35, 30, 3);
			Actor_Says(0, 955, 13);
			Actor_Says_With_Pause(35, 40, 0, 3);
			Actor_Says(35, 50, 3);
			Actor_Clue_Acquire(0, 68, 1, 35);
			Actor_Set_Goal_Number(35, 22);
			Actor_Set_Goal_Number(24, 101);
			return true;
		}
		return false;
	}
	return false;
}

bool ScriptDR04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptDR04::ClickedOnExit(int exitId) {
	if (Actor_Query_Goal_Number(35) == 21) {
		Actor_Force_Stop_Walking(0);
		Actor_Set_Goal_Number(35, 22);
		Actor_Set_Goal_Number(24, 101);
		return true;
	}
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -761.0f, -0.04f, 97.0f, 0, 1, false, 0)) {
			Async_Actor_Walk_To_XYZ(0, -683.0f, -0.04f, 43.0f, 0, false);
			Game_Flag_Set(11);
			Set_Enter(7, 25);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -1067.0f, 7.18f, 421.0f, 0, 1, false, 0)) {
			Game_Flag_Set(232);
			Game_Flag_Set(228);
			Set_Enter(35, 29);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, -851.0f, 6.98f, 560.0f, 0, 1, false, 0)) {
			Footstep_Sound_Override_On(3);
			Actor_Set_Immunity_To_Obstacles(0, true);
			Actor_Face_Heading(0, 512, false);
			Loop_Actor_Travel_Stairs(0, 7, 1, 0);
			Loop_Actor_Walk_To_XYZ(0, -899.0f, 71.64f, 647.0f, 0, 0, false, 0);
			Actor_Face_Heading(0, 0, false);
			Loop_Actor_Travel_Stairs(0, 7, 1, 0);
			Actor_Set_Immunity_To_Obstacles(0, false);
			Footstep_Sound_Override_Off();
			Game_Flag_Set(230);
			Set_Enter(36, 30);
		}
		return true;
	}
	return true; //bug?
}

bool ScriptDR04::ClickedOn2DRegion(int region) {
	return false;
}

bool ScriptDR04::sub_401160() {
	float x, y, z;
	Actor_Query_XYZ(0, &x, &y, &z);
	return (x + 1089.94f) * (x + 1089.94f) + (z - 443.49f) * (z - 443.49f) >= (360.0f * 360.0f);
}

void ScriptDR04::SceneFrameAdvanced(int frame) {
	if (Game_Flag_Query(515)) {
		Game_Flag_Reset(515);
		Game_Flag_Reset(271);
		Scene_Loop_Set_Default(1);
		Scene_Loop_Start_Special(2, 6, 1);
		Music_Stop(4);
		Actor_Set_Goal_Number(35, 99);
	} else {
		if (Game_Flag_Query(271)) {
			Game_Flag_Reset(271);
			Game_Flag_Set(272);
			Scene_Loop_Set_Default(1);
			Scene_Loop_Start_Special(2, 6, 1);
			Item_Remove_From_World(78);
		}
		switch (frame) {
		case 193:
			Sound_Play(301, 100, 0, 100, 50);
			Actor_Set_Goal_Number(35, 30);
			Player_Loses_Control();
			Actor_Force_Stop_Walking(0);
			if (sub_401160()) {
				if (Player_Query_Combat_Mode()) {
					Actor_Change_Animation_Mode(0, 22);
				} else {
					Actor_Change_Animation_Mode(0, 21);
				}
			} else {
				Sound_Play_Speech_Line(0, 9905, 100, 0, 99);
				Actor_Change_Animation_Mode(0, 48);
				Actor_Retired_Here(0, 6, 6, 1, -1);
			}
			Player_Gains_Control();
			break;
		case 235:
			if (Actor_Query_Goal_Number(35) != 20 && Actor_Query_Goal_Number(35) != 21 && Actor_Query_Goal_Number(35) != 99) {
				Actor_Set_Goal_Number(24, 101);
			}
			Scene_Exits_Enable();
			break;
		case 237:
			Overlay_Play("DR04OVER", 0, 1, 1, 0);
			break;
		}
	}
}

void ScriptDR04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptDR04::PlayerWalkedIn() {
	if (Game_Flag_Query(515)) {
		Player_Loses_Control();
		Delay(4000);
		Actor_Retired_Here(0, 6, 6, 1, -1);
	} else {
		if (Game_Flag_Query(269) && !Game_Flag_Query(272)) {
			Scene_Exits_Disable();
		}
		if (Game_Flag_Query(231)) {
			Footstep_Sound_Override_On(3);
			Actor_Set_Immunity_To_Obstacles(0, true);
			Actor_Face_Heading(0, 512, false);
			Loop_Actor_Travel_Stairs(0, 7, 0, 0);
			Loop_Actor_Walk_To_XYZ(0, -851.0f, 71.64f, 647.0f, 0, 0, false, 0);
			Actor_Face_Heading(0, 0, false);
			Loop_Actor_Travel_Stairs(0, 7, 0, 0);
			Loop_Actor_Walk_To_XYZ(0, -774.85f, 7.18f, 386.67001f, 0, 0, false, 0);
			Actor_Set_Immunity_To_Obstacles(0, false);
			Footstep_Sound_Override_Off();
		}
	}
	Game_Flag_Reset(10);
	Game_Flag_Reset(229);
	Game_Flag_Reset(231);
}

void ScriptDR04::PlayerWalkedOut() {
	Music_Stop(2);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptDR04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
