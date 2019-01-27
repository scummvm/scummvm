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

void SceneScriptNR03::InitializeScene() {
	if (Game_Flag_Query(537)) {
		Setup_Scene_Information(-301.98f, -70.19f, -348.58f, 0);
	} else if (Game_Flag_Query(437)) {
		Setup_Scene_Information(-161.0f, -70.19f, -1139.0f, 500);
		Game_Flag_Reset(437);
	} else {
		Setup_Scene_Information(410.0f, -70.19f, -715.0f, 690);
	}
	Scene_Exit_Add_2D_Exit(0, 561, 0, 639, 216, 1);
	Scene_Exit_Add_2D_Exit(1, 210, 85, 240, 145, 0);
	Scene_Exit_Add_2D_Exit(2, 0, 135, 85, 295, 3);
	Scene_2D_Region_Add(0, 331, 73, 375, 114);
	Ambient_Sounds_Add_Looping_Sound(280, 50, 38, 0);
	Ambient_Sounds_Add_Sound(252, 3, 60, 25, 25, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(254, 3, 60, 25, 25, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(255, 3, 60, 25, 25, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(256, 3, 60, 25, 25, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(257, 3, 60, 25, 25, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(258, 3, 60, 25, 25, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(259, 3, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(260, 3, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(261, 3, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(262, 3, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(573)) {
		if (Game_Flag_Query(537)) {
			Scene_Loop_Start_Special(0, 2, 0);
			Scene_Loop_Set_Default(0);
			Game_Flag_Reset(537);
		} else {
			Scene_Loop_Set_Default(0);
		}
	} else {
		Actor_Set_Goal_Number(kActorGuzza, 201);
		Scene_Loop_Start_Special(0, 2, 0);
		Scene_Loop_Set_Default(0);
	}
	if (Actor_Query_Goal_Number(kActorHanoi) > 209 && Actor_Query_Goal_Number(kActorHanoi) < 215) {
		Actor_Set_Goal_Number(kActorHanoi, 215);
	}
}

void SceneScriptNR03::SceneLoaded() {
	Obstacle_Object("PG3", true);
	Obstacle_Object("X2BACKBARBOTTOM01", true);
	Obstacle_Object("X2BACKSTAGETOP", true);
	Unclickable_Object("PG3");
	Unobstacle_Object("X2BACKBARBOTTOM02", true);
	Unobstacle_Object("NM2", true);
	Unobstacle_Object("MAN5", true);
	Unobstacle_Object("MAN7", true);
	Unobstacle_Object("X2BACKSTAGETOP", true);
}

bool SceneScriptNR03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR03::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("PG3", objectName)) {
		Actor_Face_Object(kActorMcCoy, "PG3", true);
		Actor_Voice_Over(3770, kActorVoiceOver);
		return true;
	}
	return false;
}

bool SceneScriptNR03::ClickedOnActor(int actorId) {
	if (actorId == kActorHanoi) {
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorHanoi, 48, true, false)) {
			AI_Movement_Track_Pause(kActorHanoi);
			Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
			if (Game_Flag_Query(611)) {
				Actor_Says(kActorMcCoy, 3350, 16);
				Actor_Says(kActorHanoi, 50, 17);
			} else {
				Game_Flag_Set(611);
				Actor_Says(kActorMcCoy, 3340, 3);
				Actor_Face_Actor(kActorHanoi, kActorMcCoy, true);
				Actor_Says(kActorHanoi, 30, 13);
				Actor_Says(kActorMcCoy, 3345, 14);
				Actor_Says(kActorHanoi, 40, 14);
			}
			AI_Movement_Track_Unpause(kActorHanoi);
			return true;
		}
	}
	return false;
}

bool SceneScriptNR03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 410.0f, -70.19f, -715.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagNR03toNR01);
			Set_Enter(kSetNR01, kSceneNR01);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -161.0f, -70.19f, -1139.0f, 0, 1, false, 0)) {
			if (Actor_Query_Which_Set_In(kActorHanoi) == 55) {
				AI_Movement_Track_Pause(kActorHanoi);
				Actor_Face_Actor(kActorHanoi, kActorMcCoy, true);
				Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
				int v3 = Global_Variable_Query(44);
				if (v3 == 0) {
					Actor_Says(kActorHanoi, 50, 13);
					AI_Movement_Track_Unpause(kActorHanoi);
				} else if (v3 == 1) {
					Actor_Says(kActorHanoi, 210, 15);
					AI_Movement_Track_Unpause(kActorHanoi);
				} else if (v3 == 2) {
					Actor_Set_Goal_Number(kActorHanoi, 220);
				}
			}
			Global_Variable_Increment(44, 1);
		} else {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(438);
			Set_Enter(kSetNR04, kSceneNR04);
		}

		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -151.0f, -70.19f, -476.0f, 12, 1, false, 0)) {
			if (Actor_Query_Goal_Number(kActorHanoi) == 213 || Actor_Query_Which_Set_In(kActorHanoi) != 55) {
				Player_Loses_Control();
				Player_Set_Combat_Mode(false);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -229.0f, -70.19f, -469.0f, 0, 0, false, 1);
				Actor_Face_Heading(kActorMcCoy, 656, false);
				Actor_Change_Animation_Mode(kActorMcCoy, 53);
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(536);
				Set_Enter(kSetNR05_NR08, kSceneNR05);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 2, 0);
				return true;
			}
			Actor_Face_Heading(kActorMcCoy, 680, false);
			Actor_Change_Animation_Mode(kActorMcCoy, 12);
			Delay(150);
			Actor_Change_Animation_Mode(kActorMcCoy, 0);
			AI_Movement_Track_Pause(kActorHanoi);
			Actor_Face_Actor(kActorHanoi, kActorMcCoy, true);

			int v1 = Global_Variable_Query(43);
			if (v1 == 0) {
				Actor_Says(kActorHanoi, 0, 15);
				Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
				Actor_Says(kActorMcCoy, 3335, 13);
				Actor_Says(kActorHanoi, 10, 16);
				AI_Movement_Track_Unpause(kActorHanoi);
			} else if (v1 == 1) {
				Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
				Actor_Says(kActorHanoi, 210, 12);
				AI_Movement_Track_Unpause(kActorHanoi);
			} else if (v1 == 2) {
				Actor_Set_Goal_Number(kActorHanoi, 220);
			}
			Global_Variable_Increment(43, 1);
		}
		return true;
	}
	return false;
}

bool SceneScriptNR03::ClickedOn2DRegion(int region) {
	if (region == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 79.2f, -70.19f, -984.0f, 12, 1, false, 0)) {
			Actor_Face_Actor(kActorMcCoy, kActorHysteriaPatron1, true);
			int v1 = Random_Query(0, 4);
			if (v1 == 0) {
				Actor_Says(kActorMcCoy, 1055, 3);
			} else if (v1 == 1) {
				Actor_Says(kActorMcCoy, 8590, 3);
			} else if (v1 == 2) {
				Actor_Says(kActorMcCoy, 8930, 3);
			} else if (v1 == 3) {
				Actor_Says(kActorMcCoy, 7465, 3);
			}
		}
		return true;
	}
	return false;
}

void SceneScriptNR03::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing()) {
		sub_402994();
	}
	if (frame == 72) {
		Sound_Play(345, 83, -70, -70, 50);
	}
	if (frame == 76) {
		Sound_Play(353, 62, -70, -70, 50);
	}
	if (frame > 70 && frame < 110) {
		sub_40259C(frame);
	} else {
		if (frame != 110) {
			//return false;
			return;
		}
		if (Actor_Query_Goal_Number(kActorGuzza) == 201) {
			Actor_Set_Goal_Number(kActorGuzza, 200);
		} else if (!Game_Flag_Query(536)) {
			Actor_Set_Goal_Number(kActorMcCoy, 200);
			Player_Gains_Control();
		}
	}
	//return true;
	return;
}

void SceneScriptNR03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR03::PlayerWalkedIn() {
	Player_Set_Combat_Mode(false);
	if (Game_Flag_Query(573)) {
		if (Game_Flag_Query(kFlagNR01toNR03) ) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 302.0f, -70.19f, -715.0f, 0, 0, false, 0);
			Game_Flag_Reset(kFlagNR01toNR03);
		}
	} else {
		Game_Flag_Set(573);
		Async_Actor_Walk_To_XYZ(kActorMcCoy, 206.0f, -70.19f, -643.0f, 0, false);
		Game_Flag_Reset(kFlagNR01toNR03);
		Actor_Voice_Over(1490, kActorVoiceOver);
		Actor_Voice_Over(1510, kActorVoiceOver);
		Actor_Voice_Over(1520, kActorVoiceOver);
	}
	if (Player_Query_Combat_Mode()) {
		Actor_Set_Goal_Number(kActorHanoi, 220);
	}
	//return false;
}

void SceneScriptNR03::PlayerWalkedOut() {
	if (!Game_Flag_Query(438)) {
		Music_Stop(2);
	}
	if (Game_Flag_Query(536)) {
		Player_Gains_Control();
	}
}

void SceneScriptNR03::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR03::sub_40259C(int frame) {
	int facing;
	float angle, invertedAngle;

	angle = cos((frame - 70) * (M_PI / 40.0f)) * M_PI_2;
	invertedAngle = M_PI - angle;
	if (!Game_Flag_Query(536) && Actor_Query_Goal_Number(kActorGuzza) != 201) {
		angle = angle + M_PI;
		invertedAngle = invertedAngle + M_PI;
	}
	float c = cos(invertedAngle);
	float s = sin(invertedAngle);
	float x = 36.49f * s - -60.21f * c + -265.49f;
	float z = -60.21f * s + 36.49f * c + -408.79f;

	if (Actor_Query_Goal_Number(kActorGuzza) == 201) {
		facing = angle * (512.0f / M_PI);
		facing = facing + 144;
		if (facing < 0) {
			facing = facing + 1168;
		}
		if (facing > 1023) {
			facing -= 1024;
		}
		Actor_Set_At_XYZ(kActorGuzza, x, -70.19f, z, facing);
	} else {
		facing = angle * (512.0f / M_PI);
		facing = facing + 400;
		if (facing < 0) {
			facing = facing + 1424;
		}
		if (facing > 1023) {
			facing -= 1024;
		}

		Actor_Set_At_XYZ(kActorMcCoy, x, -70.19f, z, facing);
	}
}

void SceneScriptNR03::sub_402994() {
	if (Music_Is_Playing()) {
		Music_Adjust(51, 0, 2);
	} else {
		int v0 = Global_Variable_Query(53);
		if (v0 == 0) {
			Music_Play(14, 51, 0, 2, -1, 0, 0);
		} else if (v0 == 1) {
			Music_Play(13, 51, 0, 2, -1, 0, 0);
		} else if (v0 == 2) {
			Music_Play(5, 51, 0, 2, -1, 0, 0);
		}
		v0++;
		if (v0 > 2) {
			v0 = 0;
		}
		Global_Variable_Set(53, v0);
	}
}

} // End of namespace BladeRunner
