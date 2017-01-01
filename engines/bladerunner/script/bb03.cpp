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

void ScriptBB03::InitializeScene() {
	Setup_Scene_Information(20.0f, 60.16f, 0.0f, 0);
	Game_Flag_Reset(282);
	if (Game_Flag_Query(284)) {
		Setup_Scene_Information(176.0f, 60.16f, 0.0f, 900);
	}
	if (Game_Flag_Query(286)) {
		Setup_Scene_Information(204.0f, 60.16f, -164.0f, 740);
	}
	Scene_Exit_Add_2D_Exit(0, 589, 0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 167, 372, 439, 479, 2);
	Scene_Exit_Add_2D_Exit(2, 451, 115, 547, 320, 1);
	Ambient_Sounds_Add_Looping_Sound(54, 20, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(103, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(105, 34, 100, 1);
	Ambient_Sounds_Add_Sound(443, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(446, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(309, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(310, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(90, 5, 50, 17, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(91, 5, 50, 17, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(72, 5, 80, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(73, 5, 80, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 80, 20, 20, -100, 100, -101, -101, 0, 0);
}

void ScriptBB03::SceneLoaded() {
	Obstacle_Object("BACKWALL", true);
	Unobstacle_Object("BOX08", true);
}

bool ScriptBB03::MouseClick(int x, int y) {
	return false;
}

bool ScriptBB03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptBB03::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptBB03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptBB03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 176.0f, 60.16f, -64.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(0, 176.0f, 60.16f, 0.0f, 0, 0, false, 0);
			Game_Flag_Set(283);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(1, 5);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 20.0f, 60.16f, 0.0f, 0, 1, false, 0)) {
			Game_Flag_Set(281);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(1, 3);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 204.0f, 60.16f, -164.0f, 0, 1, false, 0)) {
			if (Global_Variable_Query(1) < 4) {
				if (Actor_Query_Goal_Number(56) == 200) {
					Actor_Says(56, 70, 3);
					Actor_Says(0, 7010, 13);
					Actor_Says(56, 80, 3);
					Actor_Says(0, 7015, 12);
					Actor_Says(56, 90, 3);
					Actor_Says(0, 7020, 14);
					Actor_Says(56, 100, 3);
					Actor_Says(0, 7025, 15);
					Actor_Says(56, 110, 3);
					Actor_Set_Targetable(54, false);
					Actor_Set_Targetable(58, false);
				}
				Game_Flag_Set(285);
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(22, 6);
			} else {
				Actor_Says(0, 8522, 3);
			}
		}
		return true;
	}
	return false;
}

bool ScriptBB03::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptBB03::SceneFrameAdvanced(int frame) {
}

void ScriptBB03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptBB03::PlayerWalkedIn() {
	if (Game_Flag_Query(286)) {
		Loop_Actor_Walk_To_XYZ(0, 164.0f, 60.16f, -164.0f, 0, 0, false, 0);
		Game_Flag_Reset(286);
	}
	if (Game_Flag_Query(284)) {
		Loop_Actor_Walk_To_XYZ(0, 176.0f, 60.16f, -64.0f, 0, 0, false, 0);
		Game_Flag_Reset(284);
	}
}

void ScriptBB03::PlayerWalkedOut() {
}

void ScriptBB03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
