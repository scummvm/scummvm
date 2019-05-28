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

void SceneScriptBB10::InitializeScene() {
	if (Game_Flag_Query(kFlagBB11toBB10)) {
		Setup_Scene_Information(255.29f, 90.24f,  -103.0f, 830);
	} else if (Game_Flag_Query(kFlagBB09toBB10b)) {
		Game_Flag_Reset(kFlagBB09toBB10b);
		Setup_Scene_Information(151.67f, 66.84f, -313.06f,   0);
	} else {
		Setup_Scene_Information(199.67f,  67.4f, -169.06f, 628);
	}

	if (Global_Variable_Query(kVariableBB10ShelvesAvailable) > 2) {
		Scene_Exit_Add_2D_Exit(0, 281, 0, 531, 115, 0);
	}

	Scene_Exit_Add_2D_Exit(1, 58, 91, 193, 401, 3);

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Add_Looping_Sound(kSfxRAINALY1, 76, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY7,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT1,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT2,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);

	if (!Game_Flag_Query(kFlagBB10Shelf1Available)) {
		Scene_2D_Region_Add(0, 458, 99, 522, 133);
		Overlay_Play("BB10OVR1", 0, true, false, 0);
	}
	if (!Game_Flag_Query(kFlagBB10Shelf2Available)) {
		Scene_2D_Region_Add(1, 459, 164, 522, 193);
		Overlay_Play("BB10OVR2", 0, true, false, 0);
	}
	if (!Game_Flag_Query(kFlagBB10Shelf3Available)) {
		Scene_2D_Region_Add(2, 458, 194, 522, 223);
		Overlay_Play("BB10OVR3", 0, true, false, 0);
	}
	if (!Game_Flag_Query(kFlagBB10Shelf4Available)) {
		Scene_2D_Region_Add(3, 458, 255, 522, 278);
		Overlay_Play("BB10OVR4", 0, true, false, 0);
	}
	if (!Game_Flag_Query(kFlagBB10Shelf5Available)) {
		Scene_2D_Region_Add(4, 458, 316, 522, 335);
		Overlay_Play("BB10OVR5", 0, true, false, 0);
	}
}

void SceneScriptBB10::SceneLoaded() {
	Obstacle_Object("BARB NIGHT", true);
	Unclickable_Object("BARB NIGHT");
	Unobstacle_Object("Box-Floor Hole01", true);
	Unobstacle_Object("Box-Floor Hole02", true);
	Unobstacle_Object("Box-Floor Hole03", true);
}

bool SceneScriptBB10::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB10::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB10::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB10::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB10::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 225.58f, 67.2f, -102.1f, 0, true, false, false)) {
			Player_Set_Combat_Mode(false);
			Actor_Face_Heading(kActorMcCoy, 274, false);
			Footstep_Sound_Override_On(2);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 2, true, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
			Footstep_Sound_Override_On(2);
			Loop_Actor_Travel_Ladder(kActorMcCoy, 3, true, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB10toBB11);
			Set_Enter(kSetBB11, kSceneBB11);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 206.01f, 66.84f, -261.62f, 0, true, false, false)
		 && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, 151.67f, 66.84f, -313.06f, 0, true, false, false)
		) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Reset(kFlagBB09toBB10a);
			Game_Flag_Set(kFlagBB10toBB09);
			Set_Enter(kSetBB09, kSceneBB09);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB10::ClickedOn2DRegion(int region) {
	if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 225.58f, 67.2f, -102.1f, 0, true, false, false)) {
		Actor_Face_Heading(kActorMcCoy, 274, false);
		Sound_Play(Random_Query(kSfxGLASSY1, kSfxGLASSY2), 40, 70, 70, 50);
		switch (region) {
		case 0:
			Overlay_Remove("BB10OVR1");
			Game_Flag_Set(kFlagBB10Shelf1Available);
			Scene_2D_Region_Remove(0);
			break;

		case 1:
			Overlay_Remove("BB10OVR2");
			Game_Flag_Set(kFlagBB10Shelf2Available);
			Scene_2D_Region_Remove(1);
			break;

		case 2:
			Overlay_Remove("BB10OVR3");
			Game_Flag_Set(kFlagBB10Shelf3Available);
			Scene_2D_Region_Remove(2);
			break;

		case 3:
			Overlay_Remove("BB10OVR4");
			Game_Flag_Set(kFlagBB10Shelf4Available);
			Scene_2D_Region_Remove(3);
			break;

		case 4:
			Overlay_Remove("BB10OVR5");
			Game_Flag_Set(kFlagBB10Shelf5Available);
			Scene_2D_Region_Remove(4);
			break;
		}

		Global_Variable_Increment(kVariableBB10ShelvesAvailable, 1);
		if (Global_Variable_Query(kVariableBB10ShelvesAvailable) > 4) {
			Scene_Exit_Add_2D_Exit(0, 281, 0, 531, 115, 0);
		}
		return false;
	}
	return true;
}

void SceneScriptBB10::SceneFrameAdvanced(int frame) {
}

void SceneScriptBB10::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB10::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagBB09toBB10a)) {
		Player_Loses_Control();
		Actor_Set_At_XYZ(kActorMcCoy, 214.01f, 66.84f, -349.62f, 462);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 206.01f, 66.84f, -261.62f, 0, false, false, false);
		Player_Gains_Control();
		Game_Flag_Reset(kFlagBB09toBB10a);
	} else if (Game_Flag_Query(kFlagBB11toBB10)) {
		Actor_Set_At_XYZ(kActorMcCoy, 249.58f, 127.2f, -102.1f, 256);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Ladder(kActorMcCoy, 3, false, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
		Actor_Face_Heading(kActorMcCoy, 768, false);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 3, false, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
		Game_Flag_Reset(kFlagBB11toBB10);
	}
}

void SceneScriptBB10::PlayerWalkedOut() {
}

void SceneScriptBB10::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
