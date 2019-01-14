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

void SceneScriptUG05::InitializeScene() {
	if (Game_Flag_Query(360)) {
		if (Game_Flag_Query(663) && !Game_Flag_Query(368)) {
			Setup_Scene_Information(-356.35f, 132.77f, -1092.36f, 389);
		} else {
			Setup_Scene_Information(-180.0f, 37.28f, -1124.0f, 296);
		}
	} else {
		Setup_Scene_Information(0.0f, -1.37f, 0.0f, 0);
		Game_Flag_Reset(338);
	}
	Scene_Exit_Add_2D_Exit(0, 215, 240, 254, 331, 3);
	if (!Game_Flag_Query(663)) {
		Scene_Exit_Add_2D_Exit(1, 303, 422, 639, 479, 2);
	}
	if (!Game_Flag_Query(663) || Game_Flag_Query(368)) {
		Scene_Exit_Add_2D_Exit(2, 352, 256, 393, 344, 0);
	}
	Ambient_Sounds_Add_Looping_Sound(105, 28, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	Ambient_Sounds_Add_Sound(234, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(225, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(226, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(227, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(235, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(391, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(368, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(402, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(395, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(398, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(224, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(228, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(392, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(229, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(368)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptUG05::SceneLoaded() {
	if (!Game_Flag_Query(368)) {
		Unobstacle_Object("DROPPED CAR OBSTACL", true);
	}
	Obstacle_Object("VANBODY", true);
}

bool SceneScriptUG05::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG05::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG05::ClickedOnActor(int actorId) {
	if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, actorId, 30, 1, false)) {
		Actor_Face_Actor(kActorMcCoy, actorId, true);
		int v1 = sub_4021B0();
		if (actorId == kActorOfficerGrayford && Game_Flag_Query(368) && !Game_Flag_Query(683)) {
			Actor_Says(kActorOfficerGrayford, 220, -1);
			Actor_Says(kActorMcCoy, 5540, 14);
			Actor_Says(kActorOfficerGrayford, 230, -1);
			Actor_Says(kActorMcCoy, 5545, 17);
			Actor_Says(kActorOfficerGrayford, 240, -1);
			Actor_Says(kActorMcCoy, 5550, 3);
			Game_Flag_Set(683);
			return false;
		}
		if (actorId == v1) {
			sub_402218();
			return true;
		}
		return false;
	}
	return false;
}

bool SceneScriptUG05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (Game_Flag_Query(663) && !Game_Flag_Query(368)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -356.35f, 132.77f, -1092.36f, 0, 0, false, 0);
			Game_Flag_Set(361);
			Set_Enter(kSetHF07, kSceneHF07);
		} else if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -156.72f, 3.03f, -1118.17f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 760, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 3, 1, 0);
			Game_Flag_Set(361);
			Set_Enter(kSetHF07, kSceneHF07);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 4.0f, -11.67f, -4.0f, 0, 1, false, 0)) {
			Game_Flag_Set(339);
			Set_Enter(kSetUG04, kSceneUG04);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Game_Flag_Query(662)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 0.0f, -1.37f, -1500.0f, 0, 1, false, 0)) {
				if (!Game_Flag_Query(522)) {
					Actor_Voice_Over(2600, kActorVoiceOver);
					Actor_Voice_Over(2610, kActorVoiceOver);
					Game_Flag_Set(522);
				}
				return true;
			}
		} else {
			int v1 = sub_4021B0();
			bool v2;
			if (v1 == -1) {
				v2 = Loop_Actor_Walk_To_XYZ(kActorMcCoy, 0.0f, -1.37f, -1500.0f, 0, 1, false, 0) != 0;
			} else {
				v2 = Loop_Actor_Walk_To_Actor(kActorMcCoy, v1, 30, 1, false) != 0;
			}
			if (!v2) {
				sub_402218();
				return true;
			}
		}
	}
	return false;
}

bool SceneScriptUG05::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG05::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG05::PlayerWalkedIn() {
	if (Game_Flag_Query(663)) {
		if (Game_Flag_Query(368)) {
			Music_Stop(2);
			Actor_Put_In_Set(kActorOfficerGrayford, kSetUG05);
			Actor_Set_At_XYZ(kActorOfficerGrayford, 4.22f, -1.37f, -925.0f, 750);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 599);
			Actor_Retired_Here(kActorOfficerGrayford, 70, 36, 1, -1);
			int v0 = sub_4021B0();
			if (v0 == kActorDektora) {
				Actor_Put_In_Set(kActorDektora, kSetUG05);
				Actor_Set_At_XYZ(kActorDektora, -100.0f, -10.31f, -906.0f, 866);
				Actor_Force_Stop_Walking(kActorDektora);
			} else if (v0 == kActorLucy){
				Actor_Put_In_Set(kActorLucy, kSetUG05);
				Actor_Set_At_XYZ(kActorLucy, -100.0f, -10.31f, -906.0f, 866);
				Actor_Force_Stop_Walking(kActorLucy);
			}
		} else {
			if (!Actor_Query_In_Set(kActorOfficerLeary, kSetUG05)) {
				Actor_Put_In_Set(kActorOfficerLeary, kSetUG05);
				Actor_Set_At_XYZ(kActorOfficerLeary, 0.0f, -1.37f, -1400.0f, 768);
			}
			if (!Actor_Query_In_Set(kActorOfficerGrayford, kSetUG05)) {
				ADQ_Flush();
				ADQ_Add(kActorOfficerGrayford, 280, 3);
				Actor_Put_In_Set(kActorOfficerGrayford, kSetUG05);
				Actor_Set_At_XYZ(kActorOfficerGrayford, -16.0f, -1.37f, -960.0f, 768);
			}
		}
	}
	if (Game_Flag_Query(360)) {
		if (Game_Flag_Query(663) && !Game_Flag_Query(368)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -288.35f, 132.77f, -1092.36f, 0, 1, false, 0);
		} else {
			Loop_Actor_Travel_Stairs(kActorMcCoy, 2, 0, 0);
		}
	}
	if (Game_Flag_Query(663)) {
		Game_Flag_Query(368); // bug in game?
	}
	Game_Flag_Reset(360);
}

void SceneScriptUG05::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptUG05::DialogueQueueFlushed(int a1) {
}

int SceneScriptUG05::sub_4021B0() {
	if (Global_Variable_Query(kVariableAffectionTowards) == 2 && Actor_Query_Goal_Number(kActorDektora) != 599) {
		return kActorDektora;
	}
	if (Global_Variable_Query(kVariableAffectionTowards) == 3 && Actor_Query_Goal_Number(kActorLucy) != 599) {
		return kActorLucy;
	}
	return -1;
}

void SceneScriptUG05::sub_402218() {
	int v0 = sub_4021B0();
	if (v0 != -1) {
		Actor_Face_Actor(kActorMcCoy, v0, true);
		Actor_Face_Actor(v0, kActorMcCoy, true);
		Actor_Says(kActorMcCoy, 5535, 13);
		if (v0 == kActorDektora) {
			Actor_Says(kActorDektora, 1110, 15);
		} else {
			Actor_Says(kActorLucy, 670, 17);
		}
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (v0 == kActorLucy) {
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			if (Global_Variable_Query(kVariableDNAEvidences) > 3) {
				Outtake_Play(kOuttakeEnd1B, false, -1);
			} else {
				Outtake_Play(kOuttakeEnd1C, false, -1);
			}
		} else {
			Outtake_Play(kOuttakeEnd1A, false, -1);
		}
	} else if (v0 == kActorDektora) {
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			if (Global_Variable_Query(kVariableDNAEvidences) > 3) {
				Outtake_Play(kOuttakeEnd1E, false, -1);
			} else {
				Outtake_Play(kOuttakeEnd1F, false, -1);
			}
		} else {
			Outtake_Play(kOuttakeEnd1D, false, -1);
		}
	} else {
		Outtake_Play(kOuttakeEnd3, false, -1);
	}
	Outtake_Play(kOuttakeEnd2, false, -1);
	Game_Over();
}

} // End of namespace BladeRunner
