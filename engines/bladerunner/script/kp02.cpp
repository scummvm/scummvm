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

void ScriptKP02::InitializeScene() {
	if (Game_Flag_Query(414)) {
		Setup_Scene_Information(-884.0f, -615.49f, 3065.0f, 20);
	} else {
		Setup_Scene_Information(-1040.0f, -615.49f, 2903.0f, 339);
		Game_Flag_Reset(412);
	}
	Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
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

void ScriptKP02::SceneLoaded() {
	Obstacle_Object("VAN GRATE", true);
	Clickable_Object("VAN GRATE");
	Unobstacle_Object("VAN GRATE", true);
	Unobstacle_Object("BOX05", true);
	Unobstacle_Object("BOX08", true);
	Unobstacle_Object("BOX09", true);
	Unobstacle_Object("BOX01", true);
	Unclickable_Object("VAN GRATE");
}

bool ScriptKP02::MouseClick(int x, int y) {
	return false;
}

bool ScriptKP02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptKP02::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptKP02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptKP02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -1040.0f, -615.49f, 2903.0f, 0, 1, false, 0)) {
			if (Actor_Query_Goal_Number(65) == 406 || Actor_Query_Goal_Number(64) == 406) {
				Non_Player_Actor_Combat_Mode_Off(65);
				Non_Player_Actor_Combat_Mode_Off(64);
				Actor_Set_Goal_Number(65, 400);
				Actor_Set_Goal_Number(64, 400);
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(411);
				Set_Enter(84, 96);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(411);
				Set_Enter(84, 96);
			}
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -884.0f, -615.49f, 3065.0f, 0, 1, false, 0)) {
			if (Actor_Query_Goal_Number(65) == 406 || Actor_Query_Goal_Number(64) == 406) {
				Non_Player_Actor_Combat_Mode_Off(65);
				Non_Player_Actor_Combat_Mode_Off(64);
				Actor_Set_Goal_Number(65, 400);
				Actor_Set_Goal_Number(64, 400);
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(413);
				Set_Enter(44, 41);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(413);
				Set_Enter(44, 41);
			}
		}
		return true;
	}
	return false;
}

bool ScriptKP02::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptKP02::SceneFrameAdvanced(int frame) {
}

void ScriptKP02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptKP02::PlayerWalkedIn() {
	if (Game_Flag_Query(414)) {
		Loop_Actor_Walk_To_XYZ(0, -884.0f, -615.49f, 3035.0f, 0, 0, false, 0);
		Game_Flag_Reset(414);
	}
	if (Game_Flag_Query(653) && Actor_Query_Goal_Number(1) != 599) {
		Actor_Set_Goal_Number(1, 450);
	}
	//return false;
}

void ScriptKP02::PlayerWalkedOut() {
}

void ScriptKP02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
