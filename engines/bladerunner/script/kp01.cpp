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

void ScriptKP01::InitializeScene() {
	if (Game_Flag_Query(416)) {
		Setup_Scene_Information(-125.0f, -12.2f, -61.0f, 400);
	} else if (Game_Flag_Query(418)) {
		Setup_Scene_Information(-284.0f, -12.2f, -789.0f, 445);
	} else {
		Setup_Scene_Information(239.0f, -12.2f, -146.0f, 820);
		Game_Flag_Reset(413);
		if (!Game_Flag_Query(674) && !Game_Flag_Query(653)) {
			Game_Flag_Set(674);
			Actor_Set_Goal_Number(1, 420);
		}
	}
	Scene_Exit_Add_2D_Exit(0, 0, 0, 30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 150, 0, 200, 276, 0);
	Scene_Exit_Add_2D_Exit(2, 589, 0, 639, 479, 1);
	Ambient_Sounds_Add_Looping_Sound(464, 34, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(383, 27, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(384, 90, 1, 1);
	Ambient_Sounds_Add_Sound(440, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(441, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(442, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(443, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
}

void ScriptKP01::SceneLoaded() {
	Unobstacle_Object("TRAINCAR-1", true);
	Unobstacle_Object("FORE-JUNK-02", true);
	Obstacle_Object("OBSTACLE1", true);
	Obstacle_Object("TUBE1", true);
	Unclickable_Object("OBSTACLE1");
}

bool ScriptKP01::MouseClick(int x, int y) {
	return false;
}

bool ScriptKP01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptKP01::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptKP01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptKP01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -125.0f, -12.2f, -61.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(415);
			Set_Enter(47, 44);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -284.0f, -12.2f, -789.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(417);
			Set_Enter(46, 43);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 239.0f, 12.2f, -146.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(414);
			Set_Enter(45, 42);
		}
		return true;
	}
	return false;
}


bool ScriptKP01::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptKP01::SceneFrameAdvanced(int frame) {
}

void ScriptKP01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == 1) {
		if (newGoal == 422) {
			if (Game_Flag_Query(378) == 1) {
				Delay(500);
				Actor_Change_Animation_Mode(0, 75);
				Delay(4500);
				Actor_Face_Current_Camera(0, true);
				Actor_Says(0, 510, 3);
			} else {
				Delay(3000);
			}
			Async_Actor_Walk_To_XYZ(0, 76.56f, -12.2f, -405.48f, 0, false);
			//return true;
		} else if (newGoal == 423) {
			Player_Gains_Control();
			Actor_Force_Stop_Walking(0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(417);
			Set_Enter(46, 43);
			//return true;
		}
	}
	//return false;
}

void ScriptKP01::PlayerWalkedIn() {
	if (Game_Flag_Query(416)) {
		Loop_Actor_Walk_To_XYZ(0, -93.0f, -12.2f, -61.0f, 0, 0, false, 0);
		Game_Flag_Reset(416);
	} else if (Game_Flag_Query(418)) {
		Loop_Actor_Walk_To_XYZ(0, -240.0f, -12.2f, -789.0f, 0, 0, false, 0);
		Game_Flag_Reset(418);
	} else {
		Loop_Actor_Walk_To_XYZ(0, 211.0f, -12.2f, -146.0f, 0, 0, false, 0);
		if (!Game_Flag_Query(653)
			&& !Game_Flag_Query(714)
			&& Actor_Query_Goal_Number(1) == 420
			&& Actor_Query_Goal_Number(1) != 599) {
			Player_Loses_Control();
			Actor_Set_Goal_Number(1, 421);
		}
	}
}

void ScriptKP01::PlayerWalkedOut() {
}

void ScriptKP01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
