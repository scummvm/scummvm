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

void ScriptNR06::InitializeScene() {
	sub_401BAC();
	if (Game_Flag_Query(442)) {
		Setup_Scene_Information(48.0f, -71.88f, -26.0f, 782);
	} else {
		Setup_Scene_Information(-36.0f, 0.37f, -373.0f, 592);
	}
	Scene_Exit_Add_2D_Exit(0, 533, 234, 592, 414, 1);
	Scene_Exit_Add_2D_Exit(1, 238, 137, 337, 322, 0);
	Ambient_Sounds_Add_Looping_Sound(111, 25, 0, 1);
	Ambient_Sounds_Add_Sound(252, 3, 60, 8, 12, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(254, 3, 60, 8, 8, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(255, 3, 60, 8, 8, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(256, 3, 60, 8, 8, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(257, 3, 60, 8, 8, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(258, 3, 60, 8, 8, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(259, 3, 60, 8, 8, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(260, 3, 60, 8, 8, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(261, 3, 60, 8, 8, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(262, 3, 60, 8, 8, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
}

void ScriptNR06::SceneLoaded() {
	Obstacle_Object("CHAIR01", true);
	Unobstacle_Object("LOFT04", true);
	Unobstacle_Object("LINE02", true);
	Unobstacle_Object("WALL01", true);
	Unclickable_Object("CHAIR01");
}

bool ScriptNR06::MouseClick(int x, int y) {
	return false;
}

bool ScriptNR06::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptNR06::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptNR06::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptNR06::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 48.0f, -71.88f, -26.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Game_Flag_Set(441);
			Set_Enter(57, 60);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -137.0f, -71.88f, -243.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 95, false);
			Loop_Actor_Travel_Stairs(0, 8, 1, 0);
			Loop_Actor_Walk_To_XYZ(0, -36.0f, 0.37f, -373.0f, 0, 0, false, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(439);
			Set_Enter(13, 61);
		}
		return true;
	}
	return false;
}

bool ScriptNR06::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptNR06::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing()) {
		sub_401BAC();
	}
	//return false;
}

void ScriptNR06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptNR06::PlayerWalkedIn() {
	if (Game_Flag_Query(442)) {
		Loop_Actor_Walk_To_XYZ(0, -3.0f, -71.88f, -26.0f, 0, 0, false, 0);
		Game_Flag_Reset(442);
	} else {
		Loop_Actor_Walk_To_XYZ(0, -81.72f, 0.12f, -323.49f, 0, 0, false, 0);
		Actor_Face_Heading(0, 600, false);
		Loop_Actor_Travel_Stairs(0, 8, 0, 0);
		Game_Flag_Reset(440);
	}
	//return false;
}

void ScriptNR06::PlayerWalkedOut() {
	if (Game_Flag_Query(441)) {
		Music_Stop(2);
	}
}

void ScriptNR06::DialogueQueueFlushed(int a1) {
}

void ScriptNR06::sub_401BAC() {
	if (Music_Is_Playing()) {
		Music_Adjust(31, 80, 2);
	} else {
		int v0 = Global_Variable_Query(54);
		if (v0 == 0) {
			Music_Play(16, 61, -80, 2, -1, 0, 0);
		} else if (v0 == 1) {
			Music_Play(15, 41, -80, 2, -1, 0, 0);
		} else if (v0 == 2) {
			Music_Play(7, 41, -80, 2, -1, 0, 0);
		}
		v0++;
		if (v0 > 2) {
			v0 = 0;
		}
		Global_Variable_Set(54, v0);
	}
}

} // End of namespace BladeRunner
