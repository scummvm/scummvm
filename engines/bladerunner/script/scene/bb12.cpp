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

void SceneScriptBB12::InitializeScene() {
	if (Game_Flag_Query(kFlagBB07toBB12)) {
		Setup_Scene_Information( 138.0f, 0.0f, 104.0f, 760);
	} else if (Game_Flag_Query(kFlagBB08toBB12)) {
		Setup_Scene_Information(-129.0f, 0.0f,  64.0f, 307);
	} else {
		Setup_Scene_Information(  54.0f, 0.0f, 200.0f,   0);
		Game_Flag_Reset(kFlagBB05toBB12);
	}

	Scene_Exit_Add_2D_Exit(0,   0,   0,  30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 589,   0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(2, 377, 374, 533, 479, 2);

	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 28, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY7,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,  5,  50, 27, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,  5,  50, 27, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,  5,  50, 27, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1, 5,  50, 27, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2, 5,  50, 27, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3, 5,  50, 27, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT1,  5,  50, 27, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT2,  5,  50, 27, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);

	if (!Game_Flag_Query(kFlagBB12Entered)) {
		Overlay_Play("BB12OVER", 0, false, false, 0); // Sadik's shadow going left
		Game_Flag_Set(kFlagBB12Entered);
	}
}

void SceneScriptBB12::SceneLoaded() {
	Obstacle_Object("BALLS", true);
	Unclickable_Object("BALLS");
}

bool SceneScriptBB12::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB12::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB12::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB12::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB12::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -129.0f, 0.0f, 64.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB12toBB08);
			Set_Enter(kSetBB08, kSceneBB08);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 138.0f, 0.0f, 104.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB12toBB07);
			Set_Enter(kSetBB07, kSceneBB07);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 54.0f, 0.0f, 200.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB12toBB05);
			Set_Enter(kSetBB05, kSceneBB05);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB12::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB12::SceneFrameAdvanced(int frame) {
	if (frame == 10
	 || frame == 22
	 || frame == 33
	 || frame == 41
	) {
		Sound_Play(kSfxMONKCYM1, 17, -30, -30, 50);
	}

	if (frame == 3) {
		Sound_Play(kSfxMONKEY1,  16, -30, -30, 50);
	}
}

void SceneScriptBB12::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB12::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagBB07toBB12)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 114.0f, 0.0f, 104.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagBB07toBB12);
	} else if (Game_Flag_Query(kFlagBB08toBB12)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -101.0f, 0.0f, 64.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagBB08toBB12);
	}
}

void SceneScriptBB12::PlayerWalkedOut() {
}

void SceneScriptBB12::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
