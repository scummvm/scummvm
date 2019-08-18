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

void SceneScriptUG05::InitializeScene() {
	if (Game_Flag_Query(kFlagHF07toUG06)) {
		if ( Game_Flag_Query(kFlagHF05PoliceArrived)
		 && !Game_Flag_Query(kFlagHF07Hole)
		) {
			Setup_Scene_Information(-356.35f, 132.77f, -1092.36f, 389);
		} else {
			Setup_Scene_Information( -180.0f,  37.28f,  -1124.0f, 296);
		}
	} else {
		Setup_Scene_Information(0.0f, -1.37f, 0.0f, 0);
		Game_Flag_Reset(kFlagUG04toUG05);
	}

	Scene_Exit_Add_2D_Exit(0, 215, 240, 254, 331, 3);
	if (!Game_Flag_Query(kFlagHF05PoliceArrived)) {
		Scene_Exit_Add_2D_Exit(1, 303, 422, 639, 479, 2);
	}
	if (!Game_Flag_Query(kFlagHF05PoliceArrived)
	 ||  Game_Flag_Query(kFlagHF07Hole)
	) {
		Scene_Exit_Add_2D_Exit(2, 352, 256, 393, 344, 0);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRUNOFF, 28, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED2,   40, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxPIPER1,   2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG2,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG3,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG4,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK1,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK2,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M1,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M3,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M2, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M3, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG1,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG5,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK3,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG6,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagHF07Hole)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptUG05::SceneLoaded() {
	if (!Game_Flag_Query(kFlagHF07Hole)) {
		Unobstacle_Object("DROPPED CAR OBSTACL", true);
	}
	Obstacle_Object("VANBODY", true);
}

bool SceneScriptUG05::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG05::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG05::ClickedOnActor(int actorId) {
	if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, actorId, 30, true, false)) {
		Actor_Face_Actor(kActorMcCoy, actorId, true);

		if ( actorId == kActorOfficerGrayford
		 &&  Game_Flag_Query(kFlagHF07Hole)
		 && !Game_Flag_Query(kFlagUG05TalkToPolice)
		) {
			Actor_Says(kActorOfficerGrayford, 220, -1);
			Actor_Says(kActorMcCoy, 5540, 14);
			Actor_Says(kActorOfficerGrayford, 230, -1);
			Actor_Says(kActorMcCoy, 5545, 17);
			Actor_Says(kActorOfficerGrayford, 240, -1);
			Actor_Says(kActorMcCoy, 5550, 3);
			Game_Flag_Set(kFlagUG05TalkToPolice);
			return false;
		}

		int affectionTowardsActor = getAffectionTowardsActor();
		if (actorId == affectionTowardsActor) {
			endGame();
			return true;
		}
		return false;
	}
	return false;
}

bool SceneScriptUG05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if ( Game_Flag_Query(kFlagHF05PoliceArrived)
		 && !Game_Flag_Query(kFlagHF07Hole)
		) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -356.35f, 132.77f, -1092.36f, 0, false, false, false);
			Game_Flag_Set(kFlagUG06toHF07);
			Set_Enter(kSetHF07, kSceneHF07);
		} else if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -156.72f, 3.03f, -1118.17f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 760, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 3, true, kAnimationModeIdle);
			Game_Flag_Set(kFlagUG06toHF07);
			Set_Enter(kSetHF07, kSceneHF07);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 4.0f, -11.67f, -4.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagUG05toUG04);
			Set_Enter(kSetUG04, kSceneUG04);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Game_Flag_Query(kFlagHF01TalkToLovedOne)) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 0.0f, -1.37f, -1500.0f, 0, true, false, false)) {
				if (!Game_Flag_Query(kFlagUG05TalkAboutTunnel)) {
					Actor_Voice_Over(2600, kActorVoiceOver);
					Actor_Voice_Over(2610, kActorVoiceOver);
					Game_Flag_Set(kFlagUG05TalkAboutTunnel);
				}
				return true;
			}
		} else {
			int affectionTowardsActor = getAffectionTowardsActor();
			bool v2;
			if (affectionTowardsActor == -1) {
				v2 = Loop_Actor_Walk_To_XYZ(kActorMcCoy, 0.0f, -1.37f, -1500.0f, 0, true, false, false) != 0;
			} else {
				v2 = Loop_Actor_Walk_To_Actor(kActorMcCoy, affectionTowardsActor, 30, true, false) != 0;
			}
			if (!v2) {
				endGame();
				return true;
			}
		}
	}
	return false;
}

bool SceneScriptUG05::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG05::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG05::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagHF05PoliceArrived)) {
		if (Game_Flag_Query(kFlagHF07Hole)) {
			Music_Stop(2);

			Actor_Put_In_Set(kActorOfficerGrayford, kSetUG05);
			Actor_Set_At_XYZ(kActorOfficerGrayford, 4.22f, -1.37f, -925.0f, 750);
			Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordDead);
			Actor_Retired_Here(kActorOfficerGrayford, 70, 36, true, -1);

			int affectionTowardsActor = getAffectionTowardsActor();
			if (affectionTowardsActor == kActorDektora) {
				Actor_Put_In_Set(kActorDektora, kSetUG05);
				Actor_Set_At_XYZ(kActorDektora, -100.0f, -10.31f, -906.0f, 866);
				Actor_Force_Stop_Walking(kActorDektora);
			} else if (affectionTowardsActor == kActorLucy) {
				Actor_Put_In_Set(kActorLucy, kSetUG05);
				Actor_Set_At_XYZ(kActorLucy, -100.0f, -10.31f, -906.0f, 866);
				Actor_Force_Stop_Walking(kActorLucy);
			}
		} else {
			if (!Actor_Query_In_Set(kActorOfficerLeary, kSetUG05)) {
				Actor_Put_In_Set(kActorOfficerLeary, kSetUG05);
				Actor_Set_At_XYZ(kActorOfficerLeary, 0.0f, -1.37f, -1400.0f, 768);
			}

			if (!Actor_Query_In_Set(kActorOfficerGrayford, kSetUG05)) {
				ADQ_Flush();
				ADQ_Add(kActorOfficerGrayford, 280, kAnimationModeTalk);
				Actor_Put_In_Set(kActorOfficerGrayford, kSetUG05);
				Actor_Set_At_XYZ(kActorOfficerGrayford, -16.0f, -1.37f, -960.0f, 768);
			}
		}
	}

	if (Game_Flag_Query(kFlagHF07toUG06)) {
		if ( Game_Flag_Query(kFlagHF05PoliceArrived)
		 && !Game_Flag_Query(kFlagHF07Hole)
		) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -288.35f, 132.77f, -1092.36f, 0, true, false, false);
		} else {
			Loop_Actor_Travel_Stairs(kActorMcCoy, 2, false, kAnimationModeIdle);
		}
	}

	if (Game_Flag_Query(kFlagHF05PoliceArrived)) {
		Game_Flag_Query(kFlagHF07Hole); // bug in game?
	}
	Game_Flag_Reset(kFlagHF07toUG06);
}

void SceneScriptUG05::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptUG05::DialogueQueueFlushed(int a1) {
}

int SceneScriptUG05::getAffectionTowardsActor() {
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

void SceneScriptUG05::endGame() {
	int affectionTowardsActor = getAffectionTowardsActor();
	if (affectionTowardsActor != -1) {
		Actor_Face_Actor(kActorMcCoy, affectionTowardsActor, true);
		Actor_Face_Actor(affectionTowardsActor, kActorMcCoy, true);
		Actor_Says(kActorMcCoy, 5535, 13);
		if (affectionTowardsActor == kActorDektora) {
			Actor_Says(kActorDektora, 1110, 15);
		} else {
			Actor_Says(kActorLucy, 670, 17);
		}
	}

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);

	if (affectionTowardsActor == kActorLucy) {
		if (!Game_Flag_Query(kFlagLucyIsReplicant)) {
			Outtake_Play(kOuttakeEnd1A, false, -1);
		} else if (Global_Variable_Query(kVariableDNAEvidence) > 3) {
			Outtake_Play(kOuttakeEnd1B, false, -1);
		} else {
			Outtake_Play(kOuttakeEnd1C, false, -1);
		}
	} else if (affectionTowardsActor == kActorDektora) {
		if (!Game_Flag_Query(kFlagDektoraIsReplicant)) {
			Outtake_Play(kOuttakeEnd1D, false, -1);
		} else if (Global_Variable_Query(kVariableDNAEvidence) > 3) {
			Outtake_Play(kOuttakeEnd1E, false, -1);
		} else {
			Outtake_Play(kOuttakeEnd1F, false, -1);
		}
	} else {
		Outtake_Play(kOuttakeEnd3, false, -1);
	}
	Outtake_Play(kOuttakeEnd2, false, -1);

	Game_Over();
}

} // End of namespace BladeRunner
