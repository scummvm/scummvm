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

void ScriptUG03::InitializeScene() {
	if (Game_Flag_Query(335)) {
		Setup_Scene_Information(-51.0f, 0.03f, 255.0f, 780);
		Game_Flag_Reset(335);
	} else if (Game_Flag_Query(337)) {
		Setup_Scene_Information(-139.0f, 0.03f, -13.0f, 540);
		Game_Flag_Reset(337);
	} else {
		Setup_Scene_Information(-121.88f, 0.03f, 213.35f, 540);
	}
	Scene_Exit_Add_2D_Exit(0, 46, 137, 131, 296, 0);
	Scene_Exit_Add_2D_Exit(1, 559, 141, 639, 380, 1);
	Ambient_Sounds_Add_Looping_Sound(331, 15, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	Ambient_Sounds_Add_Sound(402, 2, 120, 10, 11, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(370, 2, 120, 10, 11, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(396, 2, 120, 10, 11, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(395, 2, 120, 10, 11, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(234, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(235, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(391, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(392, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(393, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(394, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(224, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(225, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(226, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(227, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(228, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(229, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
}

void ScriptUG03::SceneLoaded() {
	Obstacle_Object("=WALL_RIGHT_HOLE", true);
	Unobstacle_Object("=HOLERUBBLE1", true);
	Clickable_Object("CHAIR_HEADZAPPER");
	Clickable_Object("CHAIR_BACK");
	Clickable_Object("CHAIR_SEAT");
	Clickable_Object("CHAIR_STRAPLEGLEFT");
	Clickable_Object("CHAIR_STRAPLEGRIGHT");
}

bool ScriptUG03::MouseClick(int x, int y) {
	return false;
}

bool ScriptUG03::ClickedOn3DObject(const char *objectName, bool a2) {
	if ((Object_Query_Click("CHAIR_BACK", objectName) || Object_Query_Click("CHAIR_SEAT", objectName) || Object_Query_Click("CHAIR_HEADZAPPER", objectName)) && !Loop_Actor_Walk_To_Scene_Object(0, "CHAIR_BACK", 36, 1, false)) {
		Actor_Face_Object(0, "CHAIR_BACK", true);
		if (!Actor_Clue_Query(0, 120)) {
			Actor_Voice_Over(2550, 99);
			Actor_Voice_Over(2560, 99);
			Actor_Voice_Over(2570, 99);
			Actor_Voice_Over(2580, 99);
			Actor_Voice_Over(2590, 99);
			Actor_Clue_Acquire(0, 120, 1, -1);
		}
	}
	return false;
}

bool ScriptUG03::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptUG03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptUG03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -139.0f, 0.0f, -13.0f, 0, 1, false, 0)) {
			if (Global_Variable_Query(1) < 4) {
				Actor_Says(0, 8522, 14);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(336);
				Set_Enter(83, 95);
			}
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -51.0f, 0.0f, 255.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(334);
			Set_Enter(77, 89);
		}
		return true;
	}
	return false;
}

bool ScriptUG03::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptUG03::SceneFrameAdvanced(int frame) {
}

void ScriptUG03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptUG03::PlayerWalkedIn() {
}

void ScriptUG03::PlayerWalkedOut() {
}

void ScriptUG03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
