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

void SceneScriptPS15::InitializeScene() {
	Setup_Scene_Information(-360.0f, -113.43f, 50.0f, 0);
	Scene_Exit_Add_2D_Exit(0, 0, 0, 20, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 620, 0, 639, 479, 1);
	Ambient_Sounds_Add_Looping_Sound(384, 20, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(141, 80, 0, 1);
	Ambient_Sounds_Add_Sound(385, 5, 50, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(156, 5, 20, 30, 30, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(157, 5, 20, 30, 30, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(158, 5, 20, 30, 30, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(73, 5, 20, 5, 9, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 20, 5, 9, -70, 70, -101, -101, 0, 0);
	Actor_Put_In_Set(kActorSergeantWalls, 101);
	Actor_Set_At_XYZ(kActorSergeantWalls, -265.4f, -113.43f, -31.29f, 623);
}

void SceneScriptPS15::SceneLoaded() {
	Obstacle_Object("E.ARCH", true);
	if (Global_Variable_Query(1) == 2) {
		Item_Add_To_World(110, 983, 101, -208.0f, -113.43f, 30.28f, 750, 16, 12, false, true, false, true);
	}
}

bool SceneScriptPS15::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS15::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS15::ClickedOnActor(int actorId) {
	if (actorId == 34) {
		if ((Actor_Clue_Query(kActorMcCoy, kClueWeaponsOrderForm) || Actor_Clue_Query(kActorMcCoy, kCluePoliceIssueWeapons)) && !Actor_Clue_Query(kActorMcCoy, kClueShippingForm)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -256.0f, -113.43f, 43.51f, 0, 1, false, 0)) {
				Actor_Face_Actor(kActorMcCoy, kActorSergeantWalls, true);
				Actor_Face_Actor(kActorSergeantWalls, kActorMcCoy, true);
				Actor_Says(kActorMcCoy, 4470, 17);
				Actor_Says(kActorSergeantWalls, 130, 12);
				Actor_Says(kActorMcCoy, 4475, 18);
				Actor_Says(kActorMcCoy, 4480, 13);
				Actor_Says(kActorSergeantWalls, 140, 16);
				Item_Pickup_Spin_Effect(965, 211, 239);
				Actor_Says(kActorSergeantWalls, 150, 14);
				Actor_Clue_Acquire(kActorMcCoy, kClueShippingForm, 1, kActorSergeantWalls);
				if (!Game_Flag_Query(727)) {
					Item_Remove_From_World(111);
				}
			}
		} else {
			Actor_Face_Actor(kActorMcCoy, kActorSergeantWalls, true);
			Actor_Says(kActorMcCoy, 8600, 15);
			Actor_Says(kActorSergeantWalls, 190, 12);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS15::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 110) {
		if (Actor_Clue_Query(kActorMcCoy, kClueWeaponsOrderForm) && Actor_Clue_Query(kActorMcCoy, kCluePoliceIssueWeapons)) {
			Actor_Says(kActorMcCoy, 8570, 14);
		} else {
			Actor_Face_Actor(kActorMcCoy, kActorSergeantWalls, true);
			Actor_Face_Actor(kActorSergeantWalls, kActorMcCoy, true);
			Actor_Says(kActorMcCoy, 4485, 17);
			Actor_Says(kActorSergeantWalls, 160, 14);
			Actor_Says(kActorMcCoy, 4490, 12);
			Actor_Says(kActorSergeantWalls, 170, 13);
			Actor_Clue_Acquire(kActorMcCoy, kClueWeaponsOrderForm, 1, kActorMcCoy);
			Actor_Clue_Acquire(kActorMcCoy, kCluePoliceIssueWeapons, 1, kActorMcCoy);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS15::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -360.0f, -113.43f, 50.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(204);
			Set_Enter(15, 69);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -183.58f, -113.43f, 91.7f, 0, 1, false, 0)) {
			Actor_Says(kActorMcCoy, 4440, 18);
			Actor_Says(kActorSergeantWalls, 150, 17);
			Sound_Play(155, 90, 0, 0, 50);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(14, 73);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS15::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS15::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS15::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS15::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -326.93f, -113.43f, 101.42f, 0, 0, false, 0);
	if (!Game_Flag_Query(43)) {
		Actor_Face_Actor(kActorMcCoy, kActorSergeantWalls, true);
		Actor_Face_Actor(kActorSergeantWalls, kActorMcCoy, true);
		Actor_Says(kActorSergeantWalls, 0, 12);
		Actor_Says(kActorMcCoy, 4445, 18);
		Actor_Says(kActorSergeantWalls, 10, 12);
		Actor_Says(kActorMcCoy, 4450, 18);
		Actor_Says(kActorSergeantWalls, 60, 13);
		Actor_Says(kActorSergeantWalls, 70, 12);
		Actor_Says(kActorMcCoy, 4460, 15);
		Actor_Says(kActorSergeantWalls, 80, 13);
		Actor_Says(kActorMcCoy, 4465, 16);
		Actor_Says(kActorSergeantWalls, 90, 13);
		Actor_Says(kActorSergeantWalls, 100, 14);
		Actor_Says(kActorSergeantWalls, 110, 15);
		Actor_Says(kActorSergeantWalls, 120, 15);
		Actor_Says(kActorMcCoy, 4555, 14);
		Game_Flag_Set(43);
		//return true;
		return;
	} else {
		//return false;
		return;
	}
}

void SceneScriptPS15::PlayerWalkedOut() {
}

void SceneScriptPS15::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
