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

void SceneScriptHC03::InitializeScene() {
	if (Game_Flag_Query(318)) {
		Setup_Scene_Information(656.0f, 1.61f, -95.0f, 497);
		Game_Flag_Set(388);
		Game_Flag_Reset(318);
	} else {
		Setup_Scene_Information(607.0f, 0.14f, 13.0f, 57);
		Game_Flag_Reset(386);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 0, 30, 479, 3);
	if (Game_Flag_Query(403) || Global_Variable_Query(1) > 3) {
		Item_Remove_From_World(121);
		Game_Flag_Set(403);
		Scene_Exit_Add_2D_Exit(1, 400, 275, 515, 375, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(103, 50, 50, 0);
	Ambient_Sounds_Add_Looping_Sound(241, 50, 50, 0);
	Ambient_Sounds_Add_Sound(242, 3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(243, 3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(244, 3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(245, 3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(246, 3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(247, 3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(248, 3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(249, 3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(238, 3, 50, 25, 25, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(240, 3, 50, 33, 33, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(181, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(183, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(190, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(193, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(194, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(388)) {
		Scene_Loop_Set_Default(6);
	} else if (Game_Flag_Query(403) || Global_Variable_Query(1) > 3) {
		Scene_Loop_Set_Default(3);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptHC03::SceneLoaded() {
	Obstacle_Object("GUITAR01", true);
	if (Game_Flag_Query(403) || Game_Flag_Query(388) || Global_Variable_Query(1) > 3) {
		Unobstacle_Object("GPscisGate", true);
	} else {
		Obstacle_Object("GPscisGate", true);
	}
	Unclickable_Object("GUITAR01");
}

bool SceneScriptHC03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHC03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptHC03::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptHC03::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 121) {
		if (a2) {
			Scene_Loop_Set_Default(3);
			Scene_Loop_Start_Special(2, 2, 1);
			Game_Flag_Set(403);
			Item_Remove_From_World(121);
			Unobstacle_Object("GPscisGate", true);
		} else {
			Actor_Says(kActorMcCoy, 8522, 12);
		}
		return true;
	}
	if (itemId == 107) {
		Item_Remove_From_World(107);
		Item_Pickup_Spin_Effect(977, 68, 435);
		Delay(1500);
		Item_Pickup_Spin_Effect(984, 78, 435);
		Delay(1500);
		Item_Pickup_Spin_Effect(984, 58, 435);
		if (Game_Flag_Query(374)) {
			Actor_Clue_Acquire(kActorMcCoy, kCluePhotoOfMcCoy1, 1, kActorIzo);
		} else {
			Actor_Clue_Acquire(kActorMcCoy, kCluePhotoOfMcCoy2, 1, kActorIzo);
		}
		Actor_Clue_Acquire(kActorMcCoy, kClueChinaBar, 1, kActorIzo);
		return true;
	}
	return false;
}

bool SceneScriptHC03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 607.0f, 0.14f, 9.0f, 0, 1, false, 0)) {
			Game_Flag_Set(387);
			Set_Enter(8, 31);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 628.0f, 2.04f, -123.0f, 0, 1, false, 0)) {
			if (Game_Flag_Query(388)) {
				Game_Flag_Set(319);
				Game_Flag_Reset(479);
				Game_Flag_Set(259);
				Game_Flag_Set(388);
				Music_Stop(2);
				Set_Enter(75, 87);
			} else {
				Scene_Loop_Set_Default(6);
				Scene_Loop_Start_Special(2, 5, 1);
				Game_Flag_Set(388);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptHC03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHC03::SceneFrameAdvanced(int frame) {
	if (frame == 10 || frame == 19 || frame == 29 || frame == 39 || frame == 49 || frame == 59 || frame == 71 || frame == 82 || frame == 91 || frame == 101 || frame == 111 || frame == 121 || frame == 131) {
		Sound_Play(281, Random_Query(33, 50), 50, 50, 50);
	}
	if (!Game_Flag_Query(521) && frame == 66) {
		Ambient_Sounds_Play_Sound(328, 90, 0, -40, 99);
		Sound_Play(201, Random_Query(47, 47), 0, -40, 50);
		Scene_Exit_Add_2D_Exit(1, 400, 275, 515, 375, 2);
		Game_Flag_Set(521);
	}
}

void SceneScriptHC03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHC03::PlayerWalkedIn() {
}

void SceneScriptHC03::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptHC03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
