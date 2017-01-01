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

void ScriptMA07::InitializeScene() {
	if (Game_Flag_Query(356)) {
		Setup_Scene_Information(6.75f, -172.43f, 356.0f, 997);
		Game_Flag_Reset(356);
		Game_Flag_Set(665);
	} else if (Game_Flag_Query(673)) {
		Setup_Scene_Information(-312.0f, -162.8f, 180.0f, 0);
	} else {
		Setup_Scene_Information(104.0f, -162.16f, 56.0f, 519);
	}
	Ambient_Sounds_Add_Looping_Sound(381, 100, 1, 1);
	Ambient_Sounds_Add_Sound(374, 100, 300, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(68, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Global_Variable_Query(1) > 1) {
		Scene_Exit_Add_2D_Exit(1, 0, 200, 50, 479, 3);
	}
	if (Game_Flag_Query(665)) {
		Scene_Exit_Add_2D_Exit(2, 176, 386, 230, 426, 2);
	}
	Scene_Exit_Add_2D_Exit(0, 270, 216, 382, 306, 0);
}

void ScriptMA07::SceneLoaded() {
	Obstacle_Object("BARRICADE", true);
}

bool ScriptMA07::MouseClick(int x, int y) {
	return false;
}

bool ScriptMA07::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptMA07::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptMA07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptMA07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 104.0f, -162.0f, 56.0f, 12, 1, false, 0)) {
			if (Global_Variable_Query(1) == 4 && Game_Flag_Query(671)) {
				Actor_Set_Goal_Number(0, 400);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(57);
				Set_Enter(52, 52);
			}
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -400.0f, -162.8f, 185.08f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(672);
			Game_Flag_Reset(179);
			Game_Flag_Set(178);
			Set_Enter(68, 77);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 8.0f, -172.43f, 356.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(357);
			Set_Enter(90, 103);
		}
		return true;
	}
	return false;
}

bool ScriptMA07::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptMA07::SceneFrameAdvanced(int frame) {
}

void ScriptMA07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == 53 && newGoal == 302) {
		Scene_Exits_Enable();
	}
}

void ScriptMA07::PlayerWalkedIn() {
	if (Game_Flag_Query(673)) {
		Loop_Actor_Walk_To_XYZ(0, -268.0f, -162.8f, 188.0f, 0, 0, false, 0);
		Game_Flag_Reset(673);
	}
	if (Actor_Query_Goal_Number(57) == 300) {
		Actor_Set_Goal_Number(57, 305);
	}
	if (Game_Flag_Query(58)) {
		Game_Flag_Reset(58);
	}
	if (!Game_Flag_Query(648) && Game_Flag_Query(671) && Global_Variable_Query(1) == 4) {
		Scene_Exits_Disable();
		Actor_Set_Goal_Number(53, 300);
	}
	if (Game_Flag_Query(666)) {
		Actor_Voice_Over(1360, 99);
		Actor_Voice_Over(1370, 99);
		Actor_Voice_Over(1380, 99);
		Actor_Voice_Over(1390, 99);
		Actor_Voice_Over(1400, 99);
		Delay(1000);
		Game_Flag_Reset(666);
		Game_Flag_Set(34);
		Set_Enter(10, 49);
	}
	//return false;

}

void ScriptMA07::PlayerWalkedOut() {
}

void ScriptMA07::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
