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

void SceneScriptRC51::InitializeScene() {
	Setup_Scene_Information(-8.87f, -1238.89f, 108164.27f, 66);
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	Scene_Loop_Start_Special(0, 0, 0);
	Scene_Loop_Set_Default(1);
}

void SceneScriptRC51::SceneLoaded() {
	Obstacle_Object("POSTER_2", true);
	Obstacle_Object("CURTAIN", true);
	Clickable_Object("POSTER_2");
	Unclickable_Object("GRL_DSK");
	Unclickable_Object("GRL_DSKLEG");
	Unclickable_Object("CURTAIN");
	if (!Game_Flag_Query(147)) {
		Item_Add_To_World(82, 937, 16, 47.56f, -1238.89f, 108048.61f, 0, 6, 18, false, true, false, true);
	}
	if (!Game_Flag_Query(148)) {
		Item_Add_To_World(79, 933, 16, 67.28f, -1193.38f, 108011.27f, 0, 6, 6, false, true, false, true);
	}
	if (!Game_Flag_Query(149)) {
		Item_Add_To_World(98, 971, 16, -69.65f, -1238.89f, 107995.24f, 256, 18, 18, false, true, false, true);
	}
}

bool SceneScriptRC51::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptRC51::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("POSTER_2", objectName)) {
		Actor_Face_Object(kActorMcCoy, "POSTER_2", true);
		Actor_Says(kActorMcCoy, 8620, 3);
		return true;
	}
	return false;
}

bool SceneScriptRC51::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptRC51::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 82 && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, 17.97f, -1238.89f, 108053.5f, 0, 1, false, 0)) {
		Actor_Face_Item(kActorMcCoy, 82, true);
		Actor_Clue_Acquire(kActorMcCoy, kClueChopstickWrapper, 1, -1);
		Item_Remove_From_World(82);
		Item_Pickup_Spin_Effect(937, 437, 407);
		Actor_Voice_Over(2010, kActorVoiceOver);
		Game_Flag_Set(147);
		return true;
	}
	if (itemId == 79 && !Loop_Actor_Walk_To_Item(kActorMcCoy, 79, 36, 1, false)) {
		Actor_Face_Item(kActorMcCoy, 79, true);
		Actor_Clue_Acquire(kActorMcCoy, kClueCandy, 1, -1);
		Item_Remove_From_World(79);
		Item_Pickup_Spin_Effect(933, 445, 230);
		Actor_Says(kActorMcCoy, 8735, 3);
		Actor_Says(kActorMcCoy, 8529, 3);
		Game_Flag_Set(148);
		return true;
	}
	if (itemId == 98 && !Loop_Actor_Walk_To_Item(kActorMcCoy, 98, 36, 1, false)) {
		Actor_Face_Item(kActorMcCoy, 98, true);
		Actor_Clue_Acquire(kActorMcCoy, kClueToyDog, 1, -1);
		Item_Remove_From_World(98);
		Item_Pickup_Spin_Effect(971, 55, 376);
		Actor_Says(kActorMcCoy, 8525, 3);
		Actor_Says(kActorMcCoy, 8740, 3);
		Game_Flag_Set(149);
		return true;
	}
	return false;
}

bool SceneScriptRC51::ClickedOnExit(int exitId) {
	if (exitId == 0 && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, -8.87f, -1238.89f, 108173.27f, 0, 1, false, 0)) {
		Set_Enter(16, 79);
		return true;
	}
	return false;
}

bool SceneScriptRC51::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptRC51::SceneFrameAdvanced(int frame) {
}

void SceneScriptRC51::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptRC51::PlayerWalkedIn() {
	Game_Flag_Set(709);
}

void SceneScriptRC51::PlayerWalkedOut() {
}

void SceneScriptRC51::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
