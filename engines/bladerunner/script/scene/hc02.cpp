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

void SceneScriptHC02::InitializeScene() {
	Music_Play(kMusicOneTime, 45, -60, 1, -1, 1, 3);
	if (Game_Flag_Query(kFlagHC04toHC02)) {
		Setup_Scene_Information(-88.0f, 0.14f, -463.0f, 540);
	} else {
		Setup_Scene_Information(-57.0f, 0.14f,   83.0f, 746);
	}

	Scene_Exit_Add_2D_Exit(0, 589, 255, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 505,   0, 639, 170, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 50, 50, 0);
	Ambient_Sounds_Add_Looping_Sound(kSfxBARAMB1,  50, 50, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX1,  3, 60, 33, 33,  -60,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX3,  3, 60, 33, 33,  -60,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX4,  3, 60, 33, 33,  -60,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX5,  3, 60, 33, 33,  -60,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX6,  3, 60, 33, 33,  -60,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX7,  3, 60, 33, 33,  -60,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK1,   3, 60, 33, 33, -100,  20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK2,   3, 60, 33, 33, -100,  20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK3,   3, 60, 33, 33, -100,  20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK4,   3, 60, 33, 33, -100,  20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM8,   3, 30, 14, 14,   30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM2,   3, 30, 14, 14,   30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM3,   3, 30, 14, 14,   30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM4,   3, 30, 14, 14,   30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM5,   3, 30, 14, 14,   30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM6,   3, 30, 14, 14,   30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM7,   3, 30, 14, 14,   30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM1,   3, 30, 14, 14,   30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCBELL1,  3, 50, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxINDFLUT1, 3, 50, 25, 25, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagHC01toHC02)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
		Game_Flag_Reset(kFlagHC01toHC02);
	} else {
		Scene_Loop_Set_Default(1);
	}
}

void SceneScriptHC02::SceneLoaded() {
	Obstacle_Object("BARSTOOL01", true);
}

bool SceneScriptHC02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHC02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptHC02::ClickedOnActor(int actorId) {
	if (actorId == kActorHawkersBarkeep) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -150.51f, 0.14f, 62.74f, 0, true, false, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorHawkersBarkeep, true);
			if (!Game_Flag_Query(kFlagHC02HawkersBarkeepIntroduction)) {
				Actor_Says(kActorMcCoy, 1225, 13);
				Actor_Says_With_Pause(kActorHawkersBarkeep, 0, 0.0f, 13);
				Actor_Says(kActorHawkersBarkeep, 10, 16);
				Actor_Set_Goal_Number(kActorHawkersBarkeep, 1);
				Actor_Change_Animation_Mode(kActorMcCoy, 23);
				Delay(1500);
				Actor_Change_Animation_Mode(kActorMcCoy, 75);
				Delay(1500);
				Global_Variable_Increment(kVariableMcCoyDrinks, 1);
				Game_Flag_Set(kFlagHC02HawkersBarkeepIntroduction);
			} else if ( Actor_Clue_Query(kActorMcCoy, kClueChinaBarSecurityCamera)
			        && !Actor_Clue_Query(kActorMcCoy, kClueChinaBarSecurityDisc)
			) {
				Actor_Says(kActorMcCoy, 4545, 11);
				Actor_Says(kActorHawkersBarkeep, 120, 12);
				Actor_Says(kActorHawkersBarkeep, 180, 13);
				Actor_Clue_Acquire(kActorMcCoy, kClueChinaBarSecurityDisc, true, kActorHawkersBarkeep);
				Item_Pickup_Spin_Effect(kModelAnimationVideoDisc, 229, 215);
			} else if (Actor_Clue_Query(kActorMcCoy, kClueHomelessManInterview1)
			           && !Actor_Clue_Query(kActorMcCoy, kClueFlaskOfAbsinthe)
#if !BLADERUNNER_ORIGINAL_BUGS
			           // don't re-get the flask if McCoy already gave it to the transient (he loses the kClueFlaskOfAbsinthe clue when he does)
			           && !Actor_Clue_Query(kActorTransient, kClueFlaskOfAbsinthe)
#endif // !BLADERUNNER_ORIGINAL_BUGS
			           && (Global_Variable_Query(kVariableChinyen) > 20
			               || Query_Difficulty_Level() == kGameDifficultyEasy)
			) {
				Actor_Clue_Acquire(kActorMcCoy, kClueFlaskOfAbsinthe, true, kActorHawkersBarkeep);
				Actor_Says(kActorMcCoy, 1230, 13);
				Actor_Says(kActorHawkersBarkeep, 20, 12);
				Actor_Says(kActorMcCoy, 1235, 13);
				Actor_Says(kActorHawkersBarkeep, 30, 15);
				Actor_Says(kActorMcCoy, 1240, 13);
				Actor_Says(kActorHawkersBarkeep, 40, 14);
				Item_Pickup_Spin_Effect(kModelAnimationFlaskOfAbsinthe, 229, 215);
				Actor_Set_Goal_Number(kActorHawkersBarkeep, 2);
				Actor_Change_Animation_Mode(kActorMcCoy, 23);
				Delay(1500);
				Actor_Says_With_Pause(kActorHawkersBarkeep, 50, 1.6f, 17);
				if (Query_Difficulty_Level() != kGameDifficultyEasy) {
					Global_Variable_Decrement(kVariableChinyen, 20);
				}
				Actor_Says(kActorMcCoy, 1245, 13);
			} else {
				if ( Actor_Clue_Query(kActorMcCoy, kClueMaggieBracelet)
				 && !Game_Flag_Query(kFlagHC02HawkersBarkeepBraceletTalk)
				) {
					Actor_Says(kActorHawkersBarkeep, 80, 16);
					Actor_Says(kActorMcCoy, 1265, 13);
					Actor_Says(kActorHawkersBarkeep, 90, 13);
					Game_Flag_Set(kFlagHC02HawkersBarkeepBraceletTalk);
				}
				if (Global_Variable_Query(kVariableChinyen) > 5
				 || Query_Difficulty_Level() == kGameDifficultyEasy
				) {
					if (Query_Difficulty_Level() != kGameDifficultyEasy) {
						Global_Variable_Decrement(kVariableChinyen, 5);
					}
					Global_Variable_Increment(kVariableMcCoyDrinks, 1);
					Actor_Says(kActorMcCoy, 1250, 13);
					Actor_Says_With_Pause(kActorHawkersBarkeep, 60, 0.8f, 14);
					Actor_Says(kActorMcCoy, 1255, 13);
					Actor_Set_Goal_Number(kActorHawkersBarkeep, 1);
					Actor_Change_Animation_Mode(kActorMcCoy, 23);
					Delay(1500);
					Actor_Change_Animation_Mode(kActorMcCoy, 75);
					Delay(1500);
				} else {
					Actor_Says_With_Pause(kActorMcCoy, 1260, 0.3f, 13);
					Actor_Says(kActorHawkersBarkeep, 70, 14);
				}
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptHC02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptHC02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 7.0f, 0.14f, 79.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagHC02toHC01);
			Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC01);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -76.0f, 0.14f, -339.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagHC02toHC04);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -88.0f, 0.14f, -463.0f, 0, false);
			Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC04);
		}
		return true;
	}
	return false;
}

bool SceneScriptHC02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHC02::SceneFrameAdvanced(int frame) {
	if (frame == 70) {
		Sound_Play(kSfxSWEEP3, 11, 50, -90, 50);
	}

	if (frame == 58) {
		Sound_Play(kSfxSWEEP3, 11, 50, -90, 50);
	}

	if (frame == 69
	 || frame == 77
	 || frame == 86
	 || frame == 95
	 || frame == 104
	 || frame == 113
	 || frame == 119
	) {
		Sound_Play(kSfxNEON6, Random_Query(6, 7), -20, 20, 50);
	}
}

void SceneScriptHC02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHC02::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagHC04toHC02)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -76.0f, 0.14f, -339.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagHC04toHC02);
	}
}

void SceneScriptHC02::PlayerWalkedOut() {
}

void SceneScriptHC02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
