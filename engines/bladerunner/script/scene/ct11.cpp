/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bladerunner/script/scene_script.h"

namespace BladeRunner {

enum kCT11Loops {
	kCT11LoopMainLoop  = 0,
	kCT11LoopMainNoCar = 2
};

void SceneScriptCT11::InitializeScene() {
	if (Game_Flag_Query(kFlagCT12toCT11)) {
		Setup_Scene_Information(-378.0f, 9.68f, -55.0f, 440);
	} else if (Game_Flag_Query(kFlagDR01toCT11)) {
		Setup_Scene_Information( 315.0f,  0.0f, 628.0f,   0);
	} else {
		Setup_Scene_Information( 152.0f, 9.68f,  -8.0f,   0);
	}

	Scene_Exit_Add_2D_Exit(0, 257, 240, 364, 330, 1);
	Scene_Exit_Add_2D_Exit(1,  97,   0, 155, 324, 0);
	Scene_Exit_Add_2D_Exit(2,   0,   0,  20, 479, 3);

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1, 50, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2A, 5, 80, 16,  25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN1A, 5, 80, 16,  25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR1, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR2, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR3, 5, 80, 50, 100, -100, 100, -101, -101, 0, 0);

	if (Global_Variable_Query(kVariableChapter) > 3) {
		Scene_Loop_Set_Default(kCT11LoopMainNoCar);
	} else {
		Scene_Loop_Set_Default(kCT11LoopMainLoop);
	}
}

void SceneScriptCT11::SceneLoaded() {
	Obstacle_Object("TRASH CAN", true);
	Unobstacle_Object("BOX NORTHWEST 1", true);
	Unobstacle_Object("BOX SOUTH 1", true);
	if (Global_Variable_Query(kVariableChapter) < 4) {
		if (!Game_Flag_Query(kFlagCT11DogWrapperTaken)) {
			Item_Add_To_World(kItemDogWrapper, kModelAnimationLichenDogWrapper, kSetCT11, 640.21f, 30.0f, 470.0f, 512, 12, 12, false, true, false, true);
			Scene_2D_Region_Add(0, 505, 316, 513, 321);
			Game_Flag_Set(kFlagCT11DogWrapperAvailable);
		}

		if (_vm->_cutContent
		    && !Actor_Clue_Query(kActorMcCoy, kClueGrigoriansNote)
		    && (!Game_Flag_Query(kFlagDektoraIsReplicant)
		        && !Game_Flag_Query(kFlagGordoIsReplicant))
		) {
			// The car is only bought by Reps from CrazyLegs
			// if Dektora is a Replicant
			// or if Dektora  is human and Gordo is also human (so Clovis bought it from Crazylegs)
			// We place the note only for the second case here.
			// For the first case, a CrazyLegs advertisement (plus note) is placed in Dektora's room (nr07, kClueCrazysInvolvement)
			Item_Add_To_World(kItemNote, kModelAnimationGrigoriansNote, kSetCT11, 641.21f, 26.0f, 472.0f, 304, 12, 12, false, true, false, true);
			Scene_2D_Region_Add(2, 505, 321, 519, 332);
			Game_Flag_Set(kFlagCT11GrigorianNotePlaced);
		}

		if (!Actor_Clue_Query(kActorMcCoy, kClueCar)) {
#if BLADERUNNER_ORIGINAL_BUGS
			Scene_2D_Region_Add(1, 412, 258, 552, 358);
#else
			// expand region 1 a bit and add two more
			// as auxilliary in order to better cover the car area
			Scene_2D_Region_Add(1, 365, 258, 552, 358);
			Scene_2D_Region_Add(3, 267, 330, 365, 377);
			Scene_2D_Region_Add(4, 365, 358, 454, 377);
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
	} else {
		if (Game_Flag_Query(kFlagCT11DogWrapperAvailable)) {
			Item_Remove_From_World(kItemDogWrapper);
			Game_Flag_Reset(kFlagCT11DogWrapperAvailable);
			Game_Flag_Set(kFlagCT11DogWrapperTaken);
		}
		// Check if Grigorian's note is still in the world after Act 3
		// and remove it (in cut content)
		if (_vm->_cutContent
		    && Game_Flag_Query(kFlagCT11GrigorianNotePlaced)
		    && !Actor_Clue_Query(kActorMcCoy, kClueGrigoriansNote)) {
			Item_Remove_From_World(kItemNote);
			Game_Flag_Reset(kFlagCT11GrigorianNotePlaced);
		}

		Unobstacle_Object("BRIDGE SUPPORT", true);
		Unobstacle_Object("BODY", true);
		Unobstacle_Object("HEADLIGHTS", true);
		Unobstacle_Object("LICENSE PLATE-FRONT", true);
		Unobstacle_Object("LICENSE PLATE-REAR", true);
		Unobstacle_Object("BRAKE DISC RF", true);
		Unobstacle_Object("TIRE RF", true);
		Unobstacle_Object("RIM RF", true);
		Unobstacle_Object("DOOR RIGHT", true);
		Unobstacle_Object("BUMPER REAR", true);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// this street sign blocks police officers from moving to waypoint 386 after they reach waypoint 242
		Unobstacle_Object("STREET SIGN", true);
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	Unclickable_Object("TRASH CAN");
}

bool SceneScriptCT11::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptCT11::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptCT11::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptCT11::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptCT11::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 121.0f, 9.68f, -42.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagCT11toCT09);
			Set_Enter(kSetCT09, kSceneCT09);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -300.0f, 9.68f, 66.0f, 0, true, false, false)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -400.0f, 9.68f, -70.0f, 0, true, false, false);
			Game_Flag_Set(kFlagCT11toCT12);
			Set_Enter(kSetCT01_CT12, kSceneCT12);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 290.0f, 0.0f, 635.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagCT11toDR01);
			Game_Flag_Reset(kFlagMcCoyInChinaTown);
			Game_Flag_Set(kFlagMcCoyInDNARow);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
		}
		return true;
	}
	return false;
}

bool SceneScriptCT11::ClickedOn2DRegion(int region) {
	if (region == 0 && Game_Flag_Query(kFlagCT11DogWrapperAvailable)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 686.0f, 0.0f, 658.0f, 12, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 47, false);
			Item_Remove_From_World(kItemDogWrapper);
			Actor_Clue_Acquire(kActorMcCoy, kClueLichenDogWrapper, false, -1);
			Item_Pickup_Spin_Effect(kModelAnimationLichenDogWrapper, 510, 319);
			Game_Flag_Reset(kFlagCT11DogWrapperAvailable);
			Game_Flag_Set(kFlagCT11DogWrapperTaken);
			Actor_Voice_Over(550, kActorVoiceOver);
			Actor_Voice_Over(560, kActorVoiceOver);
			Actor_Voice_Over(570, kActorVoiceOver);
			Actor_Voice_Over(580, kActorVoiceOver);
#if !BLADERUNNER_ORIGINAL_BUGS
			// in the original game the hotspot would not be removed
			// after picking up the lichendog Wrapper
			Scene_2D_Region_Remove(0);
#endif // !BLADERUNNER_ORIGINAL_BUGS
		}
		return true;
	}

	if (_vm->_cutContent
	    && region == 2
	    && !Actor_Clue_Query(kActorMcCoy, kClueGrigoriansNote)
	) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 686.0f, 0.0f, 658.0f, 12, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 47, false);
			Item_Remove_From_World(kItemNote);
			Game_Flag_Reset(kFlagCT11GrigorianNotePlaced);
			Actor_Clue_Acquire(kActorMcCoy, kClueGrigoriansNote, false, -1);
			Item_Pickup_Spin_Effect(kModelAnimationGrigoriansNote, 512, 326);
			Actor_Voice_Over(8840, kActorMcCoy);
			Scene_2D_Region_Remove(2);
		}
		return true;
	}

	if (region == 1
#if !BLADERUNNER_ORIGINAL_BUGS
	    || region == 3
	    || region == 4
#endif // !BLADERUNNER_ORIGINAL_BUGS
	) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 686.0f, 0.0f, 658.0f, 12, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 47, false);
			int cluesFound = 0;
			if (Actor_Clue_Query(kActorMcCoy, kClueCrowdInterviewB)) {
				++cluesFound;
			}
			if (Actor_Clue_Query(kActorMcCoy, kCluePaintTransfer)) {
				++cluesFound;
			}
			if (Actor_Clue_Query(kActorMcCoy, kClueLabPaintTransfer)) {
				++cluesFound;
			}
			if (Actor_Clue_Query(kActorMcCoy, kClueLicensePlate)) {
				cluesFound += 2;
			}
			if (Actor_Clue_Query(kActorMcCoy, kClueCarColorAndMake)) {
				cluesFound += 2;
			}
			if (Actor_Clue_Query(kActorMcCoy, kCluePartialLicenseNumber)) {
				cluesFound += 2;
			}
			if ( cluesFound > 2
			 && !Actor_Clue_Query(kActorMcCoy, kClueCar)
			) {
				Actor_Voice_Over(510, kActorVoiceOver);
				Actor_Voice_Over(520, kActorVoiceOver);
				Actor_Voice_Over(530, kActorVoiceOver);
				Actor_Voice_Over(540, kActorVoiceOver);
				Actor_Clue_Acquire(kActorMcCoy, kClueCar, false, -1);
				Scene_2D_Region_Remove(1);
#if !BLADERUNNER_ORIGINAL_BUGS
				Scene_2D_Region_Remove(3);
				Scene_2D_Region_Remove(4);
#endif // !BLADERUNNER_ORIGINAL_BUGS

			} else {
				Actor_Says(kActorMcCoy, 8525, 12);
			}
		}
		return true;
	}
	return false;
}

void SceneScriptCT11::SceneFrameAdvanced(int frame) {
}

void SceneScriptCT11::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptCT11::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagCT12toCT11)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -358.0f, 9.68f, 32.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagCT12toCT11);
	} else if (Game_Flag_Query(kFlagDR01toCT11)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 329.0f, 0.0f, 617.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagDR01toCT11);
	} else {
		Player_Loses_Control();
		Actor_Set_Immunity_To_Obstacles(kActorMcCoy, true);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 125.0f, 9.68f, 74.0f, 0, false, false, false);
		Actor_Set_Immunity_To_Obstacles(kActorMcCoy, false);
		Player_Gains_Control();
		Game_Flag_Reset(kFlagCT09toCT11);
	}
}

void SceneScriptCT11::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1u);
}

void SceneScriptCT11::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
