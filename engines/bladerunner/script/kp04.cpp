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

void ScriptKP04::InitializeScene() {
	if (Game_Flag_Query(575)) {
		Setup_Scene_Information(-544.0f, 94.89f, 288.0f, 700);
	} else {
		Setup_Scene_Information(-905.0f, 94.89f, 1357.0f, 970);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 455, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 475, 247, 514, 416, 1);
	Ambient_Sounds_Add_Looping_Sound(464, 34, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(383, 27, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(384, 90, 1, 1);
	Ambient_Sounds_Add_Sound(440, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(441, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(442, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(443, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
}

void ScriptKP04::SceneLoaded() {
	Obstacle_Object("BUILDING04", true);
	Unobstacle_Object("BOX06", true);
	Unclickable_Object("BUILDING04");
}

bool ScriptKP04::MouseClick(int x, int y) {
	return false;
}

bool ScriptKP04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptKP04::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptKP04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptKP04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -905.0f, 94.89f, 1357.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(416);
			Set_Enter(44, 41);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -544.0f, 94.89f, 288.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(574);
			Set_Enter(9, 45);
		}
		return true;
	}
	return false;
}

bool ScriptKP04::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptKP04::SceneFrameAdvanced(int frame) {
}

void ScriptKP04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptKP04::PlayerWalkedIn() {
	if (Game_Flag_Query(575)) {
		Loop_Actor_Walk_To_XYZ(0, -584.0f, 94.89f, 288.0f, 0, 0, false, 0);
		Game_Flag_Reset(575);
	} else {
		Game_Flag_Reset(415);
	}
}

void ScriptKP04::PlayerWalkedOut() {
}

void ScriptKP04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
