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

#include "bladerunner/script/scene.h"

namespace BladeRunner {

void SceneScriptNR09::InitializeScene() {
	if (Game_Flag_Query(476)) {
		if (!Game_Flag_Query(640)) {
			Ambient_Sounds_Adjust_Looping_Sound(452, 22, 100, 2);
		}
		Game_Flag_Reset(476);
		Setup_Scene_Information(-556.07f, 0.35f, 399.04f, 440);
	} else {
		if (!Game_Flag_Query(640)) {
			Ambient_Sounds_Add_Looping_Sound(452, 22, 100, 1);
		}
		Setup_Scene_Information(-704.07f, 0.35f, 663.04f, 0);
	}
	Scene_Exit_Add_2D_Exit(0, 400, 100, 440, 280, 1);
	Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
	Ambient_Sounds_Add_Looping_Sound(205, 22, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(71, 33, 0, 1);
	Ambient_Sounds_Add_Sound(303, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
}

void SceneScriptNR09::SceneLoaded() {
	Obstacle_Object("X2NEWSPAPER", true);
	Unobstacle_Object("X2NEWSPAPER", true);
	Unclickable_Object("X2NEWSPAPER");
}

bool SceneScriptNR09::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR09::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptNR09::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptNR09::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR09::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -564.07f, 0.35f, 399.04f, 0, 1, false, 0)) {
			Game_Flag_Set(475);
			Set_Enter(59, 63);
			return true;
		}
	}
	if (exitId == 1) {
		int v1 = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -704.07f, 0.35f, 663.04f, 0, 1, false, 0);
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		if (!v1) {
			Game_Flag_Set(615);
			Set_Enter(13, 61);
			return true;
		}
	}
	return false;
}

bool SceneScriptNR09::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptNR09::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing()) {
		sub_40172C();
	}
}

void SceneScriptNR09::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR09::PlayerWalkedIn() {
	if (Game_Flag_Query(614)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -704.07001f, 0.35f, 623.04f, 0, 0, false, 0);
		Game_Flag_Reset(614);
	}
	//return false;
}

void SceneScriptNR09::PlayerWalkedOut() {
	if (Game_Flag_Query(475)) {
		Music_Stop(2);
	}
}

void SceneScriptNR09::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR09::sub_40172C() {
	if (Music_Is_Playing()) {
		Music_Adjust(31, -80, 2);
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
