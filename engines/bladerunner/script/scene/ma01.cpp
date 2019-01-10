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

enum kMA01Loops {
	kMA01LoopInshotRoof  = 0,
	kMA01LoopMain        = 1,
	kMA01LoopOutDoorAnim = 3,
	kMA01LoopOutshotRoof = 4
};

enum kMA01Exits {
	kMA01ExitMA06    = 0,
	kMA01ExitSpinner = 1
};

void SceneScriptMA01::InitializeScene() {
	Setup_Scene_Information(381.0f, 0.0f, 54.0f, 992);
	if (Game_Flag_Query(kFlagSpinnerToMA01)) {
		Setup_Scene_Information(381.0f, 0.0f, 54.0f, 992);
	}
	if (Game_Flag_Query(kFlagMA06toMA01)) {
		Setup_Scene_Information(1446.0f, 0.0f, -725.0f, 660);
	}
	Scene_Exit_Add_2D_Exit(kMA01ExitMA06, 328, 132, 426, 190, 0);
	if (Game_Flag_Query(kFlagSpinnerToMA01)) {
		Scene_Exit_Add_2D_Exit(kMA01ExitSpinner, 234, 240, 398, 328, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(101, 90, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(99, 40, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(100, 40, 100, 1);
	Ambient_Sounds_Add_Sound(68, 10, 100, 25, 50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 100, 25, 50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 10, 70, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 10, 70, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 10, 70, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(kFlagMA06toMA01)) {
		Scene_Loop_Set_Default(kMA01LoopMain);
		Game_Flag_Reset(kFlagMA06toMA01);
	} else {
		Actor_Set_Invisible(kActorMcCoy, true);
		Game_Flag_Set(273);
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kMA01LoopInshotRoof, false);
		Scene_Loop_Set_Default(kMA01LoopMain);
	}
	if (Game_Flag_Query(409)) {
		Actor_Set_Goal_Number(kActorGaff, 3);
		Game_Flag_Reset(409);
	}
}

void SceneScriptMA01::SceneLoaded() {
	Obstacle_Object("WRENCH", true);
	Unobstacle_Object("OBSTICLEBOX01", true);
	Clickable_Object("WRENCH");
	Unclickable_Object("Y2 PADRIM 01");
	Unclickable_Object("Y2 PADRIM 02");
	Unclickable_Object("NGON01");
}

bool SceneScriptMA01::MouseClick(int x, int y) {
	return Region_Check(286, 326, 348, 384);
}

bool SceneScriptMA01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptMA01::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptMA01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptMA01::ClickedOnExit(int exitId) {
	if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenMA01AttackMcCoy) {
		return true;
	}
	if (exitId == kMA01ExitMA06) {
		if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenFled) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1446.0f, 0.0f, -725.0f, 72, 1, false, 0)) {
				Actor_Set_Goal_Number(kActorZuben, kGoalZubenMA01AttackMcCoy);
				Scene_Exits_Disable();
			}
		} else if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1446.0f, 0.0f, -725.0f, 12, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagMA01toMA06);
			Set_Enter(kSetMA06, kSceneMA06);
		}
		return true;
	}
	if (exitId == kMA01ExitSpinner) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 381.0f, 0.0f, 54.0f, 0, 1, false, 0)) {
			Player_Loses_Control();
			Actor_Face_Heading(kActorMcCoy, 736, false);
			Game_Flag_Reset(176);
			Game_Flag_Reset(182);
			Game_Flag_Reset(179);
			Game_Flag_Reset(180);
			Game_Flag_Reset(261);
			Game_Flag_Reset(177);
			Game_Flag_Reset(258);
			Game_Flag_Reset(178);
			int spinnerDest = Spinner_Interface_Choose_Dest(kMA01LoopOutDoorAnim, false);
			switch (spinnerDest) {
			case kSpinnerDestinationPoliceStation:
				Game_Flag_Set(178);
				Game_Flag_Reset(kFlagSpinnerToMA01);
				Game_Flag_Set(kFlagSpinnerToPS01);
				Set_Enter(kSetPS01, kScenePS01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationRuncitersAnimals:
				Game_Flag_Set(182);
				Game_Flag_Reset(kFlagSpinnerToMA01);
				Game_Flag_Set(kFlagSpinnerToRC01);
				Set_Enter(kSetRC01, kSceneRC01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationChinatown:
				Game_Flag_Set(176);
				Game_Flag_Reset(kFlagSpinnerToMA01);
				Game_Flag_Set(kFlagSpinnerToCT01);
				Set_Enter(kSetCT01_CT12, kSceneCT01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(261);
				Game_Flag_Reset(kFlagSpinnerToMA01);
				Game_Flag_Set(kFlagSpinnerToTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationAnimoidRow:
				Game_Flag_Set(180);
				Game_Flag_Reset(kFlagSpinnerToMA01);
				Game_Flag_Set(kFlagSpinnerToAR01);
				Set_Enter(kSetAR01_AR02, kSceneAR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationDNARow:
				Game_Flag_Set(177);
				Game_Flag_Reset(kFlagSpinnerToMA01);
				Game_Flag_Set(kFlagSpinnerToDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationBradburyBuilding:
				Game_Flag_Set(258);
				Game_Flag_Reset(kFlagSpinnerToMA01);
				Game_Flag_Set(kFlagSpinnerToBB01);
				Set_Enter(kSetBB01, kSceneBB01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationNightclubRow:
				Game_Flag_Set(181);
				Game_Flag_Reset(kFlagSpinnerToMA01);
				Game_Flag_Set(kFlagSpinnerToNR01);
				Set_Enter(kSetNR01, kSceneNR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			case kSpinnerDestinationHysteriaHall:
				Game_Flag_Set(257);
				Game_Flag_Reset(kFlagSpinnerToMA01);
				Game_Flag_Set(kFlagSpinnerToHF01);
				Set_Enter(kSetHF01, kSceneHF01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA01LoopOutshotRoof, true);
				break;
			default:
				Actor_Set_Invisible(kActorMcCoy, false);
				Actor_Face_Heading(kActorMcCoy, 736, false);
				Game_Flag_Set(179);
				break;
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptMA01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptMA01::SceneFrameAdvanced(int frame) {
	if (frame == 15) {
		Ambient_Sounds_Play_Sound(102, 70, -100, 100, 0);
	}
	if (frame == 61 || frame == 183) {
		Ambient_Sounds_Play_Sound(116, 100, 40, 0, 99);
	}
	if (frame == 107 || frame == 227) {
		Ambient_Sounds_Play_Sound(119, 100, 40, 0, 99);
	}
	if (frame == 1) {
		Ambient_Sounds_Play_Sound(118, 40, -60, 20, 99);
	}
	if (frame == 241) {
		Ambient_Sounds_Play_Sound(117, 40, 0, 0, 99);
	}
	if (frame == 58) {
		Sound_Play(122, 17, 20, 20, 50);
	}
	if ((frame == 75 || frame == 196) && Game_Flag_Query(273)) {
		Actor_Face_Heading(kActorMcCoy, 736, false);
		Actor_Change_Animation_Mode(kActorMcCoy, 42);
		Game_Flag_Reset(273);
	} else {
		if (frame == 196 && !Game_Flag_Query(273)) {
			Actor_Change_Animation_Mode(kActorMcCoy, 41);
			//return true;
			return;
		}
		if (frame == 240) {
			Player_Gains_Control();
		}
	}
	//return true;
}

void SceneScriptMA01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptMA01::PlayerWalkedIn() {
}

void SceneScriptMA01::PlayerWalkedOut() {
	Actor_Set_Invisible(kActorMcCoy, false);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(kFlagMA01toMA06) && Global_Variable_Query(kVariableChapter) == 1) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeTowards2, true, -1);
		Outtake_Play(kOuttakeInside1, true, -1);
		Outtake_Play(kOuttakeTowards1, true, -1);
	}
}

void SceneScriptMA01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
