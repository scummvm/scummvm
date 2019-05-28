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

void SceneScriptHF02::InitializeScene() {
	if (Game_Flag_Query(kFlagHF04toHF03)) {
		Setup_Scene_Information(874.0f, 47.76f, -252.0f, 775);
		Game_Flag_Reset(kFlagHF04toHF03);
	} else if (Game_Flag_Query(kFlagHF03toHF02)) {
		Setup_Scene_Information(470.0f, 47.76f, -500.0f, 560);
	} else {
		Setup_Scene_Information(-18.0f, 47.76f, -288.0f, 275);
	}

	Scene_Exit_Add_2D_Exit(0,   0,  0,  30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 207, 66, 272, 207, 3);

	Ambient_Sounds_Add_Looping_Sound(kSfxARCBED1, 28, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCIRCUS1, 33,    0, 1);
	Ambient_Sounds_Add_Sound(kSfx67_0470R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0500R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0960R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1100R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1140R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
}

void SceneScriptHF02::SceneLoaded() {
	Obstacle_Object("BARD_NEON", true);
	Unclickable_Object("BARD_NEON");
	if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleHF03McCoyChasingLucy) {
		if (Game_Flag_Query(kFlagLucyRanAway)) {
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleHF02LucyRanAway);
		} else {
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleHF02ConfrontLucy);
		}
	}
}

bool SceneScriptHF02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHF02::ClickedOn3DObject(const char *objectName, bool a2) {
	Sound_Play(kSfxFORTUNE1, 47, -80, 0, 50); // a bug? Which object should this be?
	return false;
}

bool SceneScriptHF02::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptHF02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptHF02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 42.0f, 47.76f, -296.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagHF02toHF01);
			Set_Enter(kSetHF01, kSceneHF01);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 470.0f, 47.76f, -444.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagHF02toHF03);
			Set_Enter(kSetHF03, kSceneHF03);
		}
		return true;
	}
	return false;
}

bool SceneScriptHF02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHF02::SceneFrameAdvanced(int frame) {

}

void SceneScriptHF02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHF02::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleHF02ConfrontLucy) {
		Actor_Set_Goal_Number(kActorSteele, kGoalSteeleHF02ShootLucy);
	}

	if (Game_Flag_Query(kFlagHF03toHF02)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 470.0f, 47.76f, -444.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagHF03toHF02);
	} else if (Game_Flag_Query(kFlagHF01toHF02)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 42.0f, 47.76f, -296.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagHF01toHF02);
	}

	if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleHF02LucyRanAway) {
		if (Actor_Query_Goal_Number(kActorLucy) == kGoalLucyGone) {
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleHF02LucyShotByMcCoy);
		} else {
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleHF02LucyLostByMcCoy);
		}
	}
}

void SceneScriptHF02::PlayerWalkedOut() {
}

void SceneScriptHF02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
