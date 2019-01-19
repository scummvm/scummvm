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
	Scene_Exit_Add_2D_Exit(0, 0, 0, 40, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 78, 224, 162, 330, 0);
	Scene_Exit_Add_2D_Exit(2, 500, 180, 619, 346, 0);
	if (Global_Variable_Query(kVariableChapter) > 2) {
		Scene_Exit_Add_2D_Exit(3, 620, 0, 639, 479, 1);
	}
	if (Global_Variable_Query(kVariableChapter) > 3) {
		Scene_Exit_Add_2D_Exit(4, 324, 150, 435, 340, 0);
	}
	Ambient_Sounds_Add_Looping_Sound(54, 33, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(55, 20, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(56, 20, -100, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(68, 60, 180, 20, 33, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Global_Variable_Query(kVariableChapter) < 2
	 && Actor_Query_Goal_Number(kActorGaff) == 1
	) {
		Actor_Put_In_Set(kActorGaff, kSetCT01_CT12);
		Actor_Set_At_XYZ(kActorGaff, -534.0f, -6.5f, 952.0f, 367);
		Game_Flag_Set(kFlagGaffSpinnerCT12);
	}
	if (Game_Flag_Query(kFlagCT01toCT12)
	 && Game_Flag_Query(kFlagSpinnerAtCT01)
	) {
		if (Global_Variable_Query(kVariableChapter) != 2
		 && Global_Variable_Query(kVariableChapter) != 3
		) {
			Scene_Loop_Start_Special(0, 1, 0);
		}
		Scene_Loop_Set_Default(2);
		Game_Flag_Reset(kFlagCT01toCT12);
	} else if ( Game_Flag_Query(kFlagCT01toCT12)
	        && !Game_Flag_Query(kFlagSpinnerAtCT01)
	) {
		if (Global_Variable_Query(kVariableChapter) != 2
		 && Global_Variable_Query(kVariableChapter) != 3
		) {
			Scene_Loop_Start_Special(0, 0, 0);
		}
		Scene_Loop_Set_Default(2);
		Game_Flag_Reset(kFlagCT01toCT12);
	} else if (Game_Flag_Query(kFlagCT05toCT12)
	        && Game_Flag_Query(kFlagGaffSpinnerCT12)
	) {
		Game_Flag_Reset(kFlagCT05toCT12);
		Scene_Loop_Set_Default(4);
	} else if ( Game_Flag_Query(kFlagCT05toCT12)
	        && !Game_Flag_Query(kFlagGaffSpinnerCT12)
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

	if (actorId == kActorOfficerGrayford
	 && Global_Variable_Query(kVariableChapter) == 4
	 && Game_Flag_Query(671)
	 && Game_Flag_Query(703)
	) {
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Says(kActorMcCoy, 710, kAnimationModeTalk);
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
		Actor_Says(kActorOfficerGrayford, 90, kAnimationModeTalk);
		Actor_Says(kActorOfficerGrayford, 100, kAnimationModeTalk);
		Actor_Says(kActorOfficerGrayford, 110, kAnimationModeTalk);
		Game_Flag_Set(629);
		Game_Flag_Set(666);
		Actor_Set_Goal_Number(kActorMcCoy, 400);
	}
	return false;
}

bool SceneScriptCT12::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptCT12::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -419.15f, -6.5f, 696.94f, 0, true, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT12toCT01);
			Set_Enter(kSetCT01_CT12, kSceneCT01);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -182.69f, -6.5f, 696.94f, 0, true, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT12toCT03);
			Set_Enter(kSetCT03_CT04, kSceneCT03);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -386.13f, -6.5f, 1132.72f, 0, true, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT12toCT05);
			Set_Enter(kSetCT05, kSceneCT05);
		}
		return true;
	}
	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -493.0f, -6.5f, 1174.0f, 0, true, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagCT12toCT11);
			Set_Enter(kSetCT11, kSceneCT11);
		}
		return true;
	}
	if (exitId == 4) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -292.0f, -6.5f, 990.0f, 0, true, false, 0)) {
			if (Global_Variable_Query(kVariableChapter) == 4) {
				Game_Flag_Set(629);
			}
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
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
		Sound_Play(Random_Query(59, 60), 10, -80, -80, 50);
	}
	if (frame == 160) {
		Actor_Change_Animation_Mode(kActorGaff, 41);
	}
	if (frame == 152) {
		Sound_Play(116, 100, 40, 0, 50);
	}
	if (frame == 203) {
		Sound_Play(119, 100, 40, 0, 50);
	}
	if (frame == 212) {
		Sound_Play(117, 40, 0, 0, 50);
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
	 &&  Actor_Query_Goal_Number(kActorGaff) == 1
	) {
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
		CDB_Set_Crime(kClueZuben, kCrimeMoonbusHijacking);

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

		Actor_Set_Goal_Number(kActorGaff, 2);
	}
	if (Game_Flag_Query(kFlagCT11toCT12)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -520.0f, -6.5f, 1103.0f, 0, false, false, 0);
		Game_Flag_Reset(kFlagCT11toCT12);
	}
}

void SceneScriptCT12::PlayerWalkedOut() {
	Game_Flag_Reset(kFlagGenericWalkerWaiting);
	if (Game_Flag_Query(kFlagCT12ToUG09)) {
		Game_Flag_Reset(kFlagMcCoyAtCTxx);
		Game_Flag_Set(kFlagMcCoyAtUGxx);
	}
}

void SceneScriptCT12::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
