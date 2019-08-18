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

void SceneScriptNR10::InitializeScene() {
	if (Game_Flag_Query(kFlagNR09toNR10)) {
		Game_Flag_Reset(kFlagNR09toNR10);
		Setup_Scene_Information(-136.78f, 2.84f, -234.43f, 320);
	} else {
		Game_Flag_Reset(kFlagNR11toNR10);
		Setup_Scene_Information(  19.22f, 2.84f, -250.43f, 540);
	}

	Scene_Exit_Add_2D_Exit(0, 144, 163, 194, 318, 3);
	Scene_Exit_Add_2D_Exit(1, 475,  95, 568, 230, 0);
	if (_vm->_cutContent
	    && !Game_Flag_Query(kFlagMcCoyCommentsOnOldProjector)
	    && !Actor_Query_Is_In_Current_Set(kActorDektora)
	) {
		Scene_2D_Region_Add(0, 323,  86, 473, 320); // projector area 1
		Scene_2D_Region_Add(1, 280, 180, 323, 212); // projector area 2
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxCTDRONE1, 22, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxBRBED5,   33, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,  2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,  2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,  2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagNR10CameraDestroyed)) {
		Scene_Loop_Set_Default(0);
	} else {
		Ambient_Sounds_Adjust_Looping_Sound(kSfx35MM, 31, 0, 1);
		Scene_Loop_Set_Default(2);
	}
}

void SceneScriptNR10::SceneLoaded() {
	Obstacle_Object("HOOK 01", true);
	Unobstacle_Object("BOX21", true);
	Unobstacle_Object("BOX23", true);
	Unclickable_Object("BOX18");
}

bool SceneScriptNR10::MouseClick(int x, int y) {
	return Game_Flag_Query(kFlagNR10McCoyBlinded);
}

bool SceneScriptNR10::ClickedOn3DObject(const char *objectName, bool combatMode) {
	if (Object_Query_Click("BOX18", objectName)
	 && combatMode
	 && Game_Flag_Query(kFlagNR10McCoyBlinded)
	) {
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11Hiding);
		Game_Flag_Set(kFlagNR10CameraDestroyed);
		Game_Flag_Reset(kFlagNR10McCoyBlinded);
		Actor_Set_Invisible(kActorMcCoy, false);
		Actor_Set_Invisible(kActorDektora, false);
		Ambient_Sounds_Remove_Looping_Sound(kSfx35MM, 1);
		Sound_Play(kSfx35MMBRK1, 52, 0, 0, 50);
		Scene_Loop_Set_Default(0);
		Scene_Loop_Start_Special(kSceneLoopModeOnce, 0, true);
		Un_Combat_Target_Object("BOX18");
		Scene_Exits_Enable();
		if (_vm->_cutContent && !Game_Flag_Query(kFlagMcCoyCommentsOnOldProjector)) {
			// restore regions if McCoy has not commented on projector, when Dektora leaves
			Scene_2D_Region_Add(0, 323,  86, 473, 320); // projector area 1
			Scene_2D_Region_Add(1, 280, 180, 323, 212); // projector area 2
		}
		return true;
	}
	return false;
}

bool SceneScriptNR10::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptNR10::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR10::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -152.78f, 2.84f, -238.43f, 0, true, false, false)) {
			Game_Flag_Set(kFlagNR10toNR09);
			Set_Enter(kSetNR09, kSceneNR09);
			return true;
		}
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 11.5f, 2.84f, -304.46f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 55, false);
			Loop_Actor_Travel_Ladder(kActorMcCoy, 8, true, kAnimationModeIdle);
			Game_Flag_Set(kFlagNR10toNR11);
			Set_Enter(kSetNR11, kSceneNR11);
			return true;
		}
	}
	return false;
}

bool SceneScriptNR10::ClickedOn2DRegion(int region) {
	if (_vm->_cutContent) {
		if (!Game_Flag_Query(kFlagMcCoyCommentsOnOldProjector) && (region == 0 || region == 1)) {
			Game_Flag_Set(kFlagMcCoyCommentsOnOldProjector);
			Actor_Face_XYZ(kActorMcCoy, -28.90f, 55.00f, -133.81f, true);
			Actor_Voice_Over(1750, kActorVoiceOver);
			Actor_Voice_Over(1760, kActorVoiceOver);
			Scene_2D_Region_Remove(0);
			Scene_2D_Region_Remove(1);
			return true;
		}
	}
	return false;
}

void SceneScriptNR10::SceneFrameAdvanced(int frame) {
	if (frame == 122) {
		Game_Flag_Set(kFlagNR10McCoyBlinded);
		Actor_Set_Invisible(kActorMcCoy, true);
		Actor_Set_Invisible(kActorDektora, true);
		Combat_Target_Object("BOX18");
		//return true;
		return;
	}

	if (frame == 61
	 && Game_Flag_Query(kFlagNR10McCoyBlinded)
	) {
		Game_Flag_Reset(kFlagNR10McCoyBlinded);
		Player_Set_Combat_Mode(false);
		Actor_Set_Invisible(kActorMcCoy, false);
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR10AttackMcCoy);
		//return true;
		return;
	}
	//return false;
}

void SceneScriptNR10::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR10::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR08GoToNR10) {
		Player_Set_Combat_Mode(true);
		//return true;
		return;
	}

	if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleNR10Wait) {
		Actor_Face_Actor(kActorSteele, kActorMcCoy, true);
		Actor_Says(kActorSteele, 150, 13);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, true);
		Actor_Says(kActorMcCoy, 1580, 14);
		Actor_Says(kActorSteele, 160, 15);
		Actor_Says(kActorMcCoy, 1585, 16);
		Actor_Says(kActorSteele, 1160, 16);
		Delay(1000);
		Actor_Says(kActorSteele, 1290, 14);
		Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNRxxGoToSpinner);
	}
	//		return false;
}

void SceneScriptNR10::PlayerWalkedOut() {
}

void SceneScriptNR10::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
