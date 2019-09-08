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

enum kAR01Loops {
	kAR01LoopFromAR02WithSpinner = 0,
	kAR01LoopInshot              = 1,
	kAR01LoopMainLoop            = 2,
	kAR01LoopDoorAnim            = 4,
	kAR01LoopOutshot             = 5,
	kAR01LoopFromAR02NoSpinner   = 6,
	kAR01LoopMainLoopNoSpinner   = 7
};

enum kAR01Exits {
	kAR01ExitHC01    = 0,
	kAR01ExitAR02    = 1,
	kAR01ExitSpinner = 2
};

void SceneScriptAR01::InitializeScene() {
	Music_Play(kMusicArabLoop, 25, 0, 2, -1, 1, 2);
	if (Game_Flag_Query(kFlagHC01toAR01)) {
		Setup_Scene_Information(-477.0f, 0.0f, -149.0f, 333);
	} else if (Game_Flag_Query(kFlagAR02toAR01)) {
		Setup_Scene_Information(-182.0f, 0.0f, -551.0f, 518);
	} else {
		Setup_Scene_Information(-152.0f, 0.0f, 332.0f, 545);
	}
	Scene_Exit_Add_2D_Exit(kAR01ExitHC01, 134, 165, 177, 290, 3);
	Scene_Exit_Add_2D_Exit(kAR01ExitAR02, 319,   0, 639, 207, 0);
	if (Game_Flag_Query(kFlagSpinnerAtAR01)) {
		Scene_Exit_Add_2D_Exit(kAR01ExitSpinner, 0, 404, 99, 479, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1, 50,   1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxRCAMBR1, 60, 100, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxHCLOOP1, 50,   1, 1);
	Ambient_Sounds_Add_Sound(kSfx67_0480R, 5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0540R, 5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0560R, 5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0870R, 5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0900R, 5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0940R, 5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1070R, 5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1080R, 5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1160R, 5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM8,   3,  30, 11,  11,   50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM2,   3,  30, 11,  11,   50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM3,   3,  30, 11,  11,   50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM4,   3,  30, 11,  11,   50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM5,   3,  30, 11,  11,   50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM6,   3,  30, 11,  11,   50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM7,   3,  30, 11,  11,   50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM1,   3,  30, 11,  11,   50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  10, 180, 16,  25,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  10, 180, 16,  25,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 10, 180, 50, 100,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 10, 180, 50, 100,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 10, 180, 50, 100,    0,   0, -101, -101, 0, 0);
	if ( Game_Flag_Query(kFlagSpinnerAtAR01)
	 && !Game_Flag_Query(kFlagAR02toAR01)
	 && !Game_Flag_Query(kFlagHC01toAR01)
	) {
		Actor_Set_Invisible(kActorMcCoy, true);
		Game_Flag_Set(kFlagArrivedFromSpinner2);
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kAR01LoopInshot, false);
		Scene_Loop_Set_Default(kAR01LoopMainLoop);
	} else if (Game_Flag_Query(kFlagSpinnerAtAR01)
	        && Game_Flag_Query(kFlagAR02toAR01)
	) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kAR01LoopFromAR02WithSpinner, false);
		Scene_Loop_Set_Default(kAR01LoopMainLoop);
		Game_Flag_Reset(kFlagAR02toAR01);
	} else if (!Game_Flag_Query(kFlagSpinnerAtAR01)
	        &&  Game_Flag_Query(kFlagAR02toAR01)
	) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kAR01LoopFromAR02NoSpinner, false);
		Scene_Loop_Set_Default(kAR01LoopMainLoopNoSpinner);
		Game_Flag_Reset(kFlagAR02toAR01);
	} else if (Game_Flag_Query(kFlagSpinnerAtAR01)
	        && Game_Flag_Query(kFlagHC01toAR01)
	) {
		Scene_Loop_Set_Default(kAR01LoopMainLoop);
	} else if (!Game_Flag_Query(kFlagSpinnerAtAR01)
	        &&  Game_Flag_Query(kFlagHC01toAR01)
	) {
		Scene_Loop_Set_Default(kAR01LoopMainLoopNoSpinner);
	} else { // bug? branch content is equal to previous branch
		Scene_Loop_Set_Default(kAR01LoopMainLoopNoSpinner);
	}
}

void SceneScriptAR01::SceneLoaded() {
	Obstacle_Object("DF_BOOTH", true);
	Unobstacle_Object("SPINNER BODY", true);

#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (Global_Variable_Query(kVariableChapter) < 4
	    && !Game_Flag_Query(kFlagAR02DektoraBoughtScorpions)
	    && !Game_Flag_Query(kFlagScorpionsInAR02)
	) {
		// the kFlagScorpionsInAR02 flag helps keep track of the item in AR
		// and make it not blink in and out of existence
		// in the transition from AR01 to AR02
		Game_Flag_Set(kFlagScorpionsInAR02);
		Item_Add_To_World(kItemScorpions, kModelAnimationCageOfScorpions, kSetAR01_AR02, -442.84f, 36.77f, -1144.51f, 360, 36, 36, false, true, false, true);
	} else if (Global_Variable_Query(kVariableChapter) >= 4
	           && !Game_Flag_Query(kFlagAR02DektoraBoughtScorpions)
	           && Game_Flag_Query(kFlagScorpionsInAR02)
	) {
		Game_Flag_Reset(kFlagScorpionsInAR02);
		Item_Remove_From_World(kItemScorpions);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptAR01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptAR01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptAR01::ClickedOnActor(int actorId) {
	if (actorId == kActorInsectDealer
	 || actorId == kActorHasan
	) {
		Actor_Face_Actor(kActorMcCoy, actorId, true);
		Actor_Says(kActorMcCoy, 8910, 14);
		return true;
	}

	if (actorId == kActorFishDealer) {
		Actor_Set_Goal_Number(kActorFishDealer, 2);
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -120.73f, 0.0f, 219.17f, 12, true, false, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorFishDealer, true);
			Actor_Face_Actor(kActorFishDealer, kActorMcCoy, true);
			if (!Game_Flag_Query(kFlagAR01FishDealerTalk)) {
				Actor_Says(kActorMcCoy, 0, 18);
				Actor_Says(kActorFishDealer, 0, 14);
				Actor_Says(kActorFishDealer, 10, 14);
				Actor_Says(kActorFishDealer, 20, 14);
				Actor_Says(kActorFishDealer, 30, 14);
				Actor_Says(kActorMcCoy, 5, 17);
				Actor_Says(kActorFishDealer, 40, 14);
				Actor_Says(kActorMcCoy, 10, 13);
				Actor_Says(kActorFishDealer, 50, 14);
				Actor_Says(kActorMcCoy, 15, 17);
				Actor_Says(kActorFishDealer, 60, 14);
				Actor_Says(kActorFishDealer, 70, 14);
				Actor_Says(kActorFishDealer, 80, 14);
				Actor_Says(kActorFishDealer, 90, 14);
				Actor_Says(kActorMcCoy, 25, 13);
				Game_Flag_Set(kFlagAR01FishDealerTalk);
				Actor_Set_Goal_Number(kActorFishDealer, 1);
			} else {
				if (Actor_Clue_Query(kActorMcCoy, kClueStrangeScale1)
				    && !Actor_Clue_Query(kActorMcCoy, kClueFishLadyInterview)
				    && (!_vm->_cutContent || !Game_Flag_Query(kFlagWrongInvestigation))
				) {
					Actor_Says(kActorMcCoy, 40, 11);
					Actor_Says(kActorFishDealer, 120, 14);
					Actor_Says(kActorMcCoy, 45, 17);
					Actor_Says(kActorFishDealer, 130, 14);
					Actor_Says(kActorFishDealer, 140, 14);
					Actor_Says(kActorMcCoy, 50, 13);
					Actor_Says(kActorFishDealer, 150, 14);
#if BLADERUNNER_ORIGINAL_BUGS
					Actor_Clue_Acquire(kActorMcCoy, kClueFishLadyInterview, true, kActorMcCoy); // A bug? Shouldn't the last argument be -1 or kActorFishDealer here?
#else
					Actor_Clue_Acquire(kActorMcCoy, kClueFishLadyInterview, true, kActorFishDealer);
#endif // BLADERUNNER_ORIGINAL_BUGS
				} else {
					if (_vm->_cutContent) {
						switch (Global_Variable_Query(kVariableFishDealerBanterTalk)) {
						case 0:
							Global_Variable_Increment(kVariableFishDealerBanterTalk, 1);
							Actor_Says(kActorFishDealer, 230, 14);

							Item_Pickup_Spin_Effect_From_Actor(kModelAnimationGoldfish, kActorFishDealer, 0, -40);
							dialogueWithFishDealerBuyGoldfish();
							break;
						case 1:
							Global_Variable_Increment(kVariableFishDealerBanterTalk, 1);
							if (Player_Query_Agenda() == kPlayerAgendaSurly
							    || Actor_Query_Friendliness_To_Other(kActorFishDealer, kActorMcCoy) <= 45 ) {
								Actor_Says(kActorMcCoy, 8600, 17); // You keeping busy, pal?
								Actor_Says(kActorFishDealer, 180, 14); // I can't stand all day gabbing away. My fish require attention.
								Actor_Says(kActorMcCoy, 8450, 14); // Does this badge mean anything to you?
								Actor_Says(kActorFishDealer, 190, 14); // Ah! You think you get better info somewhere else? You welcome to try.
							} else {
								Actor_Says(kActorMcCoy, 8514, 11); // Got anything new to tell me?
								Actor_Says(kActorFishDealer, 170, 14); // Afraid not. But been busy today. Maybe you ask me later.
							}
							break;

						default:
							if (Random_Query(1, 2) == 1) {
								Actor_Says(kActorMcCoy, 30, 17);
								Actor_Says(kActorFishDealer, 100, 14);
								Actor_Says(kActorFishDealer, 110, 14);
								Actor_Says(kActorMcCoy, 35, 13);
							} else {
								Actor_Says(kActorMcCoy, 30, 17);
								Actor_Says(kActorFishDealer, 220, 14);
							}
						}
					} else {
						// original behavior
						if (Random_Query(1, 2) == 1) {
							Actor_Says(kActorMcCoy, 30, 17);
							Actor_Says(kActorFishDealer, 100, 14);
							Actor_Says(kActorFishDealer, 110, 14);
							Actor_Says(kActorMcCoy, 35, 13);
						} else {
							Actor_Says(kActorMcCoy, 30, 17);
							Actor_Says(kActorFishDealer, 220, 14);
						}
					}
				}
				Actor_Set_Goal_Number(kActorFishDealer, 1);
			}
			return true;
		}
	}
	return false;
}

bool SceneScriptAR01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptAR01::ClickedOnExit(int exitId) {
	if (exitId == kAR01ExitHC01) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -358.0, 0.0, -149.0, 0, true, false, false)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -477.0, 0.0, -149.0, 0, false, false, false);
			Game_Flag_Set(kFlagAR01toHC01);
			Game_Flag_Set(kFlagHC01GuzzaPrepare);
			Game_Flag_Reset(kFlagMcCoyInAnimoidRow);
			Game_Flag_Set(kFlagMcCoyInHawkersCircle);
			Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC01);
			Actor_Set_Goal_Number(kActorFishDealer, 3);
		}
		return true;
	}

	if (exitId == kAR01ExitAR02) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -182.0, 0.0, -551.0, 0, true, false, false)) {
			Game_Flag_Set(kFlagAR01toAR02);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -222.0, 0.0, -690.0, 0, false);
			Set_Enter(kSetAR01_AR02, kSceneAR02);
#if BLADERUNNER_ORIGINAL_BUGS
			// Causes the fish dealer to blink out of existence
			// during the transition to AR02
			Actor_Set_Goal_Number(kActorFishDealer, 3);
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		return true;
	}

	if (exitId == kAR01ExitSpinner) {
		if (Game_Flag_Query(kFlagDNARowAvailable)) {
			Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationDNARow, true);
		}
		int v1 = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -164.0f, 0.0f, 332.0f, 0, true, false, false);
		Actor_Face_Heading(kActorMcCoy, 545, false);
		if (Actor_Query_Goal_Number(kActorIzo) >= kGoalIzoTakePhoto
		 && Actor_Query_Goal_Number(kActorIzo) <= kGoalIzoEscape
		) {
			Player_Loses_Control();
			Actor_Put_In_Set(kActorIzo, kSetAR01_AR02);
			Actor_Set_At_XYZ(kActorIzo, -448.0, 0.0, 130.0, 0);
			Loop_Actor_Walk_To_XYZ(kActorIzo, -323.0f, 0.64f, 101.74f, 48, false, true, false);
			Loop_Actor_Walk_To_Actor(kActorIzo, kActorMcCoy, 48, false, true);
			Actor_Face_Actor(kActorIzo, kActorMcCoy, true);
			Actor_Change_Animation_Mode(kActorIzo, kAnimationModeCombatAttack);
			Actor_Says(kActorMcCoy, 1800, 21);
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
			Player_Gains_Control();
			Actor_Retired_Here(kActorMcCoy, 12, 48, 1, kActorIzo);
		} else if (!v1) {
			if ( Game_Flag_Query(kFlagDNARowAvailable)
			 && !Game_Flag_Query(kFlagDNARowAvailableTalk)
			) {
				Actor_Voice_Over(4310, kActorVoiceOver);
				Actor_Voice_Over(4320, kActorVoiceOver);
				Actor_Voice_Over(4330, kActorVoiceOver);
				Actor_Voice_Over(4340, kActorVoiceOver);
				Actor_Voice_Over(4350, kActorVoiceOver);
				Game_Flag_Set(kFlagDNARowAvailableTalk);
			}
			Game_Flag_Reset(kFlagMcCoyInChinaTown);
			Game_Flag_Reset(kFlagMcCoyInRunciters);
			Game_Flag_Reset(kFlagMcCoyInMcCoyApartment);
			Game_Flag_Reset(kFlagMcCoyInPoliceStation);
			Game_Flag_Reset(kFlagMcCoyInBradburyBuilding);
			Game_Flag_Reset(kFlagMcCoyInHysteriaHall);
			Game_Flag_Reset(kFlagMcCoyInAnimoidRow);

			int spinnerDest = Spinner_Interface_Choose_Dest(kAR01LoopDoorAnim, false);
			Actor_Face_Heading(kActorMcCoy, 545, false);

			switch (spinnerDest) {
			case kSpinnerDestinationPoliceStation:
				Game_Flag_Set(kFlagMcCoyInPoliceStation);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtPS01);
				Set_Enter(kSetPS01, kScenePS01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kAR01LoopOutshot, true);
				break;
			case kSpinnerDestinationMcCoysApartment:
				Game_Flag_Set(kFlagMcCoyInMcCoyApartment);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtMA01);
				Set_Enter(kSetMA01, kSceneMA01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kAR01LoopOutshot, true);
				break;
			case kSpinnerDestinationRuncitersAnimals:
				Game_Flag_Set(kFlagMcCoyInRunciters);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtRC01);
				Set_Enter(kSetRC01, kSceneRC01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kAR01LoopOutshot, true);
				break;
			case kSpinnerDestinationChinatown:
				Game_Flag_Set(kFlagMcCoyInChinaTown);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtCT01);
				Set_Enter(kSetCT01_CT12, kSceneCT01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kAR01LoopOutshot, true);
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(kFlagMcCoyInTyrellBuilding);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kAR01LoopOutshot, true);
				break;
			case kSpinnerDestinationDNARow:
				Game_Flag_Set(kFlagMcCoyInDNARow);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kAR01LoopOutshot, true);
				break;
			case kSpinnerDestinationBradburyBuilding:
				Game_Flag_Set(kFlagMcCoyInBradburyBuilding);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtBB01);
				Set_Enter(kSetBB01, kSceneBB01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kAR01LoopOutshot, true);
				break;
			case kSpinnerDestinationNightclubRow:
				Game_Flag_Set(kFlagMcCoyInNightclubRow);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtNR01);
				Set_Enter(kSetNR01, kSceneNR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kAR01LoopOutshot, true);
				break;
			case kSpinnerDestinationHysteriaHall:
				Game_Flag_Set(kFlagMcCoyInHysteriaHall);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtHF01);
				Set_Enter(kSetHF01, kSceneHF01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kAR01LoopOutshot, true);
				break;
			default:
				Game_Flag_Set(kFlagMcCoyInAnimoidRow);
				Actor_Set_Invisible(kActorMcCoy, false);
				break;
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptAR01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptAR01::SceneFrameAdvanced(int frame) {
	if (frame == 16) {
		Ambient_Sounds_Play_Sound(kSfxCARDOWN3,  40,   0,   0, 99);
	}
	if (frame == 78 || frame == 199) {
		Ambient_Sounds_Play_Sound(kSfxSPINOPN4, 100, -50, -50, 99);
	}
	if (frame == 122 || frame == 242) {
		Ambient_Sounds_Play_Sound(kSfxSPINCLS1, 100, -50, -50, 99);
	}
	if (frame == 256) {
		Ambient_Sounds_Play_Sound(kSfxCARUP3,    40, -50,  80, 99);
	}
	if ((frame == 75
	  || frame == 196
	 )
	 && Game_Flag_Query(kFlagArrivedFromSpinner2)
	) {
		Actor_Face_Heading(kActorMcCoy, 545, false);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeSpinnerGetOut);
		Game_Flag_Reset(kFlagArrivedFromSpinner2);
	} else if ( frame == 196
	        && !Game_Flag_Query(kFlagArrivedFromSpinner2
	)) {
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeSpinnerGetIn);
	}
}

void SceneScriptAR01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptAR01::PlayerWalkedIn() {
	if (!Game_Flag_Query(kFlagAR01Entered)) {
		Game_Flag_Set(kFlagAR01Entered);
	}
	if (Game_Flag_Query(kFlagHC01toAR01)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -358.0f, 0.0f, -149.0f, 0, true, false, false);
		Game_Flag_Reset(kFlagHC01toAR01);
	}
	if (Actor_Query_Goal_Number(kActorPhotographer) < 199) {
		Actor_Set_Goal_Number(kActorPhotographer, 199);
	}
}

void SceneScriptAR01::PlayerWalkedOut() {
	Actor_Set_Invisible(kActorMcCoy, false);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(kFlagMcCoyInHawkersCircle)) {
		Music_Stop(2);
	}
	if (!Game_Flag_Query(kFlagAR01toHC01)
	 && !Game_Flag_Query(kFlagAR01toAR02)
	) {
#if BLADERUNNER_ORIGINAL_BUGS
		Outtake_Play(kOuttakeTowards3, true, -1);
#else
		if (!Game_Flag_Query(kFlagMcCoyInTyrellBuilding)) {
			// don't play an extra outtake when going to Tyrell Building
			Outtake_Play(kOuttakeTowards3, true, -1); // Act 1, 2, 3
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
}

void SceneScriptAR01::DialogueQueueFlushed(int a1) {
}

/**
* This is only for cut content (purchasing the Goldfish)
*/
void SceneScriptAR01::dialogueWithFishDealerBuyGoldfish() {
	Dialogue_Menu_Clear_List();

	Dialogue_Menu_Clear_Never_Repeat_Was_Selected_Flag(530);
	Dialogue_Menu_Clear_Never_Repeat_Was_Selected_Flag(540);

	if (Global_Variable_Query(kVariableChinyen) >= 105
	    || Query_Difficulty_Level() == kGameDifficultyEasy
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(530, 7, 5, 3); // BUY
	}
	DM_Add_To_List_Never_Repeat_Once_Selected(540, 3, 5, 7); // NO THANKS

	Dialogue_Menu_Appear(320, 240);
	int answerValue = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	if (answerValue == 530) { // BUY
		Actor_Says(kActorMcCoy, 7000, 12);
		if (Query_Difficulty_Level() != kGameDifficultyEasy) {
			Global_Variable_Decrement(kVariableChinyen, 105);
		}
		Actor_Clue_Acquire(kActorMcCoy, kClueGoldfish, true, kActorFishDealer);
		Actor_Modify_Friendliness_To_Other(kActorFishDealer, kActorMcCoy, 5);
	} else if (answerValue == 540) { // NO THANKS
		Actor_Says(kActorMcCoy, 7005, 13);
		Actor_Modify_Friendliness_To_Other(kActorFishDealer, kActorMcCoy, -5);
	}
}

} // End of namespace BladeRunner
