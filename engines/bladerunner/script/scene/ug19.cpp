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

void SceneScriptUG19::InitializeScene() {
	if (Game_Flag_Query(kFlagMA07toUG19)) {
		Setup_Scene_Information(67.03f, 105.0f, -74.97f, 256);
	} else {
		Setup_Scene_Information(181.0f, 11.52f,  -18.0f, 777);
	}

	Scene_Exit_Add_2D_Exit(0, 351,   0, 491, 347, 0);
	Scene_Exit_Add_2D_Exit(1, 548, 124, 639, 369, 1);

	Ambient_Sounds_Add_Looping_Sound(kSfxBOILPOT2, 45, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   76, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP1,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP2,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP3,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP4,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP5,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M2,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M1, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M3, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M2, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAM1,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAM3,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAM6A,  5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxZUBWLK1,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxZUBWLK2,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxZUBWLK3,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxZUBWLK4,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
}

void SceneScriptUG19::SceneLoaded() {
	Obstacle_Object("LADDER", true);
	Unclickable_Object("LADDER");
	Footstep_Sounds_Set(1, 0);
	Footstep_Sounds_Set(0, 3);
}

bool SceneScriptUG19::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG19::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG19::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG19::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG19::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 67.03f, 7.29f, -74.97f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 256, false);
			Footstep_Sound_Override_On(3);
			Loop_Actor_Travel_Ladder(kActorMcCoy, 8, true, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG19toMA07);
			Set_Enter(kSetMA07, kSceneMA07);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 181.0f, 11.52f, -18.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG19toUG14);
			Set_Enter(kSetUG14, kSceneUG14);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG19::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG19::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG19::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG19::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagUG14toUG19)) {
		Game_Flag_Reset(kFlagUG14toUG19);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 129.0f, 11.52f, -18.0f, 0, false, false, false);
	} else {
		Game_Flag_Reset(kFlagMA07toUG19);
		Footstep_Sound_Override_On(3);
		Loop_Actor_Travel_Ladder(kActorMcCoy, 8, false, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
	}
}

void SceneScriptUG19::PlayerWalkedOut() {
}

void SceneScriptUG19::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
