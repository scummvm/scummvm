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

// Appears that names for "open" and "close" are switched
enum kMA06Loops {
	kMA06LoopDoorOpen  = 0,
	kMA06LoopMain      = 1,
	kMA06LoopDoorClose = 3
};

void SceneScriptMA06::InitializeScene() {
	Setup_Scene_Information(40.0f, 1.0f, -20.0f, 400);

	Ambient_Sounds_Add_Looping_Sound(210, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(408, 33, 0, 1);

	Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kMA06LoopDoorOpen, false);
	Scene_Loop_Set_Default(kMA06LoopMain);

	Sound_Play(209, 100, 50, 50, 100);
}

void SceneScriptMA06::SceneLoaded() {
	Obstacle_Object("PANEL", true);
	Clickable_Object("PANEL");
	Player_Loses_Control();
}

bool SceneScriptMA06::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptMA06::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptMA06::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptMA06::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptMA06::ClickedOnExit(int exitId) {
	return false;
}

bool SceneScriptMA06::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptMA06::SceneFrameAdvanced(int frame) {
}

void SceneScriptMA06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptMA06::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 40.0f, 1.35f, 0.0f, 0, 0, false, 0);
	Actor_Face_Object(kActorMcCoy, "panel", true);
	Delay(500);
	activateElevator();

	if (isElevatorOnDifferentFloor()) {
		Sound_Play(114, 25, 0, 0, 50);
		Delay(4000);
	}

	Game_Flag_Reset(kFlagMA01toMA06);
	Game_Flag_Reset(kFlagMA02toMA06);
	Game_Flag_Reset(kFlagMA07toMA06);

	if (Game_Flag_Query(kFlagMA06toMA01)) {
		Set_Enter(kSetMA01, kSceneMA01);
	} else if (Game_Flag_Query(kFlagMA06ToMA02)) {
		Set_Enter(kSetMA02_MA04, kSceneMA02);
	} else {
		Set_Enter(kSetMA07, kSceneMA07);
	}

	Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA06LoopDoorClose, true);
	Sound_Play(208, 100, 50, 50, 50);
	//return true;
}

void SceneScriptMA06::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Player_Gains_Control();
}

void SceneScriptMA06::DialogueQueueFlushed(int a1) {
}

bool SceneScriptMA06::isElevatorOnDifferentFloor() {
	return (Game_Flag_Query(kFlagMA01toMA06) && !Game_Flag_Query(kFlagMA06toMA01))
		|| (Game_Flag_Query(kFlagMA02toMA06) && !Game_Flag_Query(kFlagMA06ToMA02))
		|| (Game_Flag_Query(kFlagMA07toMA06) && !Game_Flag_Query(kFlagMA06toMA07));
}

void SceneScriptMA06::activateElevator() {
	Game_Flag_Reset(kFlagMA06toMA01);
	Game_Flag_Reset(kFlagMA06ToMA02);
	Game_Flag_Reset(kFlagMA06toMA07);
	while (true) {
		if (Game_Flag_Query(kFlagMA06ToMA02)) {
			break;
		}
		if (Game_Flag_Query(kFlagMA06toMA01)) {
			break;
		}
		if (Game_Flag_Query(kFlagMA06toMA07)) {
			break;
		}

		Actor_Says(kActorAnsweringMachine, 80, kAnimationModeTalk);
		Player_Gains_Control();
		int floor = Elevator_Activate(kElevatorMA);
		Player_Loses_Control();

		Scene_Loop_Start_Special(kSceneLoopModeOnce, kMA06LoopMain, true);

		if (floor > 1) {
			Game_Flag_Set(kFlagMA06toMA07);
		} else if (floor == 1) {
			if (Game_Flag_Query(kFlagSpinnerAtMA01)) {
				Game_Flag_Set(kFlagMA06toMA01);
			} else {
				Sound_Play(412, 100, 0, 0, 50);
				Delay(500);
				Actor_Says(kActorAnsweringMachine, 610, 3);
			}
		} else { // floor == 0
			Actor_Says(kActorMcCoy, 2940, 18);
			if (Global_Variable_Query(kVariableChapter) == 4
			 && Game_Flag_Query(kFlagMA02RajifTalk)
			) {
				Sound_Play(412, 100, 0, 0, 50);
				Delay(500);
				Actor_Says(kActorAnsweringMachine, 610, kAnimationModeTalk);
				Delay(500);
				Actor_Says(kActorMcCoy, 8527, kAnimationModeTalk);
			} else {
				Game_Flag_Set(kFlagMA06ToMA02);
				Actor_Says(kActorAnsweringMachine, 90, 3);
			}
		}
	}
}

} // End of namespace BladeRunner
