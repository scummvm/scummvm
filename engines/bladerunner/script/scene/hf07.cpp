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

void SceneScriptHF07::InitializeScene() {
	if (Game_Flag_Query(kFlagUG06toHF07) ) {
		Setup_Scene_Information(-84.0f, 58.43f, -105.0f, 524);
	} else {
		Setup_Scene_Information(298.0f, 58.43f,  -71.0f, 700);
	}

	Scene_Exit_Add_2D_Exit(0, 289, 136, 344, 305, 0);
	Scene_Exit_Add_2D_Exit(1,  69, 264, 132, 303, 2);

	Ambient_Sounds_Add_Looping_Sound(kSfxDRAMB4,   100, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxFACTAMB2,  32, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1, 5, 40, 20, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2, 5, 40, 20, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3, 5, 40, 20, 33, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagHF07Hole)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptHF07::SceneLoaded() {
	Obstacle_Object("BRIMS02", true);
	Unobstacle_Object("BOX50", true);
	Unobstacle_Object("BOX60", true);
}

bool SceneScriptHF07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHF07::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptHF07::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptHF07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptHF07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (Actor_Query_In_Set(kActorDektora, kSetHF07)) {
			Async_Actor_Walk_To_XYZ(kActorDektora, 235.0f, 58.43f, -100.0f, 0, false);
		} else if (Actor_Query_In_Set(kActorLucy, kSetHF07)) {
			Async_Actor_Walk_To_XYZ(kActorLucy, 235.0f, 58.43f, -100.0f, 0, false);
		}

		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 318.0f, 71.43f, -102.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagHF07toHF05);
			if (!Game_Flag_Query(kFlagHF01TalkToLovedOne)) {
				Actor_Face_Heading(kActorMcCoy, 0, false);
				Footstep_Sound_Override_On(3);
				Loop_Actor_Travel_Stairs(kActorMcCoy, 30, true, kAnimationModeIdle);
				Footstep_Sound_Override_Off();
			}
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetHF05, kSceneHF05);
		}
		return true;
	}

	if (exitId == 1) {
		if (Actor_Query_In_Set(kActorDektora, kSetHF07)) {
			Async_Actor_Walk_To_XYZ(kActorDektora, -73.0f, 58.43f, -7.0f, 0, false);
		} else if (Actor_Query_In_Set(kActorLucy, kSetHF07)) {
			Async_Actor_Walk_To_XYZ(kActorLucy, -73.0f, 58.43f, -7.0f, 0, false);
		}

		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -84.0f, 58.43f, -105.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagHF07toUG06);
			Set_Enter(kSetUG05, kSceneUG05);
		}
		return true;
	}
	return false;
}

bool SceneScriptHF07::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHF07::SceneFrameAdvanced(int frame) {
}

void SceneScriptHF07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHF07::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagHF01TalkToLovedOne)) {
		int actorId = getAffectionTowardsActor();
		if (Game_Flag_Query(kFlagHF01TalkToLovedOne)
		 && actorId != -1
		) {
			Actor_Put_In_Set(actorId, kSetHF07);
			if (Game_Flag_Query(kFlagUG06toHF07)) {
				Actor_Set_At_XYZ(actorId, -73.0f, 58.43f, -7.0f, 224);
			} else {
				Actor_Set_At_XYZ(actorId, 235.0f, 58.43f, -100.0f, 512);
			}
		}
	} else if (Game_Flag_Query(kFlagHF05toHF07)) {
		Actor_Set_At_XYZ(kActorMcCoy, 267.72f, 329.43f, -86.75f, 940);
		Footstep_Sound_Override_On(3);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 30, false, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
	}

	Game_Flag_Reset(kFlagHF05toHF07);
	Game_Flag_Reset(kFlagUG06toHF07);
}

void SceneScriptHF07::PlayerWalkedOut() {
}

void SceneScriptHF07::DialogueQueueFlushed(int a1) {
}

int SceneScriptHF07::getAffectionTowardsActor() {
	if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsDektora
	 && Actor_Query_Goal_Number(kActorDektora) != kGoalDektoraGone
	) {
		return kActorDektora;
	}
	if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsLucy
	 && Actor_Query_Goal_Number(kActorLucy) != kGoalLucyGone
	) {
		return kActorLucy;
	}
	return -1;
}

} // End of namespace BladeRunner
