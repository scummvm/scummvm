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

void ScriptDR05::InitializeScene() {
	Setup_Scene_Information(-22.0f, 0.3f, 221.0f, 0);
	Game_Flag_Reset(228);
	Scene_Exit_Add_2D_Exit(0, 0, 38, 80, 467, 3);
	Ambient_Sounds_Add_Looping_Sound(383, 25, 0, 1);
	if (!Game_Flag_Query(272)) {
		Overlay_Play("DR05OVER", 0, 1, 0, 0);
	}
	if (Game_Flag_Query(272)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void ScriptDR05::SceneLoaded() {
	Obstacle_Object("MAN PROPHI", true);
	Unobstacle_Object("BOX06", true);
	Unobstacle_Object("BOX183", true);
	Clickable_Object("T2 DOORWAY");
	if (!Game_Flag_Query(272)) {
		Item_Add_To_World(78, 932, 35, -1.57f, 31.33f, 75.21f, 540, 16, 16, true, true, false, true);
		if (!Actor_Query_Goal_Number(35)) {
			Item_Add_To_World(122, 931, 35, 37.35f, 1.59f, 46.72f, 0, 20, 20, true, true, false, true);
		}
	}
}

bool ScriptDR05::MouseClick(int x, int y) {
	return false;
}

bool ScriptDR05::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("T2 DOORWAY", objectName)) {
		if (Game_Flag_Query(276) || Actor_Query_Goal_Number(35)) {
			if (!Loop_Actor_Walk_To_XYZ(0, 57.61f, 0.3f, 69.27f, 0, 1, false, 0)) {
				Actor_Face_Object(0, "T2 DOORWAY", true);
				Actor_Says(0, 8522, 13);
				Actor_Says(0, 8521, 14);
			}
		} else {
			Actor_Face_Object(0, "T2 DOORWAY", true);
			Actor_Says(0, 1020, 14);
			Actor_Says(35, 90, 13);
		}
		return true;
	}
	return false;
}

bool ScriptDR05::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptDR05::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 78) {
		if (Player_Query_Combat_Mode()) {
			Game_Flag_Set(271);
			Actor_Set_Goal_Number(35, 30);
		} else if (!Game_Flag_Query(272) && !Loop_Actor_Walk_To_Item(0, 78, 24, 1, true) && Actor_Query_Goal_Number(35) != 11) {
			if (!Actor_Query_Goal_Number(35)) {
				Actor_Says_With_Pause(0, 1015, 0.1f, 12);
				Actor_Says(35, 70, 13);
			}
			Actor_Set_Goal_Number(35, 30);
		}
		//return true; //bug?
	}
	if (itemId == 122 && Player_Query_Combat_Mode() && !Actor_Query_Goal_Number(35)) {
		Overlay_Play("DR05OVER", 1, 0, 1, 0);
		Item_Remove_From_World(122);
		Game_Flag_Set(270);
		Actor_Set_Goal_Number(35, 10);
		Music_Play(18, 71, 0, 0, -1, 0, 2);
		return true;
	}
	return false;
}

bool ScriptDR05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -22.0f, 0.3f, 221.0f, 0, 1, false, 0)) {
			Game_Flag_Reset(232);
			Game_Flag_Set(229);
			Set_Enter(7, 28);
		}
		return true;
	}
	return false;
}

bool ScriptDR05::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptDR05::SceneFrameAdvanced(int frame) {
	if (frame == 49) {
		Sound_Play(148, Random_Query(50, 50), 80, 80, 50);
	}
	if (Game_Flag_Query(271)) {
		Item_Remove_From_World(78);
		Game_Flag_Reset(271);
		Game_Flag_Set(272);
		Actor_Set_Goal_Number(35, 30);
	}
}

void ScriptDR05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptDR05::PlayerWalkedIn() {
	if (!Game_Flag_Query(511) && !Game_Flag_Query(270) && Game_Flag_Query(272)) {
		Item_Remove_From_World(122);
	}
	if (Game_Flag_Query(272)) {
		Loop_Actor_Walk_To_XYZ(0, -10.0f, 0.3f, 133.0f, 0, 0, false, 0);
		if (!Game_Flag_Query(511)) {
			Game_Flag_Set(511);
			if (Game_Flag_Query(48)) {
				Actor_Voice_Over(730, 99);
				Actor_Voice_Over(740, 99);
				Actor_Voice_Over(750, 99);
				Actor_Voice_Over(760, 99);
				Actor_Clue_Acquire(0, 269, 1, -1);
			} else {
				Actor_Voice_Over(670, 99);
				Actor_Voice_Over(680, 99);
				Actor_Voice_Over(700, 99);
				Actor_Voice_Over(710, 99);
				Actor_Voice_Over(720, 99);
				Actor_Clue_Acquire(0, 270, 1, -1);
			}
		}
	} else {
		Loop_Actor_Walk_To_XYZ(0, -10.0f, 0.3f, 133.0f, 0, 0, true, 0);
	}
	if (!Game_Flag_Query(274) && !Actor_Query_Goal_Number(35)) {
		Actor_Face_Actor(0, 35, true);
		Actor_Says(0, 1010, 13);
		Actor_Face_Item(0, 78, true);
		Player_Set_Combat_Mode(true);
		Actor_Says(35, 60, 12);
		Actor_Change_Animation_Mode(0, 0);
		Game_Flag_Set(274);
		//return true;
	}
	//return false;
}

void ScriptDR05::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (Actor_Query_Goal_Number(35) == 10 || Actor_Query_Goal_Number(35) == 18 || Actor_Query_Goal_Number(35) == 19) {
		Actor_Set_Goal_Number(35, 11);
		//return true;
	}
	//return false;
}

void ScriptDR05::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
