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

void SceneScriptCT08::InitializeScene() {
	if (Game_Flag_Query(kFlagChapter3Intro)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeMovieB1, false, -1);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			Outtake_Play(kOuttakeMovieB2, false, -1);
		} else if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			Outtake_Play(kOuttakeMovieB3, false, -1);
		} else {
			Outtake_Play(kOuttakeMovieB4, false, -1);
		}
		Outtake_Play(kOuttakeMovieB5, false, -1);
		Game_Flag_Reset(kFlagChapter3Intro);
	}
	Actor_Force_Stop_Walking(kActorMcCoy);

	if (Game_Flag_Query(kFlagCT51toCT08)) {
		Setup_Scene_Information( -11.0f, 0.0f, -156.0f, 769);
	} else if (Game_Flag_Query(kFlagCT06toCT08)) {
		Setup_Scene_Information(-143.0f, 0.0f,  -92.0f, 420);
	} else {
		Setup_Scene_Information(-183.0f, 0.0f,  128.0f, 205);
	}

	Scene_Exit_Add_2D_Exit(0,   0,  0,  30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 389,  0, 639, 303, 0);
	Scene_Exit_Add_2D_Exit(2, 115, 87, 137, 267, 3);

	if (Game_Flag_Query(kFlagMcCoyTiedDown)) {
#if BLADERUNNER_ORIGINAL_BUGS
		Scene_2D_Region_Add(0, 185, 185, 230, 230);
#else
		Scene_2D_Region_Add(0, 155, 180, 214, 235);
#endif // BLADERUNNER_ORIGINAL_BUGS
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxRAIN10,   100, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTDRONE1,  20, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2A,  80, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  50, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR2,   5, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR3,   5, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4,  5, 180, 50, 100, 0, 0, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagCT51toCT08)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(1);
	}
}

void SceneScriptCT08::SceneLoaded() {
	Obstacle_Object("ASHTRAY", true);
	Unobstacle_Object("BLANKET03", true);
	if (!Actor_Clue_Query(kActorMcCoy, kClueRagDoll)) {
#if BLADERUNNER_ORIGINAL_BUGS
		Item_Add_To_World(kItemRagDoll, kModelAnimationRagDoll, kSetCT08_CT51_UG12, 44.0f, 0.0f, -95.0f, 540, 12, 12, false, true, false, true);
#else
		Item_Add_To_World(kItemRagDoll, kModelAnimationRagDoll, kSetCT08_CT51_UG12, 44.0f, 3.0f, -100.0f, 540, 12, 12, false, true, false, true);
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	if (!Actor_Clue_Query(kActorMcCoy, kClueCheese)) {
		Item_Add_To_World(kItemCheese, kModelAnimationCheese, kSetCT08_CT51_UG12, -102.0f, 2.0f, 41.0f, 432, 6, 6, false, true, false, true);
	}
}

bool SceneScriptCT08::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptCT08::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptCT08::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptCT08::ClickedOnItem(int itemId, bool a2) {
	if (itemId == kItemCheese) {
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemCheese, 36, true, false)) {
			if (!Game_Flag_Query(kFlagMcCoyTiedDown)) {
				Actor_Clue_Acquire(kActorMcCoy, kClueCheese, true, -1);
				Item_Pickup_Spin_Effect(kModelAnimationCheese, 266, 328);
				Item_Remove_From_World(kItemCheese);
				Actor_Voice_Over(480, kActorVoiceOver);
				Actor_Voice_Over(490, kActorVoiceOver);
				Actor_Voice_Over(500, kActorVoiceOver);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptCT08::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -183.0f, 0.0f, 128.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT08toCT09);
			Set_Enter(kSetCT09, kSceneCT09);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -11.0f, 0.0f, -156.0f, 0, true, false, false)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 0.0f, 0.0f, -102.0f, 0, false, false, false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT08toCT51);
			Set_Enter(kSetCT08_CT51_UG12, kSceneCT51);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -143.0f, 0.0f, -92.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT08toCT06);
			Set_Enter(kSetCT06, kSceneCT06);
		}
		return true;
	}
	return false;
}

bool SceneScriptCT08::ClickedOn2DRegion(int region) {
	if (region == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -108.0f, 0.0f, -178.0f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 512, false);
			Game_Flag_Reset(kFlagMcCoyTiedDown);
			Player_Set_Combat_Mode_Access(true);
			Scene_Exits_Enable();
			Ambient_Sounds_Play_Sound(kSfxBRKROPE1, 40, 99, 0, 0);
			Scene_2D_Region_Remove(0);
			Player_Loses_Control();
		}
		return true;
	}
	return false;
}

void SceneScriptCT08::SceneFrameAdvanced(int frame) {
}

void SceneScriptCT08::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT08::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagMcCoyTiedDown)) {
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeTalk);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
		Actor_Set_At_XYZ(kActorMcCoy, -148.0f, 0.0f, 4.0f, 256);
		Player_Set_Combat_Mode_Access(false);
		Scene_Exits_Disable();
		Game_Flag_Reset(kFlagCT51toCT08);
		Game_Flag_Reset(kFlagCT06toCT08);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// The player is now (teleported) in Chinatown (CT08) but the flag was not set here
		Game_Flag_Set(kFlagMcCoyInChinaTown);
		// if player clicked through fast enough in BB roof encounter, the fight music would be (auto-)saved here
		// and would be restored when loading the auto-save
		Music_Stop(0);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Autosave_Game(1);
	} else if (Game_Flag_Query(kFlagCT51toCT08)) {
		Game_Flag_Reset(kFlagCT51toCT08);
	} else if (Game_Flag_Query(kFlagCT06toCT08)) {
		Game_Flag_Reset(kFlagCT06toCT08);
	} else {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -156.0f, 0.0f, 128.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagCT09toCT08);
	}
}

void SceneScriptCT08::PlayerWalkedOut() {
	if (!Actor_Clue_Query(kActorMcCoy, kClueRagDoll)) {
		Item_Remove_From_World(kItemRagDoll);
	}
}

void SceneScriptCT08::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
