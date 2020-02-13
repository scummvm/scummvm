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

enum kDR01Loops {
	kDR01LoopBikerInshot          = 0, //   0 -  74
	kDR01LoopPanFromDR02          = 1, //  75 -  88
	kDR01LoopPanFromDR04Pre       = 2, //  89 - 116
	kDR01LoopPanFromDR04Post      = 3, // 117 - 144
	kDR01LoopMainLoop             = 4  // 145 - 205
};

void SceneScriptDR01::InitializeScene() {
	if (Game_Flag_Query(kFlagDR02toDR01)) {
		Setup_Scene_Information(  -835.0f, -0.04f, -118.0f, 664);
	} else if (Game_Flag_Query(kFlagDR04toDR01)) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// Part of the barrel flame glitch bug fix:
		// Disable rogue barrel flame effect during the pan from DR04 to DR01
		Screen_Effect_Skip(0, false);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Setup_Scene_Information(  -711.0f, -0.04f,   70.0f, 307);
	} else if (Game_Flag_Query(kFlagCT11toDR01)) {
		Setup_Scene_Information(-1765.28f, -0.04f, -23.82f, 269);
	} else {
		Setup_Scene_Information(  -386.0f, -0.04f,  -82.0f, 792);
	}

	Scene_Exit_Add_2D_Exit(0, 240, 60, 450, 250, 0);
	Scene_Exit_Add_2D_Exit(1,   0,   0, 30, 479, 3);
	if (Game_Flag_Query(kFlagSpinnerAtDR01)
	 && Global_Variable_Query(kVariableChapter) < 4
	) {
		Scene_Exit_Add_2D_Exit(2, 610, 0, 639, 479, 1);
	}
	if (Global_Variable_Query(kVariableChapter) >= 3) {
		Scene_Exit_Add_2D_Exit(3, 0, 45, 142, 201, 0);
	}

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1, 50,  0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxHUMMER3, 12, 85, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxBIGFAN2, 14, 85, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2A, 5, 80, 16,  25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN1A, 5, 80, 16,  25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR1, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR2, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR3, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagDR05BombExploded)
	 && Game_Flag_Query(kFlagDR04toDR01)
	) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kDR01LoopPanFromDR04Post, false);
		Scene_Loop_Set_Default(kDR01LoopMainLoop);
	} else if (!Game_Flag_Query(kFlagDR05BombExploded)
	        &&  Game_Flag_Query(kFlagDR04toDR01)
	) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kDR01LoopPanFromDR04Pre, false);
		Scene_Loop_Set_Default(kDR01LoopMainLoop);
	} else if (Game_Flag_Query(kFlagDR02toDR01)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kDR01LoopPanFromDR02, false);
		Scene_Loop_Set_Default(kDR01LoopMainLoop);
	} else if (Game_Flag_Query(kFlagCT11toDR01)) {
		Scene_Loop_Set_Default(kDR01LoopMainLoop);
	} else {
		if ((!Game_Flag_Query(kFlagDR01Visited) && Global_Variable_Query(kVariableChapter) == 2)
		     || Random_Query(1, 3) == 1)
		{
			// enhancement: don't always play after first visit
			// But first visit in 2nd chapter should always show it.
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kDR01LoopBikerInshot, false);
		}
		Scene_Loop_Set_Default(kDR01LoopMainLoop);
	}
}

void SceneScriptDR01::SceneLoaded() {
	Obstacle_Object("TRASH CAN WITH FIRE", true);
	Obstacle_Object("V2PYLON02", true);
	Obstacle_Object("V2PYLON04", true);
	Obstacle_Object("U2 CHEWDOOR", true);
	Obstacle_Object("MMTRASHCAN", true);
	Obstacle_Object("PARKMETR02", true);
	Obstacle_Object("TRANSFORMER 01", true);
	Obstacle_Object("TRANSFORMER 02", true);
	Obstacle_Object("PARKMETR01", true);
	Obstacle_Object("Z2TRSHCAN", true);
	Obstacle_Object("Z2ENTRYDR", true);
	Obstacle_Object("Z2DR2", true);
	Unobstacle_Object("V2 BUILD01", true);
}

bool SceneScriptDR01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptDR01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptDR01::ClickedOnActor(int actorId) {
	return actorId == kActorShoeshineMan;
}

bool SceneScriptDR01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptDR01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (_vm->_cutContent) {
			float x, y, z;
			Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
			bool exitFlag = true;
			bool fromFarTop = false;
			if (x < -1088) {
				fromFarTop = true;
				exitFlag = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1149.80f, 0.56f, -94.45f, 0, true, false, false);
			} else if (-1088 < x && x < -642) {
				exitFlag = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1271.89f, 6.71f, -268.63f, 0, true, false, false);
			} else {
				exitFlag = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -835.0f, -0.04f, -118.0f, 0, true, false, false);
			}
			if (!exitFlag) {
				if (fromFarTop) {
					Async_Actor_Walk_To_XYZ(kActorMcCoy,  -1066.51f, 0.51f, -110.60f, 0, false);
				} else {
					Async_Actor_Walk_To_XYZ(kActorMcCoy, -911.0f, -0.04f, -118.0f, 0, false);
				}
				Ambient_Sounds_Adjust_Looping_Sound(kSfxFACTAMB2, 10, -100, 1);
				Game_Flag_Set(kFlagDR01toDR02);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR02);
			}
		} else {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -835.0f, -0.04f, -118.0f, 0, true, false, false)) {
				Async_Actor_Walk_To_XYZ(kActorMcCoy, -911.0f, -0.04f, -118.0f, 0, false);
				Ambient_Sounds_Adjust_Looping_Sound(kSfxFACTAMB2, 10, -100, 1);
				Game_Flag_Set(kFlagDR01toDR02);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR02);
			}
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -711.0f, -0.04f, 70.0f, 0, true, false, false)) {
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -796.0f, -0.04f, 166.0f, 0, false);
			Game_Flag_Set(kFlagDR01toDR04);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR04);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -372.0f, -0.04f, -82.0f, 0, true, false, false)) {
			Game_Flag_Reset(kFlagMcCoyInChinaTown);
			Game_Flag_Reset(kFlagMcCoyInRunciters);
			Game_Flag_Reset(kFlagMcCoyInMcCoyApartment);
			Game_Flag_Reset(kFlagMcCoyInPoliceStation);
			Game_Flag_Reset(kFlagMcCoyInAnimoidRow);
			Game_Flag_Reset(kFlagMcCoyInTyrellBuilding);
			Game_Flag_Reset(kFlagMcCoyInDNARow);
			Game_Flag_Reset(kFlagMcCoyInBradburyBuilding);
			int spinnerDest = Spinner_Interface_Choose_Dest(-1, false);
			switch (spinnerDest) {
			case kSpinnerDestinationPoliceStation:
				Game_Flag_Set(kFlagMcCoyInPoliceStation);
				Game_Flag_Reset(kFlagSpinnerAtDR01);
				Game_Flag_Set(kFlagSpinnerAtPS01);
				Set_Enter(kSetPS01, kScenePS01);
				break;
			case kSpinnerDestinationMcCoysApartment:
				Game_Flag_Set(kFlagMcCoyInMcCoyApartment);
				Game_Flag_Reset(kFlagSpinnerAtDR01);
				Game_Flag_Set(kFlagSpinnerAtMA01);
				Set_Enter(kSetMA01, kSceneMA01);
				break;
			case kSpinnerDestinationRuncitersAnimals:
				Game_Flag_Set(kFlagMcCoyInRunciters);
				Game_Flag_Reset(kFlagSpinnerAtDR01);
				Game_Flag_Set(kFlagSpinnerAtRC01);
				Set_Enter(kSetRC01, kSceneRC01);
				break;
			case kSpinnerDestinationChinatown:
				Game_Flag_Set(kFlagMcCoyInChinaTown);
				Game_Flag_Reset(kFlagSpinnerAtDR01);
				Game_Flag_Set(kFlagSpinnerAtCT01);
				Set_Enter(kSetCT01_CT12, kSceneCT01);
				break;
			case kSpinnerDestinationAnimoidRow:
				Game_Flag_Set(kFlagMcCoyInAnimoidRow);
				Game_Flag_Reset(kFlagSpinnerAtDR01);
				Game_Flag_Set(kFlagSpinnerAtAR01);
				Set_Enter(kSetAR01_AR02, kSceneAR01);
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(kFlagMcCoyInTyrellBuilding);
				Game_Flag_Reset(kFlagSpinnerAtDR01);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				break;
			case kSpinnerDestinationBradburyBuilding:
				Game_Flag_Set(kFlagMcCoyInBradburyBuilding);
				Game_Flag_Reset(kFlagSpinnerAtDR01);
				Game_Flag_Set(kFlagSpinnerAtBB01);
				Set_Enter(kSetBB01, kSceneBB01);
				break;
			case kSpinnerDestinationNightclubRow:
				Game_Flag_Set(kFlagMcCoyInNightclubRow);
				Game_Flag_Reset(kFlagSpinnerAtDR01);
				Game_Flag_Set(kFlagSpinnerAtNR01);
				Set_Enter(kSetNR01, kSceneNR01);
				break;
			case kSpinnerDestinationHysteriaHall:
				Game_Flag_Set(kFlagMcCoyInHysteriaHall);
				Game_Flag_Reset(kFlagSpinnerAtDR01);
				Game_Flag_Set(kFlagSpinnerAtHF01);
				Set_Enter(kSetHF01, kSceneHF01);
				break;
			default:
				Player_Loses_Control();
				Game_Flag_Set(kFlagMcCoyInDNARow);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -447.39f, 0.16f, -92.38f, 0, false, true, false);
				Player_Gains_Control();
				break;
			}
		}
		return true;
	}

	if (exitId == 3) {
		float x, y, z;
		Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
		bool v7 = false;
		if (-1200 < x) {
			v7 = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1236.4f, -0.04f, -13.91f, 0, true, false, false);
		}
		if (!v7) {
			Game_Flag_Set(kFlagDR01toCT11);
			Game_Flag_Set(kFlagMcCoyInChinaTown);
			Game_Flag_Reset(kFlagMcCoyInDNARow);
			Set_Enter(kSetCT11, kSceneCT11);
		}
		return true;
	}
	return false;
}

bool SceneScriptDR01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptDR01::SceneFrameAdvanced(int frame) {
	if (frame < 75) {
		// TODO This keeps setting McCoy as invisible while in this frame range
		//      However the performance impact is negligible from this call
		Actor_Set_Invisible(kActorMcCoy, true);
	} else {
		// TODO This keeps setting McCoy as visible while in this frame range
		//      However the performance impact is negligible from this call
		Actor_Set_Invisible(kActorMcCoy, false);
	}
	if (frame == 2) {
		Ambient_Sounds_Play_Sound(kSfxBIKEMIX4, 40, -40, 100, 99);
	}
#if BLADERUNNER_ORIGINAL_BUGS
#else
	// Part of the barrel flame glitch bug fix:
	// Disable rogue barrel flame effect during the pan from DR04 to DR01
	// loops: kDR01LoopPanFromDR04Pre, kDR01LoopPanFromDR04Post
	if (frame == 89 || frame == 117 ){
		Screen_Effect_Skip(0, false);
	}
	// And restore the flame effect at the end of the loops
	if (frame == 116 || frame == 144) {
		Screen_Effect_Restore_All(false);
	}
#endif
}

void SceneScriptDR01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptDR01::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagCT11toDR01)) {
		Async_Actor_Walk_To_XYZ(kActorMcCoy, -757.15f, -0.04f, 24.64f, 0, false);
	} else if (!Game_Flag_Query(kFlagDR02toDR01)
	        && !Game_Flag_Query(kFlagDR04toDR01)
	) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -447.39f, 0.16f, -92.38f, 0, false, false, false);
		if (!Game_Flag_Query(kFlagDR01Visited)) {
			Game_Flag_Set(kFlagDR01Visited);
			// Make use of the kFlagDirectorsCut like in CT01 case
			// extra flags and chapter check are for compatibility / sane behavior
			// in imported original save games (or "exported" save games for the original)
			if (_vm->_cutContent) {
				if (
					Global_Variable_Query(kVariableChapter) == 2
					&& !Game_Flag_Query(kFlagDR03ChewTalk1)
					&& !Game_Flag_Query(kFlagDR05MorajiTalk)
					&& !Game_Flag_Query(kFlagDirectorsCut)) {
					Actor_Voice_Over(600, kActorVoiceOver);
					Actor_Voice_Over(610, kActorVoiceOver);
					Actor_Voice_Over(620, kActorVoiceOver);
					Actor_Voice_Over(630, kActorVoiceOver);
					Actor_Voice_Over(640, kActorVoiceOver);
					Actor_Voice_Over(650, kActorVoiceOver);
				}
			}
		}
		Player_Gains_Control();
	}
	Game_Flag_Reset(kFlagDR02toDR01);
	Game_Flag_Reset(kFlagDR04toDR01);
	Game_Flag_Reset(kFlagCT11toDR01);
}

void SceneScriptDR01::PlayerWalkedOut() {
	if (!Game_Flag_Query(kFlagDR01toDR04)
	 && !Game_Flag_Query(kFlagDR01toDR02)
	 && !Game_Flag_Query(kFlagDR01toCT11)
	) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeInside2, true, -1);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// add another transition to make it less abrupt
		if (!Game_Flag_Query(kFlagMcCoyInTyrellBuilding)) {
			// but don't play this extra outtake when going to Tyrell Building
			Outtake_Play(kOuttakeAway1,   true, -1);
		}

		// Part of the barrel flame glitch bug fix:
		// Add this effect restoration -- as a catch all case
		Screen_Effect_Restore_All(false);
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
}

void SceneScriptDR01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
