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

void SceneScriptCT51::InitializeScene() {
	Setup_Scene_Information(0.0f, 0.0f, -102.0f, 470);
	Game_Flag_Reset(379);
	Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
	Ambient_Sounds_Add_Looping_Sound(381, 100, 1, 1);
	Ambient_Sounds_Add_Sound(68, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Scene_Loop_Start_Special(0, 0, 0);
	Scene_Loop_Set_Default(1);
}

void SceneScriptCT51::SceneLoaded() {
	Unobstacle_Object("BLANKET03", true);
	Clickable_Object("BED02");
	if (!Actor_Clue_Query(kActorMcCoy, kClueRagDoll)) {
		Item_Add_To_World(85, 943, 6, 44.0f, 0.0f, -95.0f, 540, 24, 24, false, true, false, true);
	}
	if (!Actor_Clue_Query(kActorMcCoy, kClueMoonbus1)) {
		Item_Add_To_World(120, 984, 6, 44.0f, 0.0f, -22.0f, 0, 12, 12, false, true, false, true);
	}
}

bool SceneScriptCT51::MouseClick(int x, int y) {
	return true;
}

bool SceneScriptCT51::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("BED02", objectName)) {
		if (Actor_Clue_Query(kActorMcCoy, kClueHysteriaToken)) {
			Actor_Says(kActorMcCoy, 8580, 12);
			return false;
		}
		Item_Pickup_Spin_Effect(970, 203, 200);
		Actor_Clue_Acquire(kActorMcCoy, kClueHysteriaToken, true, -1);
		Actor_Voice_Over(420, kActorVoiceOver);
		return true;
	}
	return false;
}

bool SceneScriptCT51::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptCT51::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 85) {
		Actor_Clue_Acquire(kActorMcCoy, kClueRagDoll, true, -1);
		Item_Pickup_Spin_Effect(943, 260, 200);
		Ambient_Sounds_Play_Sound(563, 40, 99, 0, 0);
		Item_Remove_From_World(85);
		return true;
	}
	if (itemId == 120) {
		Actor_Clue_Acquire(kActorMcCoy, kClueMoonbus1, true, -1);
		Item_Pickup_Spin_Effect(984, 490, 307);
		Item_Remove_From_World(120);
		Actor_Says(kActorMcCoy, 8527, 3);
		return true;
	}
	return false;
}

bool SceneScriptCT51::ClickedOnExit(int exitId) {
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 0.0f, 0.0f, -102.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -11.0f, 0.0f, -156.0f, 0, 0, false, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(380);
			Set_Enter(kSetCT08_CT51_UG12, kSceneCT08);
		}
		return true;
	}
	return false;

}

bool SceneScriptCT51::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptCT51::SceneFrameAdvanced(int frame) {
}

void SceneScriptCT51::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT51::PlayerWalkedIn() {
}

void SceneScriptCT51::PlayerWalkedOut() {
	if (!Actor_Clue_Query(kActorMcCoy, kClueRagDoll)) {
		Item_Remove_From_World(85);
	}
}

void SceneScriptCT51::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
