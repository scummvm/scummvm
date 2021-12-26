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

void SceneScriptNR07::InitializeScene() {
	Setup_Scene_Information(-110.0f, -73.5f, -193.0f, 554);

	Scene_Exit_Add_2D_Exit(0, 429, 137, 506, 251, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxDRAMB5, 25, 0, 1);
}

void SceneScriptNR07::SceneLoaded() {
	Obstacle_Object("VANITY", true);
	Clickable_Object("VASE");

	if (_vm->_cutContent
	    && Global_Variable_Query(kVariableChapter) < 4) {
		// The car is only bought by Reps from CrazyLegs
		// if Dektora is a Replicant (so she will have the note from CrazyLegs)
		// or if Dektora  is human and Gordo is also human (in which case, there'll be no note in the drawer) 
		// We add the region in any case - on Restored Content mode
		// to indicate there might be something of interest there,
		// albeit not all playthroughs will have the note in there.
		Scene_2D_Region_Add(0, 501, 381, 522, 466);
	}
}

bool SceneScriptNR07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR07::ClickedOn3DObject(const char *objectName, bool a2) {
	Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiDefault);

	if (Object_Query_Click("VASE", objectName)) {
		clickedOnVase();
	}

	Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiResetTimer);
	return false;
}

bool SceneScriptNR07::ClickedOnActor(int actorId) {
	if (actorId == kActorDektora) {
		if (Actor_Query_Goal_Number(kActorHolloway) >= kGoalHollowayGoToNR07) { // Holloway is not yet comming
			return true;
		}

		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiResetTimer);
		Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
		Dialogue_Menu_Clear_List();
		if (Game_Flag_Query(kFlagNR07McCoyIsCop)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(1100, -1, 3, 8); // VOIGT-KAMPFF
			DM_Add_To_List_Never_Repeat_Once_Selected(1110, 8, -1, -1); // CRYSTAL
			if (Actor_Clue_Query(kActorMcCoy, kClueSuspectDektora)) { // cut content? clue is not obtainable
				// TODO - restore trigger
				DM_Add_To_List_Never_Repeat_Once_Selected(1120, 3, 6, 7); // MOONBUS
			}
			if (Actor_Clue_Query(kActorMcCoy, kClueCarRegistration1)) {
				DM_Add_To_List_Never_Repeat_Once_Selected(1130, 3, 5, 7); // BLACK SEDAN
			}
			if (Game_Flag_Query(kFlagNotUsed510)) { // cut content? flag is never set
				// TODO - restore trigger
				DM_Add_To_List_Never_Repeat_Once_Selected(1140, 1, 4, 7); // SCORPIONS
			}
		} else {
			DM_Add_To_List_Never_Repeat_Once_Selected(1080, 3, 5, 7); // BELT
			DM_Add_To_List_Never_Repeat_Once_Selected(1090, 7, 5, 4); // EARLY-Q
		}
		Dialogue_Menu_Add_DONE_To_List(1150); // DONE

		Dialogue_Menu_Appear(320, 240);
		int answer = Dialogue_Menu_Query_Input();
		Dialogue_Menu_Disappear();

		switch (answer) {
		case 1080: // BELT
			talkAboutBelt1();
			break;

		case 1090: // EARLY-Q
			Actor_Says(kActorMcCoy, 3650, 13);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorDektora, 630, 30);
#else
			// Dektora is interrupted here
			Actor_Says_With_Pause(kActorDektora, 630, 0.0f, 30);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorMcCoy, 3655, 16);
			Actor_Says(kActorDektora, 640, 31);
			break;

		case 1100: // VOIGT-KAMPFF
			talkAboutVoightKampff();
			break;

		case 1110: // CRYSTAL
			talkAboutSteele();
			break;

		case 1120: // MOONBUS
			talkAboutMoonbus();
			break;

		case 1130: // BLACK SEDAN
			talkAboutBlackSedan();
			break;

		case 1140: // SCORPIONS
			talkAboutScorpions();
			break;

		default:
			break;
		}

		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiDefault);
	}
	return false;
}

bool SceneScriptNR07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -102.0f, -73.5f, -233.0f, 0, true, false, false)) {
			Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiResetTimer);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1u);
			Game_Flag_Set(kFlagNR07toNR06);
			Set_Enter(kSetNR06, kSceneNR06);
		}
		return true;
	}
	return false;
}

bool SceneScriptNR07::ClickedOn2DRegion(int region) {
	if (_vm->_cutContent
	    && region == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -134.5f, -73.35f, 43.98f, 4, true, false, false)) {
			Player_Loses_Control();
			Actor_Face_Object(kActorMcCoy, "VANITY", true);
			if (Actor_Query_Is_In_Current_Set(kActorDektora)) {
				Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -2);
				Actor_Says(kActorDektora, 560, 31); // Please don't touch that. It's private.
				Actor_Says(kActorMcCoy, 8525, 19);  // Hmph.
			} else {
				if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
					if (Actor_Clue_Query(kActorMcCoy, kClueCrazysInvolvement)) {
						Actor_Says(kActorMcCoy, 8580, 12); // Nothing else there
					} else {
						Sound_Play(kSfxDRAWER1, 90, 85, 85, 50);
						Delay(700);
						Actor_Clue_Acquire(kActorMcCoy, kClueCrazysInvolvement, false, -1);
						// McCoy picks up a sales brochure (it's CrazyLeg's -- but the model is the same as the Tyrell's Pamphlet)
						Item_Pickup_Spin_Effect(kModelAnimationTyrellSalesPamphlet, 508, 401);
						// McCoy finds something inside the brochure - CrazyLeg's note to the Replicants
						// Logic is similar to HC03 (finding photos inside Izo's camera)
						Actor_Voice_Over(3690, kActorVoiceOver); // Hmm
						Delay(1200);
						Item_Pickup_Spin_Effect(kModelAnimationLetter, 508, 401);
						Actor_Says(kActorMcCoy, 6975, 12); // Interesting
						// We don't remove the region after picking the clue
						// McCoy will just point out that there's nothing more there to find.
						// (Saves us from using up a flag and having to write extra code)
					}
				} else {
					Sound_Play(kSfxDRAWER1, 90, 85, 85, 50);
					Delay(700);
					Actor_Voice_Over(3700, kActorVoiceOver); // Nothing
				}
			}
			Player_Gains_Control();
		}
		return true;
	}
	return false;
}

void SceneScriptNR07::SceneFrameAdvanced(int frame) {
}

void SceneScriptNR07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR07::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -110.0f, -73.5f, -169.0f, 0, false, false, false);

	if (Actor_Query_In_Set(kActorDektora, kSetNR07)) {
		if (!Game_Flag_Query(kFlagNR07Entered)) {
			Game_Flag_Set(kFlagNR07Entered);

			if (!Actor_Clue_Query(kActorDektora, kClueMcCoyIsABladeRunner)) {
				Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, 5);
			} else if (Actor_Clue_Query(kActorMcCoy, kClueMcCoyWarnedIzo)
			        || Actor_Clue_Query(kActorMcCoy, kClueMcCoyHelpedIzoIzoIsAReplicant)
			) {
				Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, 10);
			}

			Actor_Says(kActorDektora, 500, 30);
			Actor_Says(kActorMcCoy, 3585, 14);
			Actor_Says(kActorDektora, 510, 30);
			Actor_Start_Speech_Sample(kActorMcCoy, 3590);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -112.0f, -73.0f, -89.0f, 525, false, false, false);
			Actor_Says(kActorDektora, 520, kAnimationModeSit);
		} else {
			Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -2);
			Actor_Says(kActorDektora, 530, 31);
		}
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiDefault);
	}
	//return false;
}

void SceneScriptNR07::PlayerWalkedOut() {

}

void SceneScriptNR07::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR07::dektoraRunAway() {
	Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiResetTimer);
	Player_Loses_Control();
	Actor_Set_At_XYZ(kActorDektora, -136.0f, -73.0f, -18.0f, 300);
	Actor_Change_Animation_Mode(kActorDektora, 71);
	Actor_Change_Animation_Mode(kActorMcCoy, 21);
	Loop_Actor_Walk_To_XYZ(kActorDektora, -102.0f, -73.5f, -233.0f, 0, false, true, false);

	if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR08ReadyToRun);
	} else {
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR07RanAway);
		Game_Flag_Set(kFlagDektoraRanAway);
		Actor_Put_In_Set(kActorDektora, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorDektora, 33, 0);
	}
	Player_Gains_Control();
}

void SceneScriptNR07::callHolloway() {
	Scene_Exits_Disable();
	Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiResetTimer);
	Actor_Says_With_Pause(kActorDektora, 930, 1.0f, 30);
	Actor_Says_With_Pause(kActorDektora, 910, 1.0f, 30);
	Actor_Face_Object(kActorDektora, "VANITY", true);
	Actor_Says(kActorDektora, 940, 31);
	Actor_Says(kActorMcCoy, 3770, 19);
	Async_Actor_Walk_To_XYZ(kActorMcCoy, -193.0f, -73.5f, -13.0f, 0, false);
	Actor_Says(kActorDektora, 950, 31);
	Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
	Actor_Change_Animation_Mode(kActorDektora, kAnimationModeCombatIdle);
	Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
	Actor_Says(kActorMcCoy, 3760, 19);
	Actor_Says(kActorDektora, 960, kAnimationModeSit);
	if (_vm->_cutContent) {
		Actor_Says(kActorMcCoy, 3765, kAnimationModeTalk); // Let me show you my ID.
	}
	Actor_Says(kActorDektora, 920, kAnimationModeSit);
#if BLADERUNNER_ORIGINAL_BUGS
	Actor_Says(kActorMcCoy, 3780, kAnimationModeIdle);
#else
	// McCoy is interrupted here
	Actor_Says_With_Pause(kActorMcCoy, 3780, 0.0f, kAnimationModeIdle);
#endif // BLADERUNNER_ORIGINAL_BUGS
	Actor_Says(kActorDektora, 970, kAnimationModeSit);
	Actor_Voice_Over(1710, kActorVoiceOver);
	Actor_Voice_Over(1720, kActorVoiceOver);
	Actor_Voice_Over(1730, kActorVoiceOver);
	Actor_Set_Goal_Number(kActorHolloway, kGoalHollowayGoToNR07);
}

void SceneScriptNR07::clickedOnVase() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -109.0f, -73.0f, -89.0f, 0, false, false, false);
	Actor_Face_Object(kActorMcCoy, "VASE", true);
	if (Actor_Query_Is_In_Current_Set(kActorDektora)) {
		if (!Actor_Clue_Query(kActorMcCoy, kClueDektoraInterview3)) {
			Actor_Clue_Acquire(kActorMcCoy, kClueDektoraInterview3, true, -1);
			int friendliness = Actor_Query_Friendliness_To_Other(kActorDektora, kActorMcCoy);
			if (friendliness > 50) {
				Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, 2);
			} else {
				Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -2);
			}
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorMcCoy, 3600, 19);  // The flowers are beautiful. (McCoy fake fan voice)
			Actor_Says(kActorDektora, 550, 30); // And a extremely rare (...)
			Actor_Says(kActorMcCoy, 3605, 19);  // That's a pretty card. (McCoy fake fan voice)
			Actor_Says(kActorDektora, 560, 31); // Please don't touch that. It's private.
			Actor_Says(kActorMcCoy, 3610, 19);  // Sorry (McCoy fake fan voice)
#else
			if (!Game_Flag_Query(kFlagNR07McCoyIsCop)) {
				Actor_Says(kActorMcCoy, 3600, 19);  // The flowers are beautiful. (McCoy fake fan voice)
				Actor_Says(kActorDektora, 550, 30); // And a extremely rare (...)
				Actor_Says(kActorMcCoy, 3605, 19);  // That's a pretty card. (McCoy fake fan voice)
				Actor_Says(kActorDektora, 560, 31); // Please don't touch that. It's private.
				Actor_Says(kActorMcCoy, 3610, 19);  // Sorry (McCoy fake fan voice)
			} else {
				Actor_Says(kActorDektora, 560, 31); // Please don't touch that. It's private.
				Actor_Says(kActorMcCoy, 8525, 19);  // Hmph.
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
	} else if (!Actor_Clue_Query(kActorMcCoy, kClueDektorasCard)) {
		Actor_Clue_Acquire(kActorMcCoy, kClueDektorasCard, true, -1);
		Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "VASE", 100, true, false);
		Actor_Change_Animation_Mode(kActorMcCoy, 23);
		Item_Pickup_Spin_Effect(kModelAnimationDektorasCard, 526, 268);
		Actor_Voice_Over(1690, kActorVoiceOver);
		Actor_Voice_Over(1700, kActorVoiceOver);
	} else {
		Actor_Says(kActorMcCoy, 8585, 14);
	}
}

void SceneScriptNR07::talkAboutBelt1() {
	Actor_Clue_Acquire(kActorMcCoy, kClueDektoraInterview2, true, -1);
	Actor_Says(kActorMcCoy, 3625, 19);
	Actor_Says(kActorDektora, 570, 30);
	Actor_Says_With_Pause(kActorDektora, 580, 1.0f, 31);
	Actor_Says(kActorMcCoy, 3630, 13);
	Actor_Says_With_Pause(kActorDektora, 590, 1.0f, 30);
	Actor_Says(kActorDektora, 600, 30);
	if (_vm->_cutContent) {
		Actor_Says_With_Pause(kActorMcCoy, 3635, 1.5f, 18); // Insects, hm.
	}
	Actor_Start_Speech_Sample(kActorMcCoy, 3640);  // Tell you the truth, I'm from the LPD. (...)
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -109.0f, -73.0f, -89.0f, 0, false, false, false);
	Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
	Actor_Face_Actor(kActorDektora, kActorMcCoy, true);

	Game_Flag_Set(kFlagNR07McCoyIsCop);
	Actor_Clue_Acquire(kActorMcCoy, kClueDragonflyBelt, true, kActorDektora);

	int friendliness = Actor_Query_Friendliness_To_Other(kActorDektora, kActorMcCoy);
	if (!Game_Flag_Query(kFlagDektoraIsReplicant)
	 &&  friendliness < 40
	) {
		dektoraRunAway();
		return;
	}

	if (friendliness < 36) {
		callHolloway();
		return;
	}

	talkAboutBelt2();
}

void SceneScriptNR07::talkAboutBelt2() {
	if (Actor_Clue_Query(kActorDektora, kClueMcCoysDescription)
	 && Actor_Clue_Query(kActorDektora, kClueMcCoyIsABladeRunner)
	) {
		Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -1);
	}

#if BLADERUNNER_ORIGINAL_BUGS
	Actor_Says(kActorDektora, 610, 31);
#else
	// Dektora is interrupted here
	Actor_Says_With_Pause(kActorDektora, 610, 0.0f, 31);
#endif // BLADERUNNER_ORIGINAL_BUGS
	Actor_Says(kActorMcCoy, 3645, 12);
	Actor_Says(kActorDektora, 620, 30);

	int friendliness = Actor_Query_Friendliness_To_Other(kActorDektora, kActorMcCoy);
	if (!Game_Flag_Query(kFlagDektoraIsReplicant)
	 &&  friendliness < 40
	) {
		dektoraRunAway();
		return;
	}

	if (friendliness < 36) {
		callHolloway();
		return;
	}

	Actor_Face_Object(kActorDektora, "VANITY", true);
}

void SceneScriptNR07::talkAboutVoightKampff() {
	Actor_Clue_Acquire(kActorMcCoy, kClueDektoraInterview1, true, -1);
	Actor_Start_Speech_Sample(kActorMcCoy, 3660);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -109.0f, -73.0f, -89.0f, 0, false, false, false);
	Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
	Actor_Says(kActorDektora, 650, 30);
	Actor_Says(kActorDektora, 660, 31);
	Actor_Says(kActorMcCoy, 3665, 18);
	Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
	Actor_Says(kActorDektora, 670, 31);
	Actor_Says(kActorDektora, 680, 30);
	Actor_Says(kActorDektora, 690, 31);
	Actor_Says(kActorMcCoy, 3670, 17);
	Actor_Says(kActorDektora, 700, 30);
	Actor_Says(kActorMcCoy, 3675, 19);
	Actor_Says(kActorDektora, 710, 30);
	Actor_Says(kActorMcCoy, 3680, 19);
	Actor_Says(kActorDektora, 720, 30);
	Actor_Says(kActorDektora, 730, 30);
	Actor_Says(kActorMcCoy, 3685, 13);
	Voight_Kampff_Activate(kActorDektora, 40);

	if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
		callHolloway();
	} else {
		dektoraRunAway();
	}
}

void SceneScriptNR07::talkAboutSteele() {
	Actor_Says(kActorMcCoy, 3690, 14);
	Actor_Start_Speech_Sample(kActorDektora, 750);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -109.0f, -73.0f, -89.0f, 0, false, false, false);
	Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
	Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
	Actor_Says(kActorMcCoy, 3695, 15);
	if (_vm->_cutContent) {
		Actor_Says(kActorMcCoy, 3700, kAnimationModeTalk); // If I found you, so will she.
	}
	Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, 5);

	if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
		callHolloway();
	} else {
		dektoraRunAway();
	}
}

void SceneScriptNR07::talkAboutMoonbus() {
	// TODO cut content - restore trigger
	Actor_Says(kActorMcCoy, 3705, 19);
	Actor_Says(kActorDektora, 760, kAnimationModeSit); // Excuse me?

	if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
		Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -5);
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Says(kActorMcCoy, 3710, 18);
#else
		Actor_Says(kActorMcCoy, 3710, kAnimationModeTalk); // Somebody told me about this moonbus that got hijacked.
#endif // BLADERUNNER_ORIGINAL_BUGS
		if (_vm->_cutContent) {
			Actor_Says(kActorMcCoy, 3715, 15); // You know, the one where all those humans got killed?
		}
		callHolloway();
	} else {
		Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -3);
		Actor_Start_Speech_Sample(kActorMcCoy, 3710);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -109.0f, -73.0f, -89.0f, 0, false, false, false);
		Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
		dektoraRunAway();
	}
}

void SceneScriptNR07::talkAboutBlackSedan() {
	Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -3);
	Actor_Says(kActorMcCoy, 3615, 16);
	Actor_Says(kActorDektora, 770, 30);
	Actor_Says(kActorMcCoy, 3720, 15);
	Actor_Says_With_Pause(kActorDektora, 780, 2.0f, 30);
	Actor_Says(kActorDektora, 790, 31);
	Actor_Says(kActorMcCoy, 3725, 18);
	Actor_Says(kActorDektora, 800, 30);
	Actor_Says_With_Pause(kActorMcCoy, 3730, 2.0f, 13);
	Actor_Says_With_Pause(kActorDektora, 810, 1.0f, kAnimationModeSit);
	Actor_Says(kActorDektora, 820, 30);
	Actor_Says(kActorMcCoy, 3735, 14);
	Actor_Says(kActorDektora, 830, 31);
	Actor_Says(kActorMcCoy, 3740, 19);
}

void SceneScriptNR07::talkAboutScorpions() {
	// TODO cut content - restore trigger
	Actor_Says(kActorMcCoy, 3620, 19);
	Actor_Says(kActorDektora, 840, 30);
	Actor_Says(kActorMcCoy, 3745, 9);
#if BLADERUNNER_ORIGINAL_BUGS
	Actor_Says_With_Pause(kActorDektora, 850, 1.0f, 30);
	Actor_Says(kActorDektora, 860, 30);
	Actor_Says(kActorDektora, 870, kAnimationModeSit);
#else
	if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
		Actor_Says_With_Pause(kActorDektora, 850, 1.0f, 30); // I didn't know what they were called
		// Note: this plays a bit loud and maybe in inconsistent tone, but surely belongs here
		Actor_Says(kActorDektora, 870, 31);
	} else {
		Actor_Says(kActorDektora, 860, 31);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
	Actor_Says(kActorMcCoy, 3750, 11);
	Actor_Says(kActorDektora, 880, 30);
	Actor_Says(kActorMcCoy, 3755, 16);
	Actor_Says(kActorDektora, 890, 31);
	if (_vm->_cutContent) {
		Actor_Says(kActorDektora, 900, 30); // Who would need to add insects to the list?
	}
}

} // End of namespace BladeRunner
