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

void SceneScriptKP07::InitializeScene() {
	Setup_Scene_Information(-12.0f, -41.58f, 72.0f, 0);

	Game_Flag_Reset(kFlagKP06toKP07);

	Scene_Exit_Add_2D_Exit(0, 315, 185, 381, 285, 0);

	if (Game_Flag_Query(kFlagMcCoyIsNotHelpingReplicants)) {
		if (Game_Flag_Query(kFlagDektoraIsReplicant)
		 && Actor_Query_Goal_Number(kActorDektora) < kGoalDektoraGone
		) {
			Actor_Set_Targetable(kActorDektora, true);
			Global_Variable_Increment(kVariableReplicants, 1);
			Actor_Put_In_Set(kActorDektora, kSetKP07);
			Actor_Set_At_XYZ(kActorDektora, -52.0f, -41.52f, -5.0f, 289);
		}

		if (Actor_Query_Goal_Number(kActorZuben) < kGoalZubenGone) {
			Global_Variable_Increment(kVariableReplicants, 1);
			Actor_Set_Targetable(kActorZuben, true);
			Actor_Put_In_Set(kActorZuben, kSetKP07);
			Actor_Set_At_XYZ(kActorZuben, -26.0f, -41.52f, -135.0f, 0);
		}

		if (Game_Flag_Query(kFlagIzoIsReplicant)
		 && Actor_Query_Goal_Number(kActorIzo) < 599
		) {
			Global_Variable_Increment(kVariableReplicants, 1);
			Actor_Set_Targetable(kActorIzo, true);
			Actor_Put_In_Set(kActorIzo, kSetKP07);
			Actor_Set_At_XYZ(kActorIzo, -38.0f, -41.52f, -175.0f, 500);
		}

		if (Game_Flag_Query(kFlagGordoIsReplicant)
		 && Actor_Query_Goal_Number(kActorGordo) < kGoalGordoGone
		) {
			Global_Variable_Increment(kVariableReplicants, 1);
			Actor_Set_Targetable(kActorGordo, true);
			Actor_Put_In_Set(kActorGordo, kSetKP07);
			Actor_Set_At_XYZ(kActorGordo, 61.0f, -41.52f, -3.0f, 921);
		}

		if (Game_Flag_Query(kFlagLucyIsReplicant)
		 && Actor_Query_Goal_Number(kActorLucy) < kGoalLucyGone
		) {
			Global_Variable_Increment(kVariableReplicants, 1);
			Actor_Put_In_Set(kActorLucy, kSetKP07);
			Actor_Set_At_XYZ(kActorLucy, 78.0f, -41.52f, -119.0f, 659);
		}

		if (Actor_Query_Goal_Number(kActorLuther) < kGoalLutherGone) {
			Global_Variable_Increment(kVariableReplicants, 1);
			Actor_Put_In_Set(kActorLuther, kSetKP07);
			Actor_Set_At_XYZ(kActorLuther, -47.0f, 0.0f, 151.0f, 531);
		}
	}

	Ambient_Sounds_Add_Looping_Sound(585,  7, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(586, 52, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(109, 38, 1, 1);

	if (Game_Flag_Query(582)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptKP07::SceneLoaded() {
	if (!Game_Flag_Query(kFlagMcCoyIsNotHelpingReplicants)) {
		Music_Play(19, 25, 0, 0, -1, 1, 0);
	}
	Obstacle_Object("BUNK_TRAY01", true);
	Unobstacle_Object("BUNK_TRAY01", true);
	if (Game_Flag_Query(kFlagMcCoyIsNotHelpingReplicants)) {
		Player_Set_Combat_Mode(false);
		Scene_Exits_Disable();
	}
}

bool SceneScriptKP07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptKP07::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptKP07::ClickedOnActor(int actorId) {
	if (actorId == kActorClovis) {
		if (Game_Flag_Query(697) || actorId != kActorClovis || Actor_Query_Goal_Number(kActorClovis) == 599 || Actor_Query_Goal_Number(kActorClovis) == 515) {
			return false;
		}
		if (Game_Flag_Query(kFlagMcCoyIsNotHelpingReplicants)) {
			Actor_Set_Goal_Number(kActorClovis, 516);
		} else {
			Music_Play(20, 31, 0, 0, -1, 1, 0);
			Actor_Set_Goal_Number(kActorClovis, 514);
		}
	} else {
		Actor_Face_Actor(kActorMcCoy, actorId, true);
		Actor_Says(kActorMcCoy, 8590, 14);
	}
	return true;
}

bool SceneScriptKP07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptKP07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -41.58f, 72.0f, 0, true, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagKP07toKP06);
			Set_Enter(kSetKP05_KP06, kSceneKP06);
		}
		return true;
	}
	return false;
}

bool SceneScriptKP07::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptKP07::SceneFrameAdvanced(int frame) {
}

void SceneScriptKP07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptKP07::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 9.0f, -41.88f, -81.0f, 0, 0, false, 0);
	if (!Game_Flag_Query(658)) {
		if (Game_Flag_Query(kFlagMcCoyIsNotHelpingReplicants)) {
			Actor_Face_Actor(kActorMcCoy, kActorClovis, true);
			Actor_Says(kActorClovis, 1240, 3);
			Actor_Says(kActorMcCoy, 8500, 3);
			Actor_Says(kActorClovis, 1250, 3);
			if (Actor_Query_Goal_Number(kActorSadik) == 416) {
				Actor_Put_In_Set(kActorSadik, kSetKP07);
				Global_Variable_Increment(kVariableReplicants, 1);
				Actor_Set_At_XYZ(kActorSadik, -12.0f, -41.58f, 72.0f, 0);
				Actor_Face_Actor(kActorSadik, kActorClovis, true);
			}
		} else {
			Actor_Face_Actor(kActorMcCoy, kActorClovis, true);
			Actor_Says(kActorClovis, 160, 3);
			Actor_Retired_Here(kActorClovis, 72, 60, 0, -1);
		}
		Game_Flag_Set(658);
	}
}

void SceneScriptKP07::PlayerWalkedOut() {
	Music_Stop(3);
}

void SceneScriptKP07::DialogueQueueFlushed(int a1) {
	if (Actor_Query_Goal_Number(kActorClovis) == 515) {
		Actor_Set_Targetable(kActorClovis, false);
		Actor_Change_Animation_Mode(kActorClovis, 21);
		Actor_Retired_Here(kActorClovis, 12, 48, 1, -1);
		Actor_Set_Goal_Number(kActorClovis, 599);
	}
}

} // End of namespace BladeRunner
