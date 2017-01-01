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

#include "bladerunner/script/script.h"

namespace BladeRunner {

void ScriptUG14::InitializeScene() {
	if (Game_Flag_Query(349)) {
		Setup_Scene_Information(-278.0f, 12.97f, -152.0f, 200);
	} else if (Game_Flag_Query(344)) {
		Setup_Scene_Information(-50.0f, 129.0f, -814.0f, 533);
	} else {
		Setup_Scene_Information(233.0f, 186.04f, -32.0f, 865);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 232, 45, 427, 3);
	Scene_Exit_Add_2D_Exit(1, 175, 44, 228, 115, 0);
	Scene_Exit_Add_2D_Exit(2, 537, 0, 639, 190, 1);
	Ambient_Sounds_Add_Looping_Sound(331, 28, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	Ambient_Sounds_Add_Sound(291, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(292, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(401, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(402, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(397, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(398, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(294, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
}

void ScriptUG14::SceneLoaded() {
	Obstacle_Object("OBSTACLE02", true);
	Unobstacle_Object("WALL_E_01", true);
	Unclickable_Object("OBSTACLE02");
}

bool ScriptUG14::MouseClick(int x, int y) {
	return false;
}

bool ScriptUG14::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptUG14::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptUG14::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptUG14::ClickedOnExit(int exitId) {
	float x, y, z;
	Actor_Query_XYZ(0, &x, &y, &z);
	bool v1 = y > 57.0f;
	if (exitId > 2) {
		return false;
	}
	if (!exitId) {
		if (v1) {
			if (Loop_Actor_Walk_To_XYZ(0, 141.47f, 128.92f, -150.16f, 0, 1, false, 0)) {
				return false;
			}
			Actor_Face_XYZ(0, -14.53f, 12.12f, -150.16f, true);
			Footstep_Sound_Override_On(3);
			Loop_Actor_Travel_Stairs(0, 13, 0, 0);
			Footstep_Sound_Override_Off();
		}
		if (!Loop_Actor_Walk_To_XYZ(0, -278.0f, 12.97f, -152.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(348);
			Set_Enter(90, 103);
		}
		return true;
	}
	if (exitId == 1) {
		if (!v1) {
			if (Loop_Actor_Walk_To_XYZ(0, -14.53f, 12.12f, -150.16f, 0, 1, false, 0)) {
				return false;
			}
			Actor_Face_XYZ(0, 141.47f, 128.92f, -150.16f, true);
			Footstep_Sound_Override_On(3);
			Loop_Actor_Travel_Stairs(0, 13, 1, 0);
			Footstep_Sound_Override_Off();
		}
		if (!Loop_Actor_Walk_To_XYZ(0, -50.0f, 129.0f, -814.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(345);
			Set_Enter(6, 96);
		}
		return true;
	}

	if (exitId == 2) {
		if (!v1) {
			if (Loop_Actor_Walk_To_XYZ(0, -14.53f, 12.12f, -150.16f, 0, 1, false, 0)) {
				return false;
			}
			Actor_Face_XYZ(0, 141.47f, 128.92f, -150.16f, true);
			Footstep_Sound_Override_On(3);
			Loop_Actor_Travel_Stairs(0, 13, 1, 0);
			Footstep_Sound_Override_Off();
		}
		if (!Loop_Actor_Walk_To_XYZ(0, 157.0f, 128.92f, -108.01f, 0, 1, false, 0)) {
			Footstep_Sound_Override_On(3);
			Loop_Actor_Travel_Stairs(0, 6, 1, 0);
			Footstep_Sound_Override_Off();
			Loop_Actor_Walk_To_XYZ(0, 233.0f, 186.04f, -32.0f, 0, 0, false, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(346);
			Set_Enter(83, 95);
		}
		return true;
	}
	return false;
}

bool ScriptUG14::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptUG14::SceneFrameAdvanced(int frame) {
}

void ScriptUG14::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptUG14::PlayerWalkedIn() {
	if (Game_Flag_Query(349)) {
		Loop_Actor_Walk_To_XYZ(0, -250.0f, 12.97f, -152.0f, 0, 0, false, 0);
		Game_Flag_Reset(349);
	} else if (Game_Flag_Query(344)) {
		Loop_Actor_Walk_To_XYZ(0, -50.0f, 129.0f, -604.0f, 0, 0, false, 0);
		Game_Flag_Reset(344);
	} else {
		Loop_Actor_Walk_To_XYZ(0, 157.0f, 186.04f, -44.01f, 0, 0, false, 0);
		Actor_Face_Heading(0, 10, false);
		Footstep_Sound_Override_On(3);
		Loop_Actor_Travel_Stairs(0, 6, 0, 0);
		Footstep_Sound_Override_Off();
		Loop_Actor_Walk_To_XYZ(0, 157.0f, 128.92f, -148.01f, 0, 0, false, 0);
		Actor_Face_Heading(0, 807, false);
		Game_Flag_Reset(347);
	}
	if (Global_Variable_Query(1) == 4 && Game_Flag_Query(172) && !Game_Flag_Query(694)) {
		Overlay_Play("UG14OVER", 0, 0, 1, 0);
		Delay(1000);
		Actor_Face_Heading(0, 609, false);
		Delay(3000);
		Actor_Voice_Over(270, 99);
		Delay(2150);
		Actor_Voice_Over(300, 99);
		Game_Flag_Set(694);
	}
	//return false;
}

void ScriptUG14::PlayerWalkedOut() {
}

void ScriptUG14::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
