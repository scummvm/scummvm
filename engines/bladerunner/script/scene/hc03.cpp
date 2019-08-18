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

enum kHC03Loops {
	kHC03LoopMainLoop                 = 0,
	kHC03LoopCageDoorOpening          = 2,
	kHC03LoopMainCageOpen             = 3,
	kHC03LoopTrapDoorOpening          = 5,
	kHC03LoopMainCageOpenTrapDoorOpen = 6
};

void SceneScriptHC03::InitializeScene() {
	if (Game_Flag_Query(kFlagUG02toHC03)) {
		Setup_Scene_Information(656.0f, 1.61f, -95.0f, 497);
		Game_Flag_Set(kFlagHC03TrapDoorOpen);
		Game_Flag_Reset(kFlagUG02toHC03);
	} else {
		Setup_Scene_Information(607.0f, 0.14f, 13.0f, 57);
		Game_Flag_Reset(kFlagHC01toHC03);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 0, 30, 479, 3);

	if (Game_Flag_Query(kFlagHC03CageOpen)
	 || Global_Variable_Query(kVariableChapter) > 3
	) {
		Item_Remove_From_World(kItemGreenPawnLock);
		Game_Flag_Set(kFlagHC03CageOpen);
		Scene_Exit_Add_2D_Exit(1, 400, 275, 515, 375, 2);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 50, 50, 0);
	Ambient_Sounds_Add_Looping_Sound(kSfxHCLOOP1,  50, 50, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM8,   3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM2,   3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM3,   3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM4,   3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM5,   3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM6,   3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM7,   3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM1,   3, 30, 16, 16, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCBELL1,  3, 50, 25, 25, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxINDFLUT1, 3, 50, 33, 33, -100, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0470R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0500R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0960R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1100R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1140R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagHC03TrapDoorOpen)) {
		Scene_Loop_Set_Default(kHC03LoopMainCageOpenTrapDoorOpen);
	} else if (Game_Flag_Query(kFlagHC03CageOpen)
	        || Global_Variable_Query(kVariableChapter) > 3
	) {
		Scene_Loop_Set_Default(kHC03LoopMainCageOpen);
	} else {
		Scene_Loop_Set_Default(kHC03LoopMainLoop);
	}
}

void SceneScriptHC03::SceneLoaded() {
	Obstacle_Object("GUITAR01", true);
	if (Game_Flag_Query(kFlagHC03CageOpen)
	 || Game_Flag_Query(kFlagHC03TrapDoorOpen)
	 || Global_Variable_Query(kVariableChapter) > 3
	) {
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

bool SceneScriptHC03::ClickedOnItem(int itemId, bool combatMode) {
	if (itemId == kItemGreenPawnLock) {
		if (combatMode) {
			Scene_Loop_Set_Default(kHC03LoopMainCageOpen);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, kHC03LoopCageDoorOpening, true);
			Game_Flag_Set(kFlagHC03CageOpen);
			Item_Remove_From_World(kItemGreenPawnLock);
			Unobstacle_Object("GPscisGate", true);
		} else {
			Actor_Says(kActorMcCoy, 8522, 12);
		}
		return true;
	}

	if (itemId == kItemCamera) {
		Item_Remove_From_World(kItemCamera);
		Item_Pickup_Spin_Effect(kModelAnimationIzoCamera, 68, 435);
		Delay(1500);
		Item_Pickup_Spin_Effect(kModelAnimationPhoto, 78, 435);
		Delay(1500);
		Item_Pickup_Spin_Effect(kModelAnimationPhoto, 58, 435);
		if (Game_Flag_Query(kFlagAR02DektoraBoughtScorpions)) {
			Actor_Clue_Acquire(kActorMcCoy, kCluePhotoOfMcCoy1, true, kActorIzo);
		} else {
			Actor_Clue_Acquire(kActorMcCoy, kCluePhotoOfMcCoy2, true, kActorIzo);
		}
		Actor_Clue_Acquire(kActorMcCoy, kClueChinaBar, true, kActorIzo);
		return true;
	}
	return false;
}

bool SceneScriptHC03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 607.0f, 0.14f, 9.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagHC03toHC01);
			Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC01);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 628.0f, 2.04f, -123.0f, 0, true, false, false)) {
			if (!Game_Flag_Query(kFlagHC03TrapDoorOpen)) {
				Scene_Loop_Set_Default(kHC03LoopMainCageOpenTrapDoorOpen);
				Scene_Loop_Start_Special(kSceneLoopModeOnce, kHC03LoopTrapDoorOpening, true);
				Game_Flag_Set(kFlagHC03TrapDoorOpen);
			} else {
				Game_Flag_Set(kFlagHC03toUG02);
				Game_Flag_Reset(kFlagMcCoyInHawkersCircle);
				Game_Flag_Set(kFlagMcCoyInUnderground);
				Game_Flag_Set(kFlagHC03TrapDoorOpen);
				Music_Stop(2);
				Set_Enter(kSetUG02, kSceneUG02);
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
	if (frame == 10
	 || frame == 19
	 || frame == 29
	 || frame == 39
	 || frame == 49
	 || frame == 59
	 || frame == 71
	 || frame == 82
	 || frame == 91
	 || frame == 101
	 || frame == 111
	 || frame == 121
	 || frame == 131
	) {
		Sound_Play(kSfx14KBEEP1, Random_Query(33, 50), 50, 50, 50);
	}

	if (!Game_Flag_Query(kFlagHC03TrapDoorOpened)
	 &&  frame == 66
	) {
		Ambient_Sounds_Play_Sound(kSfxCHAINLNK, 90, 0, -40, 99);
		Sound_Play(kSfxMTLDOOR2, Random_Query(47, 47), 0, -40, 50);
		Scene_Exit_Add_2D_Exit(1, 400, 275, 515, 375, 2);
		Game_Flag_Set(kFlagHC03TrapDoorOpened);
	}
}

void SceneScriptHC03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHC03::PlayerWalkedIn() {
}

void SceneScriptHC03::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptHC03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
