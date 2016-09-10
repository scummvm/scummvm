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

void ScriptBB09::InitializeScene() {
	Setup_Scene_Information(111.2f, -8.96f, 134.65f, 0);
	if (Game_Flag_Query(221)) {
		Game_Flag_Reset(221);
		Setup_Scene_Information(115.45f, -8.96f, 134.0f, 628);
	} else if (Game_Flag_Query(218)) {
		Game_Flag_Reset(218);
		Setup_Scene_Information(107.45f, -9.14f, 166.0f, 244);
	}
	Scene_Exit_Add_2D_Exit(0, 224, 213, 286, 353, 1);
	Scene_Exit_Add_2D_Exit(1, 75, 450, 480, 479, 2);
	Ambient_Sounds_Add_Looping_Sound(54, 20, 100, 1);
	Ambient_Sounds_Add_Looping_Sound(103, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(105, 50, 55, 1);
	Ambient_Sounds_Add_Sound(297, 5, 20, 20, 25, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(298, 5, 20, 20, 25, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(299, 5, 20, 20, 25, -100, -100, -101, -101, 0, 0);
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
	Actor_Set_Targetable(8, true);
}

void ScriptBB09::SceneLoaded() {
	Obstacle_Object("WICKER CHAIR ", true);
	Unobstacle_Object("ROOM03 RIGHT WALL", true);
	Unclickable_Object("WICKER CHAIR ");
}

bool ScriptBB09::MouseClick(int x, int y) {
	return false;
}

bool ScriptBB09::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptBB09::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptBB09::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptBB09::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 454.56f, -9.0f, 190.31f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(0, 450.56f, -9.0f, 250.31f, 0, 0, false, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(216);
			Game_Flag_Set(220);
			Set_Enter(25, 11);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 71.0f, -9.0f, 136.0f, 72, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(219);
			Set_Enter(23, 9);
		}
		return true;
	}
	return false;
}

bool ScriptBB09::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptBB09::SceneFrameAdvanced(int frame) {
}

void ScriptBB09::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptBB09::PlayerWalkedIn() {
}

void ScriptBB09::PlayerWalkedOut() {
}

void ScriptBB09::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
