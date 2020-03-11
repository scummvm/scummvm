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

enum kUG18Loops {
	kUG18LoopTrainsArriving            = 0, //   0 -  59
	kUG18LoopMainLoop                  = 1, //  60 - 120 (4 seconds in 15 fps)
	kUG18LoopTrainsLeaving             = 3, // 121 - 180
	kUG18LoopMainLoopNoTrains          = 4  // 181 - 241 (4 seconds in 15 fps)
};

static const int kUG18TrainsSecondsOfLoopNoTrains    =    4;
static const int kUG18TrainsSecondsOfLoopWithTrains  =    4;
static const int kUG18TrainsCountUpMaxMarginWalkedIn =   12;
static const int kUG18TrainsCountUpMinMarginWalkedIn =    4;
static const int kUG18TrainsCountUpTargetRegular     =  600;
static const int kUG18TrainsCountUpMinMarginRegular  =   65;

// values for re-purposed global variable kVariableUG18StateOfTrains
enum kUG18TrainsState {
	kUG18NoTrains                      = 0,
	kUG18TrainsUnloading               = 1
};

// values for re-purposed global variable kVariableUG18StateOfGuzzaCorpse
enum kUG18StateOfGuzzaCorpse {
	kUG18GuzzaNoCorpse           = 0,
	kUG18GuzzaCorpseFloatsDown   = 1,
	kUG18GuzzaCorpseStuckInPipes = 2,
	kUG18GuzzaCorpseDissolves    = 3
};

void SceneScriptUG18::InitializeScene() {
	Setup_Scene_Information(-684.71f, 0.0f, 171.59f, 0);
	Game_Flag_Reset(kFlagUG13toUG18);

	Scene_Exit_Add_2D_Exit(0, 0, 158, 100, 340, 3);
	if (_vm->_cutContent) {
		if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
			Overlay_Play("UG18OVER", 0, true, false, 0); // Railing in center platform is intact
		} else {
			Overlay_Play("UG18OVER", 2, true, false, 0); // Railing has broken outwards (state after Guzza fell)
		}
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRUNOFF, 71, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxBOILPOT2, 45, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   76, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP1,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP2,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP3,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP4,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M3,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M1,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M1, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M2, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M3, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP5,  2,  20, 25, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAM1,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAM3,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAM6A,  5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxZUBWLK1,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxZUBWLK2,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxZUBWLK3,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxZUBWLK4,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);

	if (_vm->_cutContent) {
		if (Global_Variable_Query(kVariableUG18StateOfTrains) == kUG18NoTrains) {
			Scene_Loop_Set_Default(kUG18LoopMainLoopNoTrains);
			Global_Variable_Set(kVariableUG18CountUpForNextTrainAction, Random_Query(kUG18TrainsCountUpTargetRegular - kUG18TrainsCountUpMaxMarginWalkedIn, kUG18TrainsCountUpTargetRegular - kUG18TrainsCountUpMinMarginWalkedIn));
		} else {
			if (Random_Query(0, 1)) {
				Scene_Loop_Set_Default(kUG18LoopMainLoop);
				// don't set the kVariableUG18CountUpForNextTrainAction here, we only want this to play once before transitioning to trains leaving
			} else {
				Global_Variable_Set(kVariableUG18StateOfTrains, kUG18NoTrains);
				Scene_Loop_Set_Default(kUG18LoopMainLoopNoTrains);
				Global_Variable_Set(kVariableUG18CountUpForNextTrainAction, Random_Query(0, kUG18TrainsCountUpTargetRegular - kUG18TrainsCountUpMinMarginRegular));
			}
		}
	} else {
		Scene_Loop_Set_Default(kUG18LoopMainLoopNoTrains);
	}

	if ( Game_Flag_Query(kFlagCallWithGuzza)
	 && !Game_Flag_Query(kFlagUG18GuzzaScene)
	 &&  Global_Variable_Query(kVariableChapter) == 4
	) {
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaUG18Wait);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisUG18Wait);
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikUG18Wait);
	}
}

void SceneScriptUG18::SceneLoaded() {
	Obstacle_Object("MACHINE_01", true);
#if BLADERUNNER_ORIGINAL_BUGS
	Unobstacle_Object("PLATFM_RAIL 01", true);
	Unobstacle_Object("PLATFM_RAIL 02", true);
	Unobstacle_Object("OBSTACLE1", true);
#endif // BLADERUNNER_ORIGINAL_BUGS
	Clickable_Object("MACHINE_01");
	Unclickable_Object("MACHINE_01");

	if (Game_Flag_Query(kFlagUG18GuzzaScene)) {
		Actor_Put_In_Set(kActorGuzza, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorGuzza, 41, 0);
		if (Actor_Query_Which_Set_In(kActorSadik) == kSetUG18) {
			Actor_Put_In_Set(kActorSadik, kSetFreeSlotA);
			Actor_Set_At_Waypoint(kActorSadik, 33, 0);
		}
	}

	if ( Game_Flag_Query(kFlagCallWithGuzza)
	 && !Game_Flag_Query(kFlagUG18GuzzaScene)
	 &&  Global_Variable_Query(kVariableChapter) == 4
	) {
		Item_Add_To_World(kItemBriefcase, kModelAnimationBriefcase, kSetUG18, -55.21f, 0.0f, -302.17f, 0, 12, 12, false, true, false, true);
	}
}

bool SceneScriptUG18::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG18::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG18::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG18::ClickedOnItem(int itemId, bool combatMode) {
	if (itemId == kItemBriefcase) {
		if (combatMode) {
			Item_Remove_From_World(kItemBriefcase);
		} else if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemBriefcase, 12, true, false)) {
			Item_Pickup_Spin_Effect(kModelAnimationBriefcase, 368, 243);
			Item_Remove_From_World(itemId);
			Game_Flag_Set(kFlagUG18BriefcaseTaken);
			Actor_Clue_Acquire(kActorMcCoy, kClueBriefcase, true, kActorGuzza);
		}
	}
	return false;
}

bool SceneScriptUG18::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -684.71f, 0.0f, 171.59f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG18toUG13);
			Set_Enter(kSetUG13, kSceneUG13);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG18::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG18::SceneFrameAdvanced(int frame) {
//	kUG18LoopTrainsArriving            = 0, //   0 -  59
//	kUG18LoopMainLoop                  = 1, //  60 - 120
//	kUG18LoopTrainsLeaving             = 3, // 121 - 180
//	kUG18LoopMainLoopNoTrains          = 4  // 181 - 241
	if (_vm->_cutContent) {
		if (frame == 0) {
			Ambient_Sounds_Play_Sound(Random_Query(kSfxBBGRN1, kSfxBBGRN3), 35, 0, 30, 50);
			Ambient_Sounds_Play_Sound(kSfxLIGHTON, 25, 0, 30, 50);
		}

		if (frame == 2) {
			Ambient_Sounds_Play_Sound(kSfxSUBWAY1, 24, 0, 30, 50);
		}

		if (frame == 60) {
			Ambient_Sounds_Play_Sound(kSfxPIPER1, 25, 30, 30, 0);
			if (Random_Query(1, 3)) {
				Ambient_Sounds_Play_Sound(kSfxSQUEAK2, 32, 30, 30, 50);
			} else {
				Ambient_Sounds_Play_Sound(kSfxSQUEAK4, 34, 30, 30, 50);
			}
		}

		if (frame == 68) {
			Ambient_Sounds_Play_Sound(kSfxSTEAM3, 66, 30, 30, 50);
		}

		if (frame == 70) {
			Ambient_Sounds_Play_Sound(kSfxSTEAM6A, 66, 30, 30, 50);
		}

		if (frame == 115) {
			Ambient_Sounds_Play_Sound(kSfxMTLDOOR2, 40, 30, 30, 50);
		}

		if (frame == 128) {
			Ambient_Sounds_Play_Sound(kSfxSUBWAY1, 24, 30, 0, 50);
		}

		if (frame == 119    //  end of main loop unloading // works better than using the 120 value
		   || frame ==  240 //  end of main loop no trains
		) {
			if (Global_Variable_Query(kVariableUG18CountUpForNextTrainAction) < kUG18TrainsCountUpTargetRegular) {
				if (Global_Variable_Query(kVariableUG18StateOfTrains) == kUG18NoTrains) {
					Global_Variable_Increment(kVariableUG18CountUpForNextTrainAction, kUG18TrainsSecondsOfLoopNoTrains); // add seconds
				} else {
					Global_Variable_Increment(kVariableUG18CountUpForNextTrainAction, kUG18TrainsSecondsOfLoopWithTrains); // add seconds
				}
			} else {
				if (Global_Variable_Query(kVariableUG18StateOfTrains) == kUG18NoTrains) {
					Global_Variable_Set(kVariableUG18StateOfTrains, kUG18TrainsUnloading);
					Scene_Loop_Set_Default(kUG18LoopMainLoop);
					Scene_Loop_Start_Special(kSceneLoopModeOnce, kUG18LoopTrainsArriving, false);
					// don't set the kVariableUG18CountUpForNextTrainAction here, we only want this to play once before transitioning to trains leaving
				} else {
					Global_Variable_Set(kVariableUG18StateOfTrains, kUG18NoTrains);
					Scene_Loop_Set_Default(kUG18LoopMainLoopNoTrains);
					Scene_Loop_Start_Special(kSceneLoopModeOnce, kUG18LoopTrainsLeaving, true);
					Global_Variable_Set(kVariableUG18CountUpForNextTrainAction, Random_Query(0, kUG18TrainsCountUpTargetRegular - kUG18TrainsCountUpMinMarginRegular));
				}
			}
		}
	}
}

void SceneScriptUG18::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == kActorGuzza) {
		switch (newGoal) {
		case kGoalGuzzaUG18HitByMcCoy:
			Game_Flag_Set(kFlagMcCoyRetiredHuman);
			ADQ_Flush();
			Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 7);
			Actor_Modify_Friendliness_To_Other(kActorSadik, kActorMcCoy, 10);
			Player_Loses_Control();
			Actor_Face_Actor(kActorGuzza, kActorMcCoy, true);
			ADQ_Add(kActorGuzza, 1220, 58);
			Scene_Exits_Enable();
			Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaUG18ShotByMcCoy);
			break;

		case kGoalGuzzaUG18MissedByMcCoy:
			ADQ_Flush();
			Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 7);
			Actor_Modify_Friendliness_To_Other(kActorSadik, kActorMcCoy, 10);
			Player_Loses_Control();
			Actor_Face_Actor(kActorGuzza, kActorMcCoy, true);
			ADQ_Add(kActorGuzza, 1220, 58);
			Scene_Exits_Enable();
			Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaUG18ShootMcCoy);
			break;
		}
		return;
	}

	if (actorId == kActorSadik) {
		switch (newGoal) {
		case kGoalSadikUG18Decide:
			if (Actor_Query_Friendliness_To_Other(kActorClovis, kActorMcCoy) > 55
			 && Game_Flag_Query(kFlagMcCoyRetiredHuman)
			) {
				Actor_Says(kActorClovis, 660, 13); // Brother, you killed a human...
				Actor_Says(kActorMcCoy, 5995, 13);
				Actor_Says(kActorClovis, 670, 13);
				Actor_Says(kActorMcCoy, 6000, 13);
				Actor_Says_With_Pause(kActorClovis, 680, 2.0f, 13);
				Actor_Says(kActorClovis, 690, 13);
				Actor_Says(kActorClovis, 700, 13);
				Actor_Set_Goal_Number(kActorSadik, kGoalSadikUG18Leave);
				Actor_Set_Goal_Number(kActorClovis, kGoalClovisUG18Leave);
			} else {
				Actor_Set_Goal_Number(kActorSadik, kGoalSadikUG18PrepareShootMcCoy);
				Actor_Set_Goal_Number(kActorClovis, kGoalClovisUG18Leave);
			}
			break;

		// goals 303, 304 and 305 are never set, cut out part of game?
		case 304:
			Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, -3);
			ADQ_Add(kActorSadik, 380, -1);
			Actor_Set_Goal_Number(kActorSadik, kGoalSadikUG18WillShootMcCoy);
			break;

		case 305:
			// never triggered
			Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatAttack);
			Sound_Play(kSfxLGCAL1, 100, 0, 0, 50);
			Actor_Force_Stop_Walking(kActorMcCoy);
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
			Player_Loses_Control();
			Actor_Retired_Here(kActorMcCoy, 6, 6, true, kActorSadik);
			break;
		}
	}
}

void SceneScriptUG18::PlayerWalkedIn() {
	if (_vm->_cutContent) {
		if (Game_Flag_Query(kFlagUG18GuzzaScene)) {
			switch (Global_Variable_Query(kVariableUG18StateOfGuzzaCorpse)) {
			case kUG18GuzzaCorpseFloatsDown:
				Global_Variable_Set(kVariableUG18StateOfGuzzaCorpse, kUG18GuzzaCorpseStuckInPipes);
				// same logic as using the BB06OVER for doll explosion case in BB06
				Overlay_Play("UG18OVR2", 0, true, true,  0);
				Overlay_Play("UG18OVR2", 1, true, false, 0);
				break;
			case kUG18GuzzaCorpseStuckInPipes:
				Global_Variable_Set(kVariableUG18StateOfGuzzaCorpse, kUG18GuzzaCorpseDissolves);
				Overlay_Play("UG18OVR2", 1, true, true,  0);
				Overlay_Play("UG18OVR2", 2, false, false, 0);
				break;
			case kUG18GuzzaCorpseDissolves:
				Global_Variable_Set(kVariableUG18StateOfGuzzaCorpse, kUG18GuzzaNoCorpse);
				Overlay_Remove("UG18OVR2");
				break;
			default:
				break;
			}
		}
	}

	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -488.71f, 0.0f, 123.59f, 0, false, false, false);

	if ( Game_Flag_Query(kFlagCallWithGuzza)
	 && !Game_Flag_Query(kFlagUG18GuzzaScene)
	 &&  Actor_Query_Is_In_Current_Set(kActorGuzza)
	) {
		Scene_Exits_Disable();
		talkWithGuzza();
		talkWithClovis();
		Game_Flag_Set(kFlagUG18GuzzaScene);
	}
}

void SceneScriptUG18::PlayerWalkedOut() {
	if (_vm->_cutContent) {
		Overlay_Remove("UG18OVER");
	}
}

void SceneScriptUG18::DialogueQueueFlushed(int a1) {
	switch (Actor_Query_Goal_Number(kActorGuzza)) {
	case kGoalGuzzaUG18Target:
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaUG18WillGetShotBySadik);
		Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatAttack);
		Sound_Play(kSfxLGCAL3, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatHit);
		ADQ_Add(kActorClovis, 630, 13); // "Whatever is born of mortal birth, must be consumed with the earth."
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisUG18SadikWillShootGuzza);
		break;

	case kGoalGuzzaUG18ShotByMcCoy:
		// Bug in the game, shot animation is not reset so McCoy looks still while he is shooting
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAim);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAttack);
		Sound_Play(kSfxLGCAL2, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatHit);
		Delay(900);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAim);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAttack);
		Sound_Play(kSfxLGCAL3, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatHit);
		Delay(1100);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAim);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAttack);
		Sound_Play(kSfxLGCAL1, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatHit);
		Delay(900);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAim);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAttack);
		Sound_Play(kSfxLGCAL3, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, 61);
		if (_vm->_cutContent) {
			// same logic as using the BB06OVER for doll explosion case in BB06
			Overlay_Play("UG18OVER", 1, true, true,  0);
			Overlay_Play("UG18OVER", 2, true, false, 0);
			Global_Variable_Set(kVariableUG18StateOfGuzzaCorpse, kUG18GuzzaCorpseFloatsDown);
		} else {
			Overlay_Play("UG18OVER", 1, false, true, 0);
		}
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaUG18FallDown);
		Player_Gains_Control();
		ADQ_Add_Pause(2000);
		ADQ_Add(kActorSadik, 360, -1); // The Hunter, he do us a favor...
		ADQ_Add_Pause(2000);
		ADQ_Add(kActorClovis, 650, 14); // So, what should we do with this detective.
		ADQ_Add(kActorSadik, 370, 14);
		ADQ_Add(kActorClovis, 1320, 14); // Perhaps you're right
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Retired_Here(kActorGuzza, 72, 32, true, kActorMcCoy);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisUG18GuzzaDied);
		break;

	case kGoalGuzzaUG18ShootMcCoy:
		Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatAttack);
		Sound_Play(kSfxLGCAL2, 100, 0, 0, 50);
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		Player_Loses_Control();
		Actor_Retired_Here(kActorMcCoy, 6, 6, true, kActorGuzza);
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaUG18FallDown);
		break;
	}

	switch (Actor_Query_Goal_Number(kActorClovis)) {
	case kGoalClovisUG18SadikWillShootGuzza:
		Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatAttack);
		Sound_Play(kSfxLGCAL3, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, kAnimationModeCombatHit);
		ADQ_Add(kActorClovis, 640, 13);
		ADQ_Add(kActorGuzza, 1210, 13);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisUG18SadikIsShootingGuzza);
		break;

	case kGoalClovisUG18SadikIsShootingGuzza:
		Actor_Change_Animation_Mode(kActorSadik, kAnimationModeCombatAttack);
		Sound_Play(kSfxLGCAL3, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, 61);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// don't allow code to reach the overlay animation if the dialogue queue is not flushed
		// otherwise this animation is stored and if the player saves the game during the queued dialogue
		// then upon re-load it would play immediately (before Guzza falls)
		ADQ_Wait_For_All_Queued_Dialogue();
#endif // BLADERUNNER_ORIGINAL_BUGS
		if (_vm->_cutContent) {
			// same logic as using the BB06OVER for doll explosion case in BB06
			Overlay_Play("UG18OVER", 1, true, true,  0);
			Overlay_Play("UG18OVER", 2, true, false, 0);
			Global_Variable_Set(kVariableUG18StateOfGuzzaCorpse, kUG18GuzzaCorpseFloatsDown);
		}
		ADQ_Add_Pause(2000);
		ADQ_Add(kActorClovis, 650, 14);
		ADQ_Add(kActorSadik, 370, 14);
		ADQ_Add(kActorClovis, 1320, 14);
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaUG18ShotBySadik);
		Actor_Retired_Here(kActorGuzza, 72, 32, true, kActorSadik);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisUG18GuzzaDied);
		Scene_Exits_Enable();
		break;

	case kGoalClovisUG18GuzzaDied:
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikUG18Move);
#else
		// otherwise this gets repeated whenever dialogue queue re-empties
		if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikUG18Wait) {
			Actor_Set_Goal_Number(kActorSadik, kGoalSadikUG18Move);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		break;
	}

	if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikUG18WillShootMcCoy) {
		// Bug in the original game - Why is Sadik set to die animation here?
		// never triggered
		Actor_Change_Animation_Mode(kActorSadik, kAnimationModeDie);
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikUG18PrepareShootMcCoy);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisUG18Leave);
	}
}

void SceneScriptUG18::talkWithGuzza() {
	Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
	Actor_Says(kActorMcCoy, 5860, 9);
	Delay(500);
	Actor_Face_Actor(kActorGuzza, kActorMcCoy, true);
	Delay(500);
	Actor_Says(kActorGuzza, 790, 3);
	Actor_Says(kActorMcCoy, 5865, 12);
	Actor_Says(kActorGuzza, 800, 3);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -357.13f, 0.0f, -44.47f, 0, false, false, false);
	Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
	Actor_Says(kActorMcCoy, 5870, 14);
	Actor_Face_Actor(kActorGuzza, kActorMcCoy, true);
	Actor_Start_Speech_Sample(kActorGuzza, 810);
	Loop_Actor_Walk_To_XYZ(kActorGuzza, -57.21f, 0.0f, -334.17f, 0, false, false, false);
	Actor_Says(kActorMcCoy, 5875, 13);
	Actor_Says(kActorGuzza, 830, 3);
	Actor_Says(kActorGuzza, 840, 12);
	Actor_Says(kActorGuzza, 850, 14);
	Actor_Says(kActorGuzza, 860, 13);
	Actor_Says(kActorMcCoy, 5880, 15);
#if BLADERUNNER_ORIGINAL_BUGS
	Actor_Says(kActorMcCoy, 5885, 9);
#else
	Actor_Says_With_Pause(kActorMcCoy, 5885, 0.0f, 9);
#endif // BLADERUNNER_ORIGINAL_BUGS
	Actor_Says(kActorMcCoy, 5890, 13);
	Actor_Says(kActorGuzza, 870, 15);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -205.13f, 0.0f, -184.47f, 0, false, false, false);
	Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
	Actor_Says(kActorMcCoy, 5900, 15);
	Actor_Says(kActorGuzza, 880, 13);
	Actor_Says(kActorMcCoy, 5905, 9);
	Actor_Says(kActorMcCoy, 5910, 12);
	Actor_Says(kActorMcCoy, 5915, 13);
	Actor_Says(kActorGuzza, 890, 16);
	Actor_Says(kActorMcCoy, 5920, 14);
	Loop_Actor_Walk_To_XYZ(kActorGuzza, -57.21f, 0.0f, -334.17f, 0, false, false, false);
	Actor_Face_Actor(kActorGuzza, kActorMcCoy, true);
	Actor_Says(kActorGuzza, 900, 15);
	Actor_Says(kActorGuzza, 910, 12);
	Actor_Says(kActorGuzza, 920, 16);
	Actor_Says(kActorMcCoy, 5925, 14);
	Actor_Says(kActorGuzza, 940, 14);
	Actor_Says(kActorMcCoy, 5930, 18);
	Actor_Says(kActorGuzza, 950, 14);
	Actor_Says(kActorGuzza, 960, 13);
	Actor_Says(kActorGuzza, 970, 3);
	if (Game_Flag_Query(kFlagMcCoyRetiredHuman)) {
		Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 3);
		Actor_Modify_Friendliness_To_Other(kActorSadik, kActorMcCoy, 5);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -117.13f, 0.0f, -284.47f, 0, false, false, false);
		Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
		Actor_Says(kActorMcCoy, 5960, 9);
		Actor_Says(kActorMcCoy, 5965, 14);
		Actor_Says(kActorGuzza, 980, 15);
		Actor_Says(kActorGuzza, 990, 13);
		Actor_Says(kActorMcCoy, 5970, 14);
		Actor_Says(kActorGuzza, 1000, 3);
		Actor_Says(kActorMcCoy, 5975, 15);
	} else if (Player_Query_Agenda() == kPlayerAgendaPolite) {
		Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, -1);
		Actor_Modify_Friendliness_To_Other(kActorSadik, kActorMcCoy, -1);
		Actor_Says(kActorMcCoy, 5935, 14);
		Actor_Says(kActorMcCoy, 5940, 18);
		Actor_Says(kActorGuzza, 1020, 13);
		Actor_Says(kActorGuzza, 1030, 14);
	} else if (Global_Variable_Query(kVariableAffectionTowards) > 1
			|| Player_Query_Agenda() == kPlayerAgendaSurly
	) {
		Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 20);
		Actor_Modify_Friendliness_To_Other(kActorSadik, kActorMcCoy, 10);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -117.13f, 0.0f, -284.47f, 0, false, false, false);
		Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
		Actor_Says(kActorMcCoy, 5950, 16);
		Actor_Says(kActorMcCoy, 5955, 14);
		Actor_Says(kActorGuzza, 1110, 13);
		Actor_Says(kActorGuzza, 1120, 15);
		Actor_Says(kActorMcCoy, 5990, 3);
		Actor_Says(kActorGuzza, 1130, 15);
		Actor_Says(kActorGuzza, 1140, 16);
	} else {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -117.13f, 0.0f, -284.47f, 0, false, false, false);
		Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
		Actor_Says(kActorMcCoy, 5945, 12);
		Actor_Says(kActorGuzza, 1040, 15);
		Actor_Says(kActorMcCoy, 5980, 15);
		Actor_Says(kActorGuzza, 1050, 12);
		Actor_Says(kActorGuzza, 1060, 13);
		Actor_Says(kActorGuzza, 1070, 14);
		Actor_Says(kActorMcCoy, 5985, 18);
		Actor_Says(kActorGuzza, 1080, 3);
		Actor_Says(kActorGuzza, 1090, 14);
		Actor_Says(kActorGuzza, 1100, 13);
	}
}

void SceneScriptUG18::talkWithClovis() {
	ADQ_Flush();
	Actor_Start_Speech_Sample(kActorClovis, 590);
	Delay(500);
	Loop_Actor_Walk_To_XYZ(kActorGuzza, 126.79f, 0.0f, -362.17f, 0, false, false, false);
	Actor_Face_Heading(kActorGuzza, 729, false);
	Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaUG18Target);
	ADQ_Add(kActorSadik, 350, 13);
	ADQ_Add_Pause(1500);
	ADQ_Add(kActorGuzza, 1150, 58);
	ADQ_Add(kActorClovis, 600, 13);
	ADQ_Add_Pause(1000);
	ADQ_Add(kActorGuzza, 1160, 60);
	ADQ_Add_Pause(500);
	ADQ_Add(kActorGuzza, 1170, 59);
	ADQ_Add(kActorGuzza, 1180, 58);
	ADQ_Add(kActorClovis, 610, 13);
	ADQ_Add(kActorGuzza, 1190, 60);
	ADQ_Add(kActorClovis, 620, 13); // Lieutenant, we have everything we need...
	ADQ_Add(kActorGuzza, 1200, 59);
}

} // End of namespace BladeRunner
