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

void SceneScriptCT03::InitializeScene() {
	if (Game_Flag_Query(kFlagCT02McCoyFell)) {
		Setup_Scene_Information(-852.58f, -621.3f, 285.6f, 0);
	} else if (Game_Flag_Query(kFlagCT02toCT03)) {
		Game_Flag_Reset(kFlagCT02toCT03);
		Setup_Scene_Information(-557.1f, -616.31f, 224.29f, 249);
	} else if (Game_Flag_Query(kFlagCT04toCT03)) {
		Game_Flag_Reset(kFlagCT04toCT03);
		Setup_Scene_Information(-173.99f, -619.19f, 347.54f, 808);
	} else {
		Setup_Scene_Information(-708.58f, -619.19f, 277.6f, 239);
	}
	Scene_Exit_Add_2D_Exit(0,   0, 460, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1,  40,  40, 134, 302, 3);
	Scene_Exit_Add_2D_Exit(2, 390,   0, 539, 230, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1,  50,    1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTAMBR1,  22, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRUNOFF, 34, -100, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  10, 40, 33, 50,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  10, 40, 33, 50,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxDISH1,    3, 30,  8, 10, -100,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxDISH2,    3, 30,  8, 10, -100,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxDISH3,    3, 30,  8, 10, -100,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxDISH4,    3, 30,  8, 10, -100,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 10, 60, 33, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 10, 60, 33, 50, -100, 100, -101, -101, 0, 0);
}

void SceneScriptCT03::SceneLoaded() {
	Obstacle_Object("TRASH CAN", true);
	Unclickable_Object("TRASH CAN");
	Footstep_Sounds_Set(0, 0);
	Footstep_Sounds_Set(1, 1);
}

bool SceneScriptCT03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptCT03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptCT03::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptCT03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptCT03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -745.09f, -619.09f, 293.36f, 0, true, false, false)) {
			Game_Flag_Set(kFlagCT03toCT01);
			Set_Enter(kSetCT01_CT12, kSceneCT01);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -604.38f, -616.15f, 221.6f, 0, true, false, false)) {
			Game_Flag_Set(kFlagCT03toCT02);
			Set_Enter(kSetCT02, kSceneCT02);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -150.0f, -621.3f, 357.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagCT03toCT04);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -67.0f, -621.3f, 477.0f, 0, false);
			Set_Enter(kSetCT03_CT04, kSceneCT04);
		}
		return true;
	}
	return false;
}

bool SceneScriptCT03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptCT03::SceneFrameAdvanced(int frame) {
}

void SceneScriptCT03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT03::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT02RunToFreeSlotG) {
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT06HideAtFreeSlotA);
	}
}

void SceneScriptCT03::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptCT03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
