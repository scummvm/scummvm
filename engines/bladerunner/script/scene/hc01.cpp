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

void SceneScriptHC01::InitializeScene() {
	Music_Play(kMusicArabLoop, 31, 0, 2, -1, 1, 2);
	if (Game_Flag_Query(kFlagHC02toHC01)) {
		Setup_Scene_Information( 64.0f, 0.14f,  83.0f, 266);
	} else if (Game_Flag_Query(kFlagHC03toHC01)) {
		Setup_Scene_Information(607.0f, 0.14f,   9.0f, 530);
	} else {
		Setup_Scene_Information(780.0f, 0.14f, 153.0f, 815);
	}

	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	if (Game_Flag_Query(kFlagHC03Available)) {
		Scene_Exit_Add_2D_Exit(1, 394, 229, 485, 371, 1);
	}
	Scene_Exit_Add_2D_Exit(2, 117, 0, 286, 319, 0);
	if (_vm->_cutContent && !Game_Flag_Query(kFlagMcCoyCommentsOnAnimoids)) {
		Scene_2D_Region_Add(0, 110, 385, 200, 450); // cage 1
		Scene_2D_Region_Add(1, 20, 249, 110, 319);  // cage 2
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 50, 50, 0);
	Ambient_Sounds_Add_Looping_Sound(kSfxHCLOOP1,  50, 50, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM8,   3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM2,   3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM3,   3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM4,   3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM5,   3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM6,   3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM7,   3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHCANM1,   3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
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
	Scene_Loop_Set_Default(0);
}

void SceneScriptHC01::SceneLoaded() {
	Obstacle_Object("PILLAR", true);
	if (Game_Flag_Query(kFlagAR01toHC01)) {
		Preload(kModelAnimationMcCoyIdle);
		Preload(kModelGenWalkerHattedPersonWithUmbrellaStandsStill);
		Preload(kModelGenWalkerHoodedPersonWithUmbrellaStandsStill);
		Preload(kModelGenWalkerHattedLadyWithWoodenUmbrellaStandsStill);
		Preload(kModelGenWalkerHattedPersonNoUmbrellaStandsStill);
		Preload(kModelGenWalkerPunkPersonWithGlassesAndBeardStandsStill);
		Preload(kModelGenWalkerPunkPersonWithGlassesStandsStill);
		Preload(kModelGenWalkerHattedPersonWithUmbrella);
		Preload(kModelGenWalkerHoodedPersonWithUmbrella);
		Preload(kModelGenWalkerHattedPersonWithWoodenUmbrella);
		Preload(kModelGenWalkerPunkPersonWithGlassesAndBeard);
		Preload(kModelGenWalkerHattedPersonNoUmbrellaSmallSteps);
		Preload(kModelGenWalkerPunkPersonWithGlasses);
	}
}

bool SceneScriptHC01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHC01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptHC01::ClickedOnActor(int actorId) {
	if (actorId == kActorIzo
	 && (Actor_Query_Goal_Number(kActorIzo) == kGoalIzoGoToHC01
	  || Actor_Query_Goal_Number(kActorIzo) == kGoalIzoDefault
	 )
	) {
		AI_Movement_Track_Pause(kActorIzo);
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 624.43f, 0.14f, 83.0f, 0, true, false, false)) {
			if (!Game_Flag_Query(kFlagHC01IzoTalk1)) {
				Actor_Face_Actor(kActorIzo, kActorMcCoy, true);
				if (_vm->_cutContent) {
					Actor_Says_With_Pause(kActorIzo,  0, 0.2f, 13);
				}
				Actor_Says_With_Pause(kActorIzo, 10, 0.2f, 13);
				Actor_Face_Actor(kActorMcCoy, kActorIzo, true);
				Actor_Says(kActorIzo, 20, 17);
				Actor_Says(kActorMcCoy, 1035, 18);
				Actor_Says_With_Pause(kActorIzo, 30, 0.2f, 17);
				Actor_Says_With_Pause(kActorIzo, 40, 0.0f, 13);
				Actor_Says(kActorIzo, 50, 12);
				Actor_Says_With_Pause(kActorMcCoy, 1040, 1.2f, 13);
				Actor_Says(kActorIzo, 60, 16);
				Actor_Says_With_Pause(kActorIzo, 70, 1.0f, 13);
				Actor_Says_With_Pause(kActorMcCoy, 1045, 0.6f, 14);
				Actor_Says(kActorIzo, 80, 18);
				Game_Flag_Set(kFlagHC01IzoTalk1);
			} else {
				Actor_Face_Actor(kActorMcCoy, kActorIzo, true);
				Actor_Face_Actor(kActorIzo, kActorMcCoy, true);
				dialogueWithIzo();
			}
		}
		AI_Movement_Track_Unpause(kActorIzo);
	}
#if BLADERUNNER_ORIGINAL_BUGS
#else
	// barkeep is click-able from afar
	// Add behavior similar to Howie Lee (CT12), Hasan, and Insect Dealer (AR01).
	else if (actorId == kActorHawkersBarkeep) {
		Actor_Face_Actor(kActorMcCoy, actorId, true);
		Actor_Says(kActorMcCoy, 8910, 14);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
	return false;
}

bool SceneScriptHC01::ClickedOnItem(int itemId, bool a2) {
	if (itemId == kItemCamera) {
		Item_Remove_From_World(kItemCamera);
		Item_Pickup_Spin_Effect(kModelAnimationIzoCamera, 361, 381);
		Delay(1500);
		Item_Pickup_Spin_Effect(kModelAnimationPhoto, 377, 397);
		Delay(1500);
		Item_Pickup_Spin_Effect(kModelAnimationPhoto, 330, 384);
		if (Game_Flag_Query(kFlagAR02DektoraBoughtScorpions)) {
			Actor_Clue_Acquire(kActorMcCoy, kCluePhotoOfMcCoy1, true, kActorIzo);
		} else {
			Actor_Clue_Acquire(kActorMcCoy, kCluePhotoOfMcCoy2, true, kActorIzo);
		}
		Actor_Clue_Acquire(kActorMcCoy, kClueChinaBar, true, kActorIzo);
		return true;
	}
	return false;
}

bool SceneScriptHC01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 814.0f, 0.14f, 153.0f, 0, true, false, false)) {
			Music_Adjust(12, 0, 2);
			Game_Flag_Set(kFlagHC01toAR01);
			Set_Enter(kSetAR01_AR02, kSceneAR01);
			Game_Flag_Reset(kFlagMcCoyInHawkersCircle);
			Game_Flag_Set(kFlagMcCoyInAnimoidRow);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 607.0f, 0.14f, 9.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagHC01toHC03);
			Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC03);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 105.0f, 0.14f, 103.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagHC01toHC02);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -57.0f, 0.14f, 83.0f, 0, false);
			Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC02);
		}
		return true;
	}
	return false;
}

bool SceneScriptHC01::ClickedOn2DRegion(int region) {
	if (_vm->_cutContent) {
		if (!Game_Flag_Query(kFlagMcCoyCommentsOnAnimoids) && (region == 0 || region == 1) ) {
			Game_Flag_Set(kFlagMcCoyCommentsOnAnimoids);
			//Actor_Face_Heading(kActorMcCoy, 389, false);
			Actor_Face_XYZ(kActorMcCoy, 740.89f, 60.29f, 220.12f, true);
			Actor_Voice_Over(890, kActorVoiceOver);
			Actor_Voice_Over(900, kActorVoiceOver);
			Scene_2D_Region_Remove(0);
			Scene_2D_Region_Remove(1);
			return true;
		}
	}
	return false;
}

void SceneScriptHC01::SceneFrameAdvanced(int frame) {
	Set_Fade_Color(1.0f, 1.0f, 1.0f);
	if (frame >= 61
	 && frame < 65
	) {
		Set_Fade_Density((frame - 61) / 4.0f);
	} else if (frame >= 65
	        && frame < 93
	) {
		Set_Fade_Density(1.0f);
	} else if (frame >= 93
	        && frame < 106
	) {
		Set_Fade_Density((105 - frame) / 13.0f);
	} else {
		Set_Fade_Density(0.0f);
	}

	if (frame == 61) {
		Ambient_Sounds_Play_Sound(kSfxCAMERA2,  90,   0,   0, 0);
	}

	if (frame == 65) {
		Ambient_Sounds_Play_Sound(kSfxRUNAWAY1, 50,   0, 100, 0);
	}

	if (frame == 80) {
		Ambient_Sounds_Play_Sound(kSfxTRPDOOR1, 40, 100, 100, 0);
		Item_Add_To_World(kItemGreenPawnLock, kModelAnimationBadge, kSetHC01_HC02_HC03_HC04, 582.0f, 27.0f, -41.0f, 0, 8, 8, true, true, false, true); // TODO a bug? reusing still animation of kModelAnimationBadge
	}
}

void SceneScriptHC01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHC01::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagHC02toHC01)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 105.0f, 0.14f, 103.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagHC02toHC01);
	}

	if (Game_Flag_Query(kFlagHC03toHC01)) {
		Game_Flag_Reset(kFlagHC03toHC01);
	}

	if (Game_Flag_Query(kFlagAR01toHC01)) {
		Game_Flag_Reset(kFlagAR01toHC01);
	}
}

void SceneScriptHC01::PlayerWalkedOut() {
	Set_Fade_Density(0.0f);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptHC01::DialogueQueueFlushed(int a1) {
}

void SceneScriptHC01::dialogueWithIzo() {
	if (!Game_Flag_Query(kFlagHC01IzoTalk2)) {
		Actor_Says(kActorMcCoy, 1055, 13);
		Actor_Says(kActorIzo, 130, 13);
		Actor_Says_With_Pause(kActorMcCoy, 1060, 0.2f, 13);
		Actor_Says(kActorIzo, 140, 13);
		Game_Flag_Set(kFlagHC01IzoTalk2);
	}

	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(kActorMcCoy, kCluePeruvianLadyInterview)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1020, 6, 7, 3); // DRAGONFLY JEWERLY
	} else if (Actor_Clue_Query(kActorMcCoy, kClueDragonflyEarring)
	        || Actor_Clue_Query(kActorMcCoy, kClueBombingSuspect)
	        || Actor_Clue_Query(kActorMcCoy, kClueDragonflyAnklet)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1010, 6, 7, 3); // INSECT JEWELRY
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueBobInterview1)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1030, 1, 5, 7); // WEAPONS
	} else if (Actor_Clue_Query(kActorMcCoy, kClueShellCasings)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1040, 4, 4, 6); // SHELL CASINGS
	}
#if BLADERUNNER_ORIGINAL_BUGS
	if (Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB2)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1050, -1, 3, 8); // GRIGORIAN 1
	} else if (Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB1)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1060, -1, 3, 8); // GRIGORIAN 2
	}
#else
	// When McCoy has kClueGrigorianInterviewB1 then Izo is a Replicant
	// and when he has kClueGrigorianInterviewB2 then Izo is a human
	// However the dialogue menu options are actually reversed in the original
	// and inconsistent with what Grigorian says in his interviews.
	// The 1050 dialogue menu option belongs to the case where Izo is a Replicant
	// because in that dialogue McCoy mentions that Grigorian described him to a tee (which he does in kClueGrigorianInterviewB1)
	// And the 1060 dialogue menu option belongs to the case where Izo is a human
	// because in that dialogue McCoy talks about how Izo is a psychopath and was thrown out of CARS
	// which is what Grigorian says in kClueGrigorianInterviewB2
	if (Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB1)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1050, -1, 3, 8); // GRIGORIAN 1
	} else if (Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB2)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1060, -1, 3, 8); // GRIGORIAN 2
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (Dialogue_Menu_Query_List_Size() == 0) {
		Actor_Says_With_Pause(kActorMcCoy, 1105, 1.2f, 13);
		if (Actor_Query_Friendliness_To_Other(kActorIzo, kActorMcCoy) < 50) {
			Actor_Says(kActorIzo, 550, 15);
			if (_vm->_cutContent) {
				Actor_Says(kActorMcCoy, 1220, 14); // you can bet...
				Actor_Says(kActorIzo, 560, kAnimationModeTalk); // i'll be here
			}
		} else {
			Actor_Says(kActorIzo, 250, 13);
			Actor_Modify_Friendliness_To_Other(kActorIzo, kActorMcCoy, -1);
			if (Actor_Query_Friendliness_To_Other(kActorIzo, kActorMcCoy) < 47
			 && Query_Difficulty_Level() == kGameDifficultyEasy
			) {
				takePhotoAndRunAway();
			}
		}
		return;
	}

	Dialogue_Menu_Add_DONE_To_List(100); // DONE

	bool end = false;
	do {
		Dialogue_Menu_Appear(320, 240);
		int answer = Dialogue_Menu_Query_Input();
		Dialogue_Menu_Disappear();
		if (answer == 1010) { // INSECT JEWELRY
			Dialogue_Menu_Remove_From_List(1010);
			Actor_Clue_Acquire(kActorMcCoy, kClueIzoInterview, false, kActorIzo);
			Actor_Says(kActorMcCoy, 1070, 13);
			Actor_Says(kActorIzo, 200, 17);
			Actor_Says(kActorIzo, 210, 12);
			Actor_Says(kActorMcCoy, 1115, 12);
			Actor_Says(kActorIzo, 220, 16);
			Actor_Says(kActorIzo, 230, kAnimationModeTalk);
			Actor_Says(kActorIzo, 240, 15);
			if (Query_Difficulty_Level() < kGameDifficultyHard) {
				Actor_Modify_Friendliness_To_Other(kActorIzo, kActorMcCoy, -1);
			}
		}
		if (answer == 1020) { // DRAGONFLY JEWERLY
			Dialogue_Menu_Remove_From_List(1020);
			Actor_Says(kActorMcCoy, 1065, 15);
			if (_vm->_cutContent && Game_Flag_Query(kFlagIzoIsReplicant)) {
				// Restored: if Izo is a Replicant, he would probably lie
				// so this line goes here
				Actor_Says(kActorIzo, 150, kAnimationModeTalk);
			} else {
				Actor_Says(kActorIzo, 160, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 1110, 16);
				Actor_Says(kActorIzo, 170, kAnimationModeTalk);
				Actor_Says(kActorIzo, 180, kAnimationModeTalk);
				Actor_Says(kActorIzo, 190, 12);
			}
			if (Query_Difficulty_Level() < kGameDifficultyHard) {
				Actor_Modify_Friendliness_To_Other(kActorIzo, kActorMcCoy, -2);
			}
		}
		if (answer == 1010 // INSECT JEWELRY
		 || answer == 1020 // DRAGONFLY JEWERLY
		) {
			Actor_Says_With_Pause(kActorMcCoy, 1120, 0.9f, 17);
			Actor_Says(kActorIzo, 250, 13);
			Actor_Says(kActorMcCoy, 1125, 14);
			if (Actor_Query_Friendliness_To_Other(kActorIzo, kActorMcCoy) < 47) {
				takePhotoAndRunAway();
			}
			end = true;
		}
		if (answer == 1030) { // WEAPONS
			Dialogue_Menu_Remove_From_List(1030);
			Actor_Says(kActorMcCoy, 1075, 18);
			Actor_Says(kActorIzo, 260, 12);
			Actor_Says(kActorIzo, 270, 16);
			Actor_Says(kActorMcCoy, 1130, 14);
			Actor_Says(kActorIzo, 280, 17);
			Actor_Says(kActorMcCoy, 1135, 15);
			Actor_Says(kActorIzo, 290, 15);
			Actor_Says(kActorIzo, 300, 12);
			Actor_Says(kActorIzo, 310, 17);
			Actor_Says(kActorMcCoy, 1140, kAnimationModeTalk);
			if (Query_Difficulty_Level() < kGameDifficultyHard) {
				Actor_Modify_Friendliness_To_Other(kActorIzo, kActorMcCoy, -2);
			}
			if (Actor_Query_Friendliness_To_Other(kActorIzo, kActorMcCoy) < 47) {
				takePhotoAndRunAway();
			}
			end = true;
		}
		if (answer == 1040) { // SHELL CASINGS
			Dialogue_Menu_Remove_From_List(1040);
			Actor_Says(kActorMcCoy, 1080, 15);
			Actor_Says(kActorMcCoy, 1085, 17);
			Actor_Says(kActorIzo, 320, 17);
			Actor_Says(kActorMcCoy, 1145, 13);
			Actor_Says(kActorIzo, 330, 17);
			Actor_Says(kActorIzo, 340, 13);
			Actor_Says(kActorIzo, 350, 12);
			end = true;
		}
		if (answer == 1050) { // GRIGORIAN 1 // Izo is Replicant
			Dialogue_Menu_Remove_From_List(1050);
			Actor_Says(kActorMcCoy, 1090, 18); // Ever consort with a group called CARS? C.A.R.S.?
			Actor_Says(kActorIzo, 360, 14);
			Actor_Says(kActorMcCoy, 1150, 17);
			Actor_Says(kActorIzo, 370, 13);
			Actor_Says(kActorMcCoy, 1155, 15);
			Actor_Says(kActorIzo, 380, 12);
			Actor_Says(kActorMcCoy, 1160, 14); // He described you to a tee
			Actor_Says(kActorMcCoy, 1165, 18); // Even down to that stupid little ponytail you got.
			Actor_Says(kActorIzo, 390, 16);
			Actor_Says(kActorMcCoy, 1170, 12); // What would you say if I told you Grigorian named you
			Actor_Says(kActorIzo, 400, 13);
			Actor_Says(kActorMcCoy, 1180, 14); // So, you're denying all involvement?
			Actor_Says(kActorIzo, 410, 12);
			Actor_Says(kActorIzo, 420, 16);
			Actor_Says(kActorIzo, 430, 17);
			Actor_Says(kActorIzo, 440, 13);
			Actor_Modify_Friendliness_To_Other(kActorIzo, kActorMcCoy, -4);
			if (Actor_Query_Friendliness_To_Other(kActorIzo, kActorMcCoy) < 47) {
				takePhotoAndRunAway();
			}
			end = true;
		}
		if (answer == 1060) { // GRIGORIAN 2 - Izo is a human, ex member of CARS
			Dialogue_Menu_Remove_From_List(1060);
			Actor_Says(kActorMcCoy, 1095, 15);
			Actor_Says_With_Pause(kActorMcCoy, 1100, 1.2f, 18); // That go for your old buddy Spencer Grigorian, too?
			Actor_Says(kActorIzo, 450, 12);
			Actor_Says(kActorIzo, 460, 13);
			Actor_Says(kActorMcCoy, 1185, 18); // Calfskin?
			Actor_Says(kActorIzo, 470, 14);
			Actor_Says(kActorMcCoy, 1190, 14); // Grigorian said you were thrown out of C.A.R.S.
			Actor_Says(kActorIzo, 480, 13);
			Actor_Says(kActorMcCoy, 1195, 16);
			Actor_Says(kActorMcCoy, 1200, 18);
			Actor_Says(kActorIzo, 490, 12);
			Actor_Says(kActorMcCoy, 1205, 14);
			Actor_Says(kActorIzo, 500, 14);
			Actor_Says(kActorIzo, 510, 17);
			Actor_Says(kActorIzo, 520, 16);
			Actor_Says(kActorIzo, 530, 15);
			Actor_Says(kActorMcCoy, 1210, 16);
			Actor_Modify_Friendliness_To_Other(kActorIzo, kActorMcCoy, -4);
			if (Actor_Query_Friendliness_To_Other(kActorIzo, kActorMcCoy) < 47) {
				takePhotoAndRunAway();
			}
			end = true;
		}
		if (answer == 100) { // DONE
			if (_vm->_cutContent) {
				Actor_Says(kActorMcCoy, 1215, 16); // All right
			}
			end = true;
		}
	} while (!end);
}

void SceneScriptHC01::takePhotoAndRunAway() {
	Actor_Set_Goal_Number(kActorIzo, kGoalIzoPrepareCamera);
	Player_Loses_Control();
	Actor_Says(kActorIzo, 90, kAnimationModeTalk);
	Actor_Face_Actor(kActorIzo, kActorMcCoy, true);
	Actor_Says(kActorIzo, 100, kAnimationModeTalk);
	Actor_Says(kActorIzo, 110, kAnimationModeTalk);
	Actor_Says_With_Pause(kActorMcCoy, 1050, 0.2f, kAnimationModeTalk);
	Actor_Says(kActorIzo, 120, kAnimationModeTalk);
	Actor_Set_Goal_Number(kActorIzo, kGoalIzoTakePhoto);
}

} // End of namespace BladeRunner
