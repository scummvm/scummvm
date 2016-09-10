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

void ScriptPS14::InitializeScene() {
	if (Game_Flag_Query(134)) {
		Setup_Scene_Information(-1119.61f, 508.14f, -1208.22f, 315);
		Game_Flag_Reset(134);
	} else {
		Setup_Scene_Information(-785.45f, 508.14f, -1652.0f, 315);
	}
	Scene_Exit_Add_2D_Exit(0, 610, 0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 46, 51, 125, 192, 0);
	Ambient_Sounds_Add_Looping_Sound(381, 100, 1, 1);
	Ambient_Sounds_Add_Sound(374, 100, 300, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(68, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
}

void ScriptPS14::SceneLoaded() {
	Obstacle_Object("CABLES UPPER RIGHT", true);
	Unobstacle_Object("CYLINDER63", true);
	Clickable_Object("CABLES UPPER RIGHT");
	Unclickable_Object("CABLES UPPER RIGHT");
}

bool ScriptPS14::MouseClick(int x, int y) {
	return false;
}

bool ScriptPS14::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptPS14::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptPS14::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptPS14::ClickedOnExit(int exitId) {
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -2101.0f, 508.14f, -1361.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 819, false);
			Loop_Actor_Travel_Stairs(0, 3, 1, 0);
			if (Global_Variable_Query(1) == 4 && Game_Flag_Query(671)) {
				if (Actor_Clue_Query(0, 32)) {
					Game_Flag_Set(666);
					Actor_Set_Goal_Number(0, 400);
				} else {
					Actor_Set_Goal_Number(0, 500);
				}
			} else if (Global_Variable_Query(1) > 3) {
				Actor_Says(0, 8522, 12);
				Actor_Face_Heading(0, 307, false);
				Loop_Actor_Travel_Stairs(0, 3, 0, 0);
			} else {
				Game_Flag_Set(135);
				Set_Enter(63, 67);
			}
		}
		return true;
	}
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -785.45f, 508.14f, -1652.0f, 0, 1, false, 0)) {
			Game_Flag_Set(673);
			Game_Flag_Reset(178);
			Game_Flag_Set(179);
			Set_Enter(53, 53);
		}
		return true;
	}
	return false;
}

bool ScriptPS14::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptPS14::SceneFrameAdvanced(int frame) {
}

void ScriptPS14::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptPS14::PlayerWalkedIn() {
	if (Game_Flag_Query(672)) {
		Loop_Actor_Walk_To_XYZ(0, -801.45f, 508.14f, -1596.68f, 0, 0, false, 0);
		Game_Flag_Reset(672);
	}
	//return false;
}

void ScriptPS14::PlayerWalkedOut() {
}

void ScriptPS14::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
