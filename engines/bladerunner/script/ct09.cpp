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

void ScriptCT09::InitializeScene() {
	if (Game_Flag_Query(85)) {
		Setup_Scene_Information(160.0f, 349.0f, 587.0f, 490);
	} else if (Game_Flag_Query(81)) {
		Setup_Scene_Information(235.0f, 3348.52f, 599.0f, 800);
	} else {
		Setup_Scene_Information(107.0f, 348.52f, 927.0f, 200);
	}
	Scene_Exit_Add_2D_Exit(0, 321, 164, 345, 309, 1);
	Scene_Exit_Add_2D_Exit(1, 0, 0, 15, 479, 3);
	Scene_Exit_Add_2D_Exit(2, 198, 177, 263, 311, 0);
	Ambient_Sounds_Add_Looping_Sound(336, 28, 0, 1);
	Ambient_Sounds_Add_Sound(375, 6, 180, 33, 33, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 5, 180, 33, 33, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 5, 180, 33, 33, 0, 0, -101, -101, 0, 0);
}

void ScriptCT09::SceneLoaded() {
	Obstacle_Object("PHONE01", true);
	Unobstacle_Object("MAINBEAM01", true);
	Unobstacle_Object("MIDDLE WALL", true);
	Clickable_Object("BELL");
}

bool ScriptCT09::MouseClick(int x, int y) {
	return false;
}

bool ScriptCT09::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("BELL", objectName)) {
		if (Actor_Query_Which_Set_In(62) != 31) {
			if (!Loop_Actor_Walk_To_XYZ(0, 229.0f, 348.52f, 851.0f, 36, 1, false, 0)) {
				Actor_Face_Object(0, "BELL", true);
				Sound_Play(337, 100, 0, 0, 50);
				if (!Actor_Query_Goal_Number(27)) {
					Actor_Says(27, 160, 3);
				}
			}
		}
		return true;
	}
	return false;
}

bool ScriptCT09::ClickedOnActor(int actorId) {
	if (actorId == 27) {
		if (!Actor_Query_Goal_Number(27) && Actor_Query_Which_Set_In(62) != 31) {
			if (!Loop_Actor_Walk_To_XYZ(0, 270.0f, 348.52f, 846.0f, 12, 1, false, 0)) {
				Player_Loses_Control();
				Actor_Face_Actor(0, 27, true);
				if (Global_Variable_Query(1) < 3) {
					Actor_Says(0, 650, 3);
					Actor_Says(27, 250, 12);
					Actor_Says(0, 665, 18);
				} else if (Game_Flag_Query(540)) {
					Actor_Says(0, 650, 18);
					Actor_Says(27, 220, 15);
				} else {
					Game_Flag_Set(540);
					Actor_Says(27, 170, 13);
					Actor_Says(0, 630, 12);
					Actor_Says(27, 180, 14);
					Actor_Says(0, 635, 3);
					Actor_Says(27, 190, 15);
					Actor_Says(0, 640, 12);
					Actor_Says(0, 645, 3);
					Actor_Says(27, 200, 13);
					Actor_Says(27, 210, 14);
				}
				Player_Gains_Control();
			}
		}
		return true;
	}
	return false;
}

bool ScriptCT09::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptCT09::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 206.0f, 348.52f, 599.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(0, 235.0f, 348.52f, 599.0f, 0, 0, false, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(84);
			Set_Enter(6, 20);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 107.0f, 348.52f, 927.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(83);
			Set_Enter(33, 23);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 159.0f, 349.0f, 570.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(82);
			Set_Enter(32, 22);
		}
		return true;
	}
	return false;;
}

bool ScriptCT09::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptCT09::SceneFrameAdvanced(int frame) {
	if (frame == 6 || frame == 12 || frame == 19 || frame == 25 || frame == 46 || frame == 59) {
		Sound_Play(97, Random_Query(47, 47), 70, 70, 50);
	}
}

void ScriptCT09::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptCT09::PlayerWalkedIn() {
	bool v0 = false;
	if (Global_Variable_Query(1) == 3 && !Game_Flag_Query(538)) {
		Game_Flag_Set(538);
		Actor_Set_Goal_Number(62, 1);
		v0 = true;
	}
	if (Game_Flag_Query(85)) {
		Game_Flag_Reset(85);
	} else if (Game_Flag_Query(81)) {
		if (v0) {
			Async_Actor_Walk_To_XYZ(0, 206.0f, 348.52f, 599.0f, 0, false);
		} else {
			Loop_Actor_Walk_To_XYZ(0, 206.0f, 348.52f, 599.0f, 0, 0, false, 0);
		}
		Game_Flag_Reset(81);
	} else {
		if (v0) {
			Async_Actor_Walk_To_XYZ(0, 124.0f, 348.52f, 886.0f, 0, false);
		} else {
			Loop_Actor_Walk_To_XYZ(0, 124.0f, 348.52f, 886.0f, 0, 0, false, 0);
		}
		Game_Flag_Reset(304);
	}
	if (Actor_Query_Goal_Number(27) == 2) {
		if (Game_Flag_Query(539)) {
			Actor_Says(27, 70, 13);
			Actor_Face_Actor(0, 27, true);
			Actor_Says(0, 600, 17);
			Actor_Says(27, 80, 14);
			Actor_Says(0, 605, 13);
			Actor_Says(27, 90, 15);
		} else {
			Actor_Says(27, 20, 12);
			Actor_Face_Actor(0, 27, true);
			Actor_Says(0, 585, 18);
			Actor_Says(27, 40, 15);
			Actor_Says(0, 590, 16);
			Actor_Says(27, 50, 14);
			Actor_Says(0, 595, 14);
			Actor_Says(27, 60, 13);
			Actor_Modify_Friendliness_To_Other(27, 0, -1);
		}
		Actor_Set_Goal_Number(27, 0);
	}
}

void ScriptCT09::PlayerWalkedOut() {
}

void ScriptCT09::DialogueQueueFlushed(int a1) {
	Actor_Force_Stop_Walking(0);
	if (Actor_Query_Goal_Number(62) == 1 && !Game_Flag_Query(539)) {
		Player_Loses_Control();
		Actor_Set_Goal_Number(62, 2);
		//return true;
	} else {
		//return false;
	}
}

} // End of namespace BladeRunner
