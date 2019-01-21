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

void SceneScriptHF01::InitializeScene() {
	if (Game_Flag_Query(617)) {
		Setup_Scene_Information(243.94f, 8.0f, -341.9f, 342);
	} else if (Game_Flag_Query(kFlagHF05toHF01)) {
		Setup_Scene_Information(-202.0f, 0.0f, -619.0f, 407);
	} else if (Game_Flag_Query(kFlagHF03toHF01)) {
		Setup_Scene_Information(124.0f, 8.0f, -880.0f, 455);
	} else if (Game_Flag_Query(kFlagHF02toHF01)) {
		Setup_Scene_Information(406.0f, 8.0f, -813.0f, 455);
	} else {
		Setup_Scene_Information(100.0f, 0.0f, -260.0f, 0);
	}
	Scene_Exit_Add_2D_Exit(0, 81, 226, 169, 321, 0);
	if (!Game_Flag_Query(663)) {
		Scene_Exit_Add_2D_Exit(1, 304, 239, 492, 339, 0);
		Scene_Exit_Add_2D_Exit(2, 560, 231, 639, 360, 0);
		if (Game_Flag_Query(kFlagSpinnerAtHF01)) {
			Scene_Exit_Add_2D_Exit(3, 0, 311, 66, 417, 2);
		}
	}
	Ambient_Sounds_Add_Looping_Sound(54, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(340, 25, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(81, 60, 100, 1);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(68, 10, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(kFlagSpinnerAtHF01)) {
		if (!Game_Flag_Query(kFlagHF02toHF01)
		 && !Game_Flag_Query(kFlagHF03toHF01)
		 && !Game_Flag_Query(kFlagHF05toHF01)
		) {
			Scene_Loop_Start_Special(0, 0, 0);
		}
		Scene_Loop_Set_Default(1);
	} else if (Game_Flag_Query(663)) {
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(5);
	}
}

void SceneScriptHF01::SceneLoaded() {
	Obstacle_Object("LOFT41", true);
	if (!Game_Flag_Query(kFlagSpinnerAtHF01)) {
		Unobstacle_Object("OBSTACLE BOX15", true);
	}
}

bool SceneScriptHF01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHF01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptHF01::ClickedOnActor(int actorId) {
	int affectionTowardsActor;
	if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsDektora) {
		affectionTowardsActor = kActorDektora;
	} else if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsLucy) {
		affectionTowardsActor = kActorLucy;
	} else {
		affectionTowardsActor = -1;
	}
	if (actorId == kActorMia || actorId == kActorMurray) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 504.04f, 8.0f, -242.17f, 12, 1, false, 0)) {
			ADQ_Flush();
			Actor_Face_Actor(kActorMcCoy, kActorMurray, true);
			if (Game_Flag_Query(382)) {
				sub_4026B4();
			} else {
				Actor_Says(kActorMcCoy, 1455, 15);
				Actor_Says(kActorMurray, 40, 13);
				Actor_Says(kActorMcCoy, 1460, 13);
				Actor_Says(kActorMurray, 50, 12);
				Item_Pickup_Spin_Effect(951, 396, 359);
				Actor_Face_Heading(kActorMurray, 271, false);
				Actor_Says(kActorMurray, 60, 12);
				Actor_Says(kActorMcCoy, 1465, 15);
				Actor_Face_Actor(kActorMurray, kActorMcCoy, true);
				Actor_Says(kActorMurray, 70, 13);
				Actor_Says(kActorMcCoy, 1470, 14);
				Actor_Says(kActorMia, 30, 3);
				Actor_Says(kActorMurray, 80, 15);
				Actor_Says(kActorMia, 50, 3);
				Actor_Says(kActorMurray, 110, 14);
				Game_Flag_Set(382);
			}
			return true;
		}
		return false;
	}
	if (affectionTowardsActor != -1 && actorId == affectionTowardsActor) {
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, actorId, 28, 1, false)) {
			if (Actor_Query_Goal_Number(affectionTowardsActor) == 599) {
				Actor_Says(kActorMcCoy, 8630, 13);
				return true;
			}
			if (Global_Variable_Query(kVariableChapter) == 5
			 && (Actor_Clue_Query(kActorMcCoy, kCluePhoneCallDektora1)
			  || Actor_Clue_Query(kActorMcCoy, kCluePhoneCallLucy1)
			 )
			 && !Game_Flag_Query(kFlagCrazylegsArrested)
			 &&  Actor_Query_Goal_Number(kActorCrazylegs) != 2
			 &&  Game_Flag_Query(653)
			 && !Game_Flag_Query(662)
			) {
				Game_Flag_Set(662);
				Actor_Face_Actor(kActorMcCoy, affectionTowardsActor, true);
				Actor_Face_Actor(affectionTowardsActor, kActorMcCoy, true);
				if (affectionTowardsActor == kActorDektora) {
					sub_4032DC();
				} else if (affectionTowardsActor == kActorLucy) {
					sub_403484();
				}
				Async_Actor_Walk_To_XYZ(affectionTowardsActor, -175.0f, 8.0f, -617.0f, 0, false);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -137.0f, 8.0f, -577.0f, 0, 0, false, 1);
				Game_Flag_Set(kFlagHF01toHF05);
				Set_Enter(kSetHF05, kSceneHF05);
			}
		}
	}
	return false;
}

bool SceneScriptHF01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptHF01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -202.0f, 8.0f, -619.0f, 0, true, false, 0)) {
			Game_Flag_Set(kFlagHF01toHF05);
			Set_Enter(kSetHF05, kSceneHF05);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 124.0f, 8.0f, -724.0f, 0, true, false, 0)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 124.0f, 8.0f, -880.0f, 0, false, false, 0);
			Game_Flag_Set(kFlagHF01toHF03);
			Set_Enter(kSetHF03, kSceneHF03);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 406.0f, 8.0f, -717.0f, 0, true, false, 0)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 406.0f, 8.0f, -813.0f, 0, false, false, 0);
			Game_Flag_Set(kFlagHF01toHF02);
			Set_Enter(kSetHF02, kSceneHF02);
		}
		return true;
	}
	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 100.0f, 0.0f, -260.0f, 0, true, false, 0)) {
			Game_Flag_Reset(kFlagMcCoyAtCTxx);
			Game_Flag_Reset(kFlagMcCoyAtRCxx);
			Game_Flag_Reset(kFlagMcCoyAtMAxx);
			Game_Flag_Reset(kFlagMcCoyAtPSxx);
			Game_Flag_Reset(kFlagMcCoyAtBBxx);
			Game_Flag_Reset(kFlagMcCoyAtHFxx);
			int spinnerDest = Spinner_Interface_Choose_Dest(3, false);
			switch (spinnerDest) {
			case kSpinnerDestinationPoliceStation:
				Game_Flag_Set(kFlagMcCoyAtPSxx);
				Game_Flag_Reset(kFlagSpinnerAtHF01);
				Game_Flag_Set(kFlagSpinnerAtPS01);
				Set_Enter(kSetPS01, kScenePS01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 4, true);
				break;
			case kSpinnerDestinationMcCoysApartment:
				Game_Flag_Set(kFlagMcCoyAtMAxx);
				Game_Flag_Reset(kFlagSpinnerAtHF01);
				Game_Flag_Set(kFlagSpinnerAtMA01);
				Set_Enter(kSetMA01, kSceneMA01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 4, true);
				break;
			case kSpinnerDestinationRuncitersAnimals:
				Game_Flag_Set(kFlagMcCoyAtRCxx);
				Game_Flag_Reset(kFlagSpinnerAtHF01);
				Game_Flag_Set(kFlagSpinnerAtRC01);
				Set_Enter(kSetRC01, kSceneRC01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 4, true);
				break;
			case kSpinnerDestinationChinatown:
				Game_Flag_Set(kFlagMcCoyAtCTxx);
				Game_Flag_Reset(kFlagSpinnerAtHF01);
				Game_Flag_Set(kFlagSpinnerAtCT01);
				Set_Enter(kSetCT01_CT12, kSceneCT01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 4, true);
				break;
			case kSpinnerDestinationAnimoidRow:
				Game_Flag_Set(kFlagMcCoyAtTBxx);
				Game_Flag_Reset(kFlagSpinnerAtHF01);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 4, true);
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(kFlagMcCoyAtARxx);
				Game_Flag_Reset(kFlagSpinnerAtHF01);
				Game_Flag_Set(kFlagSpinnerAtAR01);
				Set_Enter(kSetAR01_AR02, kSceneAR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 4, true);
				break;
			case kSpinnerDestinationDNARow:
				Game_Flag_Set(kFlagMcCoyAtDRxx);
				Game_Flag_Reset(kFlagSpinnerAtHF01);
				Game_Flag_Set(kFlagSpinnerAtDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 4, true);
				break;
			case kSpinnerDestinationBradburyBuilding:
				Game_Flag_Set(kFlagMcCoyAtBBxx);
				Game_Flag_Reset(kFlagSpinnerAtHF01);
				Game_Flag_Set(kFlagSpinnerAtBB01);
				Set_Enter(kSetBB01, kSceneBB01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 4, true);
				break;
			case kSpinnerDestinationNightclubRow:
				Game_Flag_Set(kFlagMcCoyAtNRxx);
				Game_Flag_Reset(kFlagSpinnerAtHF01);
				Game_Flag_Set(kFlagSpinnerAtNR01);
				Set_Enter(kSetNR01, kSceneNR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 4, true);
				break;
			default:
				Game_Flag_Set(kFlagMcCoyAtHFxx);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, 100.0f, 0.0f, -300.0f, 0, true, false, 0);
				break;
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptHF01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHF01::SceneFrameAdvanced(int frame) {
	if (frame == 10) {
		Sound_Play(118, 40, 0, 0, 50);
	}
	if (frame == 72 || frame == 193) {
		Sound_Play(116, 100, -50, -50, 50);
	}
	if (frame == 88 || frame == 214) {
		Sound_Play(119, 100, -50, -50, 50);
	}
	if (frame == 242) {
		Sound_Play(117, 40, -50, 80, 50);
	}
	if (Actor_Query_Goal_Number(kActorMurray) == 1) {
		Actor_Set_Goal_Number(kActorMurray, 0);
	}
}

void SceneScriptHF01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHF01::PlayerWalkedIn() {
	if (Game_Flag_Query(663)) {
		ADQ_Flush();
		ADQ_Add(kActorOfficerGrayford, 280, 3);
		Actor_Put_In_Set(kActorOfficerLeary, kSetHF01);
		Actor_Set_At_XYZ(kActorOfficerLeary, 8.2f, 8.0f, -346.67f, 1021);
		Actor_Put_In_Set(kActorOfficerGrayford, kSetHF01);
		Actor_Set_At_XYZ(kActorOfficerGrayford, 51.21f, 8.0f, -540.78f, 796);
		Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateUncover, true, kActorMcCoy, 4, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 0, 100, 300, false);
		Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateUncover, true, kActorMcCoy, 4, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 0, 100, 300, false);
	}
	if (!Game_Flag_Query(kFlagCrazylegsArrested) && Actor_Query_Goal_Number(kActorCrazylegs) != 2) {
		if (Actor_Clue_Query(kActorMcCoy, kCluePhoneCallLucy1)
		 && Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsLucy
		 && Actor_Query_Goal_Number(kActorLucy) != 599
		) {
			Actor_Put_In_Set(kActorLucy, kSetHF01);
			Actor_Set_At_XYZ(kActorLucy, -5.0f, 8.0f, -622.0f, 419);
			Actor_Set_Targetable(kActorLucy, true);
		} else if (Actor_Clue_Query(kActorMcCoy, kCluePhoneCallDektora1)
		        && Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsDektora
		        && Actor_Query_Goal_Number(kActorDektora) != 599
		) {
			Actor_Put_In_Set(kActorDektora, kSetHF01);
			Actor_Set_At_XYZ(kActorDektora, -5.0f, 8.0f, -622.0f, 419);
			Actor_Set_Targetable(kActorDektora, true);
		}
	}
	if (Game_Flag_Query(617)) {
		Actor_Set_Goal_Number(kActorSteele, 280);
		Game_Flag_Reset(617);
		//return true;
		return;
	}
	if (Game_Flag_Query(652)) {
		Game_Flag_Reset(652);
		Actor_Voice_Over(950, kActorVoiceOver);
		Actor_Voice_Over(960, kActorVoiceOver);
		Actor_Voice_Over(970, kActorVoiceOver);
		Actor_Voice_Over(980, kActorVoiceOver);
	} else if (!Game_Flag_Query(377) && Global_Variable_Query(kVariableChapter) < 4) {
		ADQ_Flush();
		ADQ_Add(kActorMurray, 0, 14);
		ADQ_Add(kActorMurray, 10, 3);
		ADQ_Add(kActorMia, 0, 3);
		Actor_Face_Actor(kActorMurray, kActorMia, true);
		ADQ_Add(kActorMurray, 20, 13);
		ADQ_Add(kActorMia, 10, 3);
		ADQ_Add(kActorMurray, 30, 3);
		ADQ_Add(kActorMia, 20, 3);
		Actor_Face_Heading(kActorMurray, 271, false);
		Game_Flag_Set(377);
	}
	if (Game_Flag_Query(kFlagHF03toHF01)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 124.0f, 8.0f, -724.0f, 0, 1, false, 0);
	} else if (Game_Flag_Query(kFlagHF02toHF01)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 406.0f, 8.0f, -717.0f, 0, 1, false, 0);
	} else if (!Game_Flag_Query(kFlagHF05toHF01)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 100.0f, 0.0f, -300.0f, 0, 1, false, 0);
	}
	Game_Flag_Reset(kFlagHF03toHF01);
	Game_Flag_Reset(kFlagHF02toHF01);
	Game_Flag_Reset(kFlagHF05toHF01);
	//return false;
}

void SceneScriptHF01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(kFlagHF01toHF05)
	 && !Game_Flag_Query(kFlagHF01toHF02)
	 && !Game_Flag_Query(kFlagHF01toHF03)
	 && !Game_Flag_Query(722)
	) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeInside2, true, -1);
		Outtake_Play(kOuttakeTowards3, true, -1);
	}
	Game_Flag_Reset(722);
	if (Actor_Query_Goal_Number(kActorLucy) == 450) {
		Actor_Put_In_Set(kActorLucy, kSetFreeSlotG);
		Actor_Set_At_Waypoint(kActorLucy, 39, 0);
		Actor_Set_Goal_Number(kActorLucy, 599);
	}
	if (Actor_Query_Goal_Number(kActorDektora) == 450) {
		Actor_Put_In_Set(kActorDektora, kSetFreeSlotG);
		Actor_Set_At_Waypoint(kActorDektora, 39, 0);
		Actor_Set_Goal_Number(kActorDektora, 599);
	}
}

void SceneScriptHF01::DialogueQueueFlushed(int a1) {
}

void SceneScriptHF01::sub_4026B4() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(kActorMcCoy, kClueLucy)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(440, 8, 6, 3);
	}
	if (!Actor_Clue_Query(kActorMcCoy, kClueLucy)
	 &&  Actor_Clue_Query(kActorMcCoy, kClueRunciterInterviewA)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(450, 7, 6, 3);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueCheese)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(460, 3, 5, 6);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueLichenDogWrapper)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(470, -1, 3, 8);
	}
	Dialogue_Menu_Add_DONE_To_List(480);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 440:
		Actor_Says(kActorMcCoy, 1480, 15);
		Actor_Says(kActorMia, 70, 3);
		Actor_Says(kActorMurray, 120, 3);
		Actor_Says(kActorMcCoy, 1505, 14);
		Actor_Says(kActorMia, 80, 3);
		Actor_Says(kActorMurray, 130, 3);
		Actor_Says(kActorMia, 90, 3);
		Actor_Says(kActorMcCoy, 1510, 12);
		break;
	case 450:
		Actor_Says(kActorMcCoy, 1485, 16);
		Actor_Says(kActorMia, 100, 3);
		Actor_Says(kActorMurray, 140, 12);
		Actor_Says(kActorMia, 110, 3);
		Actor_Says(kActorMia, 120, 3);
		Actor_Says(kActorMurray, 150, 14);
		break;
	case 460:
		Actor_Says(kActorMcCoy, 1490, 13);
		Actor_Says(kActorMurray, 160, 15);
		Actor_Says(kActorMia, 130, 13);
		Actor_Says(kActorMurray, 170, 12);
		Actor_Says(kActorMurray, 180, 13);
		Actor_Says(kActorMurray, 190, 14);
		Actor_Says(kActorMcCoy, 1515, 15);
		Actor_Says(kActorMurray, 200, 3);
		Actor_Says(kActorMcCoy, 1520, 15);
		Actor_Says(kActorMurray, 210, 13);
		Actor_Says(kActorMurray, 220, 13);
		Actor_Says(kActorMia, 140, 12);
		Actor_Says(kActorMurray, 230, 13);
		Actor_Clue_Acquire(kActorMcCoy, kClueSpecialIngredient, false, kActorMurray);
		break;
	case 470:
		Actor_Says(kActorMcCoy, 1495, 14);
		Actor_Face_Actor(kActorMurray, kActorMcCoy, true);
		Actor_Says(kActorMurray, 240, 13);
		Actor_Face_Actor(kActorMurray, kActorMia, true);
		break;
	case 480:
		Actor_Says(kActorMcCoy, 1500, 16);
		break;
	}
}

void SceneScriptHF01::sub_4032DC() {
	Actor_Says(kActorDektora, 0, 3);
	Actor_Says(kActorMcCoy, 1400, 3);
	Actor_Says(kActorDektora, 10, 3);
	Actor_Says(kActorMcCoy, 1405, 3);
	Actor_Says(kActorDektora, 20, 3);
	Actor_Says(kActorMcCoy, 1410, 3);
	Actor_Says(kActorDektora, 30, 3);
	Actor_Says(kActorMcCoy, 1415, 3);
	Actor_Says(kActorDektora, 40, 3);
	Actor_Says(kActorDektora, 50, 3);
	Actor_Says(kActorDektora, 60, 3);
	Actor_Says(kActorDektora, 70, 3);
	Actor_Says(kActorMcCoy, 1420, 3);
	Actor_Says(kActorDektora, 80, 3);
}

void SceneScriptHF01::sub_403484() {
	Actor_Says(kActorLucy, 0, 3);
	Actor_Says(kActorMcCoy, 1425, 3);
	Actor_Says(kActorLucy, 10, 3);
	Actor_Says(kActorLucy, 20, 3);
	Actor_Says(kActorMcCoy, 1430, 3);
	Actor_Says(kActorLucy, 30, 3);
	Actor_Says(kActorMcCoy, 1435, 3);
	Actor_Says(kActorLucy, 40, 3);
	Actor_Says(kActorLucy, 50, 3);
	Actor_Says(kActorMcCoy, 1440, 3);
	Actor_Says(kActorLucy, 60, 3);
	Actor_Says(kActorLucy, 70, 3);
	Actor_Says(kActorMcCoy, 1445, 3);
	Actor_Says(kActorLucy, 80, 3);
	Actor_Says(kActorLucy, 3030, 3);
}

} // End of namespace BladeRunner
