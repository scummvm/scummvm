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

enum kBB01Loops {
	kBB01LoopInshot            = 0, //   0 - 300
	kBB01LoopMainLoop          = 1, // 301 - 361
	kBB01LoopDoorAnim          = 3, // 362 - 421
	kBB01LoopOutshot           = 4, // 422 - 480
	kBB01LoopMainLoopNoSpinner = 5  // 481 - 540
};

void SceneScriptBB01::InitializeScene() {
	if (Game_Flag_Query(kFlagDR02toBB01)) {
		Setup_Scene_Information(-253.0f, 9.0f,  715.0f, 266);
	} else if (Game_Flag_Query(kFlagBB02toBB01)) {
		Setup_Scene_Information(-128.0f, 9.0f,  342.0f, 266);
	} else {
		Setup_Scene_Information(  43.0f, 0.0f, 1058.0f,   0);
	}

#if BLADERUNNER_ORIGINAL_BUGS
	Scene_Exit_Add_2D_Exit(0,   0,   0,  72, 299, 3);
	Scene_Exit_Add_2D_Exit(1, 151, 218, 322, 290, 3);
	if (Game_Flag_Query(kFlagSpinnerAtBB01)) {
		Scene_Exit_Add_2D_Exit(2, 0, 311, 312, 479, 2);
	}
#else
	// expand the left exit downwards as much as possible, especially if Spinner is missing
	Scene_Exit_Add_2D_Exit(1, 151, 218, 322, 290, 3);
	if (Game_Flag_Query(kFlagSpinnerAtBB01)) {
		Scene_Exit_Add_2D_Exit(0, 0,   0,  72, 311, 3);
		Scene_Exit_Add_2D_Exit(2, 0, 311, 312, 479, 2);
	} else {
		Scene_Exit_Add_2D_Exit(0, 0,   0,  72, 400, 3);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1,  50,    0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRUNOFF, 25, -100, 0);
	Ambient_Sounds_Add_Sound(kSfxRCCARBY1, 5,  60, 40, 60, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxRCCARBY2, 5,  60, 40, 65, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxRCCARBY3, 5,  60, 40, 60, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN2A,   5,  80, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN1A,   5,  80, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR1,   5, 120, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR2,   5, 120, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR3,   5, 120, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);

	if ( Game_Flag_Query(kFlagSpinnerAtBB01)
	 && !Game_Flag_Query(kFlagDR02toBB01)
	 && !Game_Flag_Query(kFlagBB02toBB01)
	) {
		if (!Game_Flag_Query(kFlagBB01VisitedFirstTimeWithSpinner)
		    || Random_Query(1, 3) == 1
		) {
			// enhancement: don't always play after first visit
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kBB01LoopInshot, false);
		}
		Scene_Loop_Set_Default(kBB01LoopMainLoop);
	} else if (Game_Flag_Query(kFlagSpinnerAtBB01)
	        && Game_Flag_Query(kFlagDR02toBB01)
	) {
		Scene_Loop_Set_Default(kBB01LoopMainLoop);
	} else if (Game_Flag_Query(kFlagSpinnerAtBB01)
	        && Game_Flag_Query(kFlagBB02toBB01)
	) {
		Scene_Loop_Set_Default(kBB01LoopMainLoop);
	} else {
		Scene_Loop_Set_Default(kBB01LoopMainLoopNoSpinner);
	}
}

void SceneScriptBB01::SceneLoaded() {
	Obstacle_Object("COLUME", true);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Unclickable_Object("OBJECT03");
	Unclickable_Object("OBJECT04");
	Unclickable_Object("OBJECT05");
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptBB01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB01::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -140.0f, 9.0f, 818.0f, 0, true, false, false);
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -233.0f, 9.0f, 846.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagBB01toDR02);
			Game_Flag_Reset(kFlagMcCoyInBradburyBuilding);
			Game_Flag_Set(kFlagMcCoyInDNARow);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR02);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -125.39f, 9.0f, 372.45f, 0, true, false, false)) {
			Game_Flag_Set(kFlagBB01toBB02);
			Set_Enter(kSetBB02_BB04_BB06_BB51, kSceneBB02);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 43.0f, 0.0f, 1062.0f, 0, true, false, false)) {
			Game_Flag_Reset(kFlagMcCoyInChinaTown);
			Game_Flag_Reset(kFlagMcCoyInRunciters);
			Game_Flag_Reset(kFlagMcCoyInMcCoyApartment);
			Game_Flag_Reset(kFlagMcCoyInPoliceStation);
			Game_Flag_Reset(kFlagMcCoyInBradburyBuilding);
			int spinnerDest = Spinner_Interface_Choose_Dest(kBB01LoopDoorAnim, false);
			switch (spinnerDest) {
			case kSpinnerDestinationPoliceStation:
				Game_Flag_Set(kFlagMcCoyInPoliceStation);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtPS01);
				Set_Enter(kSetPS01, kScenePS01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kBB01LoopOutshot, true);
				break;
			case kSpinnerDestinationMcCoysApartment:
				Game_Flag_Set(kFlagMcCoyInMcCoyApartment);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtMA01);
				Set_Enter(kSetMA01, kSceneMA01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kBB01LoopOutshot, true);
				break;
			case kSpinnerDestinationRuncitersAnimals:
				Game_Flag_Set(kFlagMcCoyInRunciters);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtRC01);
				Set_Enter(kSetRC01, kSceneRC01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kBB01LoopOutshot, true);
				break;
			case kSpinnerDestinationChinatown:
				Game_Flag_Set(kFlagMcCoyInChinaTown);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtCT01);
				Set_Enter(kSetCT01_CT12, kSceneCT01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kBB01LoopOutshot, true);
				break;
			case kSpinnerDestinationAnimoidRow:
				Game_Flag_Set(kFlagMcCoyInAnimoidRow);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtAR01);
				Set_Enter(kSetAR01_AR02, kSceneAR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kBB01LoopOutshot, true);
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(kFlagMcCoyInTyrellBuilding);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kBB01LoopOutshot, true);
				break;
			case kSpinnerDestinationDNARow:
				Game_Flag_Set(kFlagMcCoyInDNARow);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kBB01LoopOutshot, true);
				break;
			case kSpinnerDestinationNightclubRow:
				Game_Flag_Set(kFlagMcCoyInNightclubRow);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtNR01);
				Set_Enter(kSetNR01, kSceneNR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kBB01LoopOutshot, true);
				break;
			case kSpinnerDestinationHysteriaHall:
				Game_Flag_Set(kFlagMcCoyInHysteriaHall);
				Game_Flag_Reset(kFlagSpinnerAtBB01);
				Game_Flag_Set(kFlagSpinnerAtHF01);
				Set_Enter(kSetHF01, kSceneHF01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kBB01LoopOutshot, true);
				break;
			default:
				Game_Flag_Set(kFlagMcCoyInBradburyBuilding);
				Scene_Loop_Start_Special(kSceneLoopModeOnce, kBB01LoopDoorAnim, true);
				break;
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptBB01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB01::SceneFrameAdvanced(int frame) {
	if (frame == 193) {
		Sound_Play(kSfxCARDOWN3, 40, 0, 0, 50);
	}

	if (frame == 241
	 || frame == 363
	) {
		Sound_Play(kSfxSPINOPN4, 100, -50, -50, 50);
	}

	if (frame == 286
	 || frame == 407
	) {
		Sound_Play(kSfxSPINCLS1, 100, -50, -50, 50);
	}

	if (frame == 433) {
		Sound_Play(kSfxCARUP3,    40, -50, 80, 50);
	}

#if BLADERUNNER_ORIGINAL_BUGS
#else
	// This is treated as a bug rather than restored content
	// This InShot loop otherwise seems silent for the first ~120 frames
	if (frame == 19) {
		if (Random_Query(0, 1)) {
			Sound_Play(kSfxSPIN3A,     90, -50, 100, 50);
		} else {
			Sound_Play(kSfxSPIN2A,     90, -50, 100, 50);
		}
	}

	if (frame == 60 && Random_Query(0, 1)) {
		Sound_Play(kSfxSIREN2, 50, 20, 80, 50);
	}

	if (frame == 90 && Random_Query(0, 1)) {
		Sound_Play(kSfxRCCARBY3, 40, 80, 100, 50);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

#if BLADERUNNER_ORIGINAL_BUGS
	if (frame == 120) {
		Sound_Play(kSfxTRUCKBY1, Random_Query(33, 33), 100, -100, 50);
	}
#else
	// delay truck passing by sfx to better match the animation in the loop
	if (frame == 124) {
		Sound_Play(kSfxTRUCKBY1, Random_Query(40, 73), 100, -100, 50);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptBB01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB01::PlayerWalkedIn() {
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationBradburyBuilding, true);
	if (Game_Flag_Query(kFlagDR02toBB01)) {
		Game_Flag_Reset(kFlagDR02toBB01);
	} else if (Game_Flag_Query(kFlagBB02toBB01)) {
		Game_Flag_Reset(kFlagBB02toBB01);
	} else {
		if (!Game_Flag_Query(kFlagBB01VisitedFirstTimeWithSpinner) && Game_Flag_Query(kFlagSpinnerAtBB01)) {
			Game_Flag_Set(kFlagBB01VisitedFirstTimeWithSpinner);
		}
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 43.0f, 0.0f, 954.0f, 0, false, false, false);
	}
}

void SceneScriptBB01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (!Game_Flag_Query(kFlagBB01toBB02)
	    && !Game_Flag_Query(kFlagBB01toDR02)) {
		// Acts 2, 3 - should still use a spinner fly-through transition
		if (!Game_Flag_Query(kFlagMcCoyInTyrellBuilding)) {
			Outtake_Play(kOuttakeTowards3, true, -1); // available in Acts 1, 2, 3
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptBB01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
