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

enum kBB02Loops {
	kBB02LoopElevatorComingDown = 0,
	kBB02LoopMainElevatorDown   = 1,
	kBB02LoopElevatorGoingUp    = 3,
	kBB02LoopMainElevatorUp     = 4
};

void SceneScriptBB02::InitializeScene() {
	if (Game_Flag_Query(kFlagBB03toBB02)) {
		Setup_Scene_Information(179.0f, -415.06f, 274.0f, 904);
	} else if (Game_Flag_Query(kFlagBB04toBB02)) {
		Setup_Scene_Information(-12.0f, -415.06f, -27.0f, 264);
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kBB02LoopElevatorComingDown, false);
	} else {
		Setup_Scene_Information(98.0f, -415.06f, -593.0f, 530);
		Game_Flag_Reset(kFlagBB01toBB02);
	}

	Scene_Exit_Add_2D_Exit(0, 313, 137, 353, 173, 0);
	Scene_Exit_Add_2D_Exit(1, 207, 291, 275, 443, 3);
	Scene_Exit_Add_2D_Exit(2, 303, 422, 639, 479, 2);

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1,  20, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 40, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,   2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,   2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,   2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY7,   2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxRCCARBY1, 5,  60, 20, 40,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxRCCARBY2, 5,  60, 20, 45,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxRCCARBY3, 5,  60, 20, 40,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPSDOOR1,  5,  50, 17, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPSDOOR2,  5,  50, 17, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2,  5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3,  5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4,  5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP2,   5,  80, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP3,   5,  80, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP4,   5,  80, 14, 16, -100, 100, -101, -101, 0, 0);

	if (!Game_Flag_Query(kFlagBB02Entered)) {
		Game_Flag_Set(kFlagBB02ElevatorDown);
		Game_Flag_Set(kFlagBB02Entered);
	}
	if (Game_Flag_Query(kFlagBB02ElevatorDown)) {
		Scene_Loop_Set_Default(kBB02LoopMainElevatorDown);
	} else {
		Scene_Loop_Set_Default(kBB02LoopMainElevatorUp);
	}
}

void SceneScriptBB02::SceneLoaded() {
	Obstacle_Object("ELEVATOR01", true);
	Obstacle_Object("U2 DOOR", true);
}

bool SceneScriptBB02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB02::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 98.0f, -415.06f, -593.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB02toBB01);
			Set_Enter(kSetBB01, kSceneBB01);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -415.06f, -27.0f, 0, true, false, false)) {
			Player_Loses_Control();
			if (!Game_Flag_Query(kFlagBB02ElevatorDown)) {
				Scene_Loop_Start_Special(kSceneLoopModeOnce, kBB02LoopElevatorComingDown, true);
			}
			Game_Flag_Set(kFlagBB02toBB04);
			Game_Flag_Reset(kFlagBB02ElevatorDown);
#if BLADERUNNER_ORIGINAL_BUGS
#else
			Actor_Set_Invisible(kActorMcCoy, true);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Set_Enter(kSetBB02_BB04_BB06_BB51, kSceneBB04);
			Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kBB02LoopElevatorGoingUp, false);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 86.0f, -415.06f, 174.0f, 0, true, false, false)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 179.0f, -415.06f, 274.0f, 0, false, false, false);
			Game_Flag_Set(kFlagBB02toBB03);
			Game_Flag_Reset(kFlagBB02ElevatorDown);
			Set_Enter(kSetBB03, kSceneBB03);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB02::SceneFrameAdvanced(int frame) {
	if (frame == 1) {
		Ambient_Sounds_Play_Sound(kSfxBBELE2, 40, -50, -50, 0);
	}

	if (frame == 124) {
		Ambient_Sounds_Play_Sound(kSfxBBELE2, 40, -50, -50, 0);
	}
}

void SceneScriptBB02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB02::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagBB03toBB02)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 86.0f, -415.06f, 174.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagBB03toBB02);
	} else if (Game_Flag_Query(kFlagBB04toBB02)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 35.0f, -415.06f, -27.0f, 0, false, false, false);
		Player_Gains_Control();
		Game_Flag_Reset(kFlagBB04toBB02);
	}
}

void SceneScriptBB02::PlayerWalkedOut() {
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Actor_Set_Invisible(kActorMcCoy, false);
#endif // BLADERUNNER_ORIGINAL_BUGS
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptBB02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
