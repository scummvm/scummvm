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

void SceneScriptUG12::InitializeScene() {
	if (Game_Flag_Query(kFlagKP02toUG12)) {
		Setup_Scene_Information(207.0f, -126.21f, -364.0f, 561);
		Game_Flag_Reset(kFlagKP02toUG12);
	} else {
		Setup_Scene_Information(375.0f, -126.21f,  180.0f, 730);
		Game_Flag_Reset(kFlagUG14toUG12);
	}

	Scene_Exit_Add_2D_Exit(0, 538, 222, 615, 346, 1);
	if (Game_Flag_Query(kFlagKP02Available)) {
		Scene_Exit_Add_2D_Exit(1, 334, 176, 426, 266, 0);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRUNOFF, 47, 60, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   40,  0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED2,   40,  0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP1,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP2,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M1,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M1, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M1, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP3,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP4,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP5,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagKP02Available)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptUG12::SceneLoaded() {
	Unobstacle_Object("GATE1", true);
	Obstacle_Object("TRAIN WRECK", true);
}

bool SceneScriptUG12::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG12::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG12::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG12::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG12::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 375.0f, -126.21f, 180.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG12toUG14);
			Set_Enter(kSetUG14, kSceneUG14);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 207.0f, -126.21f, -364.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG12toKP02);
			Set_Enter(kSetKP02, kSceneKP02);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG12::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG12::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG12::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG12::PlayerWalkedIn() {
#if !BLADERUNNER_ORIGINAL_BUGS
	// UG12 scene is in the same set as CT08 and CT51 (kSetCT08_CT51_UG12 : 6)
	// CT08 and CT51 belong to the Replicant hideout room at Yukon hotel,
	// thus kItemChair is still present and with a bit of pixel hunt also visible in this scene
	// fix: remove rogue clickable chair item in this scene only.
	// This code has to be in PlayerWalkedIn() (which is executed after SceneLoaded() when player enters the scene)
	// and not in SceneLoaded(), as in that method, the item is not yet available in the sceneObjects 
	// (SceneObjects::findById() returns -1 for it, in SceneLoaded())
	Item_Remove_From_Current_Scene(kItemChair);
#endif // !BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptUG12::PlayerWalkedOut() {
}

void SceneScriptUG12::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
