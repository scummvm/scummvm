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

void SceneScriptCT12::InitializeScene() {
	if (Game_Flag_Query(kFlagCT01toCT12)) {
		Setup_Scene_Information(-419.0f, -6.5f, 696.0f, 616);
	} else if (Game_Flag_Query(kFlagUG09toCT12)) {
		Setup_Scene_Information(-292.0f, -6.5f, 990.0f, 827);
		if (!Game_Flag_Query(kFlagNotUsed150)) {
			Game_Flag_Set(kFlagNotUsed150);
		}
		Game_Flag_Reset(kFlagUG09toCT12);
	} else if (Game_Flag_Query(kFlagCT11toCT12)) {
		Setup_Scene_Information(-493.0f, -6.5f, 1174.0f, 990);
	} else {
		Setup_Scene_Information(-386.13f, -6.5f, 1132.72f, 783);
	}

	Scene_Exit_Add_2D_Exit(0,   0,   0,  40, 479, 3);
	Scene_Exit_Add_2D_Exit(1,  78, 224, 162, 330, 0);
	Scene_Exit_Add_2D_Exit(2, 500, 180, 619, 346, 0);
	if (Global_Variable_Query(kVariableChapter) > 2) {
		Scene_Exit_Add_2D_Exit(3, 620,   0, 639, 479, 1);
	}
	if (Global_Variable_Query(kVariableChapter) > 3) {
		Scene_Exit_Add_2D_Exit(4, 324, 150, 435, 340, 0);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1, 33,    1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTAMBL1, 20, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTAMBR1, 20, -100, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  60, 180, 20,  33, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);

	if (Global_Variable_Query(kVariableChapter) < 2
	 && Actor_Query_Goal_Number(kActorGaff) == kGoalGaffCT12WaitForMcCoy
	) {
		Actor_Put_In_Set(kActorGaff, kSetCT01_CT12);
		Actor_Set_At_XYZ(kActorGaff, -534.0f, -6.5f, 952.0f, 367);
		Game_Flag_Set(kFlagCT12GaffSpinner);
	}

#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (!Actor_Query_In_Set(kActorHowieLee, kSetCT01_CT12)
	    && Global_Variable_Query(kVariableChapter) < 4) {
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 67, 0); // in kSetCT01_CT12
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (Game_Flag_Query(kFlagCT01toCT12)
	 && Game_Flag_Query(kFlagSpinnerAtCT01)
	) {
		if (Global_Variable_Query(kVariableChapter) != 2
		 && Global_Variable_Query(kVariableChapter) != 3
		) {
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 1, false);
		}
		Scene_Loop_Set_Default(2);
		Game_Flag_Reset(kFlagCT01toCT12);
	} else if ( Game_Flag_Query(kFlagCT01toCT12)
	        && !Game_Flag_Query(kFlagSpinnerAtCT01)
	) {
		if (Global_Variable_Query(kVariableChapter) != 2
		 && Global_Variable_Query(kVariableChapter) != 3
		) {
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		}
		Scene_Loop_Set_Default(2);
		Game_Flag_Reset(kFlagCT01toCT12);
	} else if (Game_Flag_Query(kFlagCT05toCT12)
	        && Game_Flag_Query(kFlagCT12GaffSpinner)
	) {
		Game_Flag_Reset(kFlagCT05toCT12);
		Scene_Loop_Set_Default(4);
	} else if ( Game_Flag_Query(kFlagCT05toCT12)
	        && !Game_Flag_Query(kFlagCT12GaffSpinner)
	) {
		Game_Flag_Reset(kFlagCT05toCT12);
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(2);
	}
}

void SceneScriptCT12::SceneLoaded() {
	Obstacle_Object("BOX18", true);
	Unobstacle_Object("SPINNER BODY", true);
	Unobstacle_Object("HOWFLOOR", true);
	Unclickable_Object("TURBINE");
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Unclickable_Object("OBJECT03");
	Unclickable_Object("OBJECT04");
	Unclickable_Object("OBJECT05");
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptCT12::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptCT12::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptCT12::ClickedOnActor(int actorId) {
	if (actorId == kActorHowieLee) {
		Actor_Face_Actor(kActorMcCoy, kActorHowieLee, true);
		Actor_Says(kActorMcCoy, 8910, 16);
	}

	// cut off feature? grayford never visit CT12 as goal 308 is never triggered
	// bug? Marking this as a bug to revisit at a later time
	if (actorId == kActorOfficerGrayford
	 && Global_Variable_Query(kVariableChapter) == 4
	 && Game_Flag_Query(kFlagUG18GuzzaScene)
	 && Game_Flag_Query(kFlagUG18BriefcaseTaken)
	) {
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Says(kActorMcCoy, 710, kAnimationModeTalk); // Hold it! I'm not a Replicant, I got proof!
		Actor_Says(kActorOfficerGrayford, 20, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 715, kAnimationModeTalk);
		Actor_Says(kActorOfficerGrayford, 30, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 720, kAnimationModeTalk);
		Actor_Says(kActorOfficerGrayford, 40, kAnimationModeTalk);
		Actor_Says(kActorOfficerGrayford, 50, kAnimationModeTalk);
		Actor_Says(kActorOfficerGrayford, 60, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 725, kAnimationModeTalk);
		Actor_Says(kActorOfficerGrayford, 70, kAnimationModeTalk);
		Actor_Says(kActorOfficerGrayford, 80, kAnimationModeTalk);
		Actor_Says(kActorOfficerGrayford, 90, kAnimationModeTalk); // if you are lying...
		Actor_Says(kActorOfficerGrayford, 100, kAnimationModeTalk); // you gonna wish...
		Actor_Says(kActorOfficerGrayford, 110, kAnimationModeTalk); // Take him in!
		Game_Flag_Set(kFlagUnpauseGenWalkers);
		Game_Flag_Set(kFlagMcCoyFreedOfAccusations);
		Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyStartChapter5);
	}
	return false;
}

bool SceneScriptCT12::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptCT12::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -419.15f, -6.5f, 696.94f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Game_Flag_Set(kFlagCT12toCT01);
			Set_Enter(kSetCT01_CT12, kSceneCT01);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -182.69f, -6.5f, 696.94f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT12toCT03);
			Set_Enter(kSetCT03_CT04, kSceneCT03);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -386.13f, -6.5f, 1132.72f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT12toCT05);
			Set_Enter(kSetCT05, kSceneCT05);
		}
		return true;
	}

	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -493.0f, -6.5f, 1174.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT12toCT11);
			Set_Enter(kSetCT11, kSceneCT11);
		}
		return true;
	}

	if (exitId == 4) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -292.0f, -6.5f, 990.0f, 0, true, false, false)) {
			if (Global_Variable_Query(kVariableChapter) == 4) {
				Game_Flag_Set(kFlagUnpauseGenWalkers);
			}
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT12ToUG09);
			Set_Enter(kSetUG09, kSceneUG09);
		}
		return true;
	}
	return false;
}

bool SceneScriptCT12::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptCT12::SceneFrameAdvanced(int frame) {
	if (((frame - 1) % 10) == 0) {
		Sound_Play(Random_Query(kSfxNEON5, kSfxNEON6), 10, -80, -80, 50);
	}

	if (frame == 160) {
		Actor_Change_Animation_Mode(kActorGaff, 41);
	}

	if (frame == 152) {
		Sound_Play(kSfxSPINOPN4, 100, 40, 0, 50);
	}

	if (frame == 203) {
		Sound_Play(kSfxSPINCLS1, 100, 40, 0, 50);
	}

	if (frame == 212) {
		Sound_Play(kSfxCARUP3,    40, 0, 0, 50);
	}

	if (frame == 269) {
		Player_Gains_Control();
		Player_Set_Combat_Mode(false);
		Actor_Set_Invisible(kActorGaff, false);
	}
}

void SceneScriptCT12::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT12::PlayerWalkedIn() {
	if ( Global_Variable_Query(kVariableChapter) < 2
	 && !Game_Flag_Query(kFlagGaffApproachedMcCoyAboutZuben)
	 &&  Actor_Query_Goal_Number(kActorGaff) == kGoalGaffCT12WaitForMcCoy
	) {
#if !BLADERUNNER_ORIGINAL_BUGS
		// Disable NPC walkers until Gaff goes to his spinner
		Actor_Set_Goal_Number(kActorGenwalkerA, kGoalGenwalkerDefault);
		Actor_Set_Goal_Number(kActorGenwalkerB, kGoalGenwalkerDefault);
		Actor_Set_Goal_Number(kActorGenwalkerC, kGoalGenwalkerDefault);
		Global_Variable_Set(kVariableGenericWalkerConfig, -1);
#endif // !BLADERUNNER_ORIGINAL_BUGS
		Player_Loses_Control();
		Loop_Actor_Walk_To_Actor(kActorGaff, kActorMcCoy, 48, false, false);
		Actor_Face_Actor(kActorGaff, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorGaff, true);
		Actor_Says(kActorGaff, 0, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 670, kAnimationModeTalk);
		Actor_Says(kActorGaff, 10, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 675, kAnimationModeTalk);
		Actor_Says(kActorGaff, 20, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 680, kAnimationModeTalk);
		Actor_Says(kActorGaff, 30, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 685, kAnimationModeTalk);
		Actor_Says(kActorGaff, 40, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 690, kAnimationModeTalk);
		Actor_Clue_Acquire(kActorMcCoy, kClueGaffsInformation, true, kActorGaff);
		Game_Flag_Set(kFlagGaffApproachedMcCoyAboutZuben);
		CDB_Set_Crime(kClueZubenSquadPhoto, kCrimeMoonbusHijacking);

		if (Game_Flag_Query(kFlagGaffApproachedMcCoyAboutZuben)
		 && Game_Flag_Query(kFlagZubenRetired)
		) {
			Actor_Says(kActorGaff, 50, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 695, kAnimationModeTalk);
			Actor_Says(kActorGaff, 60, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 700, kAnimationModeTalk);
			Actor_Says(kActorGaff, 70, kAnimationModeTalk);
			Actor_Clue_Acquire(kActorGaff, kClueMcCoyRetiredZuben, true, -1);
		} else if (Game_Flag_Query(kFlagGaffApproachedMcCoyAboutZuben)
		        && Game_Flag_Query(kFlagZubenSpared)
		) {
			Actor_Says(kActorGaff, 80, kAnimationModeTalk);
			Actor_Says(kActorGaff, 90, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 705, kAnimationModeTalk);
			Actor_Says(kActorGaff, 100, kAnimationModeTalk);
			Actor_Clue_Acquire(kActorGaff, kClueMcCoyLetZubenEscape, true, -1);
		}

#if !BLADERUNNER_ORIGINAL_BUGS
		// unpause generic walkers here, less chance to collide with Gaff
		// while he walks to his spinner
		// This happens when Gaff starts walking towards his spinner rather than
		// when he's reached his spinner, in order to keep this NPC code in one place
		// and because the walkers won't have time to interfere with his path even if
		// they're enabled early.
		if (Global_Variable_Query(kVariableGenericWalkerConfig) < 0
		) {
			Global_Variable_Set(kVariableGenericWalkerConfig, 2);
		}
#endif
		Actor_Set_Goal_Number(kActorGaff, kGoalGaffCT12GoToSpinner);
	}

	if (Game_Flag_Query(kFlagCT11toCT12)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -520.0f, -6.5f, 1103.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagCT11toCT12);
	}
}

void SceneScriptCT12::PlayerWalkedOut() {
	Game_Flag_Reset(kFlagGenericWalkerWaiting); // A bug? why is this here? Left over code?
	if (Game_Flag_Query(kFlagCT12ToUG09)) {
		Game_Flag_Reset(kFlagMcCoyInChinaTown);
		Game_Flag_Set(kFlagMcCoyInUnderground);
	}
#if BLADERUNNER_ORIGINAL_BUGS
#else
	// this enforces existing awry saved games from having a bad flag value
	else {
		Game_Flag_Set(kFlagMcCoyInChinaTown);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptCT12::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
