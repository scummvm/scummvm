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

// Bug in the game, item 77 (android control box) is showing up here...

void SceneScriptBB04::InitializeScene() {
	if (Game_Flag_Query(kFlagBB03toBB04)) {
		Setup_Scene_Information(-107.0f,  -26.6f, 397.0f,  29);
		Game_Flag_Reset(kFlagBB03toBB04);
	} else {
		Setup_Scene_Information( -15.0f, -25.17f,  45.0f, 691);
	}

	Scene_Exit_Add_2D_Exit(0, 218, 102, 360, 254, 1);
	Scene_Exit_Add_2D_Exit(1,   0, 334, 639, 479, 2);

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1,  20,    0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 40,    0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRUNOFF, 44, -100, 1);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY7,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT1,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT2,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPSDOOR1, 5,  50, 17, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPSDOOR2, 5,  50, 17, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP2,  5,  80, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP3,  5,  80, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP4,  5,  80, 20, 20, -100, 100, -101, -101, 0, 0);
}

void SceneScriptBB04::SceneLoaded() {
	Obstacle_Object("DH TRASH", true);
}

bool SceneScriptBB04::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB04::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -92.0f, -26.6f, 45.0f, 0,  true, false, false)) {
			Player_Loses_Control();
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -15.0f, -25.17f, 45.0f, 0, false, false, false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB04toBB02);
			Game_Flag_Set(kFlagBB02ElevatorDown);
			Set_Enter(kSetBB02_BB04_BB06_BB51, kSceneBB02);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -107.0f, -26.6f, 397.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB04toBB03);
			Set_Enter(kSetBB03, kSceneBB03);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB04::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB04::SceneFrameAdvanced(int frame) {
}

void SceneScriptBB04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB04::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagBB02toBB04)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -92.0f, -26.6f, 45.0f, 0, false, false, false);
		Player_Gains_Control();
		Game_Flag_Reset(kFlagBB02toBB04);
	}
}

void SceneScriptBB04::PlayerWalkedOut() {
}

void SceneScriptBB04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
