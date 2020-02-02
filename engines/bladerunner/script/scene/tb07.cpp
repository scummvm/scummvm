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

enum kTB07Loops {
	kTB07LoopMainLoop      = 0,
	kTB07LoopShadeDrop     = 2,
	kTB07LoopMainShadeDown = 3
};

void SceneScriptTB07::InitializeScene() {
	Setup_Scene_Information(68.0f, 12.0f, 288.0f, 0);

	Scene_Exit_Add_2D_Exit(0, 383, 445, 639, 479, 2);

	Ambient_Sounds_Add_Looping_Sound(kSfxBRBED3, 20, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxSUNROOM1, 2, 55, 14, 14, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSUNROOM2, 2, 55, 14, 14, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSUNROOM3, 2, 55, 14, 14, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSUNROOM4, 2, 55, 14, 14, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY1,   1, 15, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY2,   1, 15, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY3,   1, 20, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY4,   1, 15, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBELLY5,   1, 15, 20, 25, -100, 100, -101, -101, 0, 0);

	if ( Global_Variable_Query(kVariableChapter) == 4
	 && !Actor_Clue_Query(kActorMcCoy, kClueDNATyrell)
	) {
		Item_Add_To_World(kItemDNATyrell, kModelAnimationDNADataDisc, kSetTB07, 9.7f, 48.7f, -174.22f, 0, 12, 12, false, true, false, true);
	}
	if (Game_Flag_Query(kFlagTB07ShadeDown)) {
		Scene_Loop_Set_Default(kTB07LoopMainShadeDown);
	} else {
		Scene_Loop_Set_Default(kTB07LoopMainLoop);
	}
}

void SceneScriptTB07::SceneLoaded() {
	Obstacle_Object("EAGLE01", true);
	Clickable_Object("EAGLE01");
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Unclickable_Object("EAGLE01");
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptTB07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptTB07::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptTB07::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptTB07::ClickedOnItem(int itemId, bool a2) {
	if (!Loop_Actor_Walk_To_Item(kActorMcCoy, itemId, 36, true, false)) {
		Actor_Face_Item(kActorMcCoy, itemId, true);
		if (itemId == kItemDNATyrell) {
			Item_Pickup_Spin_Effect(kModelAnimationDNADataDisc, 331, 296);
			Actor_Clue_Acquire(kActorMcCoy, kClueDNATyrell, false, -1);
		}
		Item_Remove_From_World(itemId);
	}
	return false;
}

bool SceneScriptTB07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 68.0f, 12.0f, 288.0f, 0, true, false, false)) {
#if BLADERUNNER_ORIGINAL_BUGS
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
#endif // BLADERUNNER_ORIGINAL_BUGS
			if (Global_Variable_Query(kVariableChapter) == 4) {
				Game_Flag_Set(kFlagTB07toTB02);
				Set_Enter(kSetTB02_TB03, kSceneTB02);
			} else {
				Game_Flag_Reset(kFlagMcCoyInChinaTown);
				Game_Flag_Reset(kFlagMcCoyInRunciters);
				Game_Flag_Reset(kFlagMcCoyInMcCoyApartment);
				Game_Flag_Reset(kFlagMcCoyInPoliceStation);
				Game_Flag_Reset(kFlagMcCoyInBradburyBuilding);
				Game_Flag_Reset(kFlagMcCoyInHysteriaHall);
				Game_Flag_Reset(kFlagMcCoyInTyrellBuilding);
				Game_Flag_Reset(kFlagTB02ElevatorToTB05);
				switch (Spinner_Interface_Choose_Dest(-1, false)) {
				case kSpinnerDestinationPoliceStation:
					Game_Flag_Set(kFlagMcCoyInPoliceStation);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtPS01);
					Set_Enter(kSetPS01, kScenePS01);
					break;
				case kSpinnerDestinationMcCoysApartment:
					Game_Flag_Set(kFlagMcCoyInMcCoyApartment);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtMA01);
					Set_Enter(kSetMA01, kSceneMA01);
					break;
				case kSpinnerDestinationRuncitersAnimals:
					Game_Flag_Set(kFlagMcCoyInRunciters);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtRC01);
					Set_Enter(kSetRC01, kSceneRC01);
					break;
				case kSpinnerDestinationChinatown:
					Game_Flag_Set(kFlagMcCoyInChinaTown);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtCT01);
					Set_Enter(kSetCT01_CT12, kSceneCT01);
					break;
				case kSpinnerDestinationAnimoidRow:
					Game_Flag_Set(kFlagMcCoyInAnimoidRow);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtAR01);
					Set_Enter(kSetAR01_AR02, kSceneAR01);
					break;
				case kSpinnerDestinationDNARow:
					Game_Flag_Set(kFlagMcCoyInDNARow);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtDR01);
					Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
					break;
				case kSpinnerDestinationBradburyBuilding:
					Game_Flag_Set(kFlagMcCoyInBradburyBuilding);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtBB01);
					Set_Enter(kSetBB01, kSceneBB01);
					break;
				case kSpinnerDestinationNightclubRow:
					Game_Flag_Set(kFlagMcCoyInNightclubRow);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtNR01);
					Set_Enter(kSetNR01, kSceneNR01);
					break;
				case kSpinnerDestinationHysteriaHall:
					Game_Flag_Set(kFlagMcCoyInHysteriaHall);
					Game_Flag_Reset(kFlagSpinnerAtTB02);
					Game_Flag_Set(kFlagSpinnerAtHF01);
					Set_Enter(kSetHF01, kSceneHF01);
					break;
				default:
					Game_Flag_Set(kFlagMcCoyInTyrellBuilding);
					Loop_Actor_Walk_To_XYZ(kActorMcCoy, 44.0f, 12.0f, 176.0f, 0, false, false, false);
					break;
				}
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptTB07::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptTB07::SceneFrameAdvanced(int frame) {
	if (frame == 66) {
		Ambient_Sounds_Play_Sound(kSfxLIGHTON, 20, 99, 0, 0);
	}
	//return false;
}

void SceneScriptTB07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptTB07::PlayerWalkedIn() {
	int chapter = Global_Variable_Query(kVariableChapter);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 44.0f, 12.0f, 176.0f, 0, false, false, false);
	if ((chapter == 2
	  || chapter == 3
	 )
	 && !Game_Flag_Query(kFlagTB07RachaelTalk)
	) {
		Player_Set_Combat_Mode(false);
		McCoyTalkWithRachaelAndTyrell();
	}
}

void SceneScriptTB07::PlayerWalkedOut() {
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (Global_Variable_Query(kVariableChapter) < 4
	    && !Game_Flag_Query(kFlagMcCoyInTyrellBuilding)
	) {
		// Acts 2, 3 - use a spinner fly-through transition
		Outtake_Play(kOuttakeAway1,    true, -1);  // available in Acts 2, 3
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

}

void SceneScriptTB07::DialogueQueueFlushed(int a1) {
}

void SceneScriptTB07::McCoyTalkWithRachaelAndTyrell() {
	Game_Flag_Set(kFlagTB07RachaelTalk);
	Delay(1500);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 44.98f, 12.0f, 49.79f, 0, false, false, false);
	Actor_Face_Heading(kActorMcCoy, 178, true);
	Delay(3000);
	Actor_Put_In_Set(kActorRachael, kSetTB07);
	Actor_Set_At_XYZ(kActorRachael, -260.15f, 12.0f, -19.16f, 256);
	Actor_Change_Animation_Mode(kActorRachael, kAnimationModeIdle);
	Outtake_Play(kOuttakeRachel, true, -1);
	Loop_Actor_Walk_To_XYZ(kActorRachael, -146.15f, 12.0f, -5.84f, 0, false, false, false);
	Actor_Face_Actor(kActorRachael, kActorMcCoy, true);
	Actor_Says(kActorRachael, 480, 14);
	Actor_Face_Actor(kActorMcCoy, kActorRachael, true);
	Actor_Says(kActorMcCoy, 5315, 9);
	Actor_Says(kActorRachael, 490, kAnimationModeTalk);
	Actor_Face_Heading(kActorMcCoy, 178, true);
	Actor_Says(kActorMcCoy, 5320, 15);
	Actor_Says_With_Pause(kActorMcCoy, 5325, 1.0f, 19);
	Actor_Start_Speech_Sample(kActorRachael, 500);
	Loop_Actor_Walk_To_XYZ(kActorRachael, -60.15f, 12.0f, 60.84f, 0, false, false, false);
	Actor_Face_Actor(kActorRachael, kActorMcCoy, true);
	Actor_Face_Actor(kActorMcCoy, kActorRachael, true);
	Actor_Says(kActorMcCoy, 5330, 14);
	Actor_Says(kActorRachael, 510, 12);
	Actor_Says(kActorMcCoy, 5335, 16);
	Actor_Says(kActorRachael, 520, 17);
	Actor_Says(kActorMcCoy, 5340, kAnimationModeTalk);
	Actor_Start_Speech_Sample(kActorRachael, 530);
	Loop_Actor_Walk_To_XYZ(kActorRachael, -4.15f, 12.0f, 54.73f, 0, false, false, false);
	Actor_Says(kActorRachael, 540, 16);
	Actor_Says(kActorMcCoy, 5345, 18);
	if (_vm->_cutContent
	    && (_vm->_language == Common::ES_ESP
	        || _vm->_language == Common::IT_ITA)
	) {
		// Quote 560 is the second half of the sentence about Tyrell having important meetings to attend to
		// In ENG, DEU and FRA it is redundant because it, instead, re-uses a cut line from the removed elevator scene "Copies, only copies"
		// However, it's needed in ESP and ITA
		Actor_Says_With_Pause(kActorRachael, 550, 0.0f, 13);
		Actor_Says(kActorRachael, 560, kAnimationModeTalk);
	} else {
		Actor_Says(kActorRachael, 550, 13);
	}
	Actor_Says(kActorRachael, 570, 18);
	Actor_Says_With_Pause(kActorMcCoy, 5350, 0.0f, 18);
	Actor_Says(kActorRachael, 580, 16);
	Actor_Says(kActorMcCoy, 5355, 16);
	Actor_Says(kActorRachael, 590, 17);
	Actor_Says(kActorMcCoy, 5360, 17);
	Actor_Says(kActorMcCoy, 5365, 13);
	Actor_Says_With_Pause(kActorRachael, 600, 1.0f, 12);
	Actor_Says(kActorMcCoy, 5370, kAnimationModeTalk);
	Loop_Actor_Walk_To_XYZ(kActorRachael, -24.15f, 12.0f, -10.84f, 0, false, false, false);
	Actor_Says(kActorRachael, 610, 13);
	Actor_Face_Actor(kActorMcCoy, kActorRachael, true);
	Actor_Says(kActorMcCoy, 5375, 18);
	Actor_Says(kActorMcCoy, 5380, 19);
	Actor_Face_Actor(kActorRachael, kActorMcCoy, true);
	Actor_Says(kActorRachael, 620, 18);
	Actor_Says_With_Pause(kActorMcCoy, 5385, 2.0f, 12);
	Actor_Says_With_Pause(kActorMcCoy, 5390, 2.0f, 14);
	Actor_Says(kActorMcCoy, 5395, 15);
	Actor_Says_With_Pause(kActorRachael, 630, 0.0f, 14);
	Actor_Says(kActorMcCoy, 5400, 18);
	Actor_Says(kActorMcCoy, 5405, kAnimationModeTalk);
	Actor_Says(kActorRachael, 640, 12);
	Actor_Says(kActorMcCoy, 5410, 16);
	Actor_Says(kActorRachael, 650, 15);
	Actor_Says_With_Pause(kActorMcCoy, 5415, 1.0f, 17);
	Actor_Says(kActorMcCoy, 5420, 14);
	Actor_Says(kActorRachael, 660, 15);
	Actor_Put_In_Set(kActorTyrell, kSetTB07);
	Actor_Set_At_XYZ(kActorTyrell, 68.0f, 12.0f, 288.0f, 0);
	Actor_Change_Animation_Mode(kActorTyrell, kAnimationModeIdle);
	Scene_Loop_Set_Default(kTB07LoopMainShadeDown);
	Scene_Loop_Start_Special(kSceneLoopModeOnce, kTB07LoopShadeDrop, false);
	Actor_Start_Speech_Sample(kActorTyrell, 0);
	Loop_Actor_Walk_To_XYZ(kActorTyrell, 44.0f, 12.0f, 176.0f, 0, false, false, false);
	Actor_Face_Actor(kActorTyrell, kActorMcCoy, true);
	Actor_Face_Actor(kActorMcCoy, kActorTyrell, true);
	Actor_Face_Actor(kActorRachael, kActorTyrell, true);
	Actor_Says(kActorTyrell, 10, 12);
	Actor_Says(kActorTyrell, 20, kAnimationModeTalk);
	Actor_Says(kActorTyrell, 30, 12);
	Actor_Says(kActorMcCoy, 5425, 18);
	Actor_Set_Goal_Number(kActorRachael, kGoalRachaelLeavesAfterTyrellMeeting);
	Actor_Says(kActorTyrell, 40, 15);
	Actor_Start_Speech_Sample(kActorTyrell, 50);
	Loop_Actor_Walk_To_XYZ(kActorTyrell, -10.0f, 12.0f, 100.0f, 0, false, false, false);
	Actor_Face_Actor(kActorTyrell, kActorMcCoy, true);
	Actor_Face_Actor(kActorMcCoy, kActorTyrell, true);
	Actor_Says(kActorMcCoy, 5430, 17);
	Actor_Says(kActorMcCoy, 5435, 16);
	Actor_Says(kActorTyrell, 60, 14);
	Actor_Face_Actor(kActorMcCoy, kActorTyrell, true);
	Actor_Says(kActorMcCoy, 5440, 14);
	Actor_Says(kActorTyrell, 70, 13);
	Actor_Says(kActorMcCoy, 5445, 15);
	Actor_Says_With_Pause(kActorTyrell, 80, 1.0f, 12);
	Actor_Says(kActorTyrell, 90, 15);
	Actor_Says_With_Pause(kActorMcCoy, 5450, 1.0f, 15);
	Actor_Says(kActorMcCoy, 5455, 12);
	Actor_Says(kActorTyrell, 100, 14);
	Actor_Clue_Acquire(kActorMcCoy, kClueRachaelInterview, false, kActorRachael);
	Actor_Clue_Acquire(kActorMcCoy, kClueTyrellInterview, false, kActorTyrell);
	Loop_Actor_Walk_To_XYZ(kActorTyrell, -260.15f, 12.0f, -19.16f, 0, false, false, false);
}

} // End of namespace BladeRunner
