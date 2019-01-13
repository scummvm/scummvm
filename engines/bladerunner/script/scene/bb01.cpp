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

void SceneScriptBB01::InitializeScene() {
	if (Game_Flag_Query(265)) {
		Setup_Scene_Information(-253.0f, 9.0f, 715.0f, 266);
	} else if (Game_Flag_Query(263)) {
		Setup_Scene_Information(-128.0f, 9.0f, 342.0f, 266);
	} else {
		Setup_Scene_Information(43.0f, 0.0f, 1058.0f, 0);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 0, 72, 299, 3);
	Scene_Exit_Add_2D_Exit(1, 151, 218, 322, 290, 3);
	if (Game_Flag_Query(kFlagSpinnerAtBB01)) {
		Scene_Exit_Add_2D_Exit(2, 0, 311, 312, 479, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(54, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(105, 25, -100, 0);
	Ambient_Sounds_Add_Sound(82, 5, 60, 40, 60, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(83, 5, 60, 40, 65, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(84, 5, 60, 40, 60, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(67, 5, 80, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(66, 5, 80, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(378, 5, 120, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(379, 5, 120, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(380, 5, 120, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	if (Game_Flag_Query(kFlagSpinnerAtBB01) && !Game_Flag_Query(265) && !Game_Flag_Query(263)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
	} else if (Game_Flag_Query(kFlagSpinnerAtBB01) && Game_Flag_Query(265)) {
		Scene_Loop_Set_Default(1);
	} else if (Game_Flag_Query(kFlagSpinnerAtBB01) && Game_Flag_Query(263)) {
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(5);
	}
}

void SceneScriptBB01::SceneLoaded() {
	Obstacle_Object("COLUME", true);
}

bool SceneScriptBB01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB01::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -140.0f, 9.0f, 818.0f, 0, 1, false, 0);
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -233.0f, 9.0f, 846.0f, 0, 1, false, 0)) {
			Game_Flag_Set(264);
			Game_Flag_Reset(258);
			Game_Flag_Set(177);
			Set_Enter(7, kSceneDR02);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -125.39f, 9.0f, 372.45f, 0, 1, false, 0)) {
			Game_Flag_Set(262);
			Set_Enter(1, kSceneBB02);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 43.0f, 0.0f, 1062.0f, 0, 1, false, 0)) {
			Game_Flag_Reset(176);
			Game_Flag_Reset(182);
			Game_Flag_Reset(179);
			Game_Flag_Reset(178);
			Game_Flag_Reset(258);
			int spinnerDest = Spinner_Interface_Choose_Dest(3, 0);
			switch (spinnerDest) {
			case 0:
				Game_Flag_Set(178);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtPS01);
				Set_Enter(61, kScenePS01);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 1:
				Game_Flag_Set(179);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtMA01);
				Set_Enter(49, kSceneMA01);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 2:
				Game_Flag_Set(182);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtRC01);
				Set_Enter(69, kSceneRC01);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 3:
				Game_Flag_Set(176);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtCT01);
				Set_Enter(4, kSceneCT01);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 5:
				Game_Flag_Set(261);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(17, kSceneTB02);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 4:
				Game_Flag_Set(180);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtAR01);
				Set_Enter(0, kSceneAR01);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 6:
				Game_Flag_Set(177);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtDR01);
				Set_Enter(7, kSceneDR01);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 8:
				Game_Flag_Set(181);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtNR01);
				Set_Enter(54, kSceneNR01);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 9:
				Game_Flag_Set(257);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtHF01);
				Set_Enter(37, kSceneHF01);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			default:
				Game_Flag_Set(258);
				Scene_Loop_Start_Special(2, 3, 1);
				break;
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptBB01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB01::SceneFrameAdvanced(int frame) {
	if (frame == 193) {
		Sound_Play(118, 40, 0, 0, 50);
	}
	if (frame == 241 || frame == 363) {
		Sound_Play(116, 100, -50, -50, 50);
	}
	if (frame == 286 || frame == 407) {
		Sound_Play(119, 100, -50, -50, 50);
	}
	if (frame == 433) {
		Sound_Play(117, 40, -50, 80, 50);
	}
	if (frame == 120) {
		Sound_Play(286, Random_Query(33, 33), 100, -100, 50);
	}
}

void SceneScriptBB01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB01::PlayerWalkedIn() {
	Spinner_Set_Selectable_Destination_Flag(7, 1);
	if (Game_Flag_Query(265)) {
		Game_Flag_Reset(265);
	} else if (Game_Flag_Query(263)) {
		Game_Flag_Reset(263);
	} else {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 43.0f, 0.0f, 954.0f, 0, 0, false, 0);
	}
}

void SceneScriptBB01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptBB01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
