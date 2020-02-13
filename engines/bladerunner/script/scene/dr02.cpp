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

void SceneScriptDR02::InitializeScene() {
	if (Game_Flag_Query(kFlagDR03toDR02)) {
		Setup_Scene_Information(-1162.0f,  7.18f,  -322.0f, 552);
	} else if (Game_Flag_Query(kFlagDR01toDR02)) {
		Setup_Scene_Information( -835.0f, -0.04f,  -118.0f, 193);
	} else if (Game_Flag_Query(kFlagBB01toDR02)) {
		Setup_Scene_Information(-1258.0f,  7.18f,  -314.0f, 400);
	} else {
		Setup_Scene_Information( 168.78f,  0.16f, -775.72f, 193);
	}

	Scene_Exit_Add_2D_Exit(0, 605,   0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 222, 176, 279, 314, 0);
	if (Game_Flag_Query(kFlagBB01Available)) {
		Scene_Exit_Add_2D_Exit(2, 95, 0, 148, 292, 0);
	}
	if (_vm->_cutContent) {
		// improved navigation of this set of scenes
		Scene_Exit_Add_2D_Exit(3,  0,   0,  34, 479, 3);
		Scene_Exit_Add_2D_Exit(4, 34, 445, 605, 479, 2);
	}

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Remove_All_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxHUMMER3, 27, 85, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxBIGFAN2, 38, 85, 1);
	Ambient_Sounds_Add_Sound(kSfxTHNDR1, 2,  50, 33, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR2, 2,  50, 33, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR3, 2,  50, 33, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN2A, 5, 100, 16,  25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN1A, 5, 100, 16,  25, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagDR01toDR02)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(1);
	}
}

void SceneScriptDR02::SceneLoaded() {
	Obstacle_Object("TRASH CAN WITH FIRE", true);
	Obstacle_Object("V2PYLON02", true);
	Obstacle_Object("V2PYLON04", true);
	Obstacle_Object("U2 CHEWDOOR", true);
	Obstacle_Object("MMTRASHCAN", true);
	Obstacle_Object("PARKMETR02", true);
	Obstacle_Object("TRANSFORMER 01", true);
	Obstacle_Object("TRANSFORMER 02", true);
	Obstacle_Object("PARKMETR01", true);
	Obstacle_Object("Z2ENTRYDR", true);
	Obstacle_Object("Z2DR2", true);
	// there are two objects named "TRASH CAN WITH FIRE" in the scene (but not visible)
	// and none of them have to be set clickable
	Clickable_Object("TRASH CAN WITH FIRE");
	Clickable_Object("U2 CHEWDOOR");
	Clickable_Object("MMTRASHCAN");
	Clickable_Object("U2 EYE");
	Clickable_Object("U2 E");
	Clickable_Object("MMNEWSP01");
	Clickable_Object("MMNEWSP02");
	Clickable_Object("MMNEWSP04");
	Clickable_Object("MMNEWSP05");
	Clickable_Object("MMNEWSP07");
	Clickable_Object("PARKMETR02");
	Clickable_Object("TRANSFORMER 01");
	Clickable_Object("TRANSFORMER 02");
	Clickable_Object("V2CANPIPE02");
	// there are two objects named "TRASH CAN WITH FIRE" in the scene (but not visible)
	// and none of them have to be set clickable
	// and also reseting them here would only reset one of them - and one remains as clickable
	Unclickable_Object("TRASH CAN WITH FIRE");
	Unclickable_Object("U2 CHEWDOOR");
	Unclickable_Object("MMTRASHCAN");
	if (_vm->_cutContent) {
		if (Game_Flag_Query(kFlagMcCoyCommentsOnEyeOfEyeworks)) {
			Unclickable_Object("U2 EYE");
		}
	} else {
		Unclickable_Object("U2 EYE");
	}
	Unclickable_Object("U2 E");
	Unclickable_Object("MMNEWSP01");
	Unclickable_Object("MMNEWSP02");
	Unclickable_Object("MMNEWSP04");
	Unclickable_Object("MMNEWSP05");
	Unclickable_Object("MMNEWSP07");
	Unclickable_Object("PARKMETR02");
	Unclickable_Object("TRANSFORMER 01");
	Unclickable_Object("TRANSFORMER 02");
	Unclickable_Object("V2CANPIPE02");
}

bool SceneScriptDR02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptDR02::ClickedOn3DObject(const char *objectName, bool a2) {
	if (_vm->_cutContent) {
		if (!Game_Flag_Query(kFlagMcCoyCommentsOnEyeOfEyeworks)
		    && (Object_Query_Click("U2 EYE", objectName))
		) {
			Game_Flag_Set(kFlagMcCoyCommentsOnEyeOfEyeworks);
			Unclickable_Object("U2 EYE");
			Actor_Voice_Over(660, kActorVoiceOver);
			return true;
		}
	}
	return false;
}

bool SceneScriptDR02::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptDR02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptDR02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -835.0f, -0.04f, -118.0f, 0, true, false, false)) {
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -727.0f, -0.04f, -118.0f, 0, false);
			Game_Flag_Set(kFlagDR02toDR01);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
		}
		Ambient_Sounds_Adjust_Looping_Sound(kSfxHUMMER3, 12, -101, 1);
		Ambient_Sounds_Adjust_Looping_Sound(kSfxBIGFAN2, 14, -101, 1);
		return true;
	}

	if (_vm->_cutContent
	    && (exitId == 3 || exitId == 4)) {
	    if ((exitId == 3 && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1162.45f, 0.78f, -167.45f, 0, true, false, false))
		    || (exitId == 4 && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, -975.17f, 0.42f, -120.04f, 0, true, false, false))
		) {
			if (exitId == 3) {
				Async_Actor_Walk_To_XYZ(kActorMcCoy, -1176.40f, 0.58f, -86.02f, 0, false);
			} else {
				Async_Actor_Walk_To_XYZ(kActorMcCoy, -845.00f, 0.37f, -31.60f, 0, false);
			}
			Game_Flag_Set(kFlagDR02toDR01);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
		}
		Ambient_Sounds_Adjust_Looping_Sound(kSfxHUMMER3, 12, -101, 1);
		Ambient_Sounds_Adjust_Looping_Sound(kSfxBIGFAN2, 14, -101, 1);
		return true;
	}


	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1162.0f, 7.18f, -322.0f, 0, true, false, false)) {
			if (Global_Variable_Query(kVariableChapter) > 2) {
				Actor_Says(kActorMcCoy, 8522, 15);
			} else {
				Game_Flag_Set(kFlagDR02toDR03);
				Set_Enter(kSetDR03, kSceneDR03);
			}
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1258.0f, 7.18f, -314.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagDR02toBB01);
			Game_Flag_Reset(kFlagMcCoyInDNARow);
			Game_Flag_Set(kFlagMcCoyInBradburyBuilding);
			Set_Enter(kSetBB01, kSceneBB01);
		}
		return true;
	}
	return false;
}

bool SceneScriptDR02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptDR02::SceneFrameAdvanced(int frame) {
	if (frame == 1) {
		Sound_Play(kSfxSTEAM1, 10, 85, 85, 50);
	}
}

void SceneScriptDR02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptDR02::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagDR03toDR02)) {
		Game_Flag_Reset(kFlagDR03toDR02);
	}
	if (Game_Flag_Query(kFlagDR01toDR02)) {
		Game_Flag_Reset(kFlagDR01toDR02);
	}
	if (Game_Flag_Query(kFlagBB01toDR02)) {
		Game_Flag_Reset(kFlagBB01toDR02);
	}
}

void SceneScriptDR02::PlayerWalkedOut() {
}

void SceneScriptDR02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
