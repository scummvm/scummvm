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

enum kUG08Loops {
	kUG08LoopElevatorComingUp  = 0,
	kUG08LoopMainLoopElevator  = 1,
	kUG08LoopElevatorGoingDown = 3,
	kUG08LoopMainNoElevator    = 4
};

void SceneScriptUG08::InitializeScene() {
	if (Game_Flag_Query(kFlagUG13toUG08)) {
		Setup_Scene_Information(-124.0f, 93.18f,   71.0f, 745);
	} else {
		Setup_Scene_Information(-432.0f,   0.0f, -152.0f, 370);
	}

	Scene_Exit_Add_2D_Exit(0, 125, 220, 157, 303, 3);
	Scene_Exit_Add_2D_Exit(1, 353, 145, 552, 309, 1);

	Ambient_Sounds_Add_Looping_Sound(kSfxSTMLOOP7, 28, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED2,   40, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxYELL1M1,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M2,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M1, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M3, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP1,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP2,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP3,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP4,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP5,  2,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);

	if (!Game_Flag_Query(kFlagUG08Entered)) {
		Game_Flag_Set(kFlagUG08ElevatorUp);
		Game_Flag_Set(kFlagUG08Entered);
	}

#if BLADERUNNER_ORIGINAL_BUGS
#else
	// if there's no elevator here (because it's in UG13 and the player took the long way round,
	// McCoy is unable to call it up (there's no call button)
	// Additionally, since the elevator exit is added regardless, McCoy will walk up to
	// the elevator and then the elevator will blink in (BUG)
	// Solution here is to:
	// In easy mode: Have the elevator always be here.
	// In normal/hard mode: If the elevator is missing, then:
	//		by ~33% probability the elevator will be up again,
	//		and by ~66% it won't be up, so we remove the elevator exit, and force McCoy to go the long way round again (or retry the room)
	if (!Game_Flag_Query(kFlagUG08ElevatorUp)) {
		if (Query_Difficulty_Level() == kGameDifficultyEasy
		    || Random_Query(1, 3) == 1
		) {
			Game_Flag_Set(kFlagUG08ElevatorUp);
		} else {
			Scene_Exit_Remove(1);
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (Game_Flag_Query(kFlagUG13toUG08)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kUG08LoopElevatorComingUp, false);
		Scene_Loop_Set_Default(kUG08LoopMainLoopElevator);
	} else if (Game_Flag_Query(kFlagUG08ElevatorUp)) {
		Scene_Loop_Set_Default(kUG08LoopMainLoopElevator);
	} else {
		Scene_Loop_Set_Default(kUG08LoopMainNoElevator);
	}
}

void SceneScriptUG08::SceneLoaded() {
	Obstacle_Object("ELEV LEGS", true);
	Unobstacle_Object("ELEV LEGS", true);
	Unobstacle_Object("BOX RIGHT WALL ", true);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Obstacle_Object("NORTHBLOCK", true);
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptUG08::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG08::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG08::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG08::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG08::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -432.0f, 0.0f, -152.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG08toUG07);
			Set_Enter(kSetUG07, kSceneUG07);
			return false;
		}
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -312.0f, -2.0f, 152.0f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 240, false);
			Footstep_Sound_Override_On(2);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 11, true, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -118.02f, 93.02f, 52.76f, 0, false, false, false);
			Player_Loses_Control();
			Actor_Set_Invisible(kActorMcCoy, true);
			Game_Flag_Set(kFlagUG08toUG13);
			Game_Flag_Reset(kFlagUG08ElevatorUp);
			Set_Enter(kSetUG13, kSceneUG13);
			Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kUG08LoopElevatorGoingDown, false);
			return false;
		}
	}
	return false;
}

bool SceneScriptUG08::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG08::SceneFrameAdvanced(int frame) {
	if (frame == 91) {
		Ambient_Sounds_Play_Sound(kSfxCARGELE2, 90, 0, 0, 100);
	}
}

void SceneScriptUG08::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG08::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagUG13toUG08)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -167.0f, 93.18f,  71.0f, 0, false, false, false);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -180.0f, 93.18f, 134.0f, 0, false, false, false);
		Actor_Face_Heading(kActorMcCoy, 745, false);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 11, false, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
		Player_Gains_Control();
	}
	Game_Flag_Reset(kFlagUG07toUG08);
	Game_Flag_Reset(kFlagUG13toUG08);
}

void SceneScriptUG08::PlayerWalkedOut() {
}

void SceneScriptUG08::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
