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
#include "common/debug.h"

namespace BladeRunner {

// Appears that names for "open" and "close" are switched
enum kMA06Loops {
	kMA06LoopDoorOpen  = 0,
	kMA06LoopMainLoop  = 1,
	kMA06LoopDoorClose = 3
};

void SceneScriptMA06::InitializeScene() {
	Setup_Scene_Information(40.0f, 1.0f, -20.0f, 400);

#if BLADERUNNER_ORIGINAL_BUGS
#else
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
#endif // BLADERUNNER_ORIGINAL_BUGS

	Ambient_Sounds_Add_Looping_Sound(kSfxELEAMB3,  50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxAPRTFAN1, 33, 0, 1);

	Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kMA06LoopDoorOpen, false);
	Scene_Loop_Set_Default(kMA06LoopMainLoop);

	Sound_Play(kSfxELDOORC1, 100, 50, 50, 100);
}

void SceneScriptMA06::SceneLoaded() {
	Obstacle_Object("PANEL", true);
	Clickable_Object("PANEL");
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Unclickable_Object("PANEL");
#endif // BLADERUNNER_ORIGINAL_BUGS
	if (_vm->_cutContent) {
		if (Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelShouldBeInElevatorMcCoysAct3) {
			// Put Rachael in set for her first cut scene (Act 3) when it plays out in the elevator
			Actor_Put_In_Set(kActorRachael, kSetMA06);
			Actor_Set_At_XYZ(kActorRachael, 30.15f, 0.0f, 45.16f, 350);
			Actor_Set_Goal_Number(kActorRachael, kGoalRachaelIsInsideMcCoysElevatorAct3);
		} else if (Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelAtEndOfAct3IfMetWithMcCoy
			&& Game_Flag_Query(kFlagMA02RajifTalk)) {
			// Put Rachael in set for her second cut scene (Act 4)
			AI_Movement_Track_Pause(kActorRachael); // don't allow her to "travel" anywhere
			Actor_Put_In_Set(kActorRachael, kSetMA06);
			Actor_Set_At_XYZ(kActorRachael, 30.15f, 0.0f, 50.16f, 170); // different rotation and slightly different placement
			Actor_Set_Goal_Number(kActorRachael, kGoalRachaelIsInsideMcCoysElevatorAct4);
		}
	}
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
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 40.0f, 1.35f, 0.0f, 0, false, false, false);
	Actor_Face_Object(kActorMcCoy, "panel", true);
	Delay(500);
	bool normalElevatorBusiness = true;
	if (_vm->_cutContent) {
		//
		// McCoy + Rachael meetings
		//
		if ((Game_Flag_Query(kFlagMA07toMA06) || Game_Flag_Query(kFlagMA01toMA06))
		     && Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsInsideMcCoysElevatorAct3
		) {
			normalElevatorBusiness = false;
			// Act 3 Deleted scene (can happen within the elevator or outside the building)
			// Skip floor panel selection - force go to MCCOY 88F
			Game_Flag_Reset(kFlagMA06toMA01);
			Game_Flag_Reset(kFlagMA06ToMA02);
			Game_Flag_Reset(kFlagMA06toMA07);
			Ambient_Sounds_Adjust_Looping_Sound(kSfxELEAMB3, 10, 0, 1);
			Ambient_Sounds_Adjust_Looping_Sound(kSfxAPRTFAN1, 10, 0, 1);

			Actor_Says(kActorAnsweringMachine, 80, 3);     // your floor number please
			Actor_Says(kActorMcCoy, 2940, 18);             // McCoy: McCoy 88F
			Game_Flag_Set(kFlagMA06ToMA02);                // to McCoy's Apartment
			Actor_Says(kActorAnsweringMachine, 90, 3);     // 88 thank you
			Actor_Face_Actor(kActorMcCoy, kActorRachael, true);
			Actor_Says(kActorMcCoy, 2710, 14);             // Excuse me

			Actor_Says(kActorMcCoy, 2730, 12);             // RachaelRight
			AI_Movement_Track_Pause(kActorRachael);
			Actor_Set_Goal_Number(kActorRachael, kGoalRachaelIsInsideElevatorStartTalkAct3);

			Game_Flag_Reset(kFlagMA01toMA06);
			Game_Flag_Reset(kFlagMA02toMA06);
			Game_Flag_Reset(kFlagMA07toMA06);
			Ambient_Sounds_Adjust_Looping_Sound(kSfxSPINUP1,  0, 0, 1);
			Ambient_Sounds_Adjust_Looping_Sound(kSfxAPRTFAN1, 0, 0, 1);
			Ambient_Sounds_Adjust_Looping_Sound(kSfxELEAMB3,  0, 0, 1);
			Ambient_Sounds_Remove_Looping_Sound(kSfxSPINUP1,  1);         // stop elev moving sound
			Ambient_Sounds_Remove_Looping_Sound(kSfxAPRTFAN1, 1);         // stop other ambient
			Ambient_Sounds_Remove_Looping_Sound(kSfxELEAMB3,  1);         // stop other ambient
			Actor_Says_With_Pause(kActorRachael, 300, 1.0f, 14);  // GoodbyeMcCoy
			Actor_Says(kActorRachael, 310, 14);                   // Isnt ThisYourFloor --
			Actor_Says(kActorMcCoy, 2860, 14);                    // YouTakeCareOfYoursel

			Set_Enter(kSetMA02_MA04, kSceneMA02);
			Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA06LoopDoorClose, true);
			Sound_Play(kSfxELDOORO2, 100, 50, 50, 50);
		} else if (Game_Flag_Query(kFlagMA02toMA06)
			&& Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsInsideMcCoysElevatorAct4
		) {
			normalElevatorBusiness = false;
			// Act 4 deleted scene case with Rachael
			//
			// TODO fix animations - Do an Optimization pass
			// TODO Test - Skip panel selection but actually set the target floor and "MOVE elevator" to LOBBY (Floor > 1)
			//
			Game_Flag_Reset(kFlagMA06toMA01);
			Game_Flag_Reset(kFlagMA06ToMA02);
			Game_Flag_Reset(kFlagMA06toMA07);
			Ambient_Sounds_Adjust_Looping_Sound(kSfxELEAMB3, 10, 0, 1);
			Ambient_Sounds_Adjust_Looping_Sound(kSfxAPRTFAN1, 10, 0, 1);

			Actor_Says(kActorAnsweringMachine, 80, 3);      // your floor number please
			Actor_Set_Goal_Number(kActorRachael, kGoalRachaelIsInsideElevatorStartTalkAct4);
			//
			Delay(500);
			Game_Flag_Reset(kFlagMA01toMA06);
			Game_Flag_Reset(kFlagMA02toMA06);
			Game_Flag_Reset(kFlagMA07toMA06);
			Ambient_Sounds_Adjust_Looping_Sound(kSfxSPINUP1,  0, 0, 1);
			Ambient_Sounds_Adjust_Looping_Sound(kSfxAPRTFAN1, 0, 0, 1);
			Ambient_Sounds_Adjust_Looping_Sound(kSfxELEAMB3,  0, 0, 1);
			Ambient_Sounds_Remove_Looping_Sound(kSfxSPINUP1,  1);         // stop elev moving sound
			Ambient_Sounds_Remove_Looping_Sound(kSfxAPRTFAN1, 1);         // stop other ambient
			Ambient_Sounds_Remove_Looping_Sound(kSfxELEAMB3,  1);         // stop other ambient

//			Player_Gains_Control();
			Set_Enter(kSetMA07, kSceneMA07);
			Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA06LoopDoorClose, true);
			Sound_Play(kSfxELDOORO2, 100, 50, 50, 50);
		} else {
			// normal elevator business
			normalElevatorBusiness = true;
		}
	} // end of cut content
	if (normalElevatorBusiness) {
		// normal elevator business
		activateElevator();

		if (isElevatorOnDifferentFloor()) {
			Sound_Play(kSfxSPINUP1, 25, 0, 0, 50);
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
		Sound_Play(kSfxELDOORO2, 100, 50, 50, 50);
	}
}

void SceneScriptMA06::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Player_Gains_Control();
	if (_vm->_cutContent) {
		if (Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsInsideElevatorStartTalkAct3) {
			Actor_Set_Goal_Number(kActorRachael, kGoalRachaelAtEndOfAct3IfMetWithMcCoy);
		}
		else if (Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsInsideElevatorStartTalkAct4) {
			Actor_Set_Goal_Number(kActorRachael, kGoalRachaelIsOutsideMcCoysBuildingAct4);
		}
	}
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
		int floorLevel = Elevator_Activate(kElevatorMA);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// Fix for a crash/ freeze bug;
		//  To reproduce original issue: in Act 4, visit Rajiff, then exit to ground floor. Re-enter elevator and press Alt+F4
		if (floorLevel < 0) {
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		Player_Loses_Control();

		Scene_Loop_Start_Special(kSceneLoopModeOnce, kMA06LoopMainLoop, true);

		if (floorLevel > 1) {
			Game_Flag_Set(kFlagMA06toMA07);
		} else if (floorLevel == 1) {
			if (Game_Flag_Query(kFlagSpinnerAtMA01)) {
				Game_Flag_Set(kFlagMA06toMA01);
			} else {
				Sound_Play(kSfxELEBAD1, 100, 0, 0, 50);
				Delay(500);
				Actor_Says(kActorAnsweringMachine, 610, 3);
			}
		} else { // floorLevel == 0
			Actor_Says(kActorMcCoy, 2940, 18);
			if (Global_Variable_Query(kVariableChapter) == 4
			 && Game_Flag_Query(kFlagMA02RajifTalk)
			) {
				Sound_Play(kSfxELEBAD1, 100, 0, 0, 50);
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
