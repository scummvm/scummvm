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

#include "bladerunner/script/scene_script.h"

namespace BladeRunner {

void SceneScriptPS02::InitializeScene() {
	Player_Loses_Control();
	Setup_Scene_Information(-13.31f, -40.28f, -48.12f, 30);

	Scene_Exit_Add_2D_Exit(0, 0, 0, 240, 479, 3);

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Add_Looping_Sound(kSfxESPLOOP1, 20, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxELEAMB3,  20, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxCROSLOCK, 3, 20, 12, 16, 0, 0, -101, -101, 0, 0);

	Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
	Scene_Loop_Set_Default(1);
}

void SceneScriptPS02::SceneLoaded() {
	Obstacle_Object("E.DOOR01", true);
	Obstacle_Object("E.DOOR02", true);
	Clickable_Object("E.DOOR01");
	Clickable_Object("E.DOOR02");
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Unclickable_Object("E.DOOR01");
	Unclickable_Object("E.DOOR02");
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptPS02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS02::ClickedOn3DObject(const char *objectName, bool a2) {
#if BLADERUNNER_ORIGINAL_BUGS
	// McCoy never clicks on the door
	// This stuff is never called
	if (Object_Query_Click("E.DOOR01", objectName)
	 || Object_Query_Click("E.D00R02", objectName)
	) {
		if (Game_Flag_Query(kFlagPS02toPS01)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -5.0f, -40.0f, -15.0f, 0, true, false, false)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(kSetPS01, kScenePS01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
			}
		} else if (Game_Flag_Query(kFlagPS02toPS05)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -5.0f, -40.0f, -15.0f, 0, true, false, false)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(kSetPS05, kScenePS05);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
			}
		} else if (Game_Flag_Query(kFlagPS02toPS07)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -5.0f, -40.0f, -15.0f, 0, true, false, false)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(kSetPS07, kScenePS07);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
			}
		} else if (Game_Flag_Query(kFlagPS02toPS03)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -5.0f, -40.0f, -15.0f, 0, true, false, false)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(kSetPS03, kScenePS03);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
			}
		} else if (Game_Flag_Query(kFlagPS02toPS09)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -5.0f, -40.0f, -15.0f, 0, true, false, false)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(kSetPS09, kScenePS09);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
			}
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
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
		Ambient_Sounds_Play_Sound(kSfxELDOORO2, 45, 0, 0, 0);
	}

	if (frame == 91) {
		Ambient_Sounds_Play_Sound(kSfxELDOORC1, 45, 0, 0, 0);
	}
	//return true;
}

void SceneScriptPS02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS02::PlayerWalkedIn() {
	Game_Flag_Reset(kflagPS01toPS02);
	Actor_Face_XYZ(kActorMcCoy, 0.0f, 0.0f, 450.0f, true);
	Player_Gains_Control();
	activateElevator();
	Player_Loses_Control();
	if (Game_Flag_Query(kFlagPS02toPS01) ) {
		Set_Enter(kSetPS01, kScenePS01);
		Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
	} else if (Game_Flag_Query(kFlagPS02toPS05) ) {
		Set_Enter(kSetPS05, kScenePS05);
		Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
	} else if (Game_Flag_Query(kFlagPS02toPS07) ) {
		Set_Enter(kSetPS07, kScenePS07);
		Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
	} else if (Game_Flag_Query(kFlagPS02toPS03) ) {
		Set_Enter(kSetPS03, kScenePS03);
		Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
	} else if (Game_Flag_Query(kFlagPS02toPS09) ) {
		Set_Enter(kSetPS09, kScenePS09);
		Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, true);
	}
	//return true;
}

void SceneScriptPS02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Player_Gains_Control();
}

void SceneScriptPS02::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS02::activateElevator() {
	Scene_Exits_Disable();
	switch (Elevator_Activate(kElevatorPS)) {
	case 3:
		Game_Flag_Set(kFlagPS02toPS07);
		break;
	case 4:
		Game_Flag_Set(kFlagPS02toPS01);
		break;
	case 5:
		Game_Flag_Set(kFlagPS02toPS05);
		break;
	case 6:
		Game_Flag_Set(kFlagPS02toPS03);
		break;
	case 7:
		Game_Flag_Set(kFlagPS02toPS09);
		break;
	}
	Scene_Exits_Enable();
}

} // End of namespace BladeRunner
