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

void SceneScriptKP02::InitializeScene() {
	if (Game_Flag_Query(414)) {
		Setup_Scene_Information(-884.0f, -615.49f, 3065.0f, 20);
	} else {
		Setup_Scene_Information(-1040.0f, -615.49f, 2903.0f, 339);
		Game_Flag_Reset(412);
	}
	Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
	Ambient_Sounds_Add_Looping_Sound(464, 34, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(383, 27, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(384, 90, 1, 1);
	Ambient_Sounds_Add_Sound(440, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(441, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(442, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(443, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445, 2, 100, 25, 33, -100, 100, -101, -101, 0, 0);
}

void SceneScriptKP02::SceneLoaded() {
	Obstacle_Object("VAN GRATE", true);
	Clickable_Object("VAN GRATE");
	Unobstacle_Object("VAN GRATE", true);
	Unobstacle_Object("BOX05", true);
	Unobstacle_Object("BOX08", true);
	Unobstacle_Object("BOX09", true);
	Unobstacle_Object("BOX01", true);
	Unclickable_Object("VAN GRATE");
}

bool SceneScriptKP02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptKP02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptKP02::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptKP02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptKP02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1040.0f, -615.49f, 2903.0f, 0, 1, false, 0)) {
			if (Actor_Query_Goal_Number(kActorFreeSlotB) == 406 || Actor_Query_Goal_Number(kActorFreeSlotA) == 406) {
				Non_Player_Actor_Combat_Mode_Off(kActorFreeSlotB);
				Non_Player_Actor_Combat_Mode_Off(kActorFreeSlotA);
				Actor_Set_Goal_Number(kActorFreeSlotB, 400);
				Actor_Set_Goal_Number(kActorFreeSlotA, 400);
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(411);
				Set_Enter(kSetUG12, kSceneUG12);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(411);
				Set_Enter(kSetUG12, kSceneUG12);
			}
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -884.0f, -615.49f, 3065.0f, 0, 1, false, 0)) {
			if (Actor_Query_Goal_Number(kActorFreeSlotB) == 406 || Actor_Query_Goal_Number(kActorFreeSlotA) == 406) {
				Non_Player_Actor_Combat_Mode_Off(kActorFreeSlotB);
				Non_Player_Actor_Combat_Mode_Off(kActorFreeSlotA);
				Actor_Set_Goal_Number(kActorFreeSlotB, 400);
				Actor_Set_Goal_Number(kActorFreeSlotA, 400);
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(413);
				Set_Enter(kSetKP01, kSceneKP01);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(413);
				Set_Enter(kSetKP01, kSceneKP01);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptKP02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptKP02::SceneFrameAdvanced(int frame) {
}

void SceneScriptKP02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptKP02::PlayerWalkedIn() {
	if (Game_Flag_Query(414)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -884.0f, -615.49f, 3035.0f, 0, 0, false, 0);
		Game_Flag_Reset(414);
	}
	if (Game_Flag_Query(653)
	 && Actor_Query_Goal_Number(kActorSteele) != 599
	) {
		Actor_Set_Goal_Number(kActorSteele, 450);
	}
	//return false;
}

void SceneScriptKP02::PlayerWalkedOut() {
}

void SceneScriptKP02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
