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

void SceneScriptCT06::InitializeScene() {
	if (Game_Flag_Query(kFlagCT05toCT06)) {
		Setup_Scene_Information(20.41f, -58.23f, 2.17f, 247);
		Game_Flag_Reset(kFlagCT05toCT06);
	} else if (Game_Flag_Query(kFlagCT07toCT06)) {
		Setup_Scene_Information(203.91f, -58.02f, 0.47f, 768);
	} else {
		Setup_Scene_Information(175.91f, -58.23f, 24.47f, 768);
	}
	Scene_Exit_Add_2D_Exit(0,   0, 440, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 401, 162, 536, 317, 0);
	if (Game_Flag_Query(kFlagZubenRetired) && Actor_Query_In_Set(kActorZuben, kSetCT07)) {
		Actor_Put_In_Set(kActorZuben, kSetCT06);
		Actor_Set_At_XYZ(kActorZuben, 58.41f, -58.23f, -24.97f, 240);
		Actor_Retired_Here(kActorZuben, 72, 36, 1, kActorMcCoy);
	}
	Ambient_Sounds_Add_Looping_Sound(kSfxRAIN10,   100, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTDRONE1,  20, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2A,  80, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  50, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR2,  50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR3,  70, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT06HideAtFreeSlotA) {
		Ambient_Sounds_Add_Sound(kSfxZUBWLK1, 1, 5, 25, 25, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfxZUBWLK2, 1, 5, 25, 25, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfxZUBWLK3, 1, 5, 25, 25, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfxZUBWLK4, 1, 5, 25, 25, -100, 100, -101, -101, 0, 0);
	}
}

void SceneScriptCT06::SceneLoaded() {
	Obstacle_Object("BOX02", true);
	Obstacle_Object("CB BOX01", true);
	Obstacle_Object("CB BOX02", true);
	Obstacle_Object("CB BOX03", true);
	Unobstacle_Object("INSULPIP01", true);
	Unobstacle_Object("CB BOX04", true);
	Unclickable_Object("DOOR");
	if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT06HideAtFreeSlotA) {
		Preload(kModelAnimationMcCoyWithGunWalking);
		Preload(kModelAnimationMcCoyWithGunRunning);
		Preload(kModelAnimationZubenCombatHitFront);
		Preload(kModelAnimationZubenCombatHitBack);
		Preload(kModelAnimationZubenCleaverAttack);
		Preload(kModelAnimationZubenJumpDownFromCeiling);
		Preload(kModelAnimationZubenJumpDownFromCeiling); // A bug? Why is this preloaded twice?
	}
}

bool SceneScriptCT06::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptCT06::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptCT06::ClickedOnActor(int actorId) {
	if (actorId == kActorZuben) {
		Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorZuben, 24, true, false);
		Actor_Face_Actor(kActorMcCoy, kActorZuben, true);
		if (!Game_Flag_Query(kFlagCT06ZubenPhoto)) {
			Actor_Clue_Acquire(kActorMcCoy, kClueZubenSquadPhoto, true, -1);
			Item_Pickup_Spin_Effect(kModelAnimationPhoto, 340, 369);
			Actor_Voice_Over(350, kActorVoiceOver);
			Actor_Voice_Over(360, kActorVoiceOver);
			Actor_Voice_Over(370, kActorVoiceOver);
			if (!Game_Flag_Query(kFlagDirectorsCut)) {
				Actor_Voice_Over(380, kActorVoiceOver);
				Actor_Voice_Over(390, kActorVoiceOver);
				Actor_Voice_Over(400, kActorVoiceOver);
				Actor_Voice_Over(410, kActorVoiceOver);
			}
			Game_Flag_Set(kFlagCT06ZubenPhoto);
			return true;
		}
		Actor_Says(kActorMcCoy, 8570, 13);
		return false;
	}
	return false;
}

bool SceneScriptCT06::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptCT06::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 20.41f, -58.23f, -2.17f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT06toCT05);
			Set_Enter(kSetCT05, kSceneCT05);
			Game_Flag_Reset(kFlagNotUsed212);
		}
		return true;
	}
	if (exitId == 1) {
		if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT06HideAtFreeSlotA) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 203.91f, -58.02f, 0.47f, 0, true, false, false)) {
				Ambient_Sounds_Remove_Sound(kSfxZUBWLK1, true);
				Ambient_Sounds_Remove_Sound(kSfxZUBWLK2, true);
				Ambient_Sounds_Remove_Sound(kSfxZUBWLK3, true);
				Ambient_Sounds_Remove_Sound(kSfxZUBWLK4, true);
				Player_Loses_Control();
				Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT06JumpDown);
				Game_Flag_Reset(kFlagNotUsed212);
			}
			return true;
		}
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 203.91f, -58.02f, 0.47f, 0, true, false, false)) {
			if (Global_Variable_Query(kVariableChapter) < 3) {
				Actor_Face_Object(kActorMcCoy, "DOOR", true);
				Actor_Says(kActorMcCoy, 8522, 12);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(kFlagCT06toCT08);
				Set_Enter(kSetCT08_CT51_UG12, kSceneCT08);
				Game_Flag_Reset(kFlagNotUsed212);
			}
		}
	}
	return false;
}

bool SceneScriptCT06::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptCT06::SceneFrameAdvanced(int frame) {
}

void SceneScriptCT06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT06::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagCT07toCT06)) {
		Game_Flag_Reset(kFlagCT07toCT06);
	}
}

void SceneScriptCT06::PlayerWalkedOut() {
}

void SceneScriptCT06::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
