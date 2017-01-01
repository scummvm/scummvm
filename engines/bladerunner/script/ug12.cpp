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

void ScriptUG12::InitializeScene() {
	if (Game_Flag_Query(411)) {
		Setup_Scene_Information(207.0f, -126.21f, -364.0f, 561);
		Game_Flag_Reset(411);
	} else {
		Setup_Scene_Information(375.0f, -126.21f, 180.0f, 730);
		Game_Flag_Reset(345);
	}
	Scene_Exit_Add_2D_Exit(0, 538, 222, 615, 346, 1);
	if (Game_Flag_Query(373)) {
		Scene_Exit_Add_2D_Exit(1, 334, 176, 426, 266, 0);
	}
	Ambient_Sounds_Add_Looping_Sound(105, 47, 60, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	Ambient_Sounds_Add_Sound(291, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(292, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(368, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(370, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(294, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(373)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void ScriptUG12::SceneLoaded() {
	Unobstacle_Object("GATE1", true);
	Obstacle_Object("TRAIN WRECK", true);
}

bool ScriptUG12::MouseClick(int x, int y) {
	return false;
}

bool ScriptUG12::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptUG12::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptUG12::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptUG12::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 375.0f, -126.21f, 180.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(344);
			Set_Enter(86, 98);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 207.0f, -126.21f, -364.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(412);
			Set_Enter(45, 42);
		}
		return true;
	}
	return false;
}

bool ScriptUG12::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptUG12::SceneFrameAdvanced(int frame) {
}

void ScriptUG12::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptUG12::PlayerWalkedIn() {
}

void ScriptUG12::PlayerWalkedOut() {
}

void ScriptUG12::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
