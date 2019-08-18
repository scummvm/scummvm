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

void SceneScriptCT05::InitializeScene() {
	if (Game_Flag_Query(kFlagCT12toCT05)) {
		Game_Flag_Reset(kFlagCT12toCT05);
		Setup_Scene_Information(-128.42f, -109.91f, 112.83f, 516);
	} else if (Game_Flag_Query(kFlagCT06toCT05)) {
		Setup_Scene_Information( 192.35f,   43.09f, 128.97f, 768);
	} else {
		Setup_Scene_Information( -375.0f, -109.91f,  750.0f, 600);
	}

	if (Game_Flag_Query(kFlagCT05WarehouseOpen)) {
		Scene_Exit_Add_2D_Exit(0, 228, 205, 293, 300, 0);
	}
	Scene_Exit_Add_2D_Exit(1, 320, 458, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(2, 380, 110, 542, 300, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxCTROOFL1, 15, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTROOFR1, 15,  100, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTAMBR1,  13, -100, 1);
	Ambient_Sounds_Add_Sound(kSfxPSDOOR1,  5, 20,  8, 10, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPSDOOR2,  5, 20,  8, 10, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCTDRONE1, 5, 30, 18, 30, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagCT05WarehouseOpen)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
	if (Actor_Query_Goal_Number(kActorGaff) == kGoalGaffCT12WaitForMcCoy) {
		Overlay_Play("ct05over", 0, true, false, 0);
	}
}

void SceneScriptCT05::SceneLoaded() {
	Obstacle_Object("STAIR 1", true);
	Obstacle_Object("STAIR 2", true);
	Obstacle_Object("GRGDOOR", true);
	Obstacle_Object("GRGDOOR2", true);
	Obstacle_Object("TURBINE", true);
	Obstacle_Object("BARREL", true);
	Obstacle_Object("GRNDPIPE", true);
	Clickable_Object("TURBINE");
	Clickable_Object("LFTDOOR");
	Clickable_Object("BARREL");
	Clickable_Object("GRNDPIPE");
	Unclickable_Object("GDFRAME");
	Unclickable_Object("GDFRAME2");
	Unclickable_Object("WINFRAME1");
	Unclickable_Object("WINFRAME2");
	Unclickable_Object("STAIR 1");
	Unclickable_Object("STAIR 2");
	Unclickable_Object("LFTDOOR");
	Unclickable_Object("LFTDOORFRM");
}

bool SceneScriptCT05::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptCT05::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("STAIR1", objectName)) {
		Actor_Face_Object(kActorMcCoy, objectName, true);
		return true;
	}
	if (Object_Query_Click("STAIR2", objectName)) {
		Actor_Face_Object(kActorMcCoy, objectName, true);
		return true;
	}
	if (Object_Query_Click("GRGDOOR", objectName)) {
		Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "GRGDOOR", 24, true, false);
		Actor_Face_Object(kActorMcCoy, objectName, true);
		Actor_Says(kActorMcCoy, 8522, 12);
		return true;
	}
	if (Object_Query_Click("GRGDOOR2", objectName)) {
		Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "GRGDOOR2", 24, true, false);
		Actor_Face_Object(kActorMcCoy, objectName, true);
		Actor_Says(kActorMcCoy, 8522, 12);
		return true;
	}
	if (Object_Query_Click("TURBINE", objectName)) {
		Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "TURBINE", 36, true, false);
		Actor_Face_Object(kActorMcCoy, objectName, true);
		Actor_Says(kActorMcCoy, 8528, 12);
		return true;
	}
	if (Object_Query_Click("LFTDOOR", objectName)) {
		Actor_Face_Object(kActorMcCoy, objectName, true);
		Actor_Says(kActorMcCoy, 8522, 12);
		return true;
	}
	if (Object_Query_Click("BARREL", objectName)) {
		Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "BARREL", 36, true, false);
		Actor_Face_Object(kActorMcCoy, objectName, true);
		Actor_Says(kActorMcCoy, 8529, 12);
		return true;
	}
	if (Object_Query_Click("GRNDPIPE", objectName)) {
		Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "GRNDPIPE", 24, true, false);
		Actor_Face_Object(kActorMcCoy, objectName, true);
		Actor_Says(kActorMcCoy, 8528, 12);
		return true;
	}
	return false;
}

bool SceneScriptCT05::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptCT05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptCT05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -128.42f, -109.91f, 112.83f, 0, true, false, false)) {
			Game_Flag_Set(kFlagCT05toCT12);
			if (Actor_Query_Goal_Number(kActorGaff) == kGoalGaffCT12WaitForMcCoy) {
				Overlay_Remove("ct05over");
			}
			Set_Enter(kSetCT01_CT12, kSceneCT12);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -308.18f, -109.91f, 674.77f, 0, true, false, false)) {
			Game_Flag_Set(kFlagCT05toCT04);
			if (Actor_Query_Goal_Number(kActorGaff) == kGoalGaffCT12WaitForMcCoy) {
				Overlay_Remove("ct05over");
				Actor_Set_Goal_Number(kActorGaff, kGoalGaffCT12Leave);
				Game_Flag_Set(kFlagMA01GaffApproachMcCoy);
			}
			Set_Enter(kSetCT03_CT04, kSceneCT04);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 71.99f, -109.91f, 288.79f, 0, true, false, false)) {
			Footstep_Sound_Override_On(2);
			Actor_Face_Object(kActorMcCoy, "STAIR 2", true);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 9, true, kAnimationModeIdle);
			Actor_Set_At_XYZ(kActorMcCoy, 99.73f, -19.91f, 134.97f, 256);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 5, true, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
			Game_Flag_Set(kFlagCT05toCT06);
			if (Actor_Query_Goal_Number(kActorGaff) == kGoalGaffCT12WaitForMcCoy) {
				Overlay_Remove("ct05over");
			}
			Set_Enter(kSetCT06, kSceneCT06);
		}
		return true;
	}
	return false;
}

bool SceneScriptCT05::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptCT05::SceneFrameAdvanced(int frame) {
	if (frame == 7
	 || frame == 15
	 || frame == 29
	) {
		switch (Random_Query(0, 4)) {
		case 4:
			Sound_Play(kSfxDRIPPY10, Random_Query(25, 50), -70, -70, 50);
			break;
		case 3:
			Sound_Play(kSfxDRIPPY7, Random_Query(25, 50), -70, -70, 50);
			break;
		case 2:
			Sound_Play(kSfxDRIPPY6, Random_Query(25, 50), -70, -70, 50);
			break;
		case 1:
			Sound_Play(kSfxDRIPPY2, Random_Query(25, 50), -70, -70, 50);
			break;
		case 0:
			Sound_Play(kSfxDRIPPY1, Random_Query(25, 50), -70, -70, 50);
			break;
		}
	}
}

void SceneScriptCT05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT05::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagCT04toCT05)) {
		Player_Loses_Control();
		Game_Flag_Reset(kFlagCT04toCT05);
		if (Player_Query_Combat_Mode()) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -308.18f, -109.91f, 674.77f, 0, false, true, false);
		} else {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -308.18f, -109.91f, 674.77f, 0, false, false, false);
		}
		Player_Gains_Control();
	}

	if (Game_Flag_Query(kFlagCT06toCT05)) {
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 7, false, kAnimationModeIdle);
		Actor_Set_At_XYZ(kActorMcCoy, 90.73f, -19.91f, 164.97f, 520);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 10, false, kAnimationModeIdle);
		Game_Flag_Reset(kFlagCT06toCT05);
		Footstep_Sound_Override_Off();
		if (Actor_Query_Goal_Number(kActorGordo) == kGoalGordoCT01Left
		 && Game_Flag_Query(kFlagCT06ZubenPhoto)
		) {
			Actor_Set_Goal_Number(kActorGordo, kGoalGordoCT05WalkThrough);
		}
	}
}

void SceneScriptCT05::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptCT05::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
