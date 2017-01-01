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

void ScriptPS15::InitializeScene() {
	Setup_Scene_Information(-360.0f, -113.43f, 50.0f, 0);
	Scene_Exit_Add_2D_Exit(0, 0, 0, 20, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 620, 0, 639, 479, 1);
	Ambient_Sounds_Add_Looping_Sound(384, 20, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(141, 80, 0, 1);
	Ambient_Sounds_Add_Sound(385, 5, 50, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(156, 5, 20, 30, 30, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(157, 5, 20, 30, 30, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(158, 5, 20, 30, 30, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(73, 5, 20, 5, 9, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 20, 5, 9, -70, 70, -101, -101, 0, 0);
	Actor_Put_In_Set(34, 101);
	Actor_Set_At_XYZ(34, -265.4f, -113.43f, -31.29f, 623);
}

void ScriptPS15::SceneLoaded() {
	Obstacle_Object("E.ARCH", true);
	if (Global_Variable_Query(1) == 2) {
		Item_Add_To_World(110, 983, 101, -208.0f, -113.43f, 30.28f, 750, 16, 12, false, true, false, true);
	}
}

bool ScriptPS15::MouseClick(int x, int y) {
	return false;
}

bool ScriptPS15::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptPS15::ClickedOnActor(int actorId) {
	if (actorId == 34) {
		if ((Actor_Clue_Query(0, 80) || Actor_Clue_Query(0, 83)) && !Actor_Clue_Query(0, 81)) {
			if (!Loop_Actor_Walk_To_XYZ(0, -256.0f, -113.43f, 43.51f, 0, 1, false, 0)) {
				Actor_Face_Actor(0, 34, true);
				Actor_Face_Actor(34, 0, true);
				Actor_Says(0, 4470, 17);
				Actor_Says(34, 130, 12);
				Actor_Says(0, 4475, 18);
				Actor_Says(0, 4480, 13);
				Actor_Says(34, 140, 16);
				Item_Pickup_Spin_Effect(965, 211, 239);
				Actor_Says(34, 150, 14);
				Actor_Clue_Acquire(0, 81, 1, 34);
				if (!Game_Flag_Query(727)) {
					Item_Remove_From_World(111);
				}
			}
		} else {
			Actor_Face_Actor(0, 34, true);
			Actor_Says(0, 8600, 15);
			Actor_Says(34, 190, 12);
		}
		return true;
	}
	return false;
}

bool ScriptPS15::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 110) {
		if (Actor_Clue_Query(0, 80) && Actor_Clue_Query(0, 83)) {
			Actor_Says(0, 8570, 14);
		} else {
			Actor_Face_Actor(0, 34, true);
			Actor_Face_Actor(34, 0, true);
			Actor_Says(0, 4485, 17);
			Actor_Says(34, 160, 14);
			Actor_Says(0, 4490, 12);
			Actor_Says(34, 170, 13);
			Actor_Clue_Acquire(0, 80, 1, 0);
			Actor_Clue_Acquire(0, 83, 1, 0);
		}
		return true;
	}
	return false;
}

bool ScriptPS15::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -360.0f, -113.43f, 50.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(204);
			Set_Enter(15, 69);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -183.58f, -113.43f, 91.7f, 0, 1, false, 0)) {
			Actor_Says(0, 4440, 18);
			Actor_Says(34, 150, 17);
			Sound_Play(155, 90, 0, 0, 50);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(14, 73);
		}
		return true;
	}
	return false;
}

bool ScriptPS15::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptPS15::SceneFrameAdvanced(int frame) {
}

void ScriptPS15::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptPS15::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(0, -326.93f, -113.43f, 101.42f, 0, 0, false, 0);
	if (!Game_Flag_Query(43)) {
		Actor_Face_Actor(0, 34, true);
		Actor_Face_Actor(34, 0, true);
		Actor_Says(34, 0, 12);
		Actor_Says(0, 4445, 18);
		Actor_Says(34, 10, 12);
		Actor_Says(0, 4450, 18);
		Actor_Says(34, 60, 13);
		Actor_Says(34, 70, 12);
		Actor_Says(0, 4460, 15);
		Actor_Says(34, 80, 13);
		Actor_Says(0, 4465, 16);
		Actor_Says(34, 90, 13);
		Actor_Says(34, 100, 14);
		Actor_Says(34, 110, 15);
		Actor_Says(34, 120, 15);
		Actor_Says(0, 4555, 14);
		Game_Flag_Set(43);
		//return true;
		return;
	} else {
		//return false;
		return;
	}
}

void ScriptPS15::PlayerWalkedOut() {
}

void ScriptPS15::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
