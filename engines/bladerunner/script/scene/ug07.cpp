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

void SceneScriptUG07::InitializeScene() {
	if (Game_Flag_Query(428)) {
		Setup_Scene_Information(-76.0f, -12.21f, -738.0f, 505);
		Game_Flag_Reset(428);
	} else if (Game_Flag_Query(426)) {
		Setup_Scene_Information(110.0f, -12.21f, -276.0f, 605);
	} else {
		Setup_Scene_Information(-10.0f, -12.21f, -58.0f, 0);
		Game_Flag_Reset(424);
	}
	if (Game_Flag_Query(623)) {
		Scene_Exit_Add_2D_Exit(0, 0, 192, 51, 334, 0);
		Scene_Exit_Add_2D_Exit(1, 226, 224, 314, 396, 1);
	}
	Scene_Exit_Add_2D_Exit(2, 60, 440, 460, 479, 2);
	Ambient_Sounds_Add_Looping_Sound(105, 90, -45, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	Ambient_Sounds_Add_Sound(368, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(402, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(395, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(398, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(291, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(292, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(294, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
}

void SceneScriptUG07::SceneLoaded() {
	Obstacle_Object("BOX RIGHT WALL 01", true);
	Obstacle_Object("BOX RIGHT WALL 09", true);
	Obstacle_Object("SLIDING DOOR", true);
	Unobstacle_Object("BOX LEFT WALL 01", true);
	Unclickable_Object("BOX RIGHT WALL 09");
	Unclickable_Object("BOX RIGHT WALL 01");
	Unclickable_Object("SLIDING DOOR");
	Unobstacle_Object("BOX FOR WALL LEFT02", true);
	Unobstacle_Object("BOX FOR WALL LEFT03", true);
	Unobstacle_Object("BOX FOR WALL LEFT05", true);
	Unobstacle_Object("BOX FOR WALL LEFT07", true);
	Unobstacle_Object("BOX FOR WALL LEFT09", true);
	Unobstacle_Object("BOX FOR WALL LEFT10", true);
	Unobstacle_Object("BOX FOR WALL LEFT11", true);
	Unobstacle_Object("BOX FOR WALL LEFT12", true);
	Unobstacle_Object("BOX FOR WALL LEFT13", true);
}

bool SceneScriptUG07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG07::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG07::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -94.0f, -12.21f, -710.0f, 0, 1, false, 0) && Actor_Query_Goal_Number(kActorClovis) != 402) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(427);
			Set_Enter(kSetUG09, kSceneUG09);
			return true;
		}
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 110.0f, -12.21f, -274.0f, 0, 1, false, 0) && Actor_Query_Goal_Number(kActorClovis) != 402) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(425);
			Set_Enter(kSetUG08, kSceneUG08);
			return true;
		}
	}
	if (exitId == 2) {
		if (!Game_Flag_Query(607) && Game_Flag_Query(671) && Global_Variable_Query(kVariableChapter) == 4 && !Game_Flag_Query(598)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 49.0f, -12.21f, -130.0f, 0, 1, false, 0)) {
				Game_Flag_Set(598);
				Actor_Put_In_Set(kActorClovis, kSetUG07);
				Actor_Set_At_XYZ(kActorClovis, 118.02f, -12.21f, -154.0f, 768);
				Player_Set_Combat_Mode(true);
				Actor_Face_Actor(kActorMcCoy, kActorClovis, true);
				Loop_Actor_Walk_To_XYZ(kActorClovis, 98.02f, -12.21f, -154.0f, 0, 0, false, 0);
				Actor_Face_Actor(kActorClovis, kActorMcCoy, true);
				Actor_Set_Goal_Number(kActorMcCoy, 301);
				Actor_Face_Heading(kActorMcCoy, 0, true);
				Delay(1500);
				Actor_Says_With_Pause(kActorClovis, 550, 1.0f, 3);
				if (Actor_Clue_Query(kActorClovis, kClueMcCoyRetiredDektora)) {
					Actor_Says(kActorClovis, 560, 3);
					Actor_Set_Goal_Number(kActorMcCoy, 302);
					Sound_Play(561, 100, 0, 0, 50);
					Delay(2000);
				}
				if (Actor_Clue_Query(kActorClovis, kClueMcCoyRetiredLucy)) {
					Actor_Says(kActorClovis, 570, 3);
					Actor_Set_Goal_Number(kActorMcCoy, 302);
					Sound_Play(561, 100, 0, 0, 50);
					Delay(2000);
				}
				Actor_Set_Goal_Number(kActorMcCoy, 303);
				Delay(1000);
				Actor_Set_Goal_Number(kActorClovis, 401);
			}
		} else {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -10.0f, -21.47f, -58.0f, 0, 1, false, 0) && Actor_Query_Goal_Number(kActorClovis) != 402) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(423);
				Set_Enter(kSetUG10, kSceneUG10);
				return true;
			}
		}
	}
	return false;
}

bool SceneScriptUG07::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG07::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG07::PlayerWalkedIn() {
	if (Global_Variable_Query(kVariableChapter) == 4 && !Game_Flag_Query(623)) {
		Actor_Set_Goal_Number(kActorOfficerLeary, 307);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 307);
	}
	if (Game_Flag_Query(426)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 62.0f, -12.21f, -274.0f, 0, 0, false, 0);
		Game_Flag_Reset(426);
	}
}

void SceneScriptUG07::PlayerWalkedOut() {
	if (Global_Variable_Query(kVariableChapter) == 4 && (Actor_Query_Goal_Number(kActorOfficerLeary) == 307 || Actor_Query_Goal_Number(kActorOfficerGrayford) == 307)) {
		Non_Player_Actor_Combat_Mode_Off(kActorOfficerLeary);
		Non_Player_Actor_Combat_Mode_Off(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerLeary, 306);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 306);
	}
	if (Actor_Query_In_Set(kActorClovis, kSetUG07)) {
		Actor_Set_Goal_Number(kActorClovis, 400);
	}
}

void SceneScriptUG07::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
