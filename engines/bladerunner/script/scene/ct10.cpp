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

void SceneScriptCT10::InitializeScene() {
	Setup_Scene_Information(-121.0f, 0.0f, -78.0f, 446);
	Game_Flag_Reset(kFlagCT09toCT08); // a bug?
	Scene_Exit_Add_2D_Exit(0, 135, 74, 238, 340, 0);
	Ambient_Sounds_Add_Looping_Sound(336, 28, 0, 1);
	Ambient_Sounds_Add_Sound(375, 6, 180, 33, 33, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 5, 180, 33, 33, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 5, 180, 33, 33, 0, 0, -101, -101, 0, 0);
}

void SceneScriptCT10::SceneLoaded() {
	Obstacle_Object("BED", true);
	Unobstacle_Object("WINDOW", true);
	Unobstacle_Object("LOFT01", true);
	Unobstacle_Object("LOFT02", true);
	Unobstacle_Object("LOFT03", true);
	Unobstacle_Object("LOFT04", true);
	Unobstacle_Object("LOFT05", true);
	Unobstacle_Object("LOFT06", true);
	Unobstacle_Object("LOFT10", true);
	Unobstacle_Object("LOFT11", true);
	Unobstacle_Object("LOFT12", true);
	Unobstacle_Object("LINE02", true);
	Unobstacle_Object("CABINETFRONT", true);
	Unobstacle_Object("CABINTESIDE", true);
	Unobstacle_Object("BUSTEDTAPE2", true);
	Unobstacle_Object("BOX CLOSET 1", true);
	Clickable_Object("BED");
	Clickable_Object("CABINETFRONT");
	Clickable_Object("CABINETTOP");
	Clickable_Object("TUB");
	Scene_2D_Region_Add(0, 379, 229, 454, 375);
}

bool SceneScriptCT10::MouseClick(int x, int y) {
	return false;
}

void SceneScriptCT10::sub_401844() {
	if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 10.6f, 0.0f, -50.5f, 0, 1, false, 0)) {
		Player_Loses_Control();
		Actor_Face_Heading(kActorMcCoy, 0, false);
		Sound_Play(339, 100, 0, 0, 50);
		Delay(1000);
		if (Actor_Clue_Query(kActorMcCoy, kClueHoldensBadge)) {
			Actor_Voice_Over(3700, kActorVoiceOver);
		} else {
			Item_Pickup_Spin_Effect(931, 435, 258);
			Actor_Clue_Acquire(kActorMcCoy, kClueHoldensBadge, true, -1);
		}
		Player_Gains_Control();
	}
}

bool SceneScriptCT10::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("TUB", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -41.0f, 0.0f, -106.0f, 0, 1, false, 0)) {
			Player_Loses_Control();
			Actor_Face_Heading(kActorMcCoy, 850, false);
			Actor_Change_Animation_Mode(kActorMcCoy, 38);
			Delay(1000);
			Sound_Play(338, 33, 0, 0, 50);
			Delay(3000);
			if (Actor_Clue_Query(kActorMcCoy, kClueStrangeScale1)) {
				Actor_Voice_Over(3700, kActorVoiceOver);
			} else {
				Actor_Clue_Acquire(kActorMcCoy, kClueStrangeScale1, true, -1);
				Item_Pickup_Spin_Effect(969, 364, 214);
			}
			Delay(1000);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -41.0f, 0.0f, -82.0f, 0, 0, false, 1);
			Player_Gains_Control();
		}
		return true;
	}
	if (Object_Query_Click("CABINETTOP", objectName) || Object_Query_Click("CABINETFRONT", objectName)) {
		sub_401844();
		return true;
	}
	return false;
}

bool SceneScriptCT10::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptCT10::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptCT10::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -93.0f, 0.0f, -38.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -121.0f, 0.0f, -78.0f, 0, 0, false, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT10toCT09);
			Set_Enter(kSetCT09, kSceneCT09);
		}
	}
	return false;
}

bool SceneScriptCT10::ClickedOn2DRegion(int region) {
	if (region == 0) {
		sub_401844();
	}
	return false;
}

void SceneScriptCT10::SceneFrameAdvanced(int frame) {
}

void SceneScriptCT10::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT10::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -93.0f, 0.0f, -38.0f, 0, 0, false, 0);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -49.0f, 0.0f, -38.0f, 0, 0, false, 0);
	if (!Game_Flag_Query(525)) {
		Actor_Voice_Over(450, kActorVoiceOver);
		Actor_Voice_Over(460, kActorVoiceOver);
		Actor_Voice_Over(470, kActorVoiceOver);
		Game_Flag_Set(525);
	}
}

void SceneScriptCT10::PlayerWalkedOut() {
}

void SceneScriptCT10::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
