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

void SceneScriptUG02::InitializeScene() {
	if (Game_Flag_Query(kFlagHC03toUG02)) {
		Setup_Scene_Information(-313.0f, 155.73f, -128.0f, 556);
	} else {
		Setup_Scene_Information( -95.0f,  74.78f, -503.0f, 556);
	}

	Scene_Exit_Add_2D_Exit(0, 529, 130, 607, 277, 0);
	Scene_Exit_Add_2D_Exit(1, 305,  36, 335, 192, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1, 43, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED2, 43, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5, 50, 17, 37, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5, 50, 17, 37, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPNBEEP9, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPNBEEP2, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPNBEEP3, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPNBEEP4, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPNBEEP5, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPNBEEP6, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPNBEEP7, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPNBEEP8, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCANNER1, 2, 50, 27, 27,  10,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCANNER2, 2, 50, 27, 27,  10,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCANNER3, 2, 50, 27, 27,  10,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCANNER4, 2, 50, 27, 27,  10,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCANNER5, 2, 50, 27, 27,  10,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCANNER6, 2, 50, 27, 27,  10,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCANNER7, 2, 50, 27, 27,  10,  30, -101, -101, 0, 0);
}

void SceneScriptUG02::SceneLoaded() {
	Unobstacle_Object("BOX BACKROOM 2", true);
	Unobstacle_Object("BACK_ROOM HALFWALL_", true);
	Unobstacle_Object("GUN_4", true);
	Obstacle_Object("GUN_1", true);
	Unobstacle_Object("WALL_LEFT", true);
	Unobstacle_Object("BOX BY STAIRS 1", true);
	Unobstacle_Object("TANK", true);
	Unobstacle_Object("DESK_DRUM", true);
	Clickable_Object("GUN_1");
	Clickable_Object("GUN_2");
	Clickable_Object("CRATE_3");
	Footstep_Sounds_Set(0, 0);
	Footstep_Sounds_Set(8, 2);

	if (!Game_Flag_Query(kFlagUG02RadiationGogglesTaken)
	  && Game_Flag_Query(kFlagIzoIsReplicant)
	) {
		Item_Add_To_World(kItemRadiationGoogles, kModelAnimationRadiationGoggles, kSetUG02, -300.37f, 120.16f, -81.31f, 0, 8, 8, false, true, false, true);
	}
}

bool SceneScriptUG02::MouseClick(int x, int y) {
	if (Game_Flag_Query(kFlagUG02Interactive)) {
		return false;
	}

	if (Region_Check(0, 0, 245, 285)
	 || Region_Check(0, 0, 350, 257)
	) {
		return true;
	}

	if ( Region_Check(81, 224, 639, 479)
	 && !Game_Flag_Query(kFlagUG02FromUG01)
	) {
		Game_Flag_Set(kFlagUG02Interactive);
		walkToCenter();
		Game_Flag_Reset(kFlagUG02Interactive);
		return true;
	}

	return false;
}

bool SceneScriptUG02::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("GUN_1", objectName)
	 || Object_Query_Click("GUN_2", objectName)
	 || Object_Query_Click("CRATE_3", objectName)
	) {
		Actor_Face_Object(kActorMcCoy, "GUN_1", true);
		if (!Game_Flag_Query(kFlagUG02WeaponsChecked)
		 &&  Global_Variable_Query(kVariableChapter) < 4
		) {
			Actor_Voice_Over(2430, kActorVoiceOver);
			Actor_Voice_Over(2440, kActorVoiceOver);
			Actor_Voice_Over(2450, kActorVoiceOver);
			Actor_Voice_Over(2460, kActorVoiceOver);
			Game_Flag_Set(kFlagUG02WeaponsChecked);
			Actor_Clue_Acquire(kActorMcCoy, kClueWeaponsCache, true, -1);
			return true;
		}

		if (Global_Variable_Query(kVariableChapter) > 3) {
			if ( Actor_Clue_Query(kActorMcCoy, kClueWeaponsCache)
			 && !Actor_Clue_Query(kActorMcCoy, kClueIzosStashRaided)
			) {
				Actor_Voice_Over(2470, kActorVoiceOver);
				Actor_Voice_Over(2480, kActorVoiceOver);
				Actor_Voice_Over(2490, kActorVoiceOver);
				Actor_Voice_Over(2500, kActorVoiceOver);
				Actor_Clue_Acquire(kActorMcCoy, kClueIzosStashRaided, true, -1);
			} else if (!Actor_Clue_Query(kActorMcCoy, kClueWeaponsCache)) {
				Actor_Voice_Over(2510, kActorVoiceOver);
				Actor_Voice_Over(2520, kActorVoiceOver);
#if BLADERUNNER_ORIGINAL_BUGS
				Actor_Voice_Over(2530, kActorVoiceOver); // But there was no way to tell what was missing without Izo standing there checking his inventory.
#else
				// This voice over says that Izo is there in Chapter 4 checking out his weapon's cache
				// This does not happen in the original game (Izo being there) although this case can be triggered
				// TODO Restore this quote ONLY IF we restore the related cut-content so that Izo may appear here in Chapter 4.
				//Actor_Voice_Over(2530, kActorVoiceOver); // But there was no way to tell what was missing without Izo standing there checking his inventory.
#endif // BLADERUNNER_ORIGINAL_BUGS
			} else if (!Game_Flag_Query(kFlagUG02AmmoTaken)) {
				Item_Pickup_Spin_Effect(kModelAnimationAmmoType02, 360, 440);
				Actor_Says(kActorMcCoy, 8525, 14);
				Give_McCoy_Ammo(2, 18);
				Game_Flag_Set(kFlagUG02AmmoTaken);
			} else {
				Actor_Says(kActorMcCoy, 8580, 14);
			}
			return true;
		}

		Actor_Says(kActorMcCoy, 8580, 14);
	}
	return false;
}

bool SceneScriptUG02::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG02::ClickedOnItem(int itemId, bool a2) {
	if (itemId == kItemRadiationGoogles) {
		Actor_Face_Item(kActorMcCoy, kItemRadiationGoogles, true);
		Actor_Clue_Acquire(kActorMcCoy, kClueRadiationGoggles, true, -1);
		Game_Flag_Set(kFlagUG02RadiationGogglesTaken);
		Item_Remove_From_World(kItemRadiationGoogles);
		Item_Pickup_Spin_Effect(kModelAnimationRadiationGoggles, 426, 316);
		return true;
	}
	return false;
}

bool SceneScriptUG02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if ( Game_Flag_Query(kFlagUG02FromUG01)
		 || !walkToCenter()
		) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -202.0f, 120.16f, -74.0f, 0, true, Player_Query_Combat_Mode(), false)) {
				Actor_Face_Heading(kActorMcCoy, 270, false);
				Footstep_Sound_Override_On(2);
				Loop_Actor_Travel_Stairs(kActorMcCoy, 4, false, kAnimationModeIdle);
				Footstep_Sound_Override_Off();
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -96.57f, 74.87f, -271.28f, 0, false, Player_Query_Combat_Mode(), false);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -95.0f, 74.87f, -503.0f, 0, false, Player_Query_Combat_Mode(), false);
				Game_Flag_Set(kFlagUG02toUG01);
				Set_Enter(kSetUG01, kSceneUG01);
			}
		}
		return true;
	}

	if (exitId == 1) {
		if (Game_Flag_Query(kFlagUG02FromUG01)) {
			if (walkToCenter()) {
				return true;
			}
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -368.75f, 155.75f,  -63.0f, 0, false, false, false);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -340.75f, 155.75f, -119.0f, 0, false, false, false);
		}
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -304.75f, 155.75f, -171.0f, 0, false, false, false);
		Actor_Face_Heading(kActorMcCoy, 14, false);
		Loop_Actor_Travel_Ladder(kActorMcCoy, 9, true, kAnimationModeIdle);
		Game_Flag_Set(kFlagUG02toHC03);
		Game_Flag_Reset(kFlagMcCoyInUnderground);
		Game_Flag_Set(kFlagMcCoyInHawkersCircle);
		if (!Game_Flag_Query(kFlagHC03CageOpen)) {
			Game_Flag_Set(kFlagHC03TrapDoorOpen);
			Game_Flag_Set(kFlagHC03CageOpen);
			Item_Remove_From_World(kItemGreenPawnLock);
		}
		Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC03);
		return true;
	}
	return false;
}

bool SceneScriptUG02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG02::SceneFrameAdvanced(int frame) {
	//return true;
}

void SceneScriptUG02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG02::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagUG01toUG02)) {
		Actor_Set_At_XYZ(kActorMcCoy, -106.01f, 84.13f, -228.62f, 575);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -148.0f, 84.13f, -67.0f, 0, false, false, false);
		Actor_Face_Heading(kActorMcCoy, 761, false);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 4, true, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
		Game_Flag_Reset(kFlagUG01toUG02);
		Game_Flag_Set(kFlagUG02FromUG01);
	} else if (Game_Flag_Query(kFlagHC03toUG02)) {
		Actor_Set_At_XYZ(kActorMcCoy, -304.75f, 265.0f, -171.0f, 0);
		Loop_Actor_Travel_Ladder(kActorMcCoy, 9, false, kAnimationModeIdle);
		Game_Flag_Reset(kFlagHC03toUG02);
		Game_Flag_Reset(kFlagUG02FromUG01);
	} else {
		Actor_Set_At_XYZ(kActorMcCoy, -269.24f, 120.16f, -9.94f, 477);
		Game_Flag_Set(kFlagUG02FromUG01);
	}
	Game_Flag_Reset(kFlagUG02Interactive);
	//return false;
}

void SceneScriptUG02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptUG02::DialogueQueueFlushed(int a1) {
}

bool SceneScriptUG02::walkToCenter() {
	if (!Game_Flag_Query(kFlagUG02FromUG01)) {
		int combatMode = Player_Query_Combat_Mode();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -340.75f, 155.75f, -119.0f, 0, false, combatMode, false);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -368.75f, 155.75f,  -63.0f, 0, false, combatMode, false);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy,  -365.0f, 155.65f,  -19.0f, 0, false, combatMode, false);
		Actor_Face_Heading(kActorMcCoy, 318, false);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 4, false, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
		Game_Flag_Set(kFlagUG02FromUG01);
		return false;
	}

	if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -312.75f, 120.16f, 1.01f, 0, true, false, false)) {
		Actor_Face_Heading(kActorMcCoy, 830, false);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 4, true, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
		Game_Flag_Reset(kFlagUG02FromUG01);
		return false;
	}

	return true;
}

} // End of namespace BladeRunner
