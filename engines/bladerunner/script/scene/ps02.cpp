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

void SceneScriptPS02::InitializeScene() {
	Player_Loses_Control();
	Setup_Scene_Information(-13.31f, -40.28f, -48.12f, 30);
	Scene_Exit_Add_2D_Exit(0, 0, 0, 240, 479, 3);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(386, 20, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(210, 20, 1, 1);
	Ambient_Sounds_Add_Sound(0, 3, 20, 12, 16, 0, 0, -101, -101, 0, 0);
	Scene_Loop_Start_Special(0, 0, 0);
	Scene_Loop_Set_Default(1);
}

void SceneScriptPS02::SceneLoaded() {
	Obstacle_Object("E.DOOR01", true);
	Obstacle_Object("E.DOOR02", true);
	Clickable_Object("E.DOOR01");
	Clickable_Object("E.DOOR02");
}

bool SceneScriptPS02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS02::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("E.DOOR01", objectName) || Object_Query_Click("E.D00R02", objectName)) {
		if (Game_Flag_Query(130) ) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -5.0f, -40.0f, -15.0f, 0, 1, false, 0)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(61, 65);
				Scene_Loop_Start_Special(1, 3, 1);
			}
		} else if (Game_Flag_Query(22) ) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -5.0f, -40.0f, -15.0f, 0, 1, false, 0)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(15, 69);
				Scene_Loop_Start_Special(1, 3, 1);
			}
		} else if (Game_Flag_Query(131) ) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -5.0f, -40.0f, -15.0f, 0, 1, false, 0)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(66, 71);
				Scene_Loop_Start_Special(1, 3, 1);
			}
		} else if (Game_Flag_Query(132) ) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -5.0f, -40.0f, -15.0f, 0, 1, false, 0)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(63, 67);
				Scene_Loop_Start_Special(1, 3, 1);
			}
		} else if (Game_Flag_Query(133)  && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, -5.0f, -40.0f, -15.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(67, 72);
			Scene_Loop_Start_Special(1, 3, 1);
		}
	}
	return false;
}

bool SceneScriptPS02::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptPS02::ClickedOnExit(int exitId) {
	return false;
}

bool SceneScriptPS02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS02::SceneFrameAdvanced(int frame) {
	if (frame == 1) {
		Ambient_Sounds_Play_Sound(208, 45, 0, 0, 0);
	}
	if (frame == 91) {
		Ambient_Sounds_Play_Sound(209, 45, 0, 0, 0);
	}
	//return true;
}

void SceneScriptPS02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS02::PlayerWalkedIn() {
	Game_Flag_Reset(718);
	Actor_Face_XYZ(kActorMcCoy, 0, 0, 450.0f, true);
	Player_Gains_Control();
	sub_4018BC();
	Player_Loses_Control();
	if (Game_Flag_Query(130) ) {
		Set_Enter(61, 65);
		Scene_Loop_Start_Special(1, 3, 1);
	} else if (Game_Flag_Query(22) ) {
		Set_Enter(15, 69);
		Scene_Loop_Start_Special(1, 3, 1);
	} else if (Game_Flag_Query(131) ) {
		Set_Enter(66, 71);
		Scene_Loop_Start_Special(1, 3, 1);
	} else if (Game_Flag_Query(132) ) {
		Set_Enter(63, 67);
		Scene_Loop_Start_Special(1, 3, 1);
	} else if (Game_Flag_Query(133) ) {
		Set_Enter(67, 72);
		Scene_Loop_Start_Special(1, 3, 1);
	}
	//return true;
}

void SceneScriptPS02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Player_Gains_Control();
}

void SceneScriptPS02::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS02::sub_4018BC() {
	Scene_Exits_Disable();
	switch (Elevator_Activate(2)) {
	case 7:
		Game_Flag_Set(133);
		break;
	case 6:
		Game_Flag_Set(132);
		break;
	case 5:
		Game_Flag_Set(22);
		break;
	case 4:
		Game_Flag_Set(130);
		break;
	case 3:
		Game_Flag_Set(131);
		break;
	default:
		break;
	}
	Scene_Exits_Enable();
}

} // End of namespace BladeRunner
