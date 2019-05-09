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

void SceneScriptBB03::InitializeScene() {
	Setup_Scene_Information(20.0f, 60.16f, 0.0f, 0);
	Game_Flag_Reset(kFlagBB02toBB03);
	if (Game_Flag_Query(kFlagBB04toBB03)) {
		Setup_Scene_Information(176.0f, 60.16f,    0.0f, 900);
	}
	if (Game_Flag_Query(kFlagBB05toBB03)) {
		Setup_Scene_Information(204.0f, 60.16f, -164.0f, 740);
	}

	Scene_Exit_Add_2D_Exit(0, 589,   0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 167, 372, 439, 479, 2);
	Scene_Exit_Add_2D_Exit(2, 451, 115, 547, 320, 1);

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1,  20,   0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 40,   0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRUNOFF, 34, 100, 1);
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

void SceneScriptBB03::SceneLoaded() {
	Obstacle_Object("BACKWALL", true);
	Unobstacle_Object("BOX08", true);
}

bool SceneScriptBB03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB03::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 176.0f, 60.16f, -64.0f, 0, true, false, false)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 176.0f, 60.16f, 0.0f, 0, false, false, false);
			Game_Flag_Set(kFlagBB03toBB04);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetBB02_BB04_BB06_BB51, kSceneBB04);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 20.0f, 60.16f, 0.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagBB03toBB02);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetBB02_BB04_BB06_BB51, kSceneBB02);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 204.0f, 60.16f, -164.0f, 0, true, false, false)) {
			if (Global_Variable_Query(kVariableChapter) < 4) {
				if (Actor_Query_Goal_Number(kActorSebastian) == 200) {
					Actor_Says(kActorSebastian, 70, kAnimationModeTalk);
					Actor_Says(kActorMcCoy, 7010, 13);
					Actor_Says(kActorSebastian, 80, kAnimationModeTalk);
					Actor_Says(kActorMcCoy, 7015, 12);
					Actor_Says(kActorSebastian, 90, kAnimationModeTalk);
					Actor_Says(kActorMcCoy, 7020, 14);
					Actor_Says(kActorSebastian, 100, kAnimationModeTalk);
					Actor_Says(kActorMcCoy, 7025, 15);
					Actor_Says(kActorSebastian, 110, kAnimationModeTalk);
					Actor_Set_Targetable(kActorBryant, false);
					Actor_Set_Targetable(kActorGeneralDoll, false);
				}
				Game_Flag_Set(kFlagBB03toBB05);
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Set_Enter(kSetBB05, kSceneBB05);
			} else {
				Actor_Says(kActorMcCoy, 8522, kAnimationModeTalk);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptBB03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB03::SceneFrameAdvanced(int frame) {
}

void SceneScriptBB03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB03::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagBB05toBB03)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 164.0f, 60.16f, -164.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagBB05toBB03);
	}

	if (Game_Flag_Query(kFlagBB04toBB03)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 176.0f, 60.16f, -64.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagBB04toBB03);
	}
}

void SceneScriptBB03::PlayerWalkedOut() {
}

void SceneScriptBB03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
