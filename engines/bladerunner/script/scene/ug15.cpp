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

void SceneScriptUG15::InitializeScene() {
	if (Game_Flag_Query(kFlagUG17toUG15)) {
		Setup_Scene_Information( -25.0f, 26.31f, -434.0f, 520);
	} else if (Game_Flag_Query(kFlagUG16toUG15a)) {
		Setup_Scene_Information( -17.0f, 26.31f, -346.0f, 711);
	} else if (Game_Flag_Query(kFlagUG16toUG15b)) {
		Setup_Scene_Information( -18.0f, 48.07f,   62.0f, 650);
	} else {
		Setup_Scene_Information(-238.0f, 48.07f,  222.0f, 180);
		if (Game_Flag_Query(676)
		 && Random_Query(1, 10) == 10
		) {
			Game_Flag_Reset(676);
		}
	}
	if (Game_Flag_Query(682)) {
		Scene_Loop_Set_Default(3);
	}

	if (Game_Flag_Query(kFlagUG17toUG15)
	 || Game_Flag_Query(kFlagUG16toUG15a)
	) {
		Scene_Exit_Add_2D_Exit(0, 260,   0, 307, 298, 0);
		Scene_Exit_Add_2D_Exit(1, 301, 147, 337, 304, 1);
		Game_Flag_Reset(kFlagUG17toUG15);
		Game_Flag_Reset(kFlagUG16toUG15a);
	} else {
		Scene_Exit_Add_2D_Exit(2, 406, 128, 480, 316, 1);
		Scene_Exit_Add_2D_Exit(3,   0,   0,  30, 479, 3);
	}

	Ambient_Sounds_Add_Looping_Sound(105, 71, 0, 1);
	Ambient_Sounds_Add_Looping_Sound( 95, 45, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 76, 0, 1);
	Ambient_Sounds_Add_Sound(291, 2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(292, 2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(294, 2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(401, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(402, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(397, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(398, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(  1, 5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound( 57, 5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound( 58, 5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(196, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(197, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(198, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(199, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
}

void SceneScriptUG15::SceneLoaded() {
	Unobstacle_Object("CATWALK_01_RAIL02", true);
	Unobstacle_Object("LOFT01", true);
	Obstacle_Object("NUT1", true);
	Clickable_Object("NUT1");
}

bool SceneScriptUG15::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG15::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG15::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG15::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG15::ClickedOnExit(int exitId) {

	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -25.0f, 26.31f, -434.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG15toUG17);
			Set_Enter(kSetUG17, kSceneUG17);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -17.0f, 26.31f, -346.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG15toUG16a);
			Set_Enter(kSetUG16, kSceneUG16);
		}
		return true;
	}

	if (exitId == 2) {
		int v1 = Actor_Query_Goal_Number(kActorFreeSlotA);
		if (v1 >= 300
		 && v1 <= 303
		) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -137.61f, 48.07f, 147.12f, 0, 1, false, 0);
		} else if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 18.0f, 52.28f, 46.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG15toUG16b);
			Set_Enter(kSetUG16, kSceneUG16);
		}
		return true;
	}

	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -238.0f, 52.46f, 222.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG15toUG13);
			Set_Enter(kSetUG13, kSceneUG13);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG15::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG15::SceneFrameAdvanced(int frame) {
	if (Actor_Query_Goal_Number(kActorFreeSlotA) == 300) {
		float x, y, z;
		Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
		if (-160.0f <= x
		 &&  220.0f > z
		) {
			Actor_Set_Goal_Number(kActorFreeSlotA, 301);
		}
	}
	if (frame == 61) {
		Ambient_Sounds_Play_Sound(583, 80, 0, 0, 99);
	}

	if ( Game_Flag_Query(677)
	 && !Game_Flag_Query(682)
	) {
		float x, y, z;
		Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
		if ( -180.0f <= x
		 &&   220.0f > z
		 && !Game_Flag_Query(724)
		) {
			Game_Flag_Set(724);
			Game_Flag_Set(682);
			Scene_Loop_Set_Default(3);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, 2, true);
			Actor_Set_Goal_Number(kActorMcCoy, 390);
			Actor_Query_XYZ(kActorFreeSlotA, &x, &y, &z);

			if (-200.0f < x
			 &&  -62.0f > x
			) {
				Actor_Set_Goal_Number(kActorFreeSlotA, 309);
			}
		}
	}
	//	return false;
}

void SceneScriptUG15::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG15::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagUG16toUG15b)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -62.0f, 48.07f, 102.0f, 0, false, false, 0);
		Game_Flag_Reset(kFlagUG16toUG15b);
	} else if (Game_Flag_Query(kFlagUG13toUG15)) {
		Game_Flag_Reset(kFlagUG13toUG15);
		if (!Game_Flag_Query(676)) {
			Actor_Set_Goal_Number(kActorFreeSlotA, 310);
		}
	}
}

void SceneScriptUG15::PlayerWalkedOut() {
}

void SceneScriptUG15::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
