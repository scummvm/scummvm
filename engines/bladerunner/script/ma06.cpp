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

void ScriptMA06::InitializeScene() {
	Setup_Scene_Information(40.0f, 1.0f, -20.0f, 400);
	Ambient_Sounds_Add_Looping_Sound(210, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(408, 33, 0, 1);
	Scene_Loop_Start_Special(0, 0, 0);
	Scene_Loop_Set_Default(1);
	Sound_Play(209, 100, 50, 50, 100);
}

void ScriptMA06::SceneLoaded() {
	Obstacle_Object("PANEL", true);
	Clickable_Object("PANEL");
	Player_Loses_Control();
}

bool ScriptMA06::MouseClick(int x, int y) {
	return false;
}

bool ScriptMA06::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptMA06::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptMA06::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptMA06::ClickedOnExit(int exitId) {
	return false;
}

bool ScriptMA06::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptMA06::SceneFrameAdvanced(int frame) {
}

void ScriptMA06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptMA06::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(0, 40.0f, 1.35f, 0.0f, 0, 0, false, 0);
	Actor_Face_Object(0, "panel", true);
	Delay(500);
	sub_4014E4();
	if (sub_4012C0()) {
		Sound_Play(114, 25, 0, 0, 50);
		Delay(4000);
	}
	Game_Flag_Reset(37);
	Game_Flag_Reset(33);
	Game_Flag_Reset(57);
	if (Game_Flag_Query(38)) {
		Set_Enter(49, 48);
	} else if (Game_Flag_Query(34)) {
		Set_Enter(10, 49);
	} else {
		Set_Enter(53, 53);
	}
	Scene_Loop_Start_Special(1, 3, 1);
	Sound_Play(208, 100, 50, 50, 50);
	//return true;
}

void ScriptMA06::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Player_Gains_Control();
}

void ScriptMA06::DialogueQueueFlushed(int a1) {
}

bool ScriptMA06::sub_4012C0() {
	return (Game_Flag_Query(37) && !Game_Flag_Query(38)) || (Game_Flag_Query(33) && !Game_Flag_Query(34)) || (Game_Flag_Query(57) && !Game_Flag_Query(58));
}

void ScriptMA06::sub_4014E4() {
	Game_Flag_Reset(38);
	Game_Flag_Reset(34);
	Game_Flag_Reset(58);
	while (true) {
		if (Game_Flag_Query(34)) {
			break;
		}
		if (Game_Flag_Query(38)) {
			break;
		}
		if (Game_Flag_Query(58)) {
			break;
		}
		Actor_Says(39, 80, 3);
		Player_Gains_Control();
		int v1 = Elevator_Activate(1);
		Player_Loses_Control();
		Scene_Loop_Start_Special(2, 1, 1);
		if (v1 > 1) {
			Game_Flag_Set(58);
		} else if (v1 == 1) {
			if (Game_Flag_Query(250)) {
				Game_Flag_Set(38);
			} else {
				Sound_Play(412, 100, 0, 0, 50);
				Delay(500);
				Actor_Says(39, 610, 3);
			}
		} else {
			Actor_Says(0, 2940, 18);
			if (Global_Variable_Query(1) == 4 && Game_Flag_Query(655)) {
				Sound_Play(412, 100, 0, 0, 50);
				Delay(500);
				Actor_Says(39, 610, 3);
				Delay(500);
				Actor_Says(0, 8527, 3);
			} else {
				Game_Flag_Set(34);
				Actor_Says(39, 90, 3);
			}
		}
	}
}

} // End of namespace BladeRunner
