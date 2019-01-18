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

void SceneScriptUG10::InitializeScene() {
	if (Game_Flag_Query(336)) {
		Setup_Scene_Information(-92.0f, 81.33f, -652.0f, 520);
	} else if (Game_Flag_Query(423)) {
		Game_Flag_Reset(423);
		Setup_Scene_Information(-385.12f, 1.15f, 57.44f, 400);
	} else if (Game_Flag_Query(346)) {
		Setup_Scene_Information(2.5f, 1.15f, 405.0f, 200);
	} else {
		Setup_Scene_Information(235.0f, 1.15f, 29.0f, 0);
	}
	Scene_Exit_Add_2D_Exit(0, 589, 300, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 460, 70, 632, 171, 0);
	Scene_Exit_Add_2D_Exit(2, 38, 78, 83, 264, 3);
	Scene_Exit_Add_2D_Exit(3, 0, 0, 30, 479, 3);
	Scene_2D_Region_Add(0, 349, 311, 382, 364);
	Ambient_Sounds_Add_Looping_Sound(105, 71, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(95, 45, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 76, 0, 1);
	Ambient_Sounds_Add_Sound(291, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(368, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(401, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(402, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(397, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(398, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(1, 5, 150, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(58, 5, 150, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(230, 2, 20, 25, 32, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(233, 2, 20, 25, 32, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(235, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(394, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(224, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(228, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(229, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	if (Game_Flag_Query(474)) {
		Scene_Loop_Set_Default(4);
	} else {
		Scene_Loop_Set_Default(1);
	}
}

void SceneScriptUG10::SceneLoaded() {
	Obstacle_Object("SLUICEGATE_LEVER", true);
	if (Global_Variable_Query(kVariableChapter) == 4 && !Game_Flag_Query(474) && Game_Flag_Query(kFlagDumpsterEmptied) && !Game_Flag_Query(693)) {
		Scene_Loop_Set_Default(1);
		Scene_Loop_Start_Special(kSceneLoopModeOnce, 6, true);
		Game_Flag_Set(693);
		//return true;
	}
	//return false;
}

bool SceneScriptUG10::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG10::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG10::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG10::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG10::ClickedOnExit(int exitId) {
	float x, y, z;
	Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
	if (exitId == 0) {
		if ((!Game_Flag_Query(474) && x > 125.0f) || Game_Flag_Query(474)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 235.0f, 1.15f, 29.0f, 0, 1, false, 0)) {
				Game_Flag_Set(kFlagUG10toUG01);
				Set_Enter(kSetUG01, kSceneUG01);
				return true;
			}
		} else if (!Game_Flag_Query(474)) {
			Actor_Says(kActorMcCoy, 8521, 3);
		}
	} else if (exitId == 1) {
		if ((!Game_Flag_Query(474) && x < 120.0f) || Game_Flag_Query(474)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1.83f, 1.15f, -410.8f, 0, 1, false, 0)) {
				Actor_Face_Heading(kActorMcCoy, 0, false);
				Loop_Actor_Travel_Stairs(kActorMcCoy, 9, 1, 0);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -92.0f, 81.83f, -652.0f, 0, 0, false, 0);
				Game_Flag_Set(337);
				Set_Enter(kSetUG03, kSceneUG03);
				return true;
			}
		} else if (!Game_Flag_Query(474)) {
			Actor_Says(kActorMcCoy, 6165, 3);
		}
	} else if (exitId == 2) {
		if ((!Game_Flag_Query(474) && x < 120.0f) || Game_Flag_Query(474)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -385.0f, 1.15f, 57.44f, 0, 1, false, 0)) {
				Actor_Face_Heading(kActorMcCoy, 1001, false);
				Loop_Actor_Travel_Ladder(kActorMcCoy, 1, 1, 0);
				Game_Flag_Set(424);
				Set_Enter(kSetUG07, kSceneUG07);
				return true;
			}
		} else if (!Game_Flag_Query(474)) {
			Actor_Says(kActorMcCoy, 6165, 3);
		}
	} else if (exitId == 3) {
		if ((!Game_Flag_Query(474) && x < 120.0f) || Game_Flag_Query(474)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 2.5f, 1.15f, 405.0f, 0, 1, false, 0)) {
				Game_Flag_Set(347);
				Set_Enter(kSetUG14, kSceneUG14);
				return true;
			}
		} else if (!Game_Flag_Query(474)) {
			Actor_Says(kActorMcCoy, 6165, 3);
		}
	}
	return false;
}

bool SceneScriptUG10::ClickedOn2DRegion(int region) {
	float x, y, z;
	Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
	if (region == 0 && !Player_Query_Combat_Mode()) {
		if (x >= 120.0f) {
			Actor_Says(kActorMcCoy, 8525, 3);
		} else if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 4.98f, 0.38f, 83.15f, 0, 1, false, 0)) {
			if (Game_Flag_Query(474)) {
				Scene_Loop_Set_Default(1);
				Scene_Loop_Start_Special(kSceneLoopModeOnce, 0, false);
				Game_Flag_Reset(474);
				Obstacle_Object("BOX01 BRIDGE", true);
				Player_Loses_Control();
			} else {
				Scene_Loop_Set_Default(4);
				Scene_Loop_Start_Special(kSceneLoopModeOnce, 3, false);
				Game_Flag_Set(474);
				Unobstacle_Object("BOX01 BRIDGE", true);
				Player_Loses_Control();
			}
		}
		return true;
	}
	return false;
}

void SceneScriptUG10::SceneFrameAdvanced(int frame) {
	if (frame == 121) {
		Ambient_Sounds_Play_Sound(558, 90, 0, 0, 99);
	}
	if (frame == 127) {
		Ambient_Sounds_Play_Sound(353, 90, 0, 0, 99);
	}
	if (frame == 147) {
		Ambient_Sounds_Play_Sound(353, 90, 0, 0, 99);
	}
	if (frame == 1) {
		Ambient_Sounds_Play_Sound(558, 90, 0, 0, 99);
	}
	if (frame == 3) {
		Ambient_Sounds_Play_Sound(353, 90, 0, 0, 99);
	}
	if (frame == 23) {
		Ambient_Sounds_Play_Sound(353, 90, 0, 0, 99);
	}
	if (frame == 58 || frame == 179) {
		Player_Gains_Control();
	}
	//return false;
}

void SceneScriptUG10::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG10::PlayerWalkedIn() {
	if (Game_Flag_Query(346)) {
		Game_Flag_Reset(346);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 2.5f, 1.15f, 377.0f, 0, 0, false, 0);
	} else if (Game_Flag_Query(kFlagUG01toUG10)) {
		Game_Flag_Reset(kFlagUG01toUG10);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 207.0f, 1.15f, 29.0f, 0, 0, false, 0);
	} else if (Game_Flag_Query(336)) {
		Game_Flag_Reset(336);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1.83f, 81.33f, -518.8f, 0, 0, false, 0);
		Actor_Face_Heading(kActorMcCoy, 506, false);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 9, 0, 0);
	}
	if (Game_Flag_Query(474)) {
		Unobstacle_Object("BOX01 BRIDGE", true);
	} else {
		Obstacle_Object("BOX01 BRIDGE", true);
	}
}

void SceneScriptUG10::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptUG10::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
