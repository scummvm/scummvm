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

enum kHF05Loops {
	kHF05LoopMainLoopNoSpinner = 0,
	kHF05LoopMainLoopSpinner   = 2,
	kHF05LoopArmAnimation      = 4,
	kHF05LoopMainLoopHole      = 5
};

void SceneScriptHF05::InitializeScene() {
	if (Game_Flag_Query(kFlagHF06toHF05)) {
		Setup_Scene_Information(257.0f, 40.63f, 402.0f, 1000);
	} else if (Game_Flag_Query(kFlagHF07toHF05)) {
		Setup_Scene_Information(330.0f, 40.63f, -107.0f, 603);
	} else {
		Setup_Scene_Information(483.0f, 40.63f, -189.0f, 600);
	}

	Scene_Exit_Add_2D_Exit(0, 443, 270, 515, 350, 0);
	if (Global_Variable_Query(kVariableChapter) > 3) {
		Scene_Exit_Add_2D_Exit(1, 367, 298, 399, 349, 2);
	}
	Scene_Exit_Add_2D_Exit(2, 589,   0, 639, 479, 1);

	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 40, 1, 1);

	if (Game_Flag_Query(kFlagHF05Hole)) {
		Scene_Loop_Set_Default(kHF05LoopMainLoopHole);
		addAmbientSounds();
	} else if (Game_Flag_Query(kFlagHF06SteelInterruption)) {
		Scene_Loop_Set_Default(kHF05LoopMainLoopSpinner);
		addAmbientSounds();
	} else {
		Scene_Loop_Set_Default(kHF05LoopMainLoopNoSpinner);
	}
}

void SceneScriptHF05::SceneLoaded() {
	Obstacle_Object("MAINBASE", true);
	Unobstacle_Object("BTIRES02", true);
	Unobstacle_Object("LFTIRE02", true);
	if (Game_Flag_Query(kFlagHF05Hole)) {
		Unobstacle_Object("MONTE CARLO DRY", true);
	} else {
		Unobstacle_Object("OBSTACLE_HOLE", true);
	}
	Clickable_Object("TOP CON");
}

bool SceneScriptHF05::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHF05::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("TOP CON", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 95.0f, 40.63f, 308.0f, 0, true, false, false)) {
			Actor_Face_Object(kActorMcCoy, "TOP CON", true);
			if (Actor_Query_In_Set(kActorCrazylegs, kSetHF05)
			 && Actor_Query_Goal_Number(kActorCrazylegs) != kGoalCrazyLegsShotAndHit
			 && Actor_Query_Goal_Number(kActorCrazylegs) != kGoalCrazyLegsLeavesShowroom
			) {
				Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
				Actor_Says(kActorCrazylegs, 480, 13);
				return true;
			}

			if (!Game_Flag_Query(kFlagHF01TalkToLovedOne)
			 ||  Game_Flag_Query(kFlagHF05Hole)
			) {
				Actor_Change_Animation_Mode(kActorMcCoy, 23);
				Sound_Play(kSfxELEBAD1, 100, 0, 0, 50);
				return true;
			}

			Player_Loses_Control();
			Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyPoliceWait60SecondsToAttackHF05);
			Game_Flag_Set(kFlagHF05Hole);
			Game_Flag_Set(kFlagHF07Hole);
			Obstacle_Object("OBSTACLE_HOLE", true);
			Unobstacle_Object("MONTE CARLO DRY", true);

			if (getCompanionActor() != kActorMcCoy) {
				Loop_Actor_Walk_To_XYZ(getCompanionActor(), 181.54f, 40.63f, 388.09f, 0, false, true, false);
				Actor_Face_Actor(kActorMcCoy, getCompanionActor(), true);
				Actor_Face_Actor(getCompanionActor(), kActorMcCoy, true);
				Actor_Says(kActorMcCoy, 1785, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 1790, kAnimationModeTalk);
			}

			Actor_Face_Heading(kActorMcCoy, 0, false);
			Actor_Change_Animation_Mode(kActorMcCoy, 23);
			Scene_Loop_Set_Default(kHF05LoopMainLoopHole);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, kHF05LoopArmAnimation, true);


			if (getCompanionActor() == kActorMcCoy) {
				ADQ_Flush();
				ADQ_Add(kActorVoiceOver, 940, -1);
				Ambient_Sounds_Play_Sound(kSfxLABMISC2, 50, 99, 0, 0);
				Delay(1500);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, 181.54f, 40.63f, 388.09f, 0, false, true, false);
				Actor_Face_Heading(kActorMcCoy, 0, false);
				Actor_Change_Animation_Mode(kActorMcCoy, 23);
				Actor_Clue_Lose(kActorMcCoy, kClueBomb);
			} else {
				if (getCompanionActor() == kActorDektora) {
					Actor_Face_Heading(kActorDektora, 0, false);
					Ambient_Sounds_Play_Sound(kSfxLABMISC2, 50, 99, 0, 0);
					Delay(3000);
					Actor_Face_Heading(kActorDektora, 0, false);
					Actor_Change_Animation_Mode(kActorDektora, 23);
				} else {
					Actor_Face_Heading(kActorLucy, 0, false);
					Ambient_Sounds_Play_Sound(kSfxLABMISC2, 50, 99, 0, 0);
					Delay(3000);
					Actor_Face_Heading(kActorLucy, 0, false);
					Actor_Change_Animation_Mode(kActorLucy, 13);
				}
				Actor_Face_Actor(kActorMcCoy, getCompanionActor(), true);
				Actor_Says(kActorMcCoy, 1805, kAnimationModeTalk);
			}
			Player_Gains_Control();
		}
		return true;
	}
	return false;
}

bool SceneScriptHF05::ClickedOnActor(int actorId) {
	if (actorId == kActorCrazylegs) {
#if BLADERUNNER_ORIGINAL_BUGS
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorCrazylegs, 60, true, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
			Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
			dialogueWithCrazylegs1();
		}
#else
		// Don't (re)start a dialogue with CrayLegs if he is leaving or insulted by McCoy drawing his gun
		if (Actor_Query_Goal_Number(kActorCrazylegs) != kGoalCrazyLegsLeavesShowroom
		    && Actor_Query_Goal_Number(kActorCrazylegs) != kGoalCrazyLegsMcCoyDrewHisGun) {
			if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorCrazylegs, 60, true, false)) {
				Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
				Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
				dialogueWithCrazylegs1();
			}
		} else {
			Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
			Actor_Says(kActorMcCoy, 5560, 15); // Hey
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	return false;
}

bool SceneScriptHF05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptHF05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 483.0f, 40.63f, -189.0f, 0, true, false, false)) {
			if (!Game_Flag_Query(kFlagHF05PoliceAttacked)) {
				Game_Flag_Set(kFlagHF05toHF01);
				Set_Enter(kSetHF01, kSceneHF01);
			}
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 330.0f, 40.63f, -85.0f, 0, true, false, false)) {
			if (!Game_Flag_Query(kFlagHF05PoliceAttacked)) {
				int affectionTowardsActor = getAffectionTowardsActor();
				if (Game_Flag_Query(kFlagHF05PoliceArrived)
				 && Game_Flag_Query(kFlagHF07Hole)
				 && affectionTowardsActor != -1
				) {
					Actor_Face_Actor(kActorMcCoy, affectionTowardsActor, true);
					Actor_Says(kActorMcCoy, 1810, 16);
				}
				Game_Flag_Set(kFlagHF05toHF07);
				Set_Enter(kSetHF07, kSceneHF07);
			}
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 277.0f, 40.63f, 410.0f, 0, true, false, false)) {
			if (!Game_Flag_Query(kFlagHF05PoliceAttacked)) {
				Game_Flag_Set(kFlagHF05toHF06);
				Set_Enter(kSetHF06, kSceneHF06);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptHF05::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHF05::SceneFrameAdvanced(int frame) {
	switch (frame) {
	case 126:
		Sound_Play(kSfxMAGMOVE1, 90, -20,  70, 50);
		break;

	case 152:
		Sound_Play(kSfxDORSLID2, 90,   0,   0, 50);
		break;

	case 156:
		Sound_Play(kSfxLOWERN1,  47, 100, 100, 50);
		break;

	case 161:
		Sound_Play(kSfxDORSLID1, 90,   0,   0, 50);
		break;

	case 176:
		Sound_Play(kSfxMAGCHNK1, 32, 100, 100, 50);
		break;

	case 178:
		Sound_Play(kSfxRAISEY1,  47, 100, 100, 50);
		break;

	case 179:
		Sound_Play(kSfxCAREXPL1, 90,   0,   0, 50);
		Music_Play(kMusicBatl226M, 50, 0, 2, -1, 0, 0);
		break;

	case 186:
		Sound_Play(kSfxCARCREK1, 32, 100, 100, 50);
		break;

	case 209:
		Sound_Play(kSfxMAGMOVE2, 90, 100, -20, 50);
		break;

	case 243:
		Sound_Play(kSfxLOWERY1,  40, -20, -20, 50);
		break;

	case 261:
		Sound_Play(kSfxCARLAND1, 47, -20, -20, 50);
		break;

	case 268:
		Sound_Play(kSfxMAGDROP1, 58, -20, -20, 50);
		break;

	case 269:
		Sound_Play(kSfxRAISEN1,  43, -20, -20, 50);
		break;
	}
	//return true;
}

void SceneScriptHF05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == kActorOfficerLeary
	 && newGoal == kGoalOfficerLearyPoliceAboutToAttackHF05
	) {
		Game_Flag_Set(kFlagHF05PoliceAttacked);
		policeAttack();
	}
	//return false;
}

void SceneScriptHF05::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagHF01TalkToLovedOne)) {
		int affectionTowardsActor = getAffectionTowardsActor();
		if (Game_Flag_Query(kFlagHF01TalkToLovedOne)
		 && affectionTowardsActor != -1
		) {
			Actor_Put_In_Set(affectionTowardsActor, kSetHF05);
			Actor_Force_Stop_Walking(affectionTowardsActor);
			if (Game_Flag_Query(kFlagHF01toHF05)) {
				Actor_Set_At_XYZ(affectionTowardsActor, 506.81f, 40.63f, -140.92f, 0);
				Async_Actor_Walk_To_Waypoint(affectionTowardsActor, 437, 36, false);
			} else if (Game_Flag_Query(kFlagHF06toHF05)) {
				Actor_Set_At_XYZ(affectionTowardsActor, 288.0f, 40.63f, 410.0f, 909);
			} else if (Game_Flag_Query(kFlagHF07toHF05)) {
				Actor_Set_At_XYZ(affectionTowardsActor, 298.0f, 40.63f, -107.0f, 512);
			} else {
				Actor_Set_At_XYZ(affectionTowardsActor, 284.0f, 40.63f, 286.0f, 0);
			}
		}

		if (Game_Flag_Query(kFlagHF05PoliceAttacked)) {
			policeAttack();
		} else if (Actor_Clue_Query(kActorMcCoy, kClueSpinnerKeys)
		        || Game_Flag_Query(kFlagHF06SteelInterruption)
		) {
			if ( Game_Flag_Query(kFlagHF06SteelInterruption)
			 && !Game_Flag_Query(kFlagHF05PoliceArrived)
			) {
				Game_Flag_Set(kFlagHF05PoliceArrived);
				Music_Play(kMusicBatl226M, 40, 0, 2, -1, 0, 0);
				Actor_Says(kActorOfficerGrayford, 200, kAnimationModeTalk);
				Actor_Says(kActorOfficerGrayford, 210, kAnimationModeTalk);
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyPoliceWait120SecondsToAttackHF05);
				if (getCompanionActor() ==  kActorDektora) {
					talkWithDektora();
				} else if (getCompanionActor() == kActorLucy) {
					talkWithLucy();
				}
			}
		} else {
			talkWithCrazylegs3(affectionTowardsActor);
		}
	} else if (Game_Flag_Query(kFlagHF01toHF05)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 399.0f, 40.63f, -85.0f, 0, false, false, false);
	} else if (Game_Flag_Query(kFlagHF07toHF05)) {
		Actor_Set_At_XYZ(kActorMcCoy, 346.0f, 4.63f, -151.0f, 603);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 4, true, kAnimationModeIdle);
	}

	if (Actor_Query_In_Set(kActorCrazylegs, kSetHF05)) {
		if (!Game_Flag_Query(kFlagHF05CrazyLegsTalk1)) {
			talkWithCrazyLegs1();
			Game_Flag_Set(kFlagHF05CrazyLegsTalk1);
		} else if (!Game_Flag_Query(kFlagHF05CrazyLegsTalk2)
		        &&  Global_Variable_Query(kVariableChapter) == 3
		) {
			talkWithCrazyLegs2();
			Game_Flag_Set(kFlagHF05CrazyLegsTalk2);
		}
	}

	Game_Flag_Reset(kFlagHF01toHF05);
	Game_Flag_Reset(kFlagHF06toHF05);
	Game_Flag_Reset(kFlagHF07toHF05);

	//return false;
}

void SceneScriptHF05::PlayerWalkedOut() {
	if (Actor_Query_Goal_Number(kActorCrazylegs) == kGoalCrazyLegsMcCoyDrewHisGun) {
		Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsLeavesShowroom);
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptHF05::DialogueQueueFlushed(int a1) {
}

void SceneScriptHF05::talkWithCrazyLegs2() {
	Loop_Actor_Walk_To_Actor(kActorCrazylegs, kActorMcCoy, 72, false, false);
	Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
	Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
	Actor_Says(kActorCrazylegs, 370, kAnimationModeTalk);
	Actor_Says(kActorMcCoy, 1855, kAnimationModeTalk);
	Actor_Says(kActorCrazylegs, 380, 12);
	Actor_Says(kActorCrazylegs, 390, 14);
	Actor_Says(kActorCrazylegs, 400, 15);
	Actor_Says(kActorCrazylegs, 410, 16);
	Actor_Says(kActorMcCoy, 1860, kAnimationModeTalk);
	Actor_Says(kActorCrazylegs, 420, kAnimationModeTalk);
	Actor_Says(kActorMcCoy, 1865, kAnimationModeTalk);
}

void SceneScriptHF05::dialogueWithCrazylegs1() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(kActorMcCoy, kClueGrigoriansNote) // cut feature? there is no way how to obtain this clue
	 && Global_Variable_Query(kVariableChapter) == 3
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1180, 3, 6, 7); // ADVERTISEMENT
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueCrazylegsInterview1)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1190, 2, 7, 4); // WOMAN
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueDektorasDressingRoom)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1200, 5, 5, 3); // WOMAN'S PHOTO
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueLucy)
	 && Actor_Query_Goal_Number(kActorLucy) != kGoalLucyGone
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1210, 4, 6, 2); // LUCY'S PHOTO
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueGrigoriansResources) // cut feature? there is no way how to obtain this clue
	 || (Actor_Clue_Query(kActorMcCoy, kClueGrigoriansNote) // cut feature? there is no way how to obtain this clue either
	  && Global_Variable_Query(kVariableChapter) == 3
	 )
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1220, -1, 2, 8); // GRIGORIAN
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueCarRegistration1)
	 || Actor_Clue_Query(kActorMcCoy, kClueCarRegistration3)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1230, 4, 7, -1); // CAR REGISTRATION
	}

	if (!Dialogue_Menu_Query_List_Size()) {
		Actor_Says(kActorMcCoy, 1880, 15);
		Actor_Says(kActorCrazylegs, 490, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 1885, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 500, 16);
		return;
	}

	Dialogue_Menu_Add_DONE_To_List(1240); // DONE

	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	switch (answer) {
	case 1180: // ADVERTISEMENT
		Actor_Says(kActorMcCoy, 1890, 23);
		Actor_Says(kActorCrazylegs, 510, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 1920, 23);
		Actor_Says(kActorMcCoy, 1925, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 530, 12);
		Actor_Says(kActorMcCoy, 1930, 18);
		Actor_Says(kActorCrazylegs, 540, 14);
		Actor_Says(kActorMcCoy, 1935, 14);
		Actor_Says(kActorCrazylegs, 550, 16);
		Actor_Says(kActorMcCoy, 1940, 15);
		Actor_Says(kActorMcCoy, 1945, -1);
		// CrazyLegs cuts his sentence short here. He is not interrupted. 
		Actor_Says(kActorCrazylegs, 560, 15);
		Actor_Says(kActorCrazylegs, 570, 16);
		Actor_Says(kActorMcCoy, 1950, 17);
		dialogueWithCrazylegs2();
		break;

	case 1190: // WOMAN
		Actor_Says(kActorMcCoy, 1895, kAnimationModeIdle);
		Actor_Says(kActorCrazylegs, 620, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 630, 12);
		Actor_Says(kActorMcCoy, 2000, 13);
		Actor_Says(kActorCrazylegs, 640, 14);
		Actor_Says(kActorCrazylegs, 650, 15);
		Actor_Says(kActorCrazylegs, 660, 16);
		Actor_Says(kActorMcCoy, 2005, kAnimationModeIdle);
		Actor_Says(kActorMcCoy, 2010, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 670, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 680, 12);
		Actor_Says(kActorCrazylegs, 690, 14);
		Actor_Says(kActorMcCoy, 2015, 14);
		Actor_Says(kActorCrazylegs, 700, 15);
		Actor_Says(kActorMcCoy, 2020, 18);
		break;

	case 1200: // WOMAN'S PHOTO
		Actor_Says(kActorMcCoy, 1900, 23);
		Actor_Says(kActorCrazylegs, 710, 16);
		Actor_Says(kActorMcCoy, 2025, kAnimationModeIdle);
		Actor_Says(kActorCrazylegs, 720, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 730, 12);
		break;

	case 1210: // LUCY'S PHOTO
		Actor_Says(kActorMcCoy, 1905, 23);
		Actor_Says(kActorCrazylegs, 740, 14);
		Actor_Says(kActorMcCoy, 2030, 13);
		Actor_Says(kActorCrazylegs, 750, 15);
		Actor_Says(kActorMcCoy, 2035, 18);
		Actor_Says(kActorCrazylegs, 760, 16);
		Actor_Says(kActorCrazylegs, 770, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 2040, kAnimationModeIdle);
		break;

	case 1220: // GRIGORIAN
		Actor_Says(kActorMcCoy, 1910, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 780, 12);
		Actor_Says(kActorMcCoy, 2045, 17);
		Actor_Says(kActorMcCoy, 2050, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 790, 14);
		Actor_Says(kActorMcCoy, 2055, 19);
		Actor_Says(kActorMcCoy, 2060, -1);
		Actor_Says(kActorCrazylegs, 800, 15);
		Actor_Says(kActorMcCoy, 2065, 18);
		Actor_Says(kActorMcCoy, 2070, 14);
		Actor_Says(kActorCrazylegs, 810, 16);
		dialogueWithCrazylegs2();
		break;

	case 1230: // CAR REGISTRATION
		Actor_Says(kActorMcCoy, 1915, 12);
		if (Actor_Clue_Query(kActorMcCoy, kClueCarRegistration1)) {
			Actor_Says(kActorCrazylegs, 820, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 2075, 13);
			Actor_Says(kActorCrazylegs, 830, 12);
			Actor_Says(kActorCrazylegs, 840, 14);
			Actor_Says(kActorCrazylegs, 850, 15);
			Actor_Says(kActorMcCoy, 2080, kAnimationModeTalk);
			Actor_Says(kActorCrazylegs, 860, 16);
			Actor_Says(kActorCrazylegs, 870, kAnimationModeTalk);
		} else if (Actor_Clue_Query(kActorMcCoy, kClueCarRegistration3)) {
			Actor_Says(kActorCrazylegs, 880, 12);
			Actor_Says(kActorCrazylegs, 890, 14);
			Actor_Says(kActorMcCoy, 2085, kAnimationModeTalk);
			Actor_Says(kActorCrazylegs, 900, 15);
			Actor_Says(kActorMcCoy, 2090, 19);
			Actor_Says(kActorCrazylegs, 910, 16);
			Actor_Says(kActorMcCoy, 2095, 14);
			Actor_Says(kActorCrazylegs, 920, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 2100, 15);
			Actor_Says(kActorCrazylegs, 930, 12);
			Actor_Says(kActorCrazylegs, 940, 14);
			Actor_Says(kActorMcCoy, 2105, kAnimationModeTalk);
			Actor_Says(kActorCrazylegs, 950, 15);
			Actor_Says(kActorMcCoy, 2110, kAnimationModeIdle);
			Actor_Says(kActorCrazylegs, 960, 16);
		}
		break;
	}
}

void SceneScriptHF05::dialogueWithCrazylegs2() { // cut feature? it is impossible to trigger this dialog
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(1250, -1, -1, 10); // ARREST
	DM_Add_To_List_Never_Repeat_Once_Selected(1260, 10,  5, -1); // WARNING

	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	if (answer == 1250) { // ARREST
		Actor_Says(kActorMcCoy, 1955, 17);
		Actor_Says(kActorMcCoy, 1960, 23);
		Item_Pickup_Spin_Effect(kModelAnimationSpinnerKeys, 315, 327);
		Delay(2000);
		Actor_Says(kActorMcCoy, 1980, 23);
		Actor_Says(kActorMcCoy, 1985, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 580, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 590, 15);
		Actor_Says(kActorMcCoy, 1990, 17);
		Actor_Says(kActorCrazylegs, 600, 16);
		Actor_Says(kActorMcCoy, 1995, kAnimationModeTalk);
		Game_Flag_Set(kFlagCrazylegsArrested);
		Actor_Put_In_Set(kActorCrazylegs, kSetPS09);
		Actor_Set_At_XYZ(kActorCrazylegs, -315.15f, 0.0f, 241.06f, 583);
		Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsIsArrested);
		Game_Flag_Set(kFlagCrazylegsArrestedTalk);
		if (Game_Flag_Query(kFlagSpinnerAtNR01)) {
			Set_Enter(kSetNR01, kSceneNR01);
		} else {
			Game_Flag_Set(kFlagHF05toHF01);
			Set_Enter(kSetHF01, kSceneHF01);
		}
	} else if (answer == 1260) { // WARNING
		Actor_Says(kActorMcCoy, 1965, 12);
		Actor_Says(kActorMcCoy, 1970, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 1975, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 610, 16);
	}
}

int SceneScriptHF05::getAffectionTowardsActor() {
	if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsDektora
	 && Actor_Query_Goal_Number(kActorDektora) != kGoalDektoraGone
	) {
		return kActorDektora;
	}
	if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsLucy
	 && Actor_Query_Goal_Number(kActorLucy) != kGoalLucyGone
	) {
		return kActorLucy;
	}
	return -1;
}

void SceneScriptHF05::policeAttack() {
	Actor_Force_Stop_Walking(kActorMcCoy);
	Actor_Put_In_Set(kActorOfficerLeary, kSetHF05);
	Actor_Set_At_XYZ(kActorOfficerLeary, 430.4f, 40.63f, -258.18f, 300);
	Actor_Put_In_Set(kActorOfficerGrayford, kSetHF05);
	Actor_Set_At_XYZ(kActorOfficerGrayford, 526.4f, 37.18f, -138.18f, 300);
	ADQ_Flush();
	ADQ_Add(kActorOfficerGrayford, 260, -1);
	Player_Loses_Control();
	Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateUncover, true, kActorMcCoy, 4, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 100, 1200, true);
	Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateUncover, true, kActorMcCoy, 4, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 100, 300, true);
}

void SceneScriptHF05::talkWithDektora() {
	Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
	Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
	Actor_Says(kActorDektora, 2660, 12);
	Actor_Says(kActorMcCoy, 8990, kAnimationModeTalk);
	Actor_Says(kActorDektora, 2670, 13);
#if BLADERUNNER_ORIGINAL_BUGS
	Actor_Says(kActorDektora, 2680, 17);
#else
	// Dektora is interrupted here
	Actor_Says_With_Pause(kActorDektora, 2680, 0.0f, 17);
#endif // BLADERUNNER_ORIGINAL_BUGS
	Actor_Says(kActorMcCoy, 8995, 14);
	Actor_Says(kActorDektora, 2690, 15);
	Actor_Says_With_Pause(kActorMcCoy, 9000, 1.0f, 16);
	Actor_Says_With_Pause(kActorMcCoy, 9005, 1.0f, 19);
	Actor_Says(kActorMcCoy, 1765, 17);
	Actor_Says(kActorDektora, 160, 12);
	Actor_Says(kActorMcCoy, 1770, 15);
	Actor_Says(kActorMcCoy, 1775, kAnimationModeTalk);
	Actor_Says(kActorDektora, 170, kAnimationModeTalk);
	Actor_Says_With_Pause(kActorMcCoy, 1780, 1.0f, 18);
}

void SceneScriptHF05::talkWithLucy() {
	Actor_Face_Actor(kActorMcCoy, kActorLucy, true);
	Actor_Face_Actor(kActorLucy, kActorMcCoy, true);
	Actor_Says(kActorLucy, 400, 16);
	Actor_Says(kActorMcCoy, 1750, 14);
	Actor_Says(kActorLucy, 410, 12);
	Actor_Says(kActorLucy, 420, 14);
	Actor_Says(kActorMcCoy, 1755, 16);
	Actor_Says(kActorLucy, 430, 18);
	Actor_Says_With_Pause(kActorMcCoy, 1760, 1.0f, 15);
	Actor_Says(kActorMcCoy, 1765, 17);
	Actor_Says(kActorLucy, 440, kAnimationModeTalk);
	Actor_Says(kActorMcCoy, 1770, 15);
	Actor_Says(kActorMcCoy, 1775, kAnimationModeTalk);
	Actor_Says(kActorLucy, 450, 17);
	Actor_Says_With_Pause(kActorMcCoy, 1780, 1.0f, 18);
}

void SceneScriptHF05::talkWithCrazylegs3(int affectionTowardsActor) {
	if (affectionTowardsActor != -1
	 && Actor_Query_In_Set(kActorCrazylegs, kSetHF05)
	) {
		Async_Actor_Walk_To_Waypoint(affectionTowardsActor, 437, 36, false);
		Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 437, 0, false, false);
		Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
		Actor_Face_Actor(affectionTowardsActor, kActorCrazylegs, true);
		Actor_Says(kActorCrazylegs, 0, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 10, 12);
		Actor_Says(kActorMcCoy, 1715, 19);
		Actor_Says(kActorMcCoy, 1720, -1);
		Actor_Says(kActorCrazylegs, 20, 14);
		Actor_Says(kActorCrazylegs, 30, 15);
		Actor_Says(kActorMcCoy, 1725, kAnimationModeTalk);
		Actor_Says(kActorCrazylegs, 40, 16);
		Actor_Says(kActorCrazylegs, 50, kAnimationModeTalk);
		// CrazyLegs cuts his sentence short here. He is not interrupted. 
		Actor_Says(kActorCrazylegs, 60, 12);
		Actor_Says(kActorCrazylegs, 70, 13);
		Actor_Says(kActorMcCoy, 1730, kAnimationModeTalk);
		Loop_Actor_Walk_To_Actor(kActorCrazylegs, kActorMcCoy, 28, false, false);
		Item_Pickup_Spin_Effect(kModelAnimationSpinnerKeys, 315, 327);
		Actor_Says(kActorCrazylegs, 80, 23);
		Actor_Clue_Acquire(kActorMcCoy, kClueSpinnerKeys, true, kActorCrazylegs);
		Actor_Says(kActorCrazylegs, 90, 15);
		Actor_Says(kActorMcCoy, 1735, 17);
		Actor_Says(kActorCrazylegs, 100, 16);
		Actor_Says(kActorCrazylegs, 110, kAnimationModeTalk);
		Actor_Face_Actor(affectionTowardsActor, kActorMcCoy, true);
		if (affectionTowardsActor == kActorDektora) {
			Actor_Says(kActorDektora, 90, kAnimationModeTalk);
		} else {
			Actor_Says(kActorLucy, 380, kAnimationModeTalk);
		}
		Actor_Says(kActorMcCoy, 1740, 14);    // You tell her we're headed South.
		Actor_Says(kActorCrazylegs, 120, 12); // Ten Four.
		Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsLeavesShowroom);
		if (affectionTowardsActor == kActorDektora) {
			Actor_Says(kActorDektora, 100, kAnimationModeTalk);
		} else {
			Actor_Says(kActorLucy, 390, kAnimationModeTalk);
		}
		Actor_Face_Actor(kActorMcCoy, affectionTowardsActor, true);
		Actor_Says(kActorMcCoy, 1745, kAnimationModeTalk);
		Async_Actor_Walk_To_XYZ(affectionTowardsActor, 309.0f, 40.63f, 402.0f, 0, false);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 277.0f, 40.63f, 410.0f, 0, false, false, false);
		Game_Flag_Set(kFlagHF05toHF06);
		Set_Enter(kSetHF06, kSceneHF06);
	}
}

void SceneScriptHF05::talkWithCrazyLegs1() {
	Player_Loses_Control();
	if (Global_Variable_Query(kVariableChapter) == 3) {
		ADQ_Flush();
		ADQ_Add(kActorCrazylegs, 130, 18);
		ADQ_Add(kActorCrazylegs, 140, 18);
		// CrazyLegs cuts his sentence short here. He is not interrupted. 
		ADQ_Add(kActorCrazylegs, 150, 18);
		ADQ_Add(kActorCrazylegs, 160, 17);
	}
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 307.0f, 40.63f, 184.0f, 0, false, false, false);
	Loop_Actor_Walk_To_Actor(kActorCrazylegs, kActorMcCoy, 72, false, false);
	Ambient_Sounds_Play_Sound(kSfxLABMISC4, 99, 99, 0, 0);
	Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
	Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	// There is a chance here that Crazylegs will "interrupt himself"
	// and thus sometimes skip the last sentence of the above queued dialogue in chapter 3.
	// So we explicitly wait for the queue to be emptied before proceeding to his next line
	ADQ_Wait_For_All_Queued_Dialogue();
#endif // BLADERUNNER_ORIGINAL_BUGS
	Actor_Says(kActorCrazylegs, 170, kAnimationModeTalk);
	Actor_Says(kActorCrazylegs, 180, 12);
	Actor_Says(kActorCrazylegs, 190, 14);
	Actor_Says(kActorCrazylegs, 200, 15);
	Actor_Says(kActorMcCoy, 1815, 12);
	Actor_Says(kActorCrazylegs, 210, 16);
	Actor_Says(kActorMcCoy, 1820, -1);
	Actor_Says(kActorCrazylegs, 220, kAnimationModeTalk);
	Actor_Says(kActorCrazylegs, 230, 12);
	Actor_Says(kActorCrazylegs, 240, 14);
	Actor_Says(kActorMcCoy, 1825, kAnimationModeIdle);
	Actor_Says(kActorCrazylegs, 250, 15);
	Actor_Face_Object(kActorCrazylegs, "MONTE CARLO DRY", true);
	Actor_Says(kActorCrazylegs, 260, 16);
	Actor_Face_Object(kActorMcCoy, "MONTE CARLO DRY", true);
	Actor_Says(kActorMcCoy, 1830, kAnimationModeIdle);
	Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
	Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
	Actor_Says(kActorCrazylegs, 270, kAnimationModeTalk);
	Actor_Says(kActorCrazylegs, 280, 12);
	Async_Actor_Walk_To_XYZ(kActorCrazylegs, 276.0f, 40.63f, 182.0f, 12, false);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 335.0f, 40.63f, 131.0f, 12, false, false, false);
	Actor_Face_Object(kActorCrazylegs, "MONTE CARLO DRY", true);
	Actor_Face_Object(kActorMcCoy, "MONTE CARLO DRY", true);
	Actor_Says(kActorCrazylegs, 290, 14);
	Actor_Says(kActorCrazylegs, 300, 15);
	Actor_Says(kActorCrazylegs, 310, 16);
	Actor_Says(kActorMcCoy, 1835, 12);
	Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
	Actor_Says(kActorCrazylegs, 320, kAnimationModeTalk);
	Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
	Actor_Says(kActorCrazylegs, 330, 12);
	Actor_Says(kActorMcCoy, 1840, kAnimationModeTalk);
	Actor_Says(kActorCrazylegs, 340, 14);
	Actor_Says(kActorMcCoy, 1845, kAnimationModeTalk);
	Actor_Says(kActorCrazylegs, 350, 15);
	Actor_Says(kActorCrazylegs, 360, 16);
	Actor_Says(kActorMcCoy, 1850, kAnimationModeTalk);
	Player_Gains_Control();
}

void SceneScriptHF05::addAmbientSounds() {
	Ambient_Sounds_Add_Sound(kSfxSIREN2, 20, 80, 20, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(kActorOfficerLeary,    250, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorOfficerLeary,    330, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorOfficerLeary,    340, 5, 90, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorOfficerLeary,    360, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorOfficerGrayford, 380, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorOfficerGrayford, 510, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorDispatcher,       80, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorDispatcher,      160, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorDispatcher,      280, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
}

int SceneScriptHF05::getCompanionActor() {
	if (Actor_Query_In_Set(kActorDektora, kSetHF05)
	 && Actor_Query_Goal_Number(kActorDektora) != kGoalDektoraGone
	) {
		return kActorDektora;
	}

	if (Actor_Query_In_Set(kActorLucy, kSetHF05)
	 && Actor_Query_Goal_Number(kActorLucy) != kGoalLucyGone
	) {
		return kActorLucy;
	}

	return kActorMcCoy;
}

} // End of namespace BladeRunner
