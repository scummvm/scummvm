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

void SceneScriptDR04::InitializeScene() {
	if (Game_Flag_Query(kFlagDR05BombExplosionView)) {
		Setup_Scene_Information(0.0f, 0.0f, 0.0f, 0);
	} else if (Game_Flag_Query(kFlagDR01toDR04)) {
		Setup_Scene_Information(-711.0f, -0.04f, 70.0f, 472);
	} else if (Game_Flag_Query(kFlagDR05toDR04)) {
		Setup_Scene_Information(-1067.0f, 7.18f, 421.0f, 125);
	} else if (Game_Flag_Query(kFlagDR06toDR04)) {
		Setup_Scene_Information(-897.75f, 134.45f, 569.75f, 512);
	} else {
		Setup_Scene_Information(-810.0f, -0.04f, 242.0f, 125);
	}
	Scene_Exit_Add_2D_Exit(0, 589, 0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 443, 264, 488, 353, 0);
	Scene_Exit_Add_2D_Exit(2, 222, 110, 269, 207, 0);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(288, 55, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(217, 28, -100, 100);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(67, 40, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(66, 40, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(378, 5, 80, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(379, 5, 80, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(380, 5, 80, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(kFlagDR05BombExploded)) {
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(4);
	}
	if (Game_Flag_Query(kFlagDR01toDR04)) {
		if (Game_Flag_Query(kFlagDR05BombExploded)) {
			Scene_Loop_Start_Special(0, 0, 0);
		} else {
			Scene_Loop_Start_Special(0, 3, 0);
		}
	}
}

void SceneScriptDR04::SceneLoaded() {
	Obstacle_Object("TRASH CAN WITH FIRE", true);
	Obstacle_Object("V2PYLON02", true);
	Obstacle_Object("V2PYLON04", true);
	Obstacle_Object("U2 CHEWDOOR", true);
	Obstacle_Object("MMTRASHCAN", true);
	Obstacle_Object("PARKMETR02", true);
	Obstacle_Object("TRANSFORMER 01", true);
	Obstacle_Object("TRANSFORMER 02", true);
	Obstacle_Object("PARKMETR01", true);
	Obstacle_Object("Z2ENTRYDR", true);
	Obstacle_Object("Z2DR2", true);
	Unclickable_Object("PARKMETR01");
	Unclickable_Object("Z2ENTRYDR");
	Unclickable_Object("Z2DR2");
}

bool SceneScriptDR04::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptDR04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptDR04::ClickedOnActor(int actorId) {
	if ( actorId == kActorMoraji
	 && !Player_Query_Combat_Mode()
	) {
		if (Actor_Query_Goal_Number(kActorMoraji) == 21) {
			if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 109, 0, true, true)) {
				Actor_Face_Actor(kActorMcCoy, kActorMoraji, true);
				Actor_Says(kActorMcCoy, 945, 13);
				Actor_Says(kActorMoraji, 0, 3);
				Actor_Says(kActorMoraji, 10, 3);
				Actor_Says(kActorMcCoy, 950, 13);
				Actor_Says(kActorMoraji, 20, 3);
				Actor_Says(kActorMoraji, 30, 3);
				Actor_Says(kActorMcCoy, 955, 13);
				Actor_Says_With_Pause(kActorMoraji, 40, 0.0f, 3);
				Actor_Says(kActorMoraji, 50, 3);
				Actor_Clue_Acquire(kActorMcCoy, kClueMorajiInterview, true, kActorMoraji);
				Actor_Set_Goal_Number(kActorMoraji, 22);
				Actor_Set_Goal_Number(kActorOfficerGrayford, 101);
				return true;
			}
		}

		if (Actor_Query_Goal_Number(kActorMoraji) == 23) {
			if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorMoraji, 36, true, false)) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 106);
				return true;
			}
		}
	}
	return false;
}

bool SceneScriptDR04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptDR04::ClickedOnExit(int exitId) {
	if (Actor_Query_Goal_Number(kActorMoraji) == 21) {
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Set_Goal_Number(kActorMoraji, 22);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 101);
		return true;
	}
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -761.0f, -0.04f, 97.0f, 0, 1, false, 0)) {
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -683.0f, -0.04f, 43.0f, 0, false);
			Game_Flag_Set(kFlagDR04toDR01);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1067.0f, 7.18f, 421.0f, 0, 1, false, 0)) {
			Game_Flag_Set(kFlagNotUsed232);
			Game_Flag_Set(kFlagDR04toDR05);
			Set_Enter(kSetDR05, kSceneDR05);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -851.0f, 6.98f, 560.0f, 0, 1, false, 0)) {
			Footstep_Sound_Override_On(3);
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, true);
			Actor_Face_Heading(kActorMcCoy, 512, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 7, 1, kAnimationModeIdle);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -899.0f, 71.64f, 647.0f, 0, 0, false, 0);
			Actor_Face_Heading(kActorMcCoy, 0, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 7, 1, kAnimationModeIdle);
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, false);
			Footstep_Sound_Override_Off();
			Game_Flag_Set(kFlagDR04toDR06);
			Set_Enter(kSetDR06, kSceneDR06);
		}
		return true;
	}
	return true; //bug?
}

bool SceneScriptDR04::ClickedOn2DRegion(int region) {
	return false;
}

bool SceneScriptDR04::sub_401160() {
	float x, y, z;
	Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
	return (x + 1089.94f) * (x + 1089.94f) + (z - 443.49f) * (z - 443.49f) >= (360.0f * 360.0f);
}

void SceneScriptDR04::SceneFrameAdvanced(int frame) {
	if (Game_Flag_Query(kFlagDR05BombExplosionView)) {
		Game_Flag_Reset(kFlagDR05BombExplosionView);
		Game_Flag_Reset(kFlagDR05BombWillExplode);
		Scene_Loop_Set_Default(1);
		Scene_Loop_Start_Special(kSceneLoopModeOnce, 6, true);
		Music_Stop(4);
		Actor_Set_Goal_Number(kActorMoraji, 99);
	} else {
		if (Game_Flag_Query(kFlagDR05BombWillExplode)) {
			Game_Flag_Reset(kFlagDR05BombWillExplode);
			Game_Flag_Set(kFlagDR05BombExploded);
			Scene_Loop_Set_Default(1);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, 6, true);
			Item_Remove_From_World(78);
		}
		switch (frame) {
		case 193:
			Sound_Play(301, 100, 0, 100, 50);
			Actor_Set_Goal_Number(kActorMoraji, 30);
			Player_Loses_Control();
			Actor_Force_Stop_Walking(kActorMcCoy);
			if (sub_401160()) {
				if (Player_Query_Combat_Mode()) {
					Actor_Change_Animation_Mode(kActorMcCoy, 22);
				} else {
					Actor_Change_Animation_Mode(kActorMcCoy, 21);
				}
			} else {
				Sound_Play_Speech_Line(kActorMcCoy, 9905, 100, 0, 99);
				Actor_Change_Animation_Mode(kActorMcCoy, 48);
				Actor_Retired_Here(kActorMcCoy, 6, 6, 1, -1);
			}
			Player_Gains_Control();
			break;
		case 235:
			if (Actor_Query_Goal_Number(kActorMoraji) != 20
			 && Actor_Query_Goal_Number(kActorMoraji) != 21
			 && Actor_Query_Goal_Number(kActorMoraji) != 99
			) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 101);
			}
			Scene_Exits_Enable();
			break;
		case 237:
			Overlay_Play("DR04OVER", 0, 1, 1, 0);
			break;
		}
	}
}

void SceneScriptDR04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptDR04::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagDR05BombExplosionView)) {
		Player_Loses_Control();
		Delay(4000);
		Actor_Retired_Here(kActorMcCoy, 6, 6, 1, -1);
	} else {
		if ( Game_Flag_Query(kFlagDR05BombActivated)
		 && !Game_Flag_Query(kFlagDR05BombExploded)
		) {
			Scene_Exits_Disable();
		}

		if (Game_Flag_Query(kFlagDR06toDR04)) {
			Footstep_Sound_Override_On(3);
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, true);
			Actor_Face_Heading(kActorMcCoy, 512, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 7, 0, kAnimationModeIdle);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -851.0f, 71.64f, 647.0f, 0, 0, false, 0);
			Actor_Face_Heading(kActorMcCoy, 0, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 7, 0, kAnimationModeIdle);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -774.85f, 7.18f, 386.67f, 0, 0, false, 0);
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, false);
			Footstep_Sound_Override_Off();
		}
	}
	Game_Flag_Reset(kFlagDR01toDR04);
	Game_Flag_Reset(kFlagDR05toDR04);
	Game_Flag_Reset(kFlagDR06toDR04);
}

void SceneScriptDR04::PlayerWalkedOut() {
	Music_Stop(2);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptDR04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
