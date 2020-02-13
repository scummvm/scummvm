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

enum kPS01Loops {
	kPS01LoopInshot   = 0,
	kPS01LoopMainLoop  = 1,
	kPS01LoopDoorAnim  = 3,
	kPS01LoopOutshot  = 4,
	kPS01LoopNoSpinner = 5
};

void SceneScriptPS01::InitializeScene() {
	Setup_Scene_Information(1872.0f, 16592.0f, -2975.0f, 200);
	Scene_Exit_Add_2D_Exit(0, 36, 194, 138, 326, 0);
	if (Game_Flag_Query(kFlagSpinnerAtPS01)) {
		Scene_Exit_Add_2D_Exit(1, 344, 288, 584, 384, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(kSfxRAIN10, 100, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagPS02toPS01)) {
		if (Game_Flag_Query(kFlagSpinnerAtPS01)) {
			Scene_Loop_Set_Default(kPS01LoopMainLoop);
		} else {
			Scene_Loop_Set_Default(kPS01LoopNoSpinner);
		}
	} else {
		Actor_Set_Invisible(kActorMcCoy, true);
		Game_Flag_Set(kFlagArrivedFromSpinner2);
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kPS01LoopInshot, false);
		Scene_Loop_Set_Default(kPS01LoopMainLoop);
		Player_Loses_Control();
	}
}

void SceneScriptPS01::SceneLoaded() {
	Obstacle_Object("TUBE81", true);
	if (Game_Flag_Query(kFlagSpinnerAtPS01)) {
		Unobstacle_Object("Barrier Obstacle", true);
	}
	Unobstacle_Object("BOX38", true);
	Unobstacle_Object("TUBE81", true);
}

bool SceneScriptPS01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS01::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptPS01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1920.0f, 16581.0f, -2653.0f, 12, true, false, false)) {
			Game_Flag_Set(kflagPS01toPS02);
			Set_Enter(kSetPS02, kScenePS02);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1877.9f, 16592.0f, -2975.0f, 0, true, false, false)) {
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_At_XYZ(kActorMcCoy, 1872.0f, 16592.0f, -2975.0f, 870);
#else
			// reduce glitch with spinner door
			Actor_Set_At_XYZ(kActorMcCoy, 1872.0f, 16592.0f, -2994.0f, 870);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Game_Flag_Reset(kFlagMcCoyInChinaTown);
			Game_Flag_Reset(kFlagMcCoyInRunciters);
			Game_Flag_Reset(kFlagMcCoyInMcCoyApartment);
			Game_Flag_Reset(kFlagMcCoyInPoliceStation);
			Game_Flag_Reset(kFlagMcCoyInAnimoidRow);
			Game_Flag_Reset(kFlagMcCoyInTyrellBuilding);
			Game_Flag_Reset(kFlagMcCoyInDNARow);
			Game_Flag_Reset(kFlagMcCoyInBradburyBuilding);
			int spinnerDest = Spinner_Interface_Choose_Dest(kPS01LoopDoorAnim, true);
			switch (spinnerDest) {
			case kSpinnerDestinationMcCoysApartment:
				Game_Flag_Set(kFlagMcCoyInMcCoyApartment);
				Game_Flag_Reset(kFlagSpinnerAtPS01);
				Game_Flag_Set(kFlagSpinnerAtMA01);
				Set_Enter(kSetMA01, kSceneMA01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kPS01LoopOutshot, true);
				break;
			case kSpinnerDestinationRuncitersAnimals:
				Game_Flag_Set(kFlagMcCoyInRunciters);
				Game_Flag_Reset(kFlagSpinnerAtPS01);
				Game_Flag_Set(kFlagSpinnerAtRC01);
				Set_Enter(kSetRC01, kSceneRC01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kPS01LoopOutshot, true);
				break;
			case kSpinnerDestinationChinatown:
				Game_Flag_Set(kFlagMcCoyInChinaTown);
				Game_Flag_Reset(kFlagSpinnerAtPS01);
				Game_Flag_Set(kFlagSpinnerAtCT01);
				Set_Enter(kSetCT01_CT12, kSceneCT01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kPS01LoopOutshot, true);
				break;
			case kSpinnerDestinationAnimoidRow:
				Game_Flag_Set(kFlagMcCoyInAnimoidRow);
				Game_Flag_Reset(kFlagSpinnerAtPS01);
				Game_Flag_Set(kFlagSpinnerAtAR01);
				Set_Enter(kSetAR01_AR02, kSceneAR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kPS01LoopOutshot, true);
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(kFlagMcCoyInTyrellBuilding);
				Game_Flag_Reset(kFlagSpinnerAtPS01);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kPS01LoopOutshot, true);
				break;
			case kSpinnerDestinationDNARow:
				Game_Flag_Set(kFlagMcCoyInDNARow);
				Game_Flag_Reset(kFlagSpinnerAtPS01);
				Game_Flag_Set(kFlagSpinnerAtDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kPS01LoopOutshot, true);
				break;
			case kSpinnerDestinationBradburyBuilding:
				Game_Flag_Set(kFlagMcCoyInBradburyBuilding);
				Game_Flag_Reset(kFlagSpinnerAtPS01);
				Game_Flag_Set(kFlagSpinnerAtBB01);
				Set_Enter(kSetBB01, kSceneBB01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kPS01LoopOutshot, true);
				break;
			case kSpinnerDestinationNightclubRow:
				Game_Flag_Set(kFlagMcCoyInNightclubRow);
				Game_Flag_Reset(kFlagSpinnerAtPS01);
				Game_Flag_Set(kFlagSpinnerAtNR01);
				Set_Enter(kSetNR01, kSceneNR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kPS01LoopOutshot, true);
				break;
			case kSpinnerDestinationHysteriaHall:
				Game_Flag_Set(kFlagMcCoyInHysteriaHall);
				Game_Flag_Reset(kFlagSpinnerAtPS01);
				Game_Flag_Set(kFlagSpinnerAtHF01);
				Set_Enter(kSetHF01, kSceneHF01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kPS01LoopOutshot, true);
				break;
			default:
				Actor_Face_Heading(kActorMcCoy, 870, false);
				Game_Flag_Set(kFlagMcCoyInPoliceStation);
				Game_Flag_Set(kFlagArrivedFromSpinner2);
				Player_Loses_Control();
				Scene_Loop_Start_Special(kSceneLoopModeOnce, kPS01LoopDoorAnim, true);
				break;
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptPS01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS01::SceneFrameAdvanced(int frame) {
	if (frame == 71
	 || frame == 188
	) {
		Sound_Play(kSfxSPINOPN4, 100, 40, 0, 50);
	}

	if (frame == 108
	 || frame == 228
	) {
		Sound_Play(kSfxSPINCLS1, 100, 40, 0, 50);
	}

	if (frame == 1) {
		Sound_Play(kSfxCARDOWN3,  40, 0, 0, 50);
	}

	if (frame == 76) {
		Sound_Play(kSfxSERVOD1,   50, 0, 0, 50);
	}

	if (frame == 192) {
		Sound_Play(kSfxSERVOU1,   50, 0, 0, 50);
	}

	if (frame == 59) {
		Sound_Play(kSfxMTLHIT1,   15, 0, 0, 50);
	}

	if (frame == 275) {
		Sound_Play(kSfxCARUP3,    40, 0, 0, 50);
	}

	if (!Game_Flag_Query(kFlagArrivedFromSpinner2)) {
		switch (frame) {
		case 196:
			Actor_Face_Heading(kActorMcCoy, 870, false);
			Actor_Set_Frame_Rate_FPS(kActorMcCoy, -1);
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeSpinnerGetIn);
			break;

		case 220:
			Actor_Set_Frame_Rate_FPS(kActorMcCoy, 0);
			break;

		case 240:
			Actor_Set_Frame_Rate_FPS(kActorMcCoy, -2);
			break;
		}
		return; // true;
	}

	if (frame == 75) {
		Actor_Face_Heading(kActorMcCoy, 870, false);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeSpinnerGetOut);
		return; // true;
	}

	if (frame == 119) {
		Game_Flag_Reset(kFlagArrivedFromSpinner2);
		Player_Gains_Control();
		return; // true;
	}

	if (frame > 195) {
		if (frame == 239) {
			Game_Flag_Reset(kFlagArrivedFromSpinner2);
			Player_Gains_Control();
		}
		return; // true;
	}

	if (frame == 181) {
		Actor_Face_Heading(kActorMcCoy, 870, false);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeSpinnerGetOut);
	} else if (frame == 182) {
		Actor_Set_Frame_Rate_FPS(kActorMcCoy, 0);
	} else if (frame == 195) {
		Actor_Set_Frame_Rate_FPS(kActorMcCoy, -2);
	}

	return; // true;
}

void SceneScriptPS01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS01::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagPS02toPS01)) {
		Actor_Set_At_XYZ(kActorMcCoy, 1920.0f, 16581.0f, -2653.0f, 150);
		Game_Flag_Reset(kFlagPS02toPS01);
	}
	if (_vm->_cutContent) {
		if (!Game_Flag_Query(kFlagCT01Visited) && !Actor_Clue_Query(kActorMcCoy, kClueDispatchHitAndRun)) {
			Actor_Clue_Acquire(kActorMcCoy, kClueDispatchHitAndRun, false, kActorDispatcher);
			ADQ_Add(kActorDispatcher, 80, kAnimationModeTalk);
			if (Game_Flag_Query(kFlagRC01PoliceDone)) {
				ADQ_Add(kActorOfficerLeary, 340, kAnimationModeTalk);  // sector 3 - go ahead
			} else {
				ADQ_Add(kActorOfficerGrayford, 360, kAnimationModeTalk);  // sector 3 - go ahead
			}
			ADQ_Add(kActorDispatcher, 90, kAnimationModeTalk);
			ADQ_Add(kActorDispatcher, 100, kAnimationModeTalk);
			ADQ_Add(kActorDispatcher, 110, kAnimationModeTalk);
			if (Game_Flag_Query(kFlagRC01PoliceDone)) {
				ADQ_Add(kActorOfficerLeary, 350, kAnimationModeTalk);  // sector 3 - responding code 3
			} else {
				ADQ_Add(kActorOfficerGrayford, 370, kAnimationModeTalk);  // sector 3 - responding code 3
			}
			ADQ_Add_Pause(1000);
			ADQ_Add(kActorDispatcher, 120, kAnimationModeTalk);
			ADQ_Add(kActorDispatcher, 130, kAnimationModeTalk);
			ADQ_Add(kActorDispatcher, 140, kAnimationModeTalk);
			ADQ_Add(kActorDispatcher, 150, kAnimationModeTalk);
		}
	}
	//return false;
}

void SceneScriptPS01::PlayerWalkedOut() {
	Actor_Set_Invisible(kActorMcCoy, false);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);

	if (_vm->_cutContent) {
		ADQ_Flush(); // for dispatcher queue cleanup
	}

	if (!Game_Flag_Query(kflagPS01toPS02)) {
		if (Global_Variable_Query(kVariableChapter) == 1) {
			Outtake_Play(kOuttakeTowards3, true, -1);
		}
#if BLADERUNNER_ORIGINAL_BUGS
#else
		else {
			// Acts 2, 3 - should still use a spinner fly-through transition
			if (!Game_Flag_Query(kFlagMcCoyInTyrellBuilding)) {
				// don't play an extra outtake when going to Tyrell Building
				Outtake_Play(kOuttakeInside2,  true, -1); // available in Acts 1, 2, 3
				Outtake_Play(kOuttakeTowards3, true, -1); // available in Acts 1, 2, 3
			}
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
}

void SceneScriptPS01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
