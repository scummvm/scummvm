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

void SceneScriptAR01::InitializeScene() {
	Music_Play(0, 25, 0, 2, -1, 1, 2);
	if (Game_Flag_Query(kFlagHC01toAR01)) {
		Setup_Scene_Information(-477.0f, 0.0f, -149.0f, 333);
	} else if (Game_Flag_Query(kFlagAR02toAR01)) {
		Setup_Scene_Information(-182.0f, 0.0f, -551.0f, 518);
	} else {
		Setup_Scene_Information(-152.0f, 0.0f, 332.0f, 545);
	}
	Scene_Exit_Add_2D_Exit(0, 134, 165, 177, 290, 3);
	Scene_Exit_Add_2D_Exit(1, 319, 0, 639, 207, 0);
	if (Game_Flag_Query(kFlagSpinnerAtAR01)) {
		Scene_Exit_Add_2D_Exit(2, 0, 404, 99, 479, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(54, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(81, 60, 100, 1);
	Ambient_Sounds_Add_Looping_Sound(241, 50, 1, 1);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(242, 3, 30, 11, 11, 50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(243, 3, 30, 11, 11, 50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(244, 3, 30, 11, 11, 50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(245, 3, 30, 11, 11, 50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(246, 3, 30, 11, 11, 50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(247, 3, 30, 11, 11, 50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(248, 3, 30, 11, 11, 50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(249, 3, 30, 11, 11, 50, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(68, 10, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	if ( Game_Flag_Query(kFlagSpinnerAtAR01)
	 && !Game_Flag_Query(kFlagAR02toAR01)
	 && !Game_Flag_Query(kFlagHC01toAR01)
	) {
		Actor_Set_Invisible(kActorMcCoy, true);
		Game_Flag_Set(273);
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 1, false);
		Scene_Loop_Set_Default(2);
	} else if (Game_Flag_Query(kFlagSpinnerAtAR01)
	        && Game_Flag_Query(kFlagAR02toAR01)
	) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(2);
		Game_Flag_Reset(kFlagAR02toAR01);
	} else if (!Game_Flag_Query(kFlagSpinnerAtAR01)
	        &&  Game_Flag_Query(kFlagAR02toAR01)
	) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 6, false);
		Scene_Loop_Set_Default(7);
		Game_Flag_Reset(kFlagAR02toAR01);
	} else if (Game_Flag_Query(kFlagSpinnerAtAR01)
	        && Game_Flag_Query(kFlagHC01toAR01)
	) {
		Scene_Loop_Set_Default(2);
	} else if (!Game_Flag_Query(kFlagSpinnerAtAR01)
	        &&  Game_Flag_Query(kFlagHC01toAR01)
	) {
		Scene_Loop_Set_Default(7);
	} else {
		Scene_Loop_Set_Default(7);
	}
}

void SceneScriptAR01::SceneLoaded() {
	Obstacle_Object("DF_BOOTH", true);
	Unobstacle_Object("SPINNER BODY", true);
}

bool SceneScriptAR01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptAR01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptAR01::ClickedOnActor(int actorId) {
	if (actorId == kActorInsectDealer || actorId == kActorHasan) {
		Actor_Face_Actor(kActorMcCoy, actorId, true);
		Actor_Says(kActorMcCoy, 8910, 14);
		return true;
	}
	if (actorId == kActorFishDealer) {
		Actor_Set_Goal_Number(kActorFishDealer, 2);
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -120.73f, 0.0f, 219.17f, 12, 1, false, 0)) {
			Actor_Face_Actor(kActorMcCoy, kActorFishDealer, true);
			Actor_Face_Actor(kActorFishDealer, kActorMcCoy, true);
			if (Game_Flag_Query(328)) {
				if (Actor_Clue_Query(kActorMcCoy, kClueStrangeScale1) && !Actor_Clue_Query(kActorMcCoy, kClueFishLadyInterview)) {
					Actor_Says(kActorMcCoy, 40, 11);
					Actor_Says(kActorFishDealer, 120, 14);
					Actor_Says(kActorMcCoy, 45, 17);
					Actor_Says(kActorFishDealer, 130, 14);
					Actor_Says(kActorFishDealer, 140, 14);
					Actor_Says(kActorMcCoy, 50, 13);
					Actor_Says(kActorFishDealer, 150, 14);
					Actor_Clue_Acquire(kActorMcCoy, kClueFishLadyInterview, 1, kActorMcCoy);
				} else {
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
				Actor_Set_Goal_Number(kActorFishDealer, 1);
			} else {
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
				Game_Flag_Set(328);
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
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -358.0, 0.0, -149.0, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -477.0, 0.0, -149.0, 0, 0, false, 0);
			Game_Flag_Set(kFlagAR01toHC01);
			Game_Flag_Set(464);
			Game_Flag_Reset(kFlagMcCoyAtARxx);
			Game_Flag_Set(479);
			Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC01);
			Actor_Set_Goal_Number(kActorFishDealer, 3);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -182.0, 0.0, -551.0, 0, 1, false, 0)) {
			Game_Flag_Set(kFlagAR01toAR02);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -222.0, 0.0, -690.0, 0, false);
			Set_Enter(kSetAR01_AR02, kSceneAR02);
			Actor_Set_Goal_Number(kActorFishDealer, 3);
		}
		return true;
	}
	if (exitId == 2) {
		if (Game_Flag_Query(486) == 1) {
			Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationDNARow, true);
		}
		int v1 = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -164.0f, 0.0f, 332.0f, 0, 1, false, 0);
		Actor_Face_Heading(kActorMcCoy, 545, false);
		if (Actor_Query_Goal_Number(kActorIzo) >= 2 && Actor_Query_Goal_Number(kActorIzo) <= 103) {
			Player_Loses_Control();
			Actor_Put_In_Set(kActorIzo, kSetAR01_AR02);
			Actor_Set_At_XYZ(kActorIzo, -448.0, 0.0, 130.0, 0);
			Loop_Actor_Walk_To_XYZ(kActorIzo, -323.0f, 0.64f, 101.74f, 48, 0, true, 0);
			Loop_Actor_Walk_To_Actor(kActorIzo, kActorMcCoy, 48, 0, true);
			Actor_Face_Actor(kActorIzo, kActorMcCoy, true);
			Actor_Change_Animation_Mode(kActorIzo, 6);
			Actor_Says(kActorMcCoy, 1800, 21);
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
			Player_Gains_Control();
			Actor_Retired_Here(kActorMcCoy, 12, 48, 1, kActorIzo);
		} else if (!v1) {
			if (Game_Flag_Query(486) && !Game_Flag_Query(660)) {
				Actor_Voice_Over(4310, kActorVoiceOver);
				Actor_Voice_Over(4320, kActorVoiceOver);
				Actor_Voice_Over(4330, kActorVoiceOver);
				Actor_Voice_Over(4340, kActorVoiceOver);
				Actor_Voice_Over(4350, kActorVoiceOver);
				Game_Flag_Set(660);
			}
			Game_Flag_Reset(kFlagMcCoyAtCTxx);
			Game_Flag_Reset(kFlagMcCoyAtRCxx);
			Game_Flag_Reset(kFlagMcCoyAtMAxx);
			Game_Flag_Reset(kFlagMcCoyAtPSxx);
			Game_Flag_Reset(kFlagMcCoyAtBBxx);
			Game_Flag_Reset(kFlagMcCoyAtHFxx);
			Game_Flag_Reset(kFlagMcCoyAtARxx);

			int spinnerDest = Spinner_Interface_Choose_Dest(4, false);
			Actor_Face_Heading(kActorMcCoy, 545, false);

			switch (spinnerDest) {
			case kSpinnerDestinationPoliceStation:
				Game_Flag_Set(kFlagMcCoyAtPSxx);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtPS01);
				Set_Enter(kSetPS01, kScenePS01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 5, true);
				break;
			case kSpinnerDestinationMcCoysApartment:
				Game_Flag_Set(kFlagMcCoyAtMAxx);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtMA01);
				Set_Enter(kSetMA01, kSceneMA01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 5, true);
				break;
			case kSpinnerDestinationRuncitersAnimals:
				Game_Flag_Set(kFlagMcCoyAtRCxx);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtRC01);
				Set_Enter(kSetRC01, kSceneRC01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 5, true);
				break;
			case kSpinnerDestinationChinatown:
				Game_Flag_Set(kFlagMcCoyAtCTxx);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtCT01);
				Set_Enter(kSetCT01_CT12, kSceneCT01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 5, true);
				break;
			case kSpinnerDestinationTyrellBuilding:
				Game_Flag_Set(kFlagMcCoyAtTBxx);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 5, true);
				break;
			case kSpinnerDestinationDNARow:
				Game_Flag_Set(kFlagMcCoyAtDRxx);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 5, true);
				break;
			case kSpinnerDestinationBradburyBuilding:
				Game_Flag_Set(kFlagMcCoyAtBBxx);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtBB01);
				Set_Enter(kSetBB01, kSceneBB01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 5, true);
				break;
			case kSpinnerDestinationNightclubRow:
				Game_Flag_Set(kFlagMcCoyAtNRxx);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtNR01);
				Set_Enter(kSetNR01, kSceneNR01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 5, true);
				break;
			case kSpinnerDestinationHysteriaHall:
				Game_Flag_Set(kFlagMcCoyAtHFxx);
				Game_Flag_Reset(kFlagSpinnerAtAR01);
				Game_Flag_Set(kFlagSpinnerAtHF01);
				Set_Enter(kSetHF01, kSceneHF01);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 5, true);
				break;
			default:
				Game_Flag_Set(kFlagMcCoyAtARxx);
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
		Ambient_Sounds_Play_Sound(118, 40, 0, 0, 99);
	}
	if (frame == 78 || frame == 199) {
		Ambient_Sounds_Play_Sound(116, 100, -50, -50, 99);
	}
	if (frame == 122 || frame == 242) {
		Ambient_Sounds_Play_Sound(119, 100, -50, -50, 99);
	}
	if (frame == 256) {
		Ambient_Sounds_Play_Sound(117, 40, -50, 80, 99);
	}
	if ((frame == 75 || frame == 196) && Game_Flag_Query(273)) {
		Actor_Face_Heading(kActorMcCoy, 545, false);
		Actor_Change_Animation_Mode(kActorMcCoy, 42);
		Game_Flag_Reset(273);
	} else if (frame == 196 && !Game_Flag_Query(273)) {
		Actor_Change_Animation_Mode(kActorMcCoy, 41);
	}
}

void SceneScriptAR01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptAR01::PlayerWalkedIn() {
	if (!Game_Flag_Query(710)) {
		Game_Flag_Set(710);
	}
	if (Game_Flag_Query(kFlagHC01toAR01)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -358.0f, 0.0f, -149.0f, 0, true, false, 0);
		Game_Flag_Reset(kFlagHC01toAR01);
	}
	if (Actor_Query_Goal_Number(kActorPhotographer) < 199) {
		Actor_Set_Goal_Number(kActorPhotographer, 199);
	}
}

void SceneScriptAR01::PlayerWalkedOut() {
	Actor_Set_Invisible(kActorMcCoy, false);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(479)) {
		Music_Stop(2);
	}
	if (!Game_Flag_Query(kFlagAR01toHC01)
	 && !Game_Flag_Query(kFlagAR01toAR02)
	) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeTowards3, true, -1);
	}
}

void SceneScriptAR01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
