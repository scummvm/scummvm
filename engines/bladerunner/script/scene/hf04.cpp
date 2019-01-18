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

void SceneScriptHF04::InitializeScene() {
	Setup_Scene_Information(-33.85f, -0.31f, 395.0f, 0);
	Game_Flag_Reset(566);
	Scene_Exit_Add_2D_Exit(0, 602, 104, 639, 177, 1);
	Ambient_Sounds_Add_Looping_Sound(70, 35, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(109, 40, 0, 1);
	Ambient_Sounds_Add_Sound(72, 6, 70, 14, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(73, 3, 70, 14, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 70, 14, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 70, 33, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 70, 33, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 70, 33, 50, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(584)) {
		Scene_Loop_Set_Default(3);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptHF04::SceneLoaded() {
	if (Game_Flag_Query(584)) {
		Unobstacle_Object("PIVOT_WALL#1", true);
		Unobstacle_Object("PIVOT_WALL#02", true);
		Unobstacle_Object("PIVOT_WALL#03", true);
	} else {
		Unobstacle_Object("HIDE_WALL_A", true);
		Unobstacle_Object("HIDE_WALL_B", true);
	}
	if (Actor_Query_Goal_Number(kActorLucy) == 213) {
		if (Actor_Clue_Query(kActorLucy, kClueMcCoyHelpedLucy) && Global_Variable_Query(40) != 3) {
			Game_Flag_Set(593);
		} else {
			Actor_Set_Goal_Number(kActorLucy, 230);
			Game_Flag_Reset(584);
		}
	}
}

bool SceneScriptHF04::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHF04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptHF04::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptHF04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptHF04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1132.27f, -0.31f, -113.46f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(567);
			Set_Enter(kSetHF02, kSceneHF02);
		}
		return true;
	}
	return false;
}

bool SceneScriptHF04::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHF04::SceneFrameAdvanced(int frame) {
	if (frame == 62) {
		Sound_Play(359, Random_Query(43, 43), 0, 0, 50);
	}
	if (frame == 154) {
		Sound_Play(360, Random_Query(43, 43), 0, 0, 50);
	}
	if (frame == 179 && Actor_Query_Goal_Number(kActorLucy) == 235) {
		Actor_Set_Goal_Number(kActorLucy, 236);
	}
	if (Game_Flag_Query(585)) {
		Game_Flag_Reset(585);
		Scene_Loop_Set_Default(3);
		Scene_Loop_Start_Special(kSceneLoopModeOnce, 2, true);
		//return true;
		return;
	}
	if (Game_Flag_Query(586)) {
		Game_Flag_Reset(586);
		Scene_Loop_Set_Default(0);
		Scene_Loop_Start_Special(kSceneLoopModeOnce, 5, true);
		//return true;
		return;
	}
	if (frame == 89) {
		Game_Flag_Set(584);
		Obstacle_Object("HIDE_WALL_A", false);
		Obstacle_Object("HIDE_WALL_B", false);
		Unobstacle_Object("PIVOT_WALL#1", false);
		Unobstacle_Object("PIVOT_WALL#02", false);
		Unobstacle_Object("PIVOT_WALL#03", true);
		//return true;
		return;
	}
	if (frame == 180) {
		Unobstacle_Object("HIDE_WALL_A", false);
		Unobstacle_Object("HIDE_WALL_B", false);
		Obstacle_Object("PIVOT_WALL#1", false);
		Obstacle_Object("PIVOT_WALL#02", false);
		Obstacle_Object("PIVOT_WALL#03", true);
		Game_Flag_Reset(584);
		if (Actor_Query_Goal_Number(kActorLucy) == 234) {
			Actor_Set_Goal_Number(kActorLucy, 235);
		}
		//return true;
		return;
	}
	//return false;
}

void SceneScriptHF04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHF04::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorLucy) == 230 || Actor_Query_Goal_Number(kActorLucy) == 233) {
		Player_Set_Combat_Mode(true);
		Music_Play(1, 60, 0, 2, -1, 0, 0);
	}
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -45.0f, -0.31f, 307.0f, 0, 0, true, 0);
	Delay(2500);
}

void SceneScriptHF04::PlayerWalkedOut() {
	Music_Stop(5);
}

void SceneScriptHF04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
