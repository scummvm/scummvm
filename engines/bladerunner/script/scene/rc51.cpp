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

enum kRC51Loops {
	kRC51LoopInshotFromRC02 = 0,
	kRC51LoopMainLoop       = 1
};

enum kRC51Exits {
	kRC51ExitRC02 = 0
};

void SceneScriptRC51::InitializeScene() {
	Setup_Scene_Information(-8.87f, -1238.89f, 108164.27f, 66);
	Scene_Exit_Add_2D_Exit(kRC51ExitRC02, 0, 460, 639, 479, 2);
	Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kRC51LoopInshotFromRC02, false);
	Scene_Loop_Set_Default(kRC51LoopMainLoop);
}

void SceneScriptRC51::SceneLoaded() {
	Obstacle_Object("POSTER_2", true);
	Obstacle_Object("CURTAIN", true);
	Clickable_Object("POSTER_2");
	Unclickable_Object("GRL_DSK");
	Unclickable_Object("GRL_DSKLEG");
	Unclickable_Object("CURTAIN");
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Unclickable_Object("SCRTY CA03");
	Unclickable_Object("DRAPE01");
	Unclickable_Object("DRAPE02");
	Unclickable_Object("DRAPE03");
	Unclickable_Object("DRAPE04");
	Unclickable_Object("DRAPE05");
	Unclickable_Object("DRAPE06");
	Unclickable_Object("DRAPE07");
	Unclickable_Object("DRAPE08");
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (!Game_Flag_Query(kFlagRC51ChopstickWrapperTaken)) {
		Item_Add_To_World(kItemChopstickWrapper, kModelAnimationChopstickWrapper, kSetRC02_RC51, 47.56f, -1238.89f, 108048.61f, 0, 6, 18, false, true, false, true);
	}
	if (!Game_Flag_Query(kFlagRC51CandyTaken)) {
		Item_Add_To_World(kItemCandy, kModelAnimationCandy, kSetRC02_RC51, 67.28f, -1193.38f, 108011.27f, 0, 6, 6, false, true, false, true);
	}
	if (!Game_Flag_Query(kFlagRC51ToyDogTaken)) {
		Item_Add_To_World(kItemToyDog, kModelAnimationToyDog, kSetRC02_RC51, -69.65f, -1238.89f, 107995.24f, 256, 18, 18, false, true, false, true);
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
	if (itemId == kItemChopstickWrapper) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 17.97f, -1238.89f, 108053.5f, 0, false, false, false)) {
			Actor_Face_Item(kActorMcCoy, kItemChopstickWrapper, true);
			Actor_Clue_Acquire(kActorMcCoy, kClueChopstickWrapper, true, -1);
			Item_Remove_From_World(kItemChopstickWrapper);
			Item_Pickup_Spin_Effect(kModelAnimationChopstickWrapper, 437, 407);
			Actor_Voice_Over(2010, kActorVoiceOver);
			Game_Flag_Set(kFlagRC51ChopstickWrapperTaken);
			return true;
		}
	}

	if (itemId == kItemCandy) {
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemCandy, 36, true, false)) {
			Actor_Face_Item(kActorMcCoy, kItemCandy, true);
			Actor_Clue_Acquire(kActorMcCoy, kClueCandy, true, -1);
			Item_Remove_From_World(kItemCandy);
			Item_Pickup_Spin_Effect(kModelAnimationCandy, 445, 230);
			Actor_Says(kActorMcCoy, 8735, 3);
			Actor_Says(kActorMcCoy, 8529, 3);
			Game_Flag_Set(kFlagRC51CandyTaken);
			return true;
		}
	}

	if (itemId == kItemToyDog) {
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemToyDog, 36, true, false)) {
			Actor_Face_Item(kActorMcCoy, kItemToyDog, true);
			Actor_Clue_Acquire(kActorMcCoy, kClueToyDog, true, -1);
			Item_Remove_From_World(kItemToyDog);
			Item_Pickup_Spin_Effect(kModelAnimationToyDog, 55, 376);
			Actor_Says(kActorMcCoy, 8525, 3);
			Actor_Says(kActorMcCoy, 8740, 3);
			Game_Flag_Set(kFlagRC51ToyDogTaken);
			return true;
		}
	}
	return false;
}

bool SceneScriptRC51::ClickedOnExit(int exitId) {
	if (exitId == kRC51ExitRC02) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -8.87f, -1238.89f, 108173.27f, 0, true, false, false)) {
			Set_Enter(kSetRC02_RC51, kSceneRC02);
			return true;
		}
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
	Game_Flag_Set(kFlagRC51Discovered);
}

void SceneScriptRC51::PlayerWalkedOut() {
}

void SceneScriptRC51::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
