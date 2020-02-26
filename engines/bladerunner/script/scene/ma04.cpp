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

enum kMA04Loops {
	kMA04LoopInshot   = 0,
	kMA04LoopMainLoop = 1,
	kMA04LoopSleep    = 3,
	kMA04LoopWakeup   = 4
};

enum kMA04Exits {
	kMA04ExitMA02     = 0,
	kMA04ExitMA05     = 1,
	kMA04ExitKitchen  = 2
};

enum kMA04Regions {
	kMA04RegionPhone = 0,
	kMA04RegionBed   = 1
};

void SceneScriptMA04::InitializeScene() {
	if (Game_Flag_Query(kFlagMA05toMA04)) {
		Setup_Scene_Information(-7199.0f, 953.97f, 1579.0f, 502);
		if (Global_Variable_Query(kVariableChapter) != 2 && Global_Variable_Query(kVariableChapter) != 3) {
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kMA04LoopInshot, false);
		}
	} else if (Game_Flag_Query(kFlagMA02ToMA04)) {
		Setup_Scene_Information(-7099.0f, 954.0f, 1866.0f, 502);
	} else if (Game_Flag_Query(kFlagMA04McCoySleeping)) {
		Setup_Scene_Information(-7107.0f, 954.0f, 1742.0f, 502);
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kMA04LoopWakeup, false);
	} else {
		Setup_Scene_Information(-7143.0f, 954.0f, 1868.0f, 733);
	}
	Scene_Exit_Add_2D_Exit(kMA04ExitMA02,    496,  0, 639, 354, 1);
	Scene_Exit_Add_2D_Exit(kMA04ExitMA05,     33, 63, 113, 258, 0);
	Scene_Exit_Add_2D_Exit(kMA04ExitKitchen, 248, 98, 314, 284, 1);
	Scene_2D_Region_Add(kMA04RegionPhone, 343,  97, 353, 190);
	Scene_2D_Region_Add(kMA04RegionBed,     0, 340, 116, 479);
	Ambient_Sounds_Add_Looping_Sound(kSfxAPRTFAN1, 30,   0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 30, -80, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxAPRTAMB5, 12,   0, 1);
	Ambient_Sounds_Add_Sound(kSfxSWEEP2,   5,  30, 11, 11, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP3,   5,  30, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP4,   5,  30, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 10,  60, 20, 20,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 10,  60, 20, 20,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSIREN2,  10,  60, 16, 16, -100,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  60, 180, 16, 25,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  60, 180, 16, 25,    0,   0, -101, -101, 0, 0);
	if (isPhoneRinging()) {
		Ambient_Sounds_Add_Sound(kSfxVIDFONE1, 3, 3, 100, 100, 0, 0, 0, 0, 99, 0);
	}
	Scene_Loop_Set_Default(kMA04LoopMainLoop);
}

void SceneScriptMA04::SceneLoaded() {
	Obstacle_Object("BED-DOG DISH", true);
	Unobstacle_Object("BEDDog BONE", true);
	Unobstacle_Object("BED-BOOK1", true);
	Clickable_Object("BED-SHEETS");
	if (Game_Flag_Query(kFlagMA04WatchedTV)) {
		Unclickable_Object("BED-TV-1");
		Unclickable_Object("BED-TV-2");
	} else {
		Clickable_Object("BED-TV-1");
		Clickable_Object("BED-TV-2");
	}
}

bool SceneScriptMA04::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptMA04::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("BED-SHEETS", objectName)) {
		sleep();
		return false;
	}
	if (Object_Query_Click("BED-TV-1", objectName) || Object_Query_Click("BED-TV-2", objectName)) {
		if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "BED-TV-2", 24, true, false)) {
			Game_Flag_Set(kFlagMA04WatchedTV);
			Unclickable_Object("BED-TV-1");
			Unclickable_Object("BED-TV-2");
			Sound_Play(kSfxSPNBEEP4, 100, 0, 0, 50);
			turnOnTV();
			return false;
		}
		return true;
	}
	return false;
}

bool SceneScriptMA04::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptMA04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptMA04::ClickedOnExit(int exitId) {
	if (exitId == kMA04ExitMA02) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7099.0f, 954.0f, 1866.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagMA04ToMA02);
			Set_Enter(kSetMA02_MA04, kSceneMA02);
		}
		return true;
	}
	if (exitId == kMA04ExitMA05) {
		float x, y, z;
		Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
		if (z <= 1677.0f || !Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7199.0f, 955.0f, 1675.0f, 0, true, false, false)) {
			if (isPhoneMessageWaiting() || isPhoneRinging()) {
				Overlay_Remove("MA04OVER");
			}
#if BLADERUNNER_ORIGINAL_BUGS
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7199.0f, 955.0f, 1675.0f, 0, false, false, true);
#else
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7199.0f, 955.0f, 1627.0f, 0, false, false, true);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Game_Flag_Set(kFlagMA04toMA05);
			if (Global_Variable_Query(kVariableChapter) != 2 && Global_Variable_Query(kVariableChapter) != 3) {
				Async_Actor_Walk_To_XYZ(kActorMcCoy, -7199.0f, 956.17f, 1568.0f, 0, false);
			}
			Set_Enter(kSetMA05, kSceneMA05);
		}
		return true;
	}
	if (exitId == kMA04ExitKitchen) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7115.0f, 954.0f, 1742.0f, 0, true, false, false)) {
			int sounds[] = {kSfxBARSFX1, kSfxTOILET1, kSfxDRAIN1X, kSfxWASH1, kSfxGARGLE1};
			Ambient_Sounds_Play_Sound(sounds[Random_Query(0, 4)], 50, 0, 0, 0);
			Delay(3000);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7139.0f, 954.0f, 1746.0f, 0, true, false, true);
		}
	}
	return false;
}

bool SceneScriptMA04::ClickedOn2DRegion(int region) {
	if (Player_Query_Combat_Mode()) {
		return false;
	}
	if (region == kMA04RegionBed) {
		sleep();
		return true;
	}
	if (region == kMA04RegionPhone) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7176.0f, 954.0f, 1806.0f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 256, false);
			if (isPhoneRinging()) {
				Actor_Says(kActorMcCoy, 2680, 0);
				Ambient_Sounds_Remove_Sound(kSfxVIDFONE1, true);
				Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
				Overlay_Remove("MA04OVER");
				Delay(500);
				if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
					if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsDektora) {
						phoneCallWithDektora();
					} else if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsLucy) {
						phoneCallWithLucy();
					} else {
						phoneCallWithClovis();
					}
				} else {
					phoneCallWithSteele();
				}
				Music_Play(kMusicBRBlues, 52, 0, 3, -1, 0, 0);
				return false;
			}
			if (Actor_Clue_Query(kActorClovis, kClueMcCoyRetiredZuben) && !Game_Flag_Query(kFlagMA04PhoneMessageFromClovis)) {
				Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
				Overlay_Remove("MA04OVER");
				Delay(500);
				Actor_Says(kActorClovis, 310, 3);
				Actor_Says(kActorClovis, 320, 3);
				if (!Game_Flag_Query(kFlagDirectorsCut) && Global_Variable_Query(kVariableChapter) < 3) {
					Actor_Voice_Over(1300, kActorVoiceOver);
					Actor_Voice_Over(1310, kActorVoiceOver);
					Actor_Voice_Over(1320, kActorVoiceOver);
				}
				Actor_Says(kActorMcCoy, 2445, 13);
				Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
				Game_Flag_Set(kFlagMA04PhoneMessageFromClovis);
				return true;
			}
			if (Actor_Clue_Query(kActorLucy, kClueMcCoyLetZubenEscape) && !Game_Flag_Query(kFlagMA04PhoneMessageFromLucy)) {
				Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
				Overlay_Remove("MA04OVER");
				Delay(500);
				Actor_Says(kActorLucy, 500, 3);
				Actor_Says(kActorLucy, 510, 3);
				if (!Game_Flag_Query(kFlagDirectorsCut) && Global_Variable_Query(kVariableChapter) < 3) {
					Actor_Voice_Over(1330, kActorVoiceOver);
					Actor_Voice_Over(1340, kActorVoiceOver);
					Actor_Voice_Over(1350, kActorVoiceOver);
				}
				Actor_Says(kActorMcCoy, 2445, 13);
				Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
				Game_Flag_Set(kFlagMA04PhoneMessageFromLucy);
				return true;
			}
			Actor_Says(kActorMcCoy, 2670, 13);
			if (!Game_Flag_Query(kFlagDirectorsCut)) {
				Actor_Says(kActorMcCoy, 2675, 17);
			}
		}
		return true;
	}
	return false;
}

void SceneScriptMA04::SceneFrameAdvanced(int frame) {
	Set_Fade_Color(0, 0, 0);
	if (frame >= 91 && frame < 121) {
		Set_Fade_Density((frame - 91) / 30.0f);
	} else if (frame >= 121 && frame < 151) {
		Set_Fade_Density((151 - frame) / 30.0f);
	} else {
		Set_Fade_Density(0.0f);
	}
	if (frame == 121 && (Game_Flag_Query(kFlagZubenRetired) || Game_Flag_Query(kFlagZubenSpared)) && !Game_Flag_Query(kFlagPS04GuzzaTalkZubenRetired)) {
		Sound_Play(kSfxVIDFONE1, 50, 0, 0, 50);
	}
}

void SceneScriptMA04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptMA04::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagMA04McCoySleeping)) {
		Player_Gains_Control();
	}
	if (isPhoneMessageWaiting() || isPhoneRinging()) {
		Overlay_Play("MA04OVER", 0, true, false, 0);
	}
	if (Game_Flag_Query(kFlagMA04McCoySleeping)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7139.0f, 954.0f, 1746.0f, 0, true, false, false);
	} else if (Game_Flag_Query(kFlagMA02ToMA04)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7143.0f, 954.0f, 1868.0f, 0, true, false, false);
	}
	Game_Flag_Reset(kFlagMA02ToMA04);
	Game_Flag_Reset(kFlagMA05toMA04);
	Game_Flag_Reset(kFlagMA04McCoySleeping);
	if (Game_Flag_Query(kFlagChapter1Ended)) {
		if (Global_Variable_Query(kVariableChapter) == 2 && !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallGuzza)) {
			Sound_Play(kSfxVIDFONE1, 100, 0, 0, 50);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7176.0f, 954.0f, 1806.0f, 0, false, false, false);
			Actor_Face_Heading(kActorMcCoy, 256, true);
			Actor_Says(kActorMcCoy, 2680, kAnimationModeIdle);
			Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
			Delay(500);
			Actor_Says(kActorGuzza, 0, 3);
			Actor_Says(kActorMcCoy, 2685, 13);
			Actor_Says(kActorGuzza, 10, 3);
			Actor_Says(kActorMcCoy, 2690, 17);
			Actor_Says(kActorGuzza, 30, 3);
			Actor_Says(kActorMcCoy, 2695, 12);
			Actor_Says(kActorGuzza, 40, 3);
			Actor_Says(kActorGuzza, 50, 3);
			Actor_Says(kActorMcCoy, 2700, 3);
			Actor_Says(kActorGuzza, 60, 3);
			Actor_Says(kActorGuzza, 70, 3);
			Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
			Actor_Clue_Acquire(kActorMcCoy, kCluePhoneCallGuzza, false, kActorGuzza);
			Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationTyrellBuilding, true);
			Game_Flag_Set(kFlagRC01PoliceDone);
			if (!Game_Flag_Query(kFlagRC01ChromeDebrisTaken)) {
				Game_Flag_Set(kFlagRC01ChromeDebrisTaken);
				Item_Remove_From_World(kItemChromeDebris);
			}
			Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyEndOfAct1);
			Actor_Put_In_Set(kActorOfficerLeary, kSetFreeSlotC);
			Actor_Set_At_Waypoint(kActorOfficerLeary, 35, 0); // kSetFreeSlotC
			Autosave_Game(0);
		}
		//return false;
		return;
	}
	if ((Game_Flag_Query(kFlagZubenRetired) || Game_Flag_Query(kFlagZubenSpared)) && !Game_Flag_Query(kFlagChapter1Ending)) {
		Music_Play(kMusicBRBlues, 52, 0, 2, -1, 0, 0);
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7199.0f, 955.0f, 1677.0f, 0, true, false, false);
		if (isPhoneMessageWaiting() || isPhoneRinging()) {
			Overlay_Remove("MA04OVER");
		}
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7199.0f, 955.0f, 1675.0f, 0, true, false, false);
		Game_Flag_Set(kFlagChapter1Ending);

		Async_Actor_Walk_To_XYZ(kActorMcCoy, -7204.0f, 956.17f, 1568.0f, 0, false);
		Set_Enter(kSetMA05, kSceneMA05);
	}
}

void SceneScriptMA04::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (Game_Flag_Query(kFlagChapter2Intro)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeMovieA, false, -1);
		Game_Flag_Reset(kFlagChapter2Intro);
	}
}

void SceneScriptMA04::DialogueQueueFlushed(int a1) {
	Overlay_Remove("MA04OVR2");
}

bool SceneScriptMA04::isPhoneRinging() {
	return Global_Variable_Query(kVariableChapter) == 5
	   && !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallClovis)
	   && !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallCrystal)
	   && !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallDektora1)
	   && !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallDektora2)
	   && !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallLucy1)
	   && !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallLucy2);
}

bool SceneScriptMA04::isPhoneMessageWaiting() {
	return (Actor_Clue_Query(kActorClovis, kClueMcCoyRetiredZuben) && !Game_Flag_Query(kFlagMA04PhoneMessageFromClovis))
	    || (Actor_Clue_Query(kActorLucy, kClueMcCoyLetZubenEscape) && !Game_Flag_Query(kFlagMA04PhoneMessageFromLucy));
}

void SceneScriptMA04::phoneCallWithDektora() {
	int answer;
	Actor_Says(kActorDektora, 220, 3);
	Actor_Says(kActorMcCoy, 2460, 0);
	Actor_Says(kActorDektora, 230, 3);
	Actor_Says(kActorDektora, 240, 3);
	Actor_Says(kActorMcCoy, 2465, 0);
	Actor_Says(kActorDektora, 250, 3);
	Actor_Says_With_Pause(kActorMcCoy, 2470, 1.5f, 17);
	Actor_Says(kActorDektora, 260, 3);
	Actor_Says(kActorMcCoy, 2475, 15);
	Actor_Says(kActorDektora, 270, 3);
	Actor_Says(kActorMcCoy, 2480, 0);
	Actor_Says(kActorDektora, 280, 3);
	Actor_Says(kActorDektora, 290, 3);
	Actor_Says(kActorMcCoy, 2485, 19);
	Actor_Says(kActorDektora, 300, 3);
	Actor_Says(kActorDektora, 310, 3);
	Actor_Says(kActorMcCoy, 2490, 0);
	Actor_Says(kActorDektora, 330, 3);
	Actor_Says(kActorMcCoy, 2495, 0);
	Actor_Says(kActorDektora, 340, 3);
	Actor_Says(kActorDektora, 350, 3);
	if (Game_Flag_Query(kFlagCrazylegsArrested)
	    || Actor_Query_Goal_Number(kActorCrazylegs) == kGoalCrazyLegsLeavesShowroom
	) {
		answer = 1170; // CLOVIS
	} else {
		Dialogue_Menu_Clear_List();
		DM_Add_To_List_Never_Repeat_Once_Selected(1160, 1, 1, 2); // OK
		DM_Add_To_List_Never_Repeat_Once_Selected(1170, 2, 1, 1); // CLOVIS
		Dialogue_Menu_Appear(320, 240);
		answer = Dialogue_Menu_Query_Input();
		Dialogue_Menu_Disappear();
	}
	if (answer == 1160) { // OK
		Actor_Says(kActorMcCoy, 2500, 19);
		Actor_Says(kActorDektora, 360, 3);
		Actor_Says(kActorMcCoy, 2510, 0);
		Actor_Says(kActorDektora, 370, 3);
		Actor_Says(kActorDektora, 380, 3);
		Actor_Says(kActorMcCoy, 2515, 12);
		Actor_Says(kActorDektora, 390, 3);
		Actor_Says(kActorMcCoy, 2520, 13);
		Actor_Says(kActorDektora, 400, 3);
		Actor_Says(kActorDektora, 410, 3);
		Actor_Says(kActorMcCoy, 2525, 15);
		Actor_Says(kActorDektora, 420, 3);
		Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
		Actor_Clue_Acquire(kActorMcCoy, kCluePhoneCallDektora1, true, -1);
	} else {
		Actor_Says_With_Pause(kActorMcCoy, 2505, 0.5f, 19);
		Actor_Says(kActorDektora, 430, 3);
		Actor_Says(kActorDektora, 440, 3);
		Actor_Says(kActorMcCoy, 2530, 0);
		Actor_Says(kActorDektora, 450, 3);
		Actor_Says(kActorMcCoy, 2535, 12);
		Actor_Says(kActorDektora, 460, 3);
		Actor_Says_With_Pause(kActorDektora, 470, 1.0f, 3);
		Actor_Says(kActorDektora, 480, 3);
		Actor_Says(kActorDektora, 490, 3);
		Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
		Actor_Says(kActorMcCoy, 2540, 15);
		Actor_Clue_Acquire(kActorMcCoy, kCluePhoneCallDektora2, true, -1);
	}
}

void SceneScriptMA04::phoneCallWithLucy() {
	Actor_Says(kActorLucy, 530, 3);
	Actor_Says(kActorMcCoy, 2545, 19);
	Actor_Says(kActorLucy, 540, 3);
	Actor_Says(kActorLucy, 550, 3);
	Actor_Says(kActorMcCoy, 2550, 13);
	Actor_Says(kActorLucy, 560, 3);
	Actor_Says(kActorMcCoy, 2555, 19);
	Actor_Says(kActorLucy, 570, 3);
	Actor_Says(kActorMcCoy, 2560, 17);
	Actor_Says(kActorLucy, 580, 3); // You promise?
	if (Game_Flag_Query(kFlagCrazylegsArrested)
	    || Actor_Query_Goal_Number(kActorCrazylegs) == kGoalCrazyLegsLeavesShowroom
	) {
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Says(kActorLucy, 630, 3); // I'll meet you there, okay? At the place where he sells the cars.
		Actor_Says_With_Pause(kActorMcCoy, 2575, 0.0f, 15); // Wait, Lucy!
		if (!Game_Flag_Query(kFlagDirectorsCut)) {
			Actor_Says(kActorLucy, 640, 3);
		}
#else
		// When CrazyLegs is gone from his shop,
		// it does not make sense for Lucy to say "At the place where he sells the cars"
		Actor_Says_With_Pause(kActorMcCoy, 2570, 0.0f, 13); // Lucy, there's a good chance--
		if (!Game_Flag_Query(kFlagDirectorsCut)) {
			Actor_Says(kActorLucy, 640, 3);
		}
		Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50); // (Lucy hangs up)
		Actor_Says(kActorMcCoy, 2575, 15); // Wait, Lucy!
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Clue_Acquire(kActorMcCoy, kCluePhoneCallLucy2, true, -1);
	} else {
		Actor_Says(kActorLucy, 590, 3);
		Actor_Says(kActorMcCoy, 2565, 12);
		Actor_Says(kActorLucy, 600, 3);
		Actor_Says(kActorLucy, 610, 3);
		Actor_Says(kActorLucy, 620, 3);
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Says(kActorMcCoy, 2570, 13);
#else
		// McCoy is interrupted here, so use Actor_Says_With_Pause
		Actor_Says_With_Pause(kActorMcCoy, 2570, 0.0f, 13);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Says_With_Pause(kActorLucy, 630, 0.0f, 3);
		Actor_Says_With_Pause(kActorMcCoy, 2575, 0.0f, 15);
		if (!Game_Flag_Query(kFlagDirectorsCut)) {
			Actor_Says(kActorLucy, 640, 3);
		}
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Clue_Acquire(kActorMcCoy, kCluePhoneCallLucy1, true, -1);
	}
#if BLADERUNNER_ORIGINAL_BUGS
	Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptMA04::phoneCallWithSteele() {
	Actor_Says(kActorSteele, 680, 3);
	Actor_Says(kActorMcCoy, 2630, 17);
	Actor_Says(kActorSteele, 690, 3);
	Actor_Says(kActorMcCoy, 2635, 18);
	Actor_Says(kActorSteele, 700, 3);
	Actor_Says(kActorMcCoy, 2640, 14);
	Actor_Says(kActorSteele, 710, 3);
	Actor_Says(kActorSteele, 720, 3);
	Actor_Says(kActorMcCoy, 2645, 13);
	Actor_Says(kActorSteele, 740, 3);
	Actor_Says(kActorSteele, 750, 3);
	Actor_Says(kActorMcCoy, 2650, 12);
	Actor_Says(kActorSteele, 760, 3);
	Actor_Says(kActorMcCoy, 2665, 13);
	Actor_Says(kActorSteele, 810, 3);
	Actor_Says(kActorSteele, 820, 3);
	Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
	Actor_Clue_Acquire(kActorMcCoy, kCluePhoneCallCrystal, true, -1);
}

void SceneScriptMA04::phoneCallWithClovis() {
	Actor_Says(kActorClovis, 330, 3);
	Actor_Says(kActorMcCoy, 2580, 14);
	Actor_Says(kActorClovis, 340, 3);
	Actor_Says(kActorMcCoy, 2585, 19);
	Actor_Says(kActorClovis, 350, 3);
	Actor_Says(kActorClovis, 360, 3);
	Actor_Says(kActorMcCoy, 2590, 18);
	Actor_Says(kActorClovis, 370, 3);
	Actor_Says(kActorMcCoy, 2595, 15);
	Actor_Says(kActorClovis, 390, 3);
	Actor_Says(kActorClovis, 400, 3);
	Actor_Says(kActorClovis, 410, 3);
	Actor_Says(kActorMcCoy, 2600, 15);
	Actor_Says_With_Pause(kActorClovis, 420, 1.5f, 3);
	Actor_Says(kActorMcCoy, 2605, 17);
	Actor_Says(kActorClovis, 430, 3);
	Actor_Says(kActorClovis, 440, 3);
	Actor_Says(kActorMcCoy, 2610, 3);
	Actor_Says(kActorClovis, 450, 3);
	Actor_Says(kActorClovis, 460, 3);
	Actor_Says(kActorClovis, 470, 3);
	Actor_Says(kActorClovis, 480, 3);
	Actor_Says(kActorClovis, 490, 3);
	Actor_Says(kActorMcCoy, 2615, 17);
	Actor_Says(kActorClovis, 500, 3);
	Actor_Says(kActorClovis, 530, 3);
	Actor_Says(kActorClovis, 540, 3);
	Sound_Play(kSfxSPNBEEP9, 100, 0, 0, 50);
	Actor_Clue_Acquire(kActorMcCoy, kCluePhoneCallClovis, true, -1);
}

void SceneScriptMA04::turnOnTV() {
	Overlay_Play("MA04OVR2", 0, true, false, 0);
	switch (Global_Variable_Query(kVariableNextTvNews)) {
	case 0:
		ADQ_Add(kActorNewscaster, 0, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 10, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 20, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 30, kAnimationModeTalk);
		ADQ_Add(kActorTyrell, 430, kAnimationModeTalk);
		ADQ_Add(kActorTyrell, 440, kAnimationModeTalk);
		ADQ_Add(kActorTyrell, 450, kAnimationModeTalk);
		ADQ_Add(kActorTyrell, 460, kAnimationModeTalk);
		break;
	case 1:
		ADQ_Add(kActorNewscaster, 40, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 50, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 60, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 70, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 80, kAnimationModeTalk);
		break;
	case 2:
		if (Actor_Query_Friendliness_To_Other(kActorClovis, kActorMcCoy) <= Actor_Query_Friendliness_To_Other(kActorSteele, kActorMcCoy)) {
			ADQ_Add(kActorNewscaster, 90, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 100, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 110, kAnimationModeTalk);
			ADQ_Add(kActorGuzza, 1540, kAnimationModeTalk);
			ADQ_Add(kActorGuzza, 1550, kAnimationModeTalk);
			ADQ_Add(kActorGuzza, 1560, kAnimationModeTalk);
		} else {
			ADQ_Add(kActorNewscaster, 120, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 130, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 140, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 150, kAnimationModeTalk);
			ADQ_Add(kActorGuzza, 1570, kAnimationModeTalk);
			ADQ_Add(kActorGuzza, 1580, kAnimationModeTalk);
			ADQ_Add(kActorGuzza, 1590, kAnimationModeTalk);
		}
		break;
	case 3:
		ADQ_Add(kActorNewscaster, 170, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 180, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 190, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 200, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 210, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 220, kAnimationModeTalk);
		ADQ_Add(kActorGovernorKolvig, 80, kAnimationModeTalk);
		ADQ_Add(kActorGovernorKolvig, 90, kAnimationModeTalk);
		ADQ_Add(kActorGovernorKolvig, 100, kAnimationModeTalk);
		ADQ_Add(kActorGovernorKolvig, 110, kAnimationModeTalk);
		ADQ_Add(kActorGovernorKolvig, 120, kAnimationModeTalk);
		ADQ_Add(kActorGovernorKolvig, 130, kAnimationModeTalk);
		break;
	case 4:
		ADQ_Add(kActorNewscaster, 230, kAnimationModeTalk);
		ADQ_Add(kActorNewscaster, 240, kAnimationModeTalk);
		break;
	}
}

void SceneScriptMA04::sleep() {
	if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "BED-SHEETS", 12, true, false)) {
		Actor_Says(kActorMcCoy, 8530, 12);
		Music_Stop(4);
		if (isPhoneMessageWaiting() || isPhoneRinging()) {
			Overlay_Remove("MA04OVER");
		}
		Player_Loses_Control();
		Game_Flag_Set(kFlagMA04McCoySleeping);
		if ((Game_Flag_Query(kFlagZubenRetired) || Game_Flag_Query(kFlagZubenSpared)) && Global_Variable_Query(kVariableChapter) == 1) {
			if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenGone) {
				Actor_Put_In_Set(kActorZuben, kSetFreeSlotA);
				Actor_Set_At_Waypoint(kActorZuben, 33, 0);
			}
			Game_Flag_Set(kFlagChapter2Intro);
			Global_Variable_Set(kVariableChapter, 2);
			Chapter_Enter(2, kSetMA02_MA04, kSceneMA04);
#if BLADERUNNER_ORIGINAL_BUGS
			if (Query_Difficulty_Level() != kGameDifficultyEasy) {
				if (!Game_Flag_Query(kFlagZubenBountyPaid)) {
					Global_Variable_Increment(kVariableChinyen, 200);
				}
			}
#else // ensure valid kFlagZubenBountyPaid flag state
			// NOTE If not for the "Report Im" to Guzza, it would be simpler to have McCoy get the money as soon as he retires Zuben
			//		so that would be in a single place in the code
			if (!Game_Flag_Query(kFlagZubenBountyPaid) && Game_Flag_Query(kFlagZubenRetired)) { // get retirement money at end of day 1 only if Zuben was retired.
				if (Query_Difficulty_Level() != kGameDifficultyEasy) {
					Global_Variable_Increment(kVariableChinyen, 200);
				}
				Game_Flag_Set(kFlagZubenBountyPaid); // not a proper bug, but was missing from original code, so the flag would remain in non-consistent state in this case
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
		} else {
			Set_Enter(kSetMA02_MA04, kSceneMA04);
		}
		Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kMA04LoopSleep, false);
	}
}

} // End of namespace BladeRunner
