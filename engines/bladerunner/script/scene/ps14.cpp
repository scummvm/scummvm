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

void SceneScriptPS14::InitializeScene() {
	if (Game_Flag_Query(kFlagPS03toPS14)) {
		Setup_Scene_Information(-1119.61f, 508.14f, -1208.22f, 315);
		Game_Flag_Reset(kFlagPS03toPS14);
	} else {
		Setup_Scene_Information( -785.45f, 508.14f,  -1652.0f, 315);
	}

	Scene_Exit_Add_2D_Exit(0, 610,  0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1,  46, 51, 125, 192, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxRAIN10, 100, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxCOLONY,  100, 300, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,   60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,   60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2,  60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3,  50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4,  50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
}

void SceneScriptPS14::SceneLoaded() {
	Obstacle_Object("CABLES UPPER RIGHT", true);
	Unobstacle_Object("CYLINDER63", true);
	Clickable_Object("CABLES UPPER RIGHT");
	Unclickable_Object("CABLES UPPER RIGHT");
}

bool SceneScriptPS14::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS14::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS14::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS14::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptPS14::ClickedOnExit(int exitId) {
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -2101.0f, 508.14f, -1361.0f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 819, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 3, true, kAnimationModeIdle);

			if (Global_Variable_Query(kVariableChapter) == 4
			 && Game_Flag_Query(kFlagUG18GuzzaScene)
			) {
				if (Actor_Clue_Query(kActorMcCoy, kClueBriefcase)) {
					Game_Flag_Set(kFlagMcCoyFreedOfAccusations);
					Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyStartChapter5);
				} else {
					Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyArrested);
				}
			} else if (Global_Variable_Query(kVariableChapter) > 3) {
				Actor_Says(kActorMcCoy, 8522, 12);
				Actor_Face_Heading(kActorMcCoy, 307, false);
				Loop_Actor_Travel_Stairs(kActorMcCoy, 3, false, kAnimationModeIdle);
			} else {
				Game_Flag_Set(kFlagPS14toPS03);
				Set_Enter(kSetPS03, kScenePS03);
			}
		}
		return true;
	}

	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -785.45f, 508.14f, -1652.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagPS14toMA07);
			Game_Flag_Reset(kFlagMcCoyInPoliceStation);
			Game_Flag_Set(kFlagMcCoyInMcCoyApartment);
			Set_Enter(kSetMA07, kSceneMA07);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS14::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS14::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS14::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS14::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagMA07toPS14)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -801.45f, 508.14f, -1596.68f, 0, false, false, false);
		Game_Flag_Reset(kFlagMA07toPS14);
	}
	//return false;
}

void SceneScriptPS14::PlayerWalkedOut() {
}

void SceneScriptPS14::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
