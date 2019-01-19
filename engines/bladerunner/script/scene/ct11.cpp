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

void SceneScriptCT11::InitializeScene() {
	if (Game_Flag_Query(91)) {
		Setup_Scene_Information(-378.0f, 9.68f, -55.0f, 440);
	} else if (Game_Flag_Query(558)) {
		Setup_Scene_Information(315.0f, 0.0f, 628.0f, 0);
	} else {
		Setup_Scene_Information(152.0f, 9.68f, -8.0f, 0);
	}
	Scene_Exit_Add_2D_Exit(0, 257, 240, 364, 330, 1);
	Scene_Exit_Add_2D_Exit(1, 97, 0, 155, 324, 0);
	Scene_Exit_Add_2D_Exit(2, 0, 0, 20, 479, 3);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 0, 1);
	Ambient_Sounds_Add_Sound(67, 5, 80, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(66, 5, 80, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(378, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(379, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(380, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);
	if (Global_Variable_Query(kVariableChapter) <= 3) {
		Scene_Loop_Set_Default(0);
	} else {
		Scene_Loop_Set_Default(2);
	}
}

void SceneScriptCT11::SceneLoaded() {
	Obstacle_Object("TRASH CAN", true);
	Unobstacle_Object("BOX NORTHWEST 1", true);
	Unobstacle_Object("BOX SOUTH 1", true);
	if (Global_Variable_Query(kVariableChapter) < 4) {
		if (!Game_Flag_Query(645)) {
			Item_Add_To_World(115, 951, 33, 640.21f, 30.0f, 470.0f, 512, 12, 12, false, true, false, true);
			Scene_2D_Region_Add(0, 505, 316, 513, 321);
			Game_Flag_Set(725);
		}
		if (!Actor_Clue_Query(kActorMcCoy, kClueCar)) {
			Scene_2D_Region_Add(1, 412, 258, 552, 358);
		}
	} else {
		if (Game_Flag_Query(725)) {
			Item_Remove_From_World(115);
			Game_Flag_Reset(725);
			Game_Flag_Set(645);
		}
		Unobstacle_Object("BRIDGE SUPPORT", true);
		Unobstacle_Object("BODY", true);
		Unobstacle_Object("HEADLIGHTS", true);
		Unobstacle_Object("LICENSE PLATE-FRONT", true);
		Unobstacle_Object("LICENSE PLATE-REAR", true);
		Unobstacle_Object("BRAKE DISC RF", true);
		Unobstacle_Object("TIRE RF", true);
		Unobstacle_Object("RIM RF", true);
		Unobstacle_Object("DOOR RIGHT", true);
		Unobstacle_Object("BUMPER REAR", true);
	}
	Unclickable_Object("TRASH CAN");
}

bool SceneScriptCT11::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptCT11::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptCT11::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptCT11::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptCT11::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 121.0f, 9.68f, -42.0f, 0, 1, false, 0)) {
			Game_Flag_Set(304);
			Set_Enter(kSetCT09, kSceneCT09);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -300.0f, 9.68f, 66.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -400.0f, 9.68f, -70.0f, 0, 1, false, 0);
			Game_Flag_Set(kFlagCT11toCT12);
			Set_Enter(kSetCT01_CT12, kSceneCT12);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 290.0f, 0.0f, 635.0f, 0, 1, false, 0)) {
			Game_Flag_Set(531);
			Game_Flag_Reset(kFlagMcCoyAtCTxx);
			Game_Flag_Set(kFlagMcCoyAtDRxx);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
		}
		return true;
	}
	return false;
}

bool SceneScriptCT11::ClickedOn2DRegion(int region) {
	if (region == 0 && Game_Flag_Query(725)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 686.0f, 0.0f, 658.0f, 12, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 47, false);
			Item_Remove_From_World(115);
			Actor_Clue_Acquire(kActorMcCoy, kClueLichenDogWrapper, false, -1);
			Item_Pickup_Spin_Effect(951, 510, 319);
			Game_Flag_Reset(725);
			Game_Flag_Set(645);
			Actor_Voice_Over(550, kActorVoiceOver);
			Actor_Voice_Over(560, kActorVoiceOver);
			Actor_Voice_Over(570, kActorVoiceOver);
			Actor_Voice_Over(580, kActorVoiceOver);
		}
		return true;
	}
	if (region == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 686.0f, 0.0f, 658.0f, 12, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 47, false);
			int temp = 0;
			if (Actor_Clue_Query(kActorMcCoy, kClueCrowdInterviewB)) {
				temp = 1;
			}
			if (Actor_Clue_Query(kActorMcCoy, kCluePaintTransfer)) {
				++temp;
			}
			if (Actor_Clue_Query(kActorMcCoy, kClueLabPaintTransfer)) {
				++temp;
			}
			if (Actor_Clue_Query(kActorMcCoy, kClueLicensePlate)) {
				temp += 2;
			}
			if (Actor_Clue_Query(kActorMcCoy, kClueCarColorAndMake)) {
				temp += 2;
			}
			if (Actor_Clue_Query(kActorMcCoy, kCluePartialLicenseNumber)) {
				temp += 2;
			}
			if (temp <= 2 || Actor_Clue_Query(kActorMcCoy, kClueCar)) {
				Actor_Says(kActorMcCoy, 8525, 12);
			} else {
				Actor_Voice_Over(510, kActorVoiceOver);
				Actor_Voice_Over(520, kActorVoiceOver);
				Actor_Voice_Over(530, kActorVoiceOver);
				Actor_Voice_Over(540, kActorVoiceOver);
				Actor_Clue_Acquire(kActorMcCoy, kClueCar, false, -1);
				Scene_2D_Region_Remove(1);
			}
		}
		return true;
	}
	return false;
}

void SceneScriptCT11::SceneFrameAdvanced(int frame) {
}

void SceneScriptCT11::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT11::PlayerWalkedIn() {
	if (Game_Flag_Query(91)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -358.0f, 9.68f, 32.0f, 0, 0, false, 0);
		Game_Flag_Reset(91);
	} else if (Game_Flag_Query(558)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 329.0f, 0.0f, 617.0f, 0, 0, false, 0);
		Game_Flag_Reset(558);
	} else {
		Player_Loses_Control();
		Actor_Set_Immunity_To_Obstacles(kActorMcCoy, true);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 125.0f, 9.68f, 74.0f, 0, 0, false, 0);
		Actor_Set_Immunity_To_Obstacles(kActorMcCoy, false);
		Player_Gains_Control();
		Game_Flag_Reset(kFlagCT09toCT11);
	}
}

void SceneScriptCT11::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptCT11::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
