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

void ScriptCT06::InitializeScene() {
	if (Game_Flag_Query(77)) {
		Setup_Scene_Information(20.41f, -58.23f, 2.17f, 247);
		Game_Flag_Reset(77);
	} else if (Game_Flag_Query(144)) {
		Setup_Scene_Information(203.91f, -58.02f, 0.47f, 768);
	} else {
		Setup_Scene_Information(175.91f, -58.23f, 24.47f, 768);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 440, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 401, 162, 536, 317, 0);
	if (Game_Flag_Query(40) && Actor_Query_In_Set(19, 30)) {
		Actor_Put_In_Set(19, 29);
		Actor_Set_At_XYZ(19, 58.41f, -58.23f, -24.97f, 240);
		Actor_Retired_Here(19, 72, 36, 1, 0);
	}
	Ambient_Sounds_Add_Looping_Sound(381, 100, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(205, 20, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(67, 80, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(68, 50, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(379, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(380, 70, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Actor_Query_Goal_Number(19) == 13) {
		Ambient_Sounds_Add_Sound(196, 1, 5, 25, 25, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(197, 1, 5, 25, 25, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(198, 1, 5, 25, 25, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(199, 1, 5, 25, 25, -100, 100, -101, -101, 0, 0);
	}
}

void ScriptCT06::SceneLoaded() {
	Obstacle_Object("BOX02", true);
	Obstacle_Object("CB BOX01", true);
	Obstacle_Object("CB BOX02", true);
	Obstacle_Object("CB BOX03", true);
	Unobstacle_Object("INSULPIP01", true);
	Unobstacle_Object("CB BOX04", true);
	Unclickable_Object("DOOR");
	if (Actor_Query_Goal_Number(19) == 13) {
		Preload(3);
		Preload(4);
		Preload(389);
		Preload(390);
		Preload(398);
		Preload(421);
		Preload(421);
	}
}

bool ScriptCT06::MouseClick(int x, int y) {
	return false;
}

bool ScriptCT06::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptCT06::ClickedOnActor(int actorId) {
	if (actorId == 19) {
		Loop_Actor_Walk_To_Actor(0, 19, 24, 1, false);
		Actor_Face_Actor(0, 19, true);
		if (Game_Flag_Query(145)) {
			Actor_Says(0, 8570, 13);
			return false;
		}
		Actor_Clue_Acquire(0, 20, 1, -1);
		Item_Pickup_Spin_Effect(984, 340, 369);
		Actor_Voice_Over(350, 99);
		Actor_Voice_Over(360, 99);
		Actor_Voice_Over(370, 99);
		if (!Game_Flag_Query(378)) {
			Actor_Voice_Over(380, 99);
			Actor_Voice_Over(390, 99);
			Actor_Voice_Over(400, 99);
			Actor_Voice_Over(410, 99);
		}
		Game_Flag_Set(145);
		return true;
	}
	return false;
}

bool ScriptCT06::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptCT06::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 20.41f, -58.23f, -2.17f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(78);
			Set_Enter(28, 17);
			Game_Flag_Reset(212);
		}
		return true;
	}
	if (exitId == 1) {
		if (Actor_Query_Goal_Number(19) == 13) {
			if (!Loop_Actor_Walk_To_XYZ(0, 203.91f, -58.02f, 0.47f, 0, 1, false, 0)) {
				Ambient_Sounds_Remove_Sound(196, true);
				Ambient_Sounds_Remove_Sound(197, true);
				Ambient_Sounds_Remove_Sound(198, true);
				Ambient_Sounds_Remove_Sound(199, true);
				Player_Loses_Control();
				Actor_Set_Goal_Number(19, 11);
				Game_Flag_Reset(212);
			}
			return true;
		}
		if (!Loop_Actor_Walk_To_XYZ(0, 203.91f, -58.02f, 0.47f, 0, 1, false, 0)) {
			if (Global_Variable_Query(1) < 3) {
				Actor_Face_Object(0, "DOOR", true);
				Actor_Says(0, 8522, 12);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(79);
				Set_Enter(6, 20);
				Game_Flag_Reset(212);
			}
		}
	}
	return false;
}

bool ScriptCT06::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptCT06::SceneFrameAdvanced(int frame) {
}

void ScriptCT06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptCT06::PlayerWalkedIn() {
	if (Game_Flag_Query(144) == 1) {
		Game_Flag_Reset(144);
	}
}

void ScriptCT06::PlayerWalkedOut() {
}

void ScriptCT06::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
