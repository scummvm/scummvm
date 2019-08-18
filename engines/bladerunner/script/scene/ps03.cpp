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

void SceneScriptPS03::InitializeScene() {
	if (Game_Flag_Query(kFlagPS04toPS03)) {
		Actor_Set_At_XYZ(kActorMcCoy, -674.0f, -354.0f, 550.0f, 900);
		Setup_Scene_Information(-674.0f, -354.62f, 550.0f, 900);
		Game_Flag_Reset(kFlagPS04toPS03);
	} else if (Game_Flag_Query(kFlagPS14toPS03)) {
		Setup_Scene_Information(-875.0f, -354.62f, -1241.0f, 450);
		Game_Flag_Reset(kFlagPS14toPS03);
	} else {
		Setup_Scene_Information(-569.54f, -354.62f, -1076.15f, 475);
		Game_Flag_Reset(kFlagPS02toPS03);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 449, 273, 508, 329, 0);
	if (Global_Variable_Query(kVariableChapter) > 1) {
		Scene_Exit_Add_2D_Exit(2, 358, 245, 411, 288, 0);
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Add_Looping_Sound(kSfxPSAMB6, 35, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxPSDOOR1, 5, 50,  7,  7, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPSDOOR2, 5, 50,  7,  7, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPSPA6,   5, 60, 33, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPSPA7,   5, 60, 33, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPSPA8,   5, 60, 33, 33, -100, 100, -101, -101, 0, 0);
	Scene_Loop_Set_Default(1);
}

void SceneScriptPS03::SceneLoaded() {
	Obstacle_Object("TABLE05", true);
	Unclickable_Object("COP1PS03");
	Unclickable_Object("COP2PS03");
}

bool SceneScriptPS03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS03::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptPS03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
#if BLADERUNNER_ORIGINAL_BUGS
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -674.0f, -354.0f, 550.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagPS03toPS04);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS04, kScenePS04);
		}
#else
		// Make McCoy move more forward till he reaches the exit to avoid blinking out at transition to PS03
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -674.0f, -354.0f, 690.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagPS03toPS04);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS04, kScenePS04);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -569.54f, -354.62f, -1076.15f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS02, kScenePS02);
			Game_Flag_Reset(kFlagMcCoyAtPS03);
			if (Global_Variable_Query(kVariableChapter) < 4) {
				Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaLeftOffice);
			}
		}
		return true;
	}
	if (exitId == 2) {
#if BLADERUNNER_ORIGINAL_BUGS
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -875.0f, -354.0f, -1241.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagPS03toPS14);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS14, kScenePS14);
		}
#else
		// exit Police Station earlier (lower z) to avoid some glitch of blending McCoy with background
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -875.0f, -354.0f, -1231.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagPS03toPS14);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS14, kScenePS14);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		return true;
	}
	return false;
}

bool SceneScriptPS03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS03::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS03::PlayerWalkedIn() {
	if (!Game_Flag_Query(kFlagMcCoyAtPS03)) {
		Game_Flag_Set(kFlagMcCoyAtPS03);
		//return true;
	}
	//return false;
}

void SceneScriptPS03::PlayerWalkedOut() {
}

void SceneScriptPS03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
