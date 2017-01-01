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

void ScriptUG04::InitializeScene() {
	if (Game_Flag_Query(339)) {
		Setup_Scene_Information(0.0f, -1.74f, -2400.0f, 496);
		Game_Flag_Reset(339);
	} else if (Game_Flag_Query(341)) {
		Setup_Scene_Information(164.0f, 11.87f, -1013.0f, 83);
	} else {
		Setup_Scene_Information(-172.0f, 16.29f, -735.0f, 380);
		Game_Flag_Reset(334);
	}
	Scene_Exit_Add_2D_Exit(0, 123, 308, 159, 413, 3);
	if (Global_Variable_Query(1) > 3) {
		Scene_Exit_Add_2D_Exit(1, 256, 333, 290, 373, 0);
	}
	Scene_Exit_Add_2D_Exit(2, 344, 298, 451, 390, 1);
	Ambient_Sounds_Add_Looping_Sound(331, 25, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	Ambient_Sounds_Add_Sound(234, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(224, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(225, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(227, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(229, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(368, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(370, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(235, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(392, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(394, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Scene_Loop_Start_Special(0, 0, 0);
	Scene_Loop_Set_Default(1);
}

void ScriptUG04::SceneLoaded() {
	Obstacle_Object("NAV", true);
	Unobstacle_Object("RUBBLE", true);
	Unobstacle_Object("FLOOR DEBRIS WADS", true);
	Unobstacle_Object("FLOOR DEBRIS WADS01", true);
	Unobstacle_Object("FLOOR DEBRIS WADS02", true);
}

bool ScriptUG04::MouseClick(int x, int y) {
	return false;
}

bool ScriptUG04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptUG04::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptUG04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptUG04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -172.0f, 16.29f, -735.0f, 0, 1, false, 0)) {
			Game_Flag_Set(335);
			Set_Enter(76, 88);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 0.0f, -1.74f, -2400.0f, 0, 1, false, 0)) {
			Game_Flag_Set(338);
			Set_Enter(78, 90);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 164.0f, 11.87f, -1013.0f, 0, 1, false, 0)) {
			Game_Flag_Set(340);
			Set_Enter(79, 91);
		}
		return true;
	}
	return false;
}

bool ScriptUG04::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptUG04::SceneFrameAdvanced(int frame) {
	if (frame == 1) {
		Ambient_Sounds_Play_Sound(367, 90, -100, 100, 100);
	}
}

void ScriptUG04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptUG04::PlayerWalkedIn() {
	if (Game_Flag_Query(341)) {
		Loop_Actor_Walk_To_XYZ(0, 60.0f, -1.74f, -976.0f, 6, 0, false, 0);
		Game_Flag_Reset(341);
	}
}

void ScriptUG04::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptUG04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
