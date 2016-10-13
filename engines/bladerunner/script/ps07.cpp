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

void ScriptPS07::InitializeScene() {
	Setup_Scene_Information(609.07f, 0.22f, -598.67f, 768);
	Scene_Exit_Add_2D_Exit(0, 610, 0, 639, 479, 1);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(141, 80, 0, 1);
	Ambient_Sounds_Add_Sound(142, 5, 20, 5, 10, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(146, 5, 30, 5, 10, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(147, 2, 20, 5, 10, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(148, 2, 10, 10, 20, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(149, 2, 10, 10, 20, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(150, 2, 10, 10, 20, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(151, 2, 10, 10, 20, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(152, 2, 30, 10, 15, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(153, 2, 20, 10, 15, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(154, 5, 20, 10, 15, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(145, 5, 30, 5, 8, -100, 100, -101, -101, 0, 0);
}

void ScriptPS07::SceneLoaded() {
	Obstacle_Object("RICE BOX01", true);
	Unobstacle_Object("RICE BOX01", true);
}

bool ScriptPS07::MouseClick(int x, int y) {
	return false;
}

bool ScriptPS07::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("L.MOUSE", objectName)) {
		Sound_Play(155, 70, 0, 0, 50);
		if (Actor_Query_Goal_Number(30) < 4 && Actor_Query_Goal_Number(30) > 0) {
			Actor_Face_Actor(0, 30, true);
			Actor_Set_Goal_Number(30, 3);
			Actor_Modify_Friendliness_To_Other(30, 0, -3);
		}
		return true;
	}
	return false;
}

bool ScriptPS07::ClickedOnActor(int actorId) {
	if (actorId == 30) {
		Actor_Face_Actor(0, 30, true);
		Actor_Set_Goal_Number(30, 3);
		if (!Game_Flag_Query(111)) {
			Actor_Says(0, 4115, 13);
		}
		if (!Game_Flag_Query(111) && (Game_Flag_Query(125) || Game_Flag_Query(126) || Game_Flag_Query(127) || Game_Flag_Query(128))) {
			Actor_Face_Actor(30, 0, true);
			Actor_Says(30, 30, 12);
			Game_Flag_Set(111);
		} else {
			if (Game_Flag_Query(111)) {
				Actor_Says(0, 4130, 18);
			}
		}
		if (Game_Flag_Query(125) && !Game_Flag_Query(12)) {
			Game_Flag_Set(12);
			Actor_Clue_Acquire(0, 11, 0, 30);
			Actor_Says(30, 50, 16);
			Actor_Says(0, 4135, 13);
			Actor_Says(30, 60, 15);
			Actor_Says(30, 70, 12);
			Actor_Says(0, 4140, 18);
			Actor_Says(30, 80, 14);
			Actor_Says(30, 90, 14);
			Actor_Set_Goal_Number(30, 1);
			return true;
		}
		if (Game_Flag_Query(126) && !Game_Flag_Query(13)) {
			Game_Flag_Set(13);
			Actor_Clue_Acquire(0, 10, 0, 30);
			sub_401D60();
			Actor_Set_Goal_Number(30, 1);
			return true;
		}
		if (Game_Flag_Query(127) && !Game_Flag_Query(104)) {
			Game_Flag_Set(104);
			Actor_Clue_Acquire(0, 39, 0, 30);
			Actor_Says(30, 170, 14);
			Actor_Says(0, 4180, 13);
			Actor_Says(30, 180, 12);
			Actor_Says(30, 190, 13);
			Actor_Says(30, 200, 16);
			Actor_Says(0, 4185, 18);
			Actor_Says(30, 210, 12);
			Actor_Modify_Friendliness_To_Other(30, 0, -12);
			Actor_Set_Goal_Number(30, 1);
			return true;
		}
		if (Game_Flag_Query(128) && !Game_Flag_Query(105)) {
			Game_Flag_Set(105);
			Actor_Says(30, 220, 12);
			Actor_Says(0, 4190, 13);
			Actor_Says(30, 230, 14);
			Actor_Set_Goal_Number(30, 1);
			return true;
		}
		Actor_Says(30, 0, 13);
		Actor_Set_Goal_Number(30, 1);
		return true;
	}
	return false;

}

bool ScriptPS07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptPS07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 609.07f, 0.22f, -598.67f, 0, 0, false, 0)) {
			Set_Enter(62, 66);
		}
		return true;
	}
	return false;
}

bool ScriptPS07::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptPS07::SceneFrameAdvanced(int frame) {
}

void ScriptPS07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptPS07::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(0, 561.07f, 0.34f, -606.67f, 6, 0, false, 0);
	Game_Flag_Reset(131);
	//return false;
}

void ScriptPS07::PlayerWalkedOut() {
	if (!Game_Flag_Query(138) && Global_Variable_Query(1) == 1) {
		Actor_Set_Goal_Number(30, 0);
	}
}

void ScriptPS07::DialogueQueueFlushed(int a1) {
}

void ScriptPS07::sub_401D60() {
	Actor_Says(30, 100, 13);
	Actor_Says(0, 4145, 13);
	Actor_Says(30, 110, 12);
	Actor_Says(0, 4150, 13);
	Actor_Says(30, 120, 14);
	Actor_Says(0, 4155, 17);
	Actor_Says(30, 130, 15);
	Actor_Says(0, 4160, 13);
	Actor_Says(30, 140, 16);
	Actor_Says(0, 4165, 18);
	Actor_Says(30, 160, 13);
	Actor_Says(0, 4170, 19);
	Actor_Says(0, 4175, 19);
	Actor_Modify_Friendliness_To_Other(30, 0, 3);
}

} // End of namespace BladeRunner
