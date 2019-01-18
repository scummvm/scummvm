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

void SceneScriptPS07::InitializeScene() {
	Setup_Scene_Information(609.07f, 0.22f, -598.67f, 768);
	Scene_Exit_Add_2D_Exit(0, 610, 0, 639, 479, 1);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Add_Looping_Sound(141, 80, 0, 1);
	Ambient_Sounds_Add_Sound(142, 5, 20, 5, 10, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(146, 5, 30, 5, 10, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(147, 2, 20, 5, 10, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(148, 2, 10, 10, 20, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(149, 2, 10, 10, 20, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(150, 2, 10, 10, 20, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(151, 2, 10, 10, 20, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(152, 2, 30, 10, 15, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(153, 2, 20, 10, 15, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(154, 5, 20, 10, 15, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(145, 5, 30, 5, 8, -100, 100, -101, -101, 0, 0);
}

void SceneScriptPS07::SceneLoaded() {
	Obstacle_Object("RICE BOX01", true);
	Unobstacle_Object("RICE BOX01", true);
}

bool SceneScriptPS07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS07::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("L.MOUSE", objectName)) {
		Sound_Play(155, 70, 0, 0, 50);
		if (Actor_Query_Goal_Number(kActorKlein) < 4 && Actor_Query_Goal_Number(kActorKlein) > 0) {
			Actor_Face_Actor(kActorMcCoy, kActorKlein, true);
			Actor_Set_Goal_Number(kActorKlein, 3);
			Actor_Modify_Friendliness_To_Other(kActorKlein, kActorMcCoy, -3);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS07::ClickedOnActor(int actorId) {
	if (actorId == kActorKlein) {
		Actor_Face_Actor(kActorMcCoy, kActorKlein, true);
		Actor_Set_Goal_Number(kActorKlein, 3);
		if (!Game_Flag_Query(kFlagKleinTalkClues)) {
			Actor_Says(kActorMcCoy, 4115, 13);
		}
		if (!Game_Flag_Query(kFlagKleinTalkClues) && (Game_Flag_Query(kFlagMcCoyHasShellCasings) || Game_Flag_Query(kFlagMcCoyHasOfficersStatement) || Game_Flag_Query(kFlagMcCoyHasPaintTransfer) || Game_Flag_Query(kFlagMcCoyHasChromeDebris))) {
			Actor_Face_Actor(kActorKlein, kActorMcCoy, true);
			Actor_Says(kActorKlein, 30, 12);
			Game_Flag_Set(kFlagKleinTalkClues);
		} else {
			if (Game_Flag_Query(kFlagKleinTalkClues)) {
				Actor_Says(kActorMcCoy, 4130, 18);
			}
		}
		if (Game_Flag_Query(kFlagMcCoyHasShellCasings) && !Game_Flag_Query(kFlagKleinTalkShellCasings)) {
			Game_Flag_Set(kFlagKleinTalkShellCasings);
			Actor_Clue_Acquire(kActorMcCoy, kClueLabShellCasings, 0, kActorKlein);
			Actor_Says(kActorKlein, 50, 16);
			Actor_Says(kActorMcCoy, 4135, 13);
			Actor_Says(kActorKlein, 60, 15);
			Actor_Says(kActorKlein, 70, 12);
			Actor_Says(kActorMcCoy, 4140, 18);
			Actor_Says(kActorKlein, 80, 14);
			Actor_Says(kActorKlein, 90, 14);
			Actor_Set_Goal_Number(kActorKlein, 1);
			return true;
		}
		if (Game_Flag_Query(kFlagMcCoyHasOfficersStatement) && !Game_Flag_Query(kFlagKleinTalkOfficersStatement)) {
			Game_Flag_Set(kFlagKleinTalkOfficersStatement);
			Actor_Clue_Acquire(kActorMcCoy, kClueLabCorpses, 0, kActorKlein);
			Actor_Says(kActorKlein, 100, 13);
			Actor_Says(kActorMcCoy, 4145, 13);
			Actor_Says(kActorKlein, 110, 12);
			Actor_Says(kActorMcCoy, 4150, 13);
			Actor_Says(kActorKlein, 120, 14);
			Actor_Says(kActorMcCoy, 4155, 17);
			Actor_Says(kActorKlein, 130, 15);
			Actor_Says(kActorMcCoy, 4160, 13);
			Actor_Says(kActorKlein, 140, 16);
			Actor_Says(kActorMcCoy, 4165, 18);
			Actor_Says(kActorKlein, 160, 13);
			Actor_Says(kActorMcCoy, 4170, 19);
			Actor_Says(kActorMcCoy, 4175, 19);
			Actor_Modify_Friendliness_To_Other(kActorKlein, kActorMcCoy, 3);
			Actor_Set_Goal_Number(kActorKlein, 1);
			return true;
		}
		if (Game_Flag_Query(kFlagMcCoyHasPaintTransfer) && !Game_Flag_Query(kFlagKleinTalkPaintTransfer)) {
			Game_Flag_Set(kFlagKleinTalkPaintTransfer);
			Actor_Clue_Acquire(kActorMcCoy, kClueLabPaintTransfer, 0, kActorKlein);
			Actor_Says(kActorKlein, 170, 14);
			Actor_Says(kActorMcCoy, 4180, 13);
			Actor_Says(kActorKlein, 180, 12);
			Actor_Says(kActorKlein, 190, 13);
			Actor_Says(kActorKlein, 200, 16);
			Actor_Says(kActorMcCoy, 4185, 18);
			Actor_Says(kActorKlein, 210, 12);
			Actor_Modify_Friendliness_To_Other(kActorKlein, kActorMcCoy, -12);
			Actor_Set_Goal_Number(kActorKlein, 1);
			return true;
		}
		if (Game_Flag_Query(kFlagMcCoyHasChromeDebris) && !Game_Flag_Query(kFlagKleinTalkChromeDebris)) {
			Game_Flag_Set(kFlagKleinTalkChromeDebris);
			Actor_Says(kActorKlein, 220, 12);
			Actor_Says(kActorMcCoy, 4190, 13);
			Actor_Says(kActorKlein, 230, 14);
			Actor_Set_Goal_Number(kActorKlein, 1);
			return true;
		}
		Actor_Says(kActorKlein, 0, 13);
		Actor_Set_Goal_Number(kActorKlein, 1);
		return true;
	}
	return false;

}

bool SceneScriptPS07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptPS07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 609.07f, 0.22f, -598.67f, 0, 0, false, 0)) {
			Set_Enter(kSetPS02, kScenePS02);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS07::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS07::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS07::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 561.07f, 0.34f, -606.67f, 6, 0, false, 0);
	Game_Flag_Reset(kFlagPS02toPS07);
	//return false;
}

void SceneScriptPS07::PlayerWalkedOut() {
	if (!Game_Flag_Query(kFlagKleinInsulted) && Global_Variable_Query(kVariableChapter) == 1) {
		Actor_Set_Goal_Number(kActorKlein, 0);
	}
}

void SceneScriptPS07::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
