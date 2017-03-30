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

#include "bladerunner/script/scene.h"

namespace BladeRunner {

void SceneScriptCT03::InitializeScene() {
	if (Game_Flag_Query(719)) {
		Setup_Scene_Information(-852.58f, -621.3f, 285.6f, 0);
	} else if (Game_Flag_Query(69)) {
		Game_Flag_Reset(69);
		Setup_Scene_Information(-557.1f, -616.31f, 224.29f, 249);
	} else if (Game_Flag_Query(73)) {
		Game_Flag_Reset(73);
		Setup_Scene_Information(-173.99f, -619.19f, 347.54f, 808);
	} else {
		Setup_Scene_Information(-708.58f, -619.19f, 277.6f, 239);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 40, 40, 134, 302, 3);
	Scene_Exit_Add_2D_Exit(2, 390, 0, 539, 230, 1);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(56, 22, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(105, 34, -100, 1);
	Ambient_Sounds_Add_Sound(68, 10, 40, 33, 50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 40, 33, 50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(61, 3, 30, 8, 10, -100, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(62, 3, 30, 8, 10, -100, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(63, 3, 30, 8, 10, -100, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(64, 3, 30, 8, 10, -100, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(376, 10, 60, 33, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 10, 60, 33, 50, -100, 100, -101, -101, 0, 0);
}

void SceneScriptCT03::SceneLoaded() {
	Obstacle_Object("TRASH CAN", true);
	Unclickable_Object("TRASH CAN");
	Footstep_Sounds_Set(0, 0);
	Footstep_Sounds_Set(1, 1);
}

bool SceneScriptCT03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptCT03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptCT03::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptCT03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptCT03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -745.09f, -619.09f, 293.36f, 0, 1, false, 0)) {
			Game_Flag_Set(71);
			Set_Enter(4, 13);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -604.38f, -616.15f, 221.6f, 0, 1, false, 0)) {
			Game_Flag_Set(70);
			Set_Enter(27, 14);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -150.0f, -621.3f, 357.0f, 0, 1, false, 0)) {
			Game_Flag_Set(72);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -67.0f, -621.3f, 477.0f, 0, false);
			Set_Enter(5, 16);
		}
		return true;
	}
	return false;
}

bool SceneScriptCT03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptCT03::SceneFrameAdvanced(int frame) {
}

void SceneScriptCT03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT03::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorZuben) == 2) {
		Actor_Set_Goal_Number(kActorZuben, 13);
	}
}

void SceneScriptCT03::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptCT03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
