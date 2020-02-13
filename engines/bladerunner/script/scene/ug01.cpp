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

enum kUG01Loops {
	kUG01LoopMainLoop         = 0, //   0 -  60
	kUG01LoopSteamDissapating = 2, //  61 - 120
	kUG01LoopMainLoopNoSteam  = 3  // 121 - 181
};

void SceneScriptUG01::InitializeScene() {
	if (Game_Flag_Query(kFlagUG10toUG01)) {
		Setup_Scene_Information(34.47f, -50.13f, -924.11f, 500);
		Game_Flag_Reset(kFlagUG10toUG01);
	} else if (Game_Flag_Query(kFlagRC03toUG01)) {
		Setup_Scene_Information(-68.0f, -50.13f, -504.0f, 377);
	} else {
		Setup_Scene_Information(-126.0f, -50.13f, -286.0f, 0);
	}
	Scene_Exit_Add_2D_Exit(0, 280, 204, 330, 265, 0);
	Scene_Exit_Add_2D_Exit(1, 144,   0, 210, 104, 0);
	Scene_Exit_Add_2D_Exit(2,   0, 173, 139, 402, 3);

#if BLADERUNNER_ORIGINAL_BUGS
	Ambient_Sounds_Add_Looping_Sound(kSfxSTMLOOP7, 28, 0, 1);
#else
	if (!Game_Flag_Query(kFlagUG01SteamOff)) {
		Ambient_Sounds_Add_Looping_Sound(kSfxSTMLOOP7, 28, 0, 1);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED2,   40, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP1,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP3,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M3,  2, 120, 10, 11,   20, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M1, 2, 120, 10, 11,   20, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M2, 2, 120, 10, 11,   20, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M3, 2, 120, 10, 11,   20, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP4,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP5,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPIPER1,   2, 190, 12, 16,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK2,  2, 190, 12, 16,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK5,  2, 190, 12, 16,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG1,    2, 190, 12, 16,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG4,    2, 190, 12, 16,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG5,    2, 190, 12, 16,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG6,    2, 190, 12, 16,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 17, 37,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 17, 37,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 17, 37,    0, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagUG01SteamOff)) {
		Scene_Loop_Set_Default(kUG01LoopMainLoopNoSteam);
	} else {
		Scene_Loop_Set_Default(kUG01LoopMainLoop);
	}
}

void SceneScriptUG01::SceneLoaded() {
	Unobstacle_Object("BEAM02", true);
	Unobstacle_Object("BEAM03", true);
	Unobstacle_Object("BEAM04", true);
	Clickable_Object("PIPES_FG_LFT");
#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (Game_Flag_Query(kFlagUG01SteamOff)) {
		Screen_Effect_Skip(0, true);
	}

	// TODO: Is there a possibility that the fog boxes
	//       start with this density by default,
	//       so we don't have to set it explicitly?
	if (!Game_Flag_Query(kFlagUG01SteamOff)) {
		float density = 60.0f / 29500.0f;
		Set_Fog_Density("BoxFog01", density);
		Set_Fog_Density("BoxFog02", density);
		Set_Fog_Density("BoxFog03", density);
		Set_Fog_Density("BoxFog04", density);
	} else {
		Set_Fog_Density("BoxFog01", 0.0f);
		Set_Fog_Density("BoxFog02", 0.0f);
		Set_Fog_Density("BoxFog03", 0.0f);
		Set_Fog_Density("BoxFog04", 0.0f);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptUG01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG01::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("PIPES_FG_LFT", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -9.0f, -50.13f, -148.0f, 0, true, false, false)
		    && !Game_Flag_Query(kFlagUG01SteamOff)
		) {
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorMcCoy, 8525, 13);
			Scene_Loop_Set_Default(kUG01LoopMainLoopNoSteam);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, kUG01LoopSteamDissapating, true);
#else
			Sound_Play(kSfxSQUEAK1,  40, 0, 0, 50);
			Screen_Effect_Skip(0, true);
			Scene_Loop_Set_Default(kUG01LoopMainLoopNoSteam);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, kUG01LoopSteamDissapating, false);
			Sound_Play(kSfxSTEAM6A,  40, 0, 0, 50);
			Ambient_Sounds_Remove_Looping_Sound(kSfxSTMLOOP7, 2);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Game_Flag_Set(kFlagUG01SteamOff);
		} else {
			Actor_Says(kActorMcCoy, 8525, 13);
		}
	}
	return false;
}

bool SceneScriptUG01::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -32.0f, -50.13f, -1350.0f, 12, true, false, false)) {
			Game_Flag_Set(kFlagUG01toUG10);
			Set_Enter(kSetUG10, kSceneUG10);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -70.0f, -50.13f, -500.0f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 768, false);
			Loop_Actor_Travel_Ladder(kActorMcCoy, 12, true, kAnimationModeIdle);
			Game_Flag_Set(kFlagUG01toRC03);
			Game_Flag_Reset(kFlagMcCoyInUnderground);
			Game_Flag_Set(kFlagMcCoyInRunciters);
			Set_Enter(kSetRC03, kSceneRC03);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -126.0f, -50.13f, -286.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagUG01toUG02);
			Set_Enter(kSetUG02, kSceneUG02);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG01::SceneFrameAdvanced(int frame) {
	if (frame >= 61 && frame <= 120) {
		// fog dispersing
		// Divide with a large number because otherwise, thick fog appears too white
		float density = (120.0f - frame) / 29500.0f;
		Set_Fog_Density("BoxFog01", density);
		Set_Fog_Density("BoxFog02", density);
		Set_Fog_Density("BoxFog03", density);
		Set_Fog_Density("BoxFog04", density);
	} else if (frame > 120) {
		// fog dispersed
		// TODO does it have to constantly be set?
		Set_Fog_Density("BoxFog01", 0.0f);
		Set_Fog_Density("BoxFog02", 0.0f);
		Set_Fog_Density("BoxFog03", 0.0f);
		Set_Fog_Density("BoxFog04", 0.0f);
	}
	//return false;
}

void SceneScriptUG01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG01::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagUG02toUG01)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -55.0f, -50.13f, -288.0f, 12, false, false, false);
		Game_Flag_Reset(kFlagUG02toUG01);
	}

	if (Game_Flag_Query(kFlagRC03toUG01)) {
		Actor_Set_At_XYZ(kActorMcCoy, -70.0f, 93.87f, -500.0f, 768);
		Loop_Actor_Travel_Ladder(kActorMcCoy, 12, false, kAnimationModeIdle);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -58.0f, -50.13f, -488.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagRC03toUG01);
	}

	if (Actor_Query_Goal_Number(kActorLucy) == kGoalLucyUG01Wait) {
		Music_Play(kMusicLoveSong, 35, 0, 3, -1, 0, 0);
		Actor_Set_Goal_Number(kActorLucy, kGoalLucyUG01VoightKampff);
	}
	//return false;
}

void SceneScriptUG01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Screen_Effect_Restore_All(false);
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptUG01::DialogueQueueFlushed(int a1) {
}


} // End of namespace BladeRunner
