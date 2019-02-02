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

void SceneScriptKP03::InitializeScene() {
	if (Game_Flag_Query(420)) {
		Setup_Scene_Information(   1.0f, -36.55f, 111.0f, 200);
	} else {
		Setup_Scene_Information(-321.0f, -36.55f,  26.0f, 350);
	}
	Scene_Exit_Add_2D_Exit(0,   0,   0,  30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 287, 104, 367, 255, 0);

	Ambient_Sounds_Add_Looping_Sound(381, 100, 1, 1);
	Ambient_Sounds_Add_Sound( 68, 60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound( 69, 60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);

	if (Game_Flag_Query(422)) {
		Scene_Loop_Set_Default(5);
	} else if (Game_Flag_Query(484)) {
		Scene_Loop_Set_Default(7);
	} else {
		Scene_Loop_Set_Default(2);
		Game_Flag_Set(421);
	}
	if (( Actor_Query_Goal_Number(kActorSteele) != 599
	  && !Game_Flag_Query(422)
	  && !Game_Flag_Query(484)
	 )
	 && ((Game_Flag_Query(kFlagMcCoyIsNotHelpingReplicants)
	   && Game_Flag_Query(420)
	  )
	  || (!Game_Flag_Query(kFlagMcCoyIsNotHelpingReplicants)
	   &&  Game_Flag_Query(417)
	  )
	 )
	) {
		Actor_Put_In_Set(kActorSteele, kSetKP03);
		Actor_Set_At_XYZ(kActorSteele, -300.0f, -36.55f, 26.0f, 350);
	}
}

void SceneScriptKP03::SceneLoaded() {
	Obstacle_Object("BRACK MID", true);
	Unobstacle_Object("OBSTACLE_REMOVE", true);
	Unobstacle_Object("BOX11", true);
	Unobstacle_Object("OBSTACLE05", true);
	Clickable_Object("BRACK MID");
}

bool SceneScriptKP03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptKP03::ClickedOn3DObject(const char *objectName, bool combatMode) {
	Actor_Face_Object(kActorSteele, "BRACK MID", true);
	if (Object_Query_Click("BRACK MID", objectName) && !Game_Flag_Query(422)) {
		if (combatMode) {
			Scene_Loop_Set_Default(5);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, 4, true);
			Actor_Change_Animation_Mode(kActorMcCoy, 39);
			Actor_Retired_Here(kActorMcCoy, 72, 18, 1, -1);
			Game_Flag_Set(422);
			Game_Flag_Reset(421);
			return false;
		}
		if (Actor_Query_Goal_Number(kActorSteele) == 411) {
			Scene_Exits_Enable();
			sub_401E54();
		} else {
			if (Game_Flag_Query(417)) {
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -137.0f, -36.55f, 26.0f, 0, 0, true, 0);
			} else if (Game_Flag_Query(420)) {
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -50.0f, -36.55f, 78.0f, 0, 0, true, 0);
			}
			if (Game_Flag_Query(421)) {
				Actor_Face_Object(kActorMcCoy, "BRACK MID", true);
				Game_Flag_Set(484);
				Game_Flag_Reset(421);
				Scene_Loop_Set_Default(7);
				Scene_Loop_Start_Special(kSceneLoopModeOnce, 0, false);
				Actor_Voice_Over(1110, kActorVoiceOver);
				Actor_Voice_Over(1120, kActorVoiceOver);
			} else {
				Actor_Face_Object(kActorMcCoy, "BRACK MID", true);
				Actor_Says(kActorMcCoy, 8580, 13);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptKP03::ClickedOnActor(int actorId) {
	if (actorId == kActorSteele
	 && Actor_Query_Goal_Number(kActorSteele) == 411
	) {
		Actor_Face_Object(kActorSteele, "BRACK MID", true);
		sub_401E54();
	}
	return false;
}

bool SceneScriptKP03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptKP03::ClickedOnExit(int exitId) {
	if (Actor_Query_Goal_Number(kActorSteele) == 410) {
		Actor_Set_Goal_Number(kActorSteele, 418);
	} else {
		if (exitId == 0) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1.0f, -36.55f, 111.0f, 0, 1, false, 0)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Music_Stop(2);
				Game_Flag_Reset(417);
				Game_Flag_Reset(420);
				Game_Flag_Set(419);
				Set_Enter(kSetKP05_KP06, kSceneKP05);
			}
			return true;
		}
		if (exitId == 1) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -321.0f, -36.55f, 26.0f, 0, 1, false, 0)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Music_Stop(2);
				Game_Flag_Reset(417);
				Game_Flag_Reset(420);
				Game_Flag_Set(418);
				Set_Enter(kSetKP01, kSceneKP01);
			}
			return true;
		}
	}
	return false;
}

bool SceneScriptKP03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptKP03::SceneFrameAdvanced(int frame) {
	int v1;
	float x, y, z;

	if (frame == 123) {
		Ambient_Sounds_Play_Sound(491, 99, -60, 100, 99);
	}
	if (Game_Flag_Query(421) && !Game_Flag_Query(484)) {
		v1 = -1;
		if (!Game_Flag_Query(422)) {
			Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
			if ((Game_Flag_Query(417) && -130.0f < x) || (Game_Flag_Query(420) && -130.0f > x)) {
				v1 = 0;
			}
		}
		if (!Game_Flag_Query(422)) { //todo ? same condition as before
			Actor_Query_XYZ(kActorSteele, &x, &y, &z);
			if (Game_Flag_Query(kFlagMcCoyIsNotHelpingReplicants) && Actor_Query_Which_Set_In(kActorSteele) == 46) {
				if ((Game_Flag_Query(417) && -130.0f > x) || (Game_Flag_Query(420) && -130.0f < x)) {
					v1 = 1;
				}
			} else if ((Game_Flag_Query(417) && -130.0f < x) || (Game_Flag_Query(420) && -130.0f > x)) {
				v1 = 1;
			}
		}
		if (v1 != -1) {
			Scene_Loop_Set_Default(5);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, 4, true);
			Game_Flag_Set(422);
			Game_Flag_Reset(421);
			Unclickable_Object("BRACK MID");
			Scene_Exits_Enable();
			if (v1 == 1) {
				Actor_Set_Goal_Number(kActorSteele, 415);
				Music_Play(12, 25, 0, 1, -1, 0, 0);
				if (Actor_Query_Inch_Distance_From_Actor(kActorMcCoy, kActorSteele) <= 120) {
					v1 = 0;
				}
			}
			if (v1) {
				Actor_Change_Animation_Mode(kActorMcCoy, 21);
			} else {
				Actor_Force_Stop_Walking(kActorMcCoy);
				Actor_Change_Animation_Mode(kActorMcCoy, 48);
				Actor_Retired_Here(kActorMcCoy, 72, 18, 1, -1);
			}
		}
	}
}

void SceneScriptKP03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptKP03::PlayerWalkedIn() {
	if (Game_Flag_Query(420)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1.0f, -36.55f, 87.0f, 0, 0, false, 0);
	}
	if (Actor_Query_Is_In_Current_Set(kActorSteele) && Actor_Query_Goal_Number(kActorSteele) != 419) {
		if (Game_Flag_Query(kFlagMcCoyIsNotHelpingReplicants)) {
			if (Game_Flag_Query(420)) {
				Actor_Set_Goal_Number(kActorSteele, 410);
			}
		} else if (!Game_Flag_Query(422) && !Game_Flag_Query(484) && Game_Flag_Query(417)) {
			Scene_Exits_Disable();
			Delay(1000);
			Actor_Set_Goal_Number(kActorSteele, 411);
		}
	}
}

void SceneScriptKP03::PlayerWalkedOut() {
}

void SceneScriptKP03::DialogueQueueFlushed(int a1) {
}

void SceneScriptKP03::sub_401E54() {
	Player_Loses_Control();
	Actor_Says(kActorMcCoy, 2180, 14);
	Actor_Set_Goal_Number(kActorSteele, 412);
	Actor_Says(kActorSteele, 480, 60);
	Actor_Face_Object(kActorMcCoy, "BRACK MID", true);
	Actor_Says(kActorMcCoy, 2185, 14);
	Loop_Actor_Walk_To_XYZ(kActorSteele, -137.0f, -36.55f, 26.0f, 0, 0, false, 0);
	Actor_Face_Object(kActorSteele, "BRACK MID", true);
	Actor_Says(kActorSteele, 490, 58);
	Actor_Says(kActorMcCoy, 2190, 14);
	Actor_Says(kActorSteele, 500, 58);
	Actor_Says(kActorSteele, 510, 59);
	Actor_Says(kActorSteele, 520, 60);
	Game_Flag_Set(484);
	Game_Flag_Reset(421);
	Scene_Loop_Set_Default(7);
	Scene_Loop_Start_Special(kSceneLoopModeOnce, 7, false);
	Actor_Set_Goal_Number(kActorSteele, 413);
	Actor_Says(kActorMcCoy, 2195, 14);
	Ambient_Sounds_Play_Sound(151, 40, -60, -60, 0);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1.0f, -36.55f, 111.0f, 0, 0, false, 0);
	Actor_Set_Goal_Number(kActorSteele, 430);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Game_Flag_Reset(417);
	Game_Flag_Reset(420);
	Game_Flag_Set(419);
	Set_Enter(kSetKP05_KP06, kSceneKP05);
	Player_Gains_Control();
}

} // End of namespace BladeRunner
