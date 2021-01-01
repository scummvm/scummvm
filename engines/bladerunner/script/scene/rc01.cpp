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

enum kRC01Loops {
	kRC01LoopInshotWithCrowd   = 0,
	kRC01LoopWithCrowd         = 1,
	kRC01LoopDoorAnimWithCrowd = 3,
	kRC01LoopOutshotWithCrowd  = 4,
	kRC01LoopInshotNoCrowd     = 5,
	kRC01LoopNoCrowd           = 6,
	kRC01LoopDoorAnimNoCrowd   = 8,
	kRC01LoopOutshotNoCrowd    = 9,
	kRC01LoopNoCrowdNoSpinner  = 10
};

enum kRC01Exits {
	kRC01ExitRC02    = 0,
	kRC01ExitSpinner = 1,
	kRC01ExitRC03    = 2
};

enum kRC01Regions {
	kRC01RegionCrowd = 0
};

void SceneScriptRC01::InitializeScene() {
	if (!Game_Flag_Query(kFlagIntroPlayed)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeWestwood, true, -1);
		Outtake_Play(kOuttakeBladeRunner, true, -1);
		Outtake_Play(kOuttakeIntro, false, -1);
		Outtake_Play(kOuttakeDescent, true, -1);
	}
	if (Game_Flag_Query(kFlagRC02toRC01)) {
		Setup_Scene_Information(-171.16f, 5.55f, 27.28f, 616);
	} else if (Game_Flag_Query(kFlagRC03toRC01)) {
		Setup_Scene_Information(-471.98f, -0.30f, 258.15f, 616);
	} else {
		Setup_Scene_Information(-10.98f, -0.30f, 318.15f, 616);
	}
	Scene_Exit_Add_2D_Exit(kRC01ExitRC02, 314, 145, 340, 255, 0);
	if (Game_Flag_Query(kFlagSpinnerAtRC01)) {
		Scene_Exit_Add_2D_Exit(kRC01ExitSpinner, 482, 226, 639, 280, 2);
	}
	if (Global_Variable_Query(kVariableChapter) > 1
	 && Game_Flag_Query(kFlagAR01Entered)
	) {
		Scene_Exit_Add_2D_Exit(kRC01ExitRC03, 0, 0, 10, 479, 3);
	}
	if (!Game_Flag_Query(kFlagRC01PoliceDone)) {
		Scene_2D_Region_Add(kRC01RegionCrowd, 0, 294, 296, 479);
	}

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1, 30, 0, 1);

	if (!Game_Flag_Query(kFlagRC01PoliceDone)) {
		Ambient_Sounds_Add_Sound(kSfx67_0470R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0500R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_0960R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1100R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1140R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxRCAMBR1, 60, 100, 1);
	Ambient_Sounds_Add_Sound(kSfxRCCARBY1,  5, 30, 30, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxRCCARBY2,  5, 30, 30, 55, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxRCCARBY3,  5, 30, 30, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN2A,   10, 50, 30, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSIREN2,   20, 80, 20, 40, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagRC01PoliceDone)) {
		if (!Game_Flag_Query(kFlagRC02toRC01) && !Game_Flag_Query(kFlagRC03toRC01)) {
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kRC01LoopInshotNoCrowd, false);
		}
		if (Game_Flag_Query(kFlagSpinnerAtRC01)) {
			Scene_Loop_Set_Default(kRC01LoopNoCrowd);
		} else {
			Scene_Loop_Set_Default(kRC01LoopNoCrowdNoSpinner);
		}
	} else {
		if (!Game_Flag_Query(kFlagRC02toRC01) && !Game_Flag_Query(kFlagRC03toRC01)) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
			// set leary at his original position since otherwise he may overlap badly
			// with scenery during the video loop of the spinner landing
			// This only applies in Act 1, before the flag kFlagRC01PoliceDone is set
			if (Actor_Query_Is_In_Current_Set(kActorOfficerLeary)) {
				Actor_Set_At_XYZ(kActorOfficerLeary, -261.80f, 6.00f, 79.58f, 512);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kRC01LoopInshotWithCrowd, false);
		}
		Scene_Loop_Set_Default(kRC01LoopWithCrowd);
	}

	I_Sez("Blade Runner");
	I_Sez("");
	I_Sez("From the dark recesses of David Leary's imagination comes a game unlike any");
	I_Sez("other. Blade Runner immerses you in the underbelly of future Los Angeles. Right");
	I_Sez("from the start, the story pulls you in with graphic descriptions of a");
	I_Sez("grandmother doing the shimmy in her underwear, child molestation, brutal");
	I_Sez("cold-blooded slaying of innocent animals, vomiting on desks, staring at a");
	I_Sez("woman's ass, the list goes on. And when the game starts, the real fun begins -");
	I_Sez("shoot down-on-their-luck homeless people and toss them into a dumpster. Watch");
	I_Sez("with sadistic glee as a dog gets blown into chunky, bloody, bits by an");
	I_Sez("explosive, and even murder a shy little girl who loves you. If you think David");
	I_Sez("Leary is sick, and you like sick, this is THE game for you.");
	I_Sez("");
	I_Sez("JW: Don't forget the wasting of helpless mutated cripples in the underground.");
	I_Sez("It's such a beautiful thing!");
	I_Sez("");
	I_Sez("DL: Go ahead.  Just keep beating that snarling pit bull...ignore the foam");
	I_Sez("around his jaws. There's room on the top shelf of my fridge for at least one");
	I_Sez("more head... - Psychotic Dave");
	I_Sez("");
}

void SceneScriptRC01::SceneLoaded() {
	Obstacle_Object("HYDRANT02", true);
	Obstacle_Object("PARKING METER 04", true);
	Obstacle_Object("CHEVY PROP", true);
	Obstacle_Object("PARKING METER 01", true);
	Obstacle_Object("T-CAN01", true);
	Obstacle_Object("BARICADE01", true);
	Obstacle_Object("BARICADE02", true);
	Obstacle_Object("DOOR LEFT", true);
	Unobstacle_Object("BOX06", true);
	Clickable_Object("DOORWAY01");
	Clickable_Object("DOOR LEFT");
	Clickable_Object("HYDRANT02");
	Clickable_Object("T-CAN01");
	Clickable_Object("BARICADE01");
	Clickable_Object("70_1");
	Clickable_Object("70_2");
	Clickable_Object("70_3");
	Clickable_Object("70_5");
	Clickable_Object("70_6");
	Unclickable_Object("BARICADE02");
	Unclickable_Object("BARICADE05");
	Unclickable_Object("SPINNER BODY");
	Unclickable_Object("HORSE01");
	Unclickable_Object("DOORWAY01");
	Unobstacle_Object("DOORWAY01", true);

	if (Game_Flag_Query(kFlagRC01PoliceDone)) {
		Unclickable_Object("70_1");
		Unclickable_Object("70_2");
		Unclickable_Object("70_3");
		Unclickable_Object("70_5");
		Unclickable_Object("70_6");
		Unclickable_Object("BARICADE01");
		Unclickable_Object("BARICADE03");
		Unclickable_Object("BARICADE04");
		Unobstacle_Object("70_1", true);
		Unobstacle_Object("70_2", true);
		Unobstacle_Object("70_3", true);
		Unobstacle_Object("70_5", true);
		Unobstacle_Object("70_6", true);
		Unobstacle_Object("BARICADE01", true);
		Unobstacle_Object("BARICADE02", true);
		Unobstacle_Object("BARICADE03", true);
		Unobstacle_Object("BARICADE04", true);
		Unobstacle_Object("BARICADE05", true);
	}

	if (!Game_Flag_Query(kFlagRC01PoliceDone)) {
		Preload(kModelAnimationMcCoyWalking);
		Preload(kModelAnimationMcCoyRunning);
		Preload(kModelAnimationMcCoyIdle);
		Preload(kModelAnimationOfficerLearyWalking);
		Preload(kModelAnimationOfficerLearyOscillateIdle);
	}

	if (!Game_Flag_Query(kFlagRC01ChromeDebrisTaken)) {
		Item_Add_To_World(kItemChromeDebris, kModelAnimationChromeDebris, kSetRC01, -148.60f, -0.30f, 225.15f, 256, 24, 24, false, true, false, true);
	}

	if (!Game_Flag_Query(kFlagIntroPlayed)) {
		ADQ_Flush();
		Actor_Voice_Over(1830, kActorVoiceOver);
		Actor_Voice_Over(1850, kActorVoiceOver);
		if (!Game_Flag_Query(kFlagDirectorsCut)) {
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Voice_Over(1860, kActorVoiceOver);
#else
			// Quote 1860 is muted in the DEU version
			// The quote "No self-respecting human would own one of those frauds."
			// is completely missing from the DEU sound files (it's not part of the previous quote)
			if (_vm->_language != Common::DE_DEU) {
				Actor_Voice_Over(1860, kActorVoiceOver);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			I_Sez("MG: Is David Leary a self-respecting human or is he powered by rechargeable");
			I_Sez("batteries?\n");
		}
		Game_Flag_Set(kFlagIntroPlayed);
	}
}

bool SceneScriptRC01::MouseClick(int x, int y) {
	return y >= 430;
}

bool SceneScriptRC01::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("BARICADE01", objectName)
	 || Object_Query_Click("BARICADE03", objectName)
	 || Object_Query_Click("BARICADE04", objectName)
	 || Object_Query_Click("70_1", objectName)
	 || Object_Query_Click("70_2", objectName)
	 || Object_Query_Click("70_3", objectName)
	 || Object_Query_Click("70_5", objectName)
	 || Object_Query_Click("70_6", objectName)
	) {
		interrogateCrowd();
		return true;
	}

	if (Object_Query_Click("HYDRANT02", objectName)) {
		if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "HYDRANT02", 60, true, false)) {
			if (Actor_Clue_Query(kActorMcCoy, kCluePaintTransfer)) {
				Actor_Says(kActorMcCoy, 6975, kAnimationModeTalk);
			} else {
				Actor_Face_Object(kActorMcCoy, "HYDRANT02", true);
				Actor_Voice_Over(1880, kActorVoiceOver);
				Actor_Voice_Over(1890, kActorVoiceOver);
				I_Sez("JM: That McCoy--he's one funny guy! Jet-black fire truck, hehehehe...");
				Actor_Clue_Acquire(kActorMcCoy, kCluePaintTransfer, true, -1);
			}
		}
		return true;
	}

	if (Object_Query_Click("DOOR LEFT", objectName)) {
		if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "DOOR LEFT", 48, true, false)) {
			Actor_Face_Object(kActorMcCoy, "DOOR LEFT", true);
			if (!Actor_Clue_Query(kActorMcCoy, kClueDoorForced2)
			    && Actor_Query_In_Set(kActorOfficerLeary, kSetRC01)
			    && Global_Variable_Query(kVariableChapter) > 0
			) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
				bool officerLearyWasInterrogatingTheCrowd = Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01CrowdInterrogation
				                                            || Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01WalkToCrowd
				                                            || Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01ResumeWalkToCrowd;
#endif // BLADERUNNER_ORIGINAL_BUGS
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDefault);
				Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
				Actor_Says(kActorOfficerLeary, 0, 12);
				Actor_Says(kActorMcCoy, 4495, 13);
				Actor_Clue_Acquire(kActorMcCoy, kClueDoorForced2, true, kActorOfficerLeary);
#if BLADERUNNER_ORIGINAL_BUGS
#else
				if (officerLearyWasInterrogatingTheCrowd) {
					Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyRC01ResumeWalkToCrowd);
				}
#endif // BLADERUNNER_ORIGINAL_BUGS
			}
#if BLADERUNNER_ORIGINAL_BUGS
#else
			else if (!Actor_Clue_Query(kActorMcCoy, kClueDoorForced2)
			         && !Actor_Clue_Query(kActorMcCoy, kClueDoorForced1)
			         && !Actor_Query_In_Set(kActorOfficerLeary, kSetRC01)
			         && Global_Variable_Query(kVariableChapter) == 1) {
				if (_vm->_cutContent) {
					// keep in mind, this line in only available in Act 1 (1.TLK) unless _vm->_cutContent is selected (provided that cut content now loads all TLKs)
					Actor_Voice_Over(1870, kActorVoiceOver);
				} else {
					Actor_Says(kActorMcCoy, 8570, 14);
				}
				Actor_Clue_Acquire(kActorMcCoy, kClueDoorForced1, true, -1);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			else {
				Actor_Says(kActorMcCoy, 8570, 14);
			}
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Clue_Acquire(kActorMcCoy, kClueDoorForced1, true, -1);
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		return true;
	}

	if (Object_Query_Click("T-CAN01", objectName)) {
		if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "T-CAN01", 24, true, false)) {
			Actor_Face_Object(kActorMcCoy, "T-CAN01", true);
			Actor_Voice_Over(1810, kActorVoiceOver);
			Actor_Voice_Over(1820, kActorVoiceOver);
		}
		return true;
	}

	return false;
}

bool SceneScriptRC01::ClickedOnActor(int actorId) {
	if (actorId == kActorOfficerLeary && Global_Variable_Query(kVariableChapter) == 1) {
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorOfficerLeary, 36, true, false)) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
			bool officerLearyWasInterrogatingTheCrowd = Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01CrowdInterrogation
			                                            || Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01WalkToCrowd
			                                            || Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01ResumeWalkToCrowd;
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Face_Actor(kActorMcCoy, kActorOfficerLeary, true);
			Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
			if (Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01WalkToCrowd) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDefault);
			}
			if (Game_Flag_Query(kFlagRC01GotOfficersStatement)) {
				Actor_Says(kActorMcCoy, 4535, 13);
				Game_Flag_Set(kFlagRC01McCoyAndOfficerLearyTalking);
				if (Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewA) && !Actor_Clue_Query(kActorMcCoy, kClueCrowdInterviewA)) {
					Actor_Face_Object(kActorOfficerLeary, "70_1", true);
					if (_vm->_cutContent
					    && (_vm->_language == Common::ES_ESP
					        || _vm->_language == Common::IT_ITA)
					) {
						// Quote 110 is the second half of the sentence about Lucy hanging around with Zuben ("a fat guy")
						// in ENG, DEU and FRA it is redundant, but it's needed in ESP and ITA
						Actor_Says_With_Pause(kActorOfficerLeary, 100, 0.0f, 15);
						Actor_Says(kActorOfficerLeary, 110, kAnimationModeTalk);
					} else {
						Actor_Says(kActorOfficerLeary, 100, 15);
					}
					Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
					Actor_Clue_Acquire(kActorMcCoy, kClueCrowdInterviewA, true, kActorOfficerLeary);
					Game_Flag_Reset(kFlagRC01McCoyAndOfficerLearyTalking);
				} else if (Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewB) && !Actor_Clue_Query(kActorMcCoy, kClueCrowdInterviewB)) {
					Actor_Face_Object(kActorOfficerLeary, "70_5", true);
					Actor_Says(kActorOfficerLeary, 120, 19);
					Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
					Actor_Says(kActorOfficerLeary, 130, 14);
					I_Sez("JM: Did it have a huge, ugly piece of chrome on it?");
					Actor_Clue_Acquire(kActorMcCoy, kClueCrowdInterviewB, true, kActorOfficerLeary);
					Game_Flag_Reset(kFlagRC01McCoyAndOfficerLearyTalking);
				} else {
					Actor_Says(kActorOfficerLeary, 90, 16);
					I_Sez("JM: This officer has a talent for vivid metaphors.");
					if (!Game_Flag_Query(kFlagRC01McCoyAndOfficerLearyArtMetaphor)) {
						I_Sez("DL: What is that supposed to mean?  I didn't write this line...");
						Actor_Says(kActorMcCoy, 4540, 16);
						Game_Flag_Set(kFlagRC01McCoyAndOfficerLearyArtMetaphor);
					}
					Game_Flag_Reset(kFlagRC01McCoyAndOfficerLearyTalking);
				}
			} else {
				I_Sez("MG: Hey, leave that officer alone. Can't you see he's busy?");
				I_Sez("JM: (...mmm, donuts...)");
				Game_Flag_Set(kFlagRC01GotOfficersStatement);
				Actor_Clue_Acquire(kActorMcCoy, kClueOfficersStatement, true, kActorOfficerLeary);
				Actor_Says(kActorMcCoy, 4515, 13);
				Game_Flag_Set(kFlagRC01McCoyAndOfficerLearyTalking);
				Actor_Says(kActorOfficerLeary, 40, 13);
				if (!Game_Flag_Query(kFlagRC02Entered)) {
					Actor_Says(kActorOfficerLeary, 50, 14);
					Actor_Says(kActorOfficerLeary, 60, 15);
					I_Sez("MG: It's all fun and games until someone loses a tiger cub.");
					Actor_Says(kActorMcCoy, 4520, 18);
					Actor_Says(kActorOfficerLeary, 70, 16);
					Actor_Says(kActorMcCoy, 4525, 14);
					Actor_Says(kActorOfficerLeary, 80, 18);
					Actor_Says(kActorMcCoy, 4530, 15);
				}
				Game_Flag_Reset(kFlagRC01McCoyAndOfficerLearyTalking);
			}
#if BLADERUNNER_ORIGINAL_BUGS
#else
			if (officerLearyWasInterrogatingTheCrowd) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyRC01ResumeWalkToCrowd);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		return true;
	}
	return false;
}

bool SceneScriptRC01::ClickedOnItem(int itemId, bool a2) {
	if (itemId == kItemChromeDebris) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
		bool officerLearyWasInterrogatingTheCrowd = Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01CrowdInterrogation
		                                            || Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01WalkToCrowd
		                                            || Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01ResumeWalkToCrowd;
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDefault);
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemChromeDebris, 36, true, false)) {
			Actor_Face_Item(kActorMcCoy, kItemChromeDebris, true);
			Actor_Clue_Acquire(kActorMcCoy, kClueChromeDebris, true, -1);
			Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
			Actor_Says(kActorOfficerLeary, 20, 12);
			Game_Flag_Set(kFlagRC01ChromeDebrisTaken);
			Item_Remove_From_World(kItemChromeDebris);
			Item_Pickup_Spin_Effect(kModelAnimationChromeDebris, 426, 316);
			I_Sez("JM: Chrome...is that what that is?");
			Actor_Says(kActorMcCoy, 4505, 13);
			ADQ_Flush();
			ADQ_Add(kActorOfficerLeary, 30, -1);
			ADQ_Add(kActorMcCoy, 4510, 13);
			I_Sez("JM: It's hard to imagine that thing on either a car or a horse.");
			I_Sez("MG: McCoy! What a witty chap...");
			I_Sez("JM: He keeps me chuckling non-stop!\n");
			Loop_Actor_Walk_To_Actor(kActorOfficerLeary, kActorMcCoy, 36, false, false);
		}
#if BLADERUNNER_ORIGINAL_BUGS
#else
		if (officerLearyWasInterrogatingTheCrowd) {
			Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyRC01ResumeWalkToCrowd);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		return true;
	}
	return false;
}

void SceneScriptRC01::walkToCenter() {
	Player_Loses_Control();
	Game_Flag_Set(kFlagMcCoyInRunciters);
	Actor_Set_Immunity_To_Obstacles(kActorMcCoy, true);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -151.98f, -0.3f, 318.15f, 0, false, false, false);
	Actor_Set_Immunity_To_Obstacles(kActorMcCoy, false);
	Player_Gains_Control();
}

bool SceneScriptRC01::ClickedOnExit(int exitId) {
	if (exitId == kRC01ExitRC02) {
#if BLADERUNNER_ORIGINAL_BUGS
		bool walkToRC02ExitResult = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -174.77f, 5.55f, 25.95f, 12, true, false, false);
#else
		// fixes clipping into the closed door of the shop when McCoy says locked or goes in
		// This was reproducible mainly by entering from Bullet Bob's  (RC03) and clicking on the door immediately
		bool walkToRC02ExitResult = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -170.38f, 5.55f, 35.00f, 12, true, false, false);
#endif // BLADERUNNER_ORIGINAL_BUGS
		if (!walkToRC02ExitResult) {
			if (Game_Flag_Query(kFlagRC02RunciterTalkWithGun)) {
				Actor_Says(kActorMcCoy, 8522, 14); // Locked
			} else {
				switch (Global_Variable_Query(kVariableChapter)) {
				case 1:
				case 4:
					Game_Flag_Set(kFlagRC01toRC02);
					Set_Enter(kSetRC02_RC51, kSceneRC02);
					break;
				case 2:
				case 3:
				case 5:
					Actor_Says(kActorMcCoy, 8522, 12);
					break;
				default:
					return true;
				}
			}
		}
		return true;
	}

	if (exitId == kRC01ExitSpinner) {
		if (Game_Flag_Query(kFlagDNARowAvailable)) {
			Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationDNARow, true);
		}
		I_Sez("MG: Leaving already?  The fun is just beginning!");
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -151.98f, -0.3f, 318.15f, 0, true, false, false)) {
			Player_Loses_Control();
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, true);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -10.98f, -0.3f, 318.15f, 0, false, false, false);
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
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, false);
			Player_Gains_Control();
			Game_Flag_Reset(kFlagMcCoyInChinaTown);
			Game_Flag_Reset(kFlagMcCoyInRunciters);
			Game_Flag_Reset(kFlagMcCoyInMcCoyApartment);
			Game_Flag_Reset(kFlagMcCoyInPoliceStation);
			int spinnerDest;
			if (Game_Flag_Query(kFlagRC01PoliceDone)) {
				spinnerDest = Spinner_Interface_Choose_Dest(kRC01LoopDoorAnimNoCrowd, true);
			} else {
				spinnerDest = Spinner_Interface_Choose_Dest(kRC01LoopDoorAnimWithCrowd, true);
			}

			switch (spinnerDest) {
			case kSpinnerDestinationPoliceStation:
				Game_Flag_Set(kFlagMcCoyInPoliceStation);
				Game_Flag_Reset(kFlagSpinnerAtRC01);
				Game_Flag_Set(kFlagSpinnerAtPS01);
				Set_Enter(kSetPS01, kScenePS01);
				if (Game_Flag_Query(kFlagRC01PoliceDone)) {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotNoCrowd, true);
				} else {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotWithCrowd, true);
				}
				break;
			case kSpinnerDestinationMcCoysApartment:
				Game_Flag_Set(kFlagMcCoyInMcCoyApartment);
				Game_Flag_Reset(kFlagSpinnerAtRC01);
				Game_Flag_Set(kFlagSpinnerAtMA01);
				Set_Enter(kSetMA01, kSceneMA01);
				if (Game_Flag_Query(kFlagRC01PoliceDone)) {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotNoCrowd, true);
				} else {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotWithCrowd, true);
				}
				break;
			case kSpinnerDestinationChinatown:
				Game_Flag_Set(kFlagMcCoyInChinaTown);
				Game_Flag_Reset(kFlagSpinnerAtRC01);
				Game_Flag_Set(kFlagSpinnerAtCT01);
				Set_Enter(kSetCT01_CT12, kSceneCT01);
				if (Game_Flag_Query(kFlagRC01PoliceDone)) {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotNoCrowd, true);
				} else {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotWithCrowd, true);
				}
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(kFlagMcCoyInTyrellBuilding);
				Game_Flag_Reset(kFlagSpinnerAtRC01);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				if (Game_Flag_Query(kFlagRC01PoliceDone)) {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotNoCrowd, true);
				} else {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotWithCrowd, true);
				}
				break;
			case kSpinnerDestinationAnimoidRow:
				Game_Flag_Set(kFlagMcCoyInAnimoidRow);
				Game_Flag_Reset(kFlagSpinnerAtRC01);
				Game_Flag_Set(kFlagSpinnerAtAR01);
				Set_Enter(kSetAR01_AR02, kSceneAR01);
				if (Game_Flag_Query(kFlagRC01PoliceDone)) {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotNoCrowd, true);
				} else {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotWithCrowd, true);
				}
				break;
			case kSpinnerDestinationDNARow:
				Game_Flag_Set(kFlagMcCoyInDNARow);
				Game_Flag_Reset(kFlagSpinnerAtRC01);
				Game_Flag_Set(kFlagSpinnerAtDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
				if (Game_Flag_Query(kFlagRC01PoliceDone)) {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotNoCrowd, true);
				} else {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotWithCrowd, true);
				}
				break;
			case kSpinnerDestinationBradburyBuilding:
				Game_Flag_Set(kFlagMcCoyInBradburyBuilding);
				Game_Flag_Reset(kFlagSpinnerAtRC01);
				Game_Flag_Set(kFlagSpinnerAtBB01);
				Set_Enter(kSetBB01, kSceneBB01);
				if (Game_Flag_Query(kFlagRC01PoliceDone)) {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotNoCrowd, true);
				} else {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotWithCrowd, true);
				}
				break;
			case kSpinnerDestinationNightclubRow:
				Game_Flag_Set(kFlagMcCoyInNightclubRow);
				Game_Flag_Reset(kFlagSpinnerAtRC01);
				Game_Flag_Set(kFlagSpinnerAtNR01);
				Set_Enter(kSetNR01, kSceneNR01);
				if (Game_Flag_Query(kFlagRC01PoliceDone)) {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotNoCrowd, true);
				} else {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotWithCrowd, true);
				}
				break;
			case kSpinnerDestinationHysteriaHall:
				Game_Flag_Set(kFlagMcCoyInHysteriaHall);
				Game_Flag_Reset(kFlagSpinnerAtRC01);
				Game_Flag_Set(kFlagSpinnerAtHF01);
				Set_Enter(kSetHF01, kSceneHF01);
				if (Game_Flag_Query(kFlagRC01PoliceDone)) {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotNoCrowd, true);
				} else {
					Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kRC01LoopOutshotWithCrowd, true);
				}
				break;
			default:
				walkToCenter();
				break;
			}
		}
		return true;
	}

	if (exitId == kRC01ExitRC03) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -471.98f, -0.3f, 258.15f, 4, true, false, false)) {
			Game_Flag_Set(kFlagRC01toRC03);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetRC03, kSceneRC03);
		}
		return true;
	}
	return false;
}

void SceneScriptRC01::interrogateCrowd() {
	if (!Game_Flag_Query(kFlagRC01PoliceDone)) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
		bool officerLearyWasInterrogatingTheCrowd = Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01CrowdInterrogation
		                                            || Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01WalkToCrowd
		                                            || Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01ResumeWalkToCrowd;
		if (officerLearyWasInterrogatingTheCrowd
		    || (Actor_Clue_Query(kActorMcCoy, kClueCrowdInterviewA)
		        && Actor_Clue_Query(kActorMcCoy, kClueCrowdInterviewB) )
		) {
			Actor_Says(kActorMcCoy, 8525, kAnimationModeTalk); // generic "hmph"
			return;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS

		if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "BARICADE03", 36, true, false)) {
			Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDefault);
			Actor_Face_Object(kActorMcCoy, "BARICADE03", true);
			Loop_Actor_Walk_To_Actor(kActorOfficerLeary, kActorMcCoy, 36, true, false);
			Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
			Actor_Says(kActorMcCoy, 4500, 14);
			I_Sez("MG: We don't want any of that abstract art oozing out onto the street.");
			Actor_Says(kActorOfficerLeary, 10, 14);
			Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyRC01WalkToCrowd);
		}
	}
}

bool SceneScriptRC01::ClickedOn2DRegion(int region) {
	if (region == kRC01RegionCrowd) {
		interrogateCrowd();
		return true;
	}
	return false;
}

void SceneScriptRC01::SceneFrameAdvanced(int frame) {
	if (frame == 1) {
		Sound_Play(kSfxCARDOWN3, 40, 0, 0, 50);
	}
	if (frame == 61 || frame == 362) {
		Sound_Play(kSfxSPINOPN4, 100, 80, 80, 50);
	}
	if (frame == 108 || frame == 409) {
		Sound_Play(kSfxSPINCLS1, 100, 80, 80, 50);
	}
	if (frame == 183 || frame == 484) {
		Sound_Play(kSfxSPINOPN4, 100, 80, 80, 50);
	}
	if (frame == 228 || frame == 523) {
		Sound_Play(kSfxSPINCLS1, 100, 80, 80, 50);
	}
	if (frame == 243 || frame == 545) {
		Sound_Play(kSfxCARUP3, 40, 80, 80, 50);
	}
	if (frame == 315) {
		Sound_Play(kSfxCARDOWN3, 40, 80, 80, 50);
	}
}

void SceneScriptRC01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptRC01::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagSpinnerAtRC01)
	    && !Game_Flag_Query(kFlagRC02toRC01)
	    && !Game_Flag_Query(kFlagRC03toRC01)
	) {
		walkToCenter();
	}

	if (Game_Flag_Query(kFlagRC03toRC01)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -415.98f, -0.30f, 262.15f, 0, false, false, false);
		Player_Gains_Control();
		Game_Flag_Reset(kFlagRC03toRC01);
	}

	if (Game_Flag_Query(kFlagRC02toRC01)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -203.45f, 5.55f, 85.05f, 0, false, false, false);
		Player_Gains_Control();
		Game_Flag_Reset(kFlagRC02toRC01);

		if ( Game_Flag_Query(kFlagRC02Entered)
		 && !Game_Flag_Query(kFlagRC02Left)
		) {
			Actor_Voice_Over(1910, kActorVoiceOver);
			Actor_Voice_Over(1920, kActorVoiceOver);
			Actor_Voice_Over(1930, kActorVoiceOver);
			Game_Flag_Set(kFlagRC02Left);
		}
		//return true;
	}
	//return false;
}

void SceneScriptRC01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(kFlagRC01toRC02)
	 && !Game_Flag_Query(kFlagRC01toRC03)
	) {
		if (Global_Variable_Query(kVariableChapter) == 1) {
			if (_vm->_cutContent) {
				Outtake_Play(kOuttakeAscent, true, -1);
			}
			Outtake_Play(kOuttakeAway2, true, -1);
		}
#if BLADERUNNER_ORIGINAL_BUGS
#else
		else {
			// Acts 2, 3 - should still use a spinner fly-through transition
			if (!Game_Flag_Query(kFlagMcCoyInTyrellBuilding)) {
				// don't play this extra outtake when going to Tyrell Building
				Outtake_Play(kOuttakeAway1, true, -1);    // available in Acts 2, 3
			}
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	//	return 1;
}

void SceneScriptRC01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
