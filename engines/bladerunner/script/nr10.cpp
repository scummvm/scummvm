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

void ScriptNR10::InitializeScene() {
	if (Game_Flag_Query(475)) {
		Game_Flag_Reset(475);
		Setup_Scene_Information(-136.78f, 2.84f, -234.43f, 320);
	} else {
		Game_Flag_Reset(477);
		Setup_Scene_Information(19.22f, 2.84f, -250.43f, 540);
	}
	Scene_Exit_Add_2D_Exit(0, 144, 163, 194, 318, 3);
	Scene_Exit_Add_2D_Exit(1, 475, 95, 568, 230, 0);
	Ambient_Sounds_Add_Looping_Sound(205, 22, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(71, 33, 0, 1);
	Ambient_Sounds_Add_Sound(303, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(640)) {
		Scene_Loop_Set_Default(0);
	} else {
		Ambient_Sounds_Adjust_Looping_Sound(452, 31, 0, 1);
		Scene_Loop_Set_Default(2);
	}
}

void ScriptNR10::SceneLoaded() {
	Obstacle_Object("HOOK 01", true);
	Unobstacle_Object("BOX21", true);
	Unobstacle_Object("BOX23", true);
	Unclickable_Object("BOX18");
}

bool ScriptNR10::MouseClick(int x, int y) {
	return Game_Flag_Query(642);
}

bool ScriptNR10::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("BOX18", objectName) && a2 && Game_Flag_Query(642)) {
		Actor_Set_Goal_Number(3, 250);
		Game_Flag_Set(640);
		Game_Flag_Reset(642);
		Actor_Set_Invisible(0, false);
		Actor_Set_Invisible(3, false);
		Ambient_Sounds_Remove_Looping_Sound(452, true);
		Sound_Play(453, 52, 0, 0, 50);
		Scene_Loop_Set_Default(0);
		Scene_Loop_Start_Special(2, 0, 1);
		Un_Combat_Target_Object("BOX18");
		Scene_Exits_Enable();
		return true;
	}
	return false;
}

bool ScriptNR10::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptNR10::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptNR10::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -152.78f, 2.84f, -238.43f, 0, 1, false, 0)) {
			Game_Flag_Set(476);
			Set_Enter(58, 62);
			return true;
		}
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 11.5f, 2.84f, -304.46f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 55, false);
			Loop_Actor_Travel_Ladder(0, 8, 1, 0);
			Game_Flag_Set(641);
			Set_Enter(60, 64);
			return true;
		}
	}
	return false;
}

bool ScriptNR10::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptNR10::SceneFrameAdvanced(int frame) {
	if (frame == 122) {
		Game_Flag_Set(642);
		Actor_Set_Invisible(0, true);
		Actor_Set_Invisible(3, true);
		Combat_Target_Object("BOX18");
		//return true;
		return;
	}
	if (frame == 61 && Game_Flag_Query(642)) {
		Game_Flag_Reset(642);
		Player_Set_Combat_Mode(false);
		Actor_Set_Invisible(0, false);
		Actor_Set_Goal_Number(3, 247);
		//return true;
		return;
	}
	//return false;
}

void ScriptNR10::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptNR10::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(3) == 246) {
		Player_Set_Combat_Mode(true);
		//return true;
		return;
	}
	if (Actor_Query_Goal_Number(1) == 236) {
		Actor_Face_Actor(1, 0, true);
		Actor_Says(1, 150, 13);
		Actor_Face_Actor(0, 1, true);
		Actor_Says(0, 1580, 14);
		Actor_Says(1, 160, 15);
		Actor_Says(0, 1585, 16);
		Actor_Says(1, 1160, 16);
		Delay(1000);
		Actor_Says(1, 1290, 14);
		Actor_Set_Goal_Number(1, 275);
	}
	//		return false;
}

void ScriptNR10::PlayerWalkedOut() {
}

void ScriptNR10::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
