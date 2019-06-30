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

void SceneScriptUG14::InitializeScene() {
	if (Game_Flag_Query(kFlagUG19toUG14)) {
		Setup_Scene_Information(-278.0f,  12.97f, -152.0f, 200);
	} else if (Game_Flag_Query(kFlagUG12toUG14)) {
		Setup_Scene_Information( -50.0f,  129.0f, -814.0f, 533);
	} else {
		Setup_Scene_Information( 233.0f, 186.04f,  -32.0f, 865);
	}

	Scene_Exit_Add_2D_Exit(0,   0, 232,  45, 427, 3);
	Scene_Exit_Add_2D_Exit(1, 175,  44, 228, 115, 0);
	Scene_Exit_Add_2D_Exit(2, 537,   0, 639, 190, 1);

	Ambient_Sounds_Add_Looping_Sound(kSfxSTMLOOP7, 28, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED2,   40, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP1,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP2,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M2,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M3,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M1, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M2, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M3, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP3,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP4,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP5,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
}

void SceneScriptUG14::SceneLoaded() {
	Obstacle_Object("OBSTACLE02", true);
	Unobstacle_Object("WALL_E_01", true);
	Unclickable_Object("OBSTACLE02");
}

bool SceneScriptUG14::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG14::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG14::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG14::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG14::ClickedOnExit(int exitId) {
	float x, y, z;
	Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
	bool v1 = y > 57.0f;


	if (exitId == 0) {
		if (v1) {
			if (Loop_Actor_Walk_To_XYZ(kActorMcCoy, 141.47f, 128.92f, -150.16f, 0, true, false, false)) {
				return false;
			}
			Actor_Face_XYZ(kActorMcCoy, -14.53f, 12.12f, -150.16f, true);
			Footstep_Sound_Override_On(3);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 13, false, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
		}

		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -278.0f, 12.97f, -152.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG14toUG19);
			Set_Enter(kSetUG19, kSceneUG19);
		}
		return true;
	}

	if (exitId == 1) {
		if (!v1) {
			if (Loop_Actor_Walk_To_XYZ(kActorMcCoy, -14.53f, 12.12f, -150.16f, 0, true, false, false)) {
				return false;
			}
			Actor_Face_XYZ(kActorMcCoy, 141.47f, 128.92f, -150.16f, true);
			Footstep_Sound_Override_On(3);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 13, true, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
		}

		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -50.0f, 129.0f, -814.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG14toUG12);
			Set_Enter(kSetCT08_CT51_UG12, kSceneUG12);
		}
		return true;
	}

	if (exitId == 2) {
		if (!v1) {
			if (Loop_Actor_Walk_To_XYZ(kActorMcCoy, -14.53f, 12.12f, -150.16f, 0, true, false, false)) {
				return false;
			}
			Actor_Face_XYZ(kActorMcCoy, 141.47f, 128.92f, -150.16f, true);
			Footstep_Sound_Override_On(3);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 13, true, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
		}

		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 157.0f, 128.92f, -108.01f, 0, true, false, false)) {
			Footstep_Sound_Override_On(3);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 6, true, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 233.0f, 186.04f, -32.0f, 0, false, false, false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG14toUG10);
			Set_Enter(kSetUG10, kSceneUG10);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG14::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG14::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG14::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG14::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagUG19toUG14)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -250.0f, 12.97f, -152.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagUG19toUG14);
	} else if (Game_Flag_Query(kFlagUG12toUG14)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -50.0f, 129.0f, -604.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagUG12toUG14);
	} else {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 157.0f, 186.04f, -44.01f, 0, false, false, false);
		Actor_Face_Heading(kActorMcCoy, 10, false);
		Footstep_Sound_Override_On(3);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 6, false, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 157.0f, 128.92f, -148.01f, 0, false, false, false);
		Actor_Face_Heading(kActorMcCoy, 807, false);
		Game_Flag_Reset(kFlagUG10toUG14);
	}

	if ( Global_Variable_Query(kVariableChapter) == 4
	 &&  Game_Flag_Query(kFlagCT04HomelessBodyThrownAway)
	 && !Game_Flag_Query(kFlagUG14DeadHomeless)
	) {
		Overlay_Play("UG14OVER", 0, false, true, 0);
		Delay(1000);
		Actor_Face_Heading(kActorMcCoy, 609, false);
		Delay(3000);
		Actor_Voice_Over(270, kActorVoiceOver);
		Delay(2150);
		Actor_Voice_Over(300, kActorVoiceOver);
		Game_Flag_Set(kFlagUG14DeadHomeless);
	}
	//return false;
}

void SceneScriptUG14::PlayerWalkedOut() {
}

void SceneScriptUG14::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
