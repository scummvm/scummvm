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

void ScriptKP07::InitializeScene() {
	Setup_Scene_Information(-12.0f, -41.58f, 72.0f, 0);
	Game_Flag_Reset(578);
	Scene_Exit_Add_2D_Exit(0, 315, 185, 381, 285, 0);
	if (Game_Flag_Query(653)) {
		if (Game_Flag_Query(47) && Actor_Query_Goal_Number(3) < 599) {
			Actor_Set_Targetable(3, true);
			Global_Variable_Increment(51, 1);
			Actor_Put_In_Set(3, 48);
			Actor_Set_At_XYZ(3, -52.0f, -41.52f, -5.0f, 289);
		}
		if (Actor_Query_Goal_Number(19) < 599) {
			Global_Variable_Increment(51, 1);
			Actor_Set_Targetable(19, true);
			Actor_Put_In_Set(19, 48);
			Actor_Set_At_XYZ(19, -26.0f, -41.52f, -135.0f, 0);
		}
		if (Game_Flag_Query(44) && Actor_Query_Goal_Number(7) < 599) {
			Global_Variable_Increment(51, 1);
			Actor_Set_Targetable(7, true);
			Actor_Put_In_Set(7, 48);
			Actor_Set_At_XYZ(7, -38.0f, -41.52f, -175.0f, 500);
		}
		if (Game_Flag_Query(45) && Actor_Query_Goal_Number(2) < 599) {
			Global_Variable_Increment(51, 1);
			Actor_Set_Targetable(2, true);
			Actor_Put_In_Set(2, 48);
			Actor_Set_At_XYZ(2, 61.0f, -41.52f, -3.0f, 921);
		}
		if (Game_Flag_Query(46) && Actor_Query_Goal_Number(6) < 599) {
			Global_Variable_Increment(51, 1);
			Actor_Put_In_Set(6, 48);
			Actor_Set_At_XYZ(6, 78.0f, -41.52f, -119.0f, 659);
		}
		if (Actor_Query_Goal_Number(10) < 599) {
			Global_Variable_Increment(51, 1);
			Actor_Put_In_Set(10, 48);
			Actor_Set_At_XYZ(10, -47.0f, 0.0f, 151.0f, 531);
		}
	}
	Ambient_Sounds_Add_Looping_Sound(585, 7, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(586, 52, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(109, 38, 1, 1);
	if (Game_Flag_Query(582)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void ScriptKP07::SceneLoaded() {
	if (!Game_Flag_Query(653)) {
		Music_Play(19, 25, 0, 0, -1, 1, 0);
	}
	Obstacle_Object("BUNK_TRAY01", true);
	Unobstacle_Object("BUNK_TRAY01", true);
	if (Game_Flag_Query(653)) {
		Player_Set_Combat_Mode(false);
		Scene_Exits_Disable();
	}
}

bool ScriptKP07::MouseClick(int x, int y) {
	return false;
}

bool ScriptKP07::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptKP07::ClickedOnActor(int actorId) {
	if (actorId == 5) {
		if (Game_Flag_Query(697) || actorId != 5 || Actor_Query_Goal_Number(5) == 599 || Actor_Query_Goal_Number(5) == 515) {
			return false;
		}
		if (Game_Flag_Query(653)) {
			Actor_Set_Goal_Number(5, 516);
		} else {
			Music_Play(20, 31, 0, 0, -1, 1, 0);
			Actor_Set_Goal_Number(5, 514);
		}
	} else {
		Actor_Face_Actor(0, actorId, true);
		Actor_Says(0, 8590, 14);
	}
	return true;
}

bool ScriptKP07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptKP07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -12.0f, -41.58f, 72.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(579);
			Set_Enter(9, 46);
		}
		return true;
	}
	return false;
}

bool ScriptKP07::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptKP07::SceneFrameAdvanced(int frame) {
}

void ScriptKP07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptKP07::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(0, 9.0f, -41.88f, -81.0f, 0, 0, false, 0);
	if (!Game_Flag_Query(658)) {
		if (Game_Flag_Query(653)) {
			Actor_Face_Actor(0, 5, true);
			Actor_Says(5, 1240, 3);
			Actor_Says(0, 8500, 3);
			Actor_Says(5, 1250, 3);
			if (Actor_Query_Goal_Number(8) == 416) {
				Actor_Put_In_Set(8, 48);
				Global_Variable_Increment(51, 1);
				Actor_Set_At_XYZ(8, -12.0f, -41.58f, 72.0f, 0);
				Actor_Face_Actor(8, 5, true);
			}
		} else {
			Actor_Face_Actor(0, 5, true);
			Actor_Says(5, 160, 3);
			Actor_Retired_Here(5, 72, 60, 0, -1);
		}
		Game_Flag_Set(658);
	}
}

void ScriptKP07::PlayerWalkedOut() {
	Music_Stop(3);
}

void ScriptKP07::DialogueQueueFlushed(int a1) {
	if (Actor_Query_Goal_Number(5) == 515) {
		Actor_Set_Targetable(5, false);
		Actor_Change_Animation_Mode(5, 21);
		Actor_Retired_Here(5, 12, 48, 1, -1);
		Actor_Set_Goal_Number(5, 599);
	}
}

} // End of namespace BladeRunner
