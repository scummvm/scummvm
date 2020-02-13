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

void SceneScriptTB03::InitializeScene() {
	if (Game_Flag_Query(kFlagUG17toTB03)) {
		Setup_Scene_Information(-260.0f, 0.15f, 2014.0f, 276);
	} else {
		Setup_Scene_Information(-152.0f,  0.0f, 1890.0f, 500);
	}

	Scene_Exit_Add_2D_Exit(0, 25, 227, 81, 300, 0);
	Scene_Exit_Add_2D_Exit(1, 298, 0, 639, 305, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxFOUNTAIN, 16, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBELLY1, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY2, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY3, 2, 20, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY4, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY5, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	if (Global_Variable_Query(kVariableChapter) <= 3) {
		Ambient_Sounds_Add_Looping_Sound(kSfxPSAMB6, 25, 0, 1);
		Ambient_Sounds_Add_Sound(kSfx67_0470R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0500R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0960R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1100R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1140R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	}

	Actor_Put_In_Set(kActorTyrellGuard, kSetTB02_TB03);
	Actor_Set_At_XYZ(kActorTyrellGuard, -38.53f, 2.93f, 1475.97f, 673);
	if (Global_Variable_Query(kVariableChapter) == 4) {
		int goal = Actor_Query_Goal_Number(kActorTyrellGuard);
		if (goal == kGoalTyrellGuardWait) {
			Actor_Change_Animation_Mode(kActorTyrellGuard, kAnimationModeIdle);
			Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordArrestMcCoyInTB03Act4);
		} else if (goal != kGoalTyrellGuardWakeUp) {
			Actor_Set_Goal_Number(kActorTyrellGuard, kGoalTyrellGuardSleeping);
		}
	}

	if (Game_Flag_Query(kFlagUG17toTB03)) {
		if (!Game_Flag_Query(kFlagTB03Entered)) {
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
			Scene_Loop_Set_Default(1);
			Game_Flag_Set(kFlagTB03Entered);
		} else {
			Scene_Loop_Set_Default(1);
		}
		Game_Flag_Reset(kFlagUG17toTB03);
	} else {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
	}
}

void SceneScriptTB03::SceneLoaded() {
	Obstacle_Object("SPHERE02", true);
}

bool SceneScriptTB03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptTB03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptTB03::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptTB03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptTB03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -260.0f, 0.15f, 2014.0f, 0, true, false, false)) {
			Actor_Set_Goal_Number(kActorTyrellGuard, kGoalTyrellGuardWait);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagTB03toUG17);
			Set_Enter(kSetUG17, kSceneUG17);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -152.0f, 0.0f, 1774.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagTB03toTB02);
			Set_Enter(kSetTB02_TB03, kSceneTB02);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -152.0f, 0.0f, 1702.0f, 0, false);
		}
		return true;
	}
	return false;
}

bool SceneScriptTB03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptTB03::SceneFrameAdvanced(int frame) {
}

void SceneScriptTB03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptTB03::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorTyrellGuard) == kGoalTyrellGuardWait) {
		Player_Set_Combat_Mode(false);
		Actor_Says(kActorOfficerGrayford, 260, -1);
		Actor_Says(kActorMcCoy, 170, 14);
		Delay(1000);
		Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyArrested);
	}
}

void SceneScriptTB03::PlayerWalkedOut() {
	Music_Stop(2);
}

void SceneScriptTB03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
