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

void SceneScriptDR01::InitializeScene() {
	if (Game_Flag_Query(225)) {
		Setup_Scene_Information(-835.0f, -0.04f, -118.0f, 664);
	} else if (Game_Flag_Query(11)) {
		Setup_Scene_Information(-711.0f, -0.04f, 70.0f, 307);
	} else if (Game_Flag_Query(531)) {
		Setup_Scene_Information(-1765.28f, -0.04f, -23.82f, 269);
	} else {
		Setup_Scene_Information(-386.0f, -0.04f, -82.0f, 792);
	}
	Scene_Exit_Add_2D_Exit(0, 240, 60, 450, 250, 0);
	Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
	if (Game_Flag_Query(kFlagSpinnerToDR01) && Global_Variable_Query(kVariableChapter) < 4) {
		Scene_Exit_Add_2D_Exit(2, 610, 0, 639, 479, 1);
	}
	if (Global_Variable_Query(kVariableChapter) >= 3) {
		Scene_Exit_Add_2D_Exit(3, 0, 45, 142, 201, 0);
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(219, 12, 85, 1);
	Ambient_Sounds_Add_Looping_Sound(98, 14, 85, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(67, 5, 80, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(66, 5, 80, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(378, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(379, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(380, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(272) && Game_Flag_Query(11)) {
		Scene_Loop_Start_Special(0, 3, 0);
		Scene_Loop_Set_Default(4);
	} else if (!Game_Flag_Query(272) && Game_Flag_Query(11)) {
		Scene_Loop_Start_Special(0, 2, 0);
		Scene_Loop_Set_Default(4);
	} else if (Game_Flag_Query(225)) {
		Scene_Loop_Start_Special(0, 1, 0);
		Scene_Loop_Set_Default(4);
	} else if (Game_Flag_Query(531) == 1) {
		Scene_Loop_Set_Default(4);
	} else {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(4);
	}
}

void SceneScriptDR01::SceneLoaded() {
	Obstacle_Object("TRASH CAN WITH FIRE", true);
	Obstacle_Object("V2PYLON02", true);
	Obstacle_Object("V2PYLON04", true);
	Obstacle_Object("U2 CHEWDOOR", true);
	Obstacle_Object("MMTRASHCAN", true);
	Obstacle_Object("PARKMETR02", true);
	Obstacle_Object("TRANSFORMER 01", true);
	Obstacle_Object("TRANSFORMER 02", true);
	Obstacle_Object("PARKMETR01", true);
	Obstacle_Object("Z2TRSHCAN", true);
	Obstacle_Object("Z2ENTRYDR", true);
	Obstacle_Object("Z2DR2", true);
	Unobstacle_Object("V2 BUILD01", true);
}

bool SceneScriptDR01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptDR01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptDR01::ClickedOnActor(int actorId) {
	return actorId == 50;
}

bool SceneScriptDR01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptDR01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -835.0f, -0.04f, -118.0f, 0, 1, false, 0)) {
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -911.0f, -0.04f, -118.0f, 0, false);
			Ambient_Sounds_Adjust_Looping_Sound(112, 10, -100, 1);
			Game_Flag_Set(224);
			Set_Enter(7, kSceneDR02);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -711.0f, -0.04f, 70.0f, 0, 1, false, 0)) {
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -796.0f, -0.04f, 166.0f, 0, false);
			Game_Flag_Set(10);
			Set_Enter(7, kSceneDR04);
		}
		return true;
	}
	if (exitId == 2) {
		if (Loop_Actor_Walk_To_XYZ(kActorMcCoy, -372.0f, -0.04f, -82.0f, 0, 1, false, 0)) {
			Game_Flag_Reset(176);
			Game_Flag_Reset(182);
			Game_Flag_Reset(179);
			Game_Flag_Reset(178);
			Game_Flag_Reset(180);
			Game_Flag_Reset(261);
			Game_Flag_Reset(177);
			Game_Flag_Reset(258);
			int spinnerDest = Spinner_Interface_Choose_Dest(-1, 0);
			switch (spinnerDest) {
			case 2:
				Game_Flag_Set(182);
				Game_Flag_Reset(kFlagSpinnerToDR01);
				Game_Flag_Set(kFlagSpinnerToRC01);
				Set_Enter(69, kSceneRC01);
				break;
			case 3:
				Game_Flag_Set(176);
				Game_Flag_Reset(kFlagSpinnerToDR01);
				Game_Flag_Set(kFlagSpinnerToCT01);
				Set_Enter(4, kSceneCT01);
				break;
			case 1:
				Game_Flag_Set(179);
				Game_Flag_Reset(kFlagSpinnerToDR01);
				Game_Flag_Set(kFlagSpinnerToMA01);
				Set_Enter(49, kSceneMA01);
				break;
			case 0:
				Game_Flag_Set(178);
				Game_Flag_Reset(kFlagSpinnerToDR01);
				Game_Flag_Set(kFlagSpinnerToPS01);
				Set_Enter(61, kScenePS01);
				break;
			case 5:
				Game_Flag_Set(261);
				Game_Flag_Reset(kFlagSpinnerToDR01);
				Game_Flag_Set(kFlagSpinnerToTB02);
				Set_Enter(17, kSceneTB02);
				break;
			case 4:
				Game_Flag_Set(180);
				Game_Flag_Reset(kFlagSpinnerToDR01);
				Game_Flag_Set(kFlagSpinnerToAR01);
				Set_Enter(0, kSceneAR01);
				break;
			case 7:
				Game_Flag_Set(258);
				Game_Flag_Reset(kFlagSpinnerToDR01);
				Game_Flag_Set(kFlagSpinnerToBB01);
				Set_Enter(20, kSceneBB01);
				break;
			case 8:
				Game_Flag_Set(181);
				Game_Flag_Reset(kFlagSpinnerToDR01);
				Game_Flag_Set(kFlagSpinnerToNR01);
				Set_Enter(54, kSceneNR01);
				break;
			case 9:
				Game_Flag_Set(257);
				Game_Flag_Reset(kFlagSpinnerToDR01);
				Game_Flag_Set(kFlagSpinnerToHF01);
				Set_Enter(37, kSceneHF01);
				break;
			default:
				Player_Loses_Control();
				Game_Flag_Set(177);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -447.39f, 0.16f, -92.38f, 0, 0, true, 0);
				Player_Gains_Control();
				break;
			}
		}
		return true;
	}

	if (exitId == 3) {
		float x, y, z;
		Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
		bool v7 = false;
		if (-1200 < x) {
			v7 = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1236.4f, -0.04f, -13.91f, 0, 1, false, 0);
		}
		if (!v7) {
			Game_Flag_Set(558);
			Game_Flag_Set(176);
			Game_Flag_Reset(177);
			Set_Enter(33, kSceneCT11);
		}
		return true;
	}
	return false;
}

bool SceneScriptDR01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptDR01::SceneFrameAdvanced(int frame) {
	if (frame < 75) {
		Actor_Set_Invisible(kActorMcCoy, true);
	} else {
		Actor_Set_Invisible(kActorMcCoy, false);
	}
	if (frame == 2) {
		Ambient_Sounds_Play_Sound(487, 40, -40, 100, 99);
	}
}

void SceneScriptDR01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptDR01::PlayerWalkedIn() {
	if (Game_Flag_Query(531)) {
		Async_Actor_Walk_To_XYZ(kActorMcCoy, -757.15f, -0.04f, 24.64f, 0, false);
	} else if (!Game_Flag_Query(225) && !Game_Flag_Query(11)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -447.39f, 0.16f, -92.38f, 0, 0, false, 0);
		Player_Gains_Control();
	}
	Game_Flag_Reset(225);
	Game_Flag_Reset(11);
	Game_Flag_Reset(531);
}

void SceneScriptDR01::PlayerWalkedOut() {
	if (!Game_Flag_Query(10) && !Game_Flag_Query(224) && !Game_Flag_Query(558)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeInside2, true, -1);
	}
}

void SceneScriptDR01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
