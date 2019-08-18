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

void SceneScriptMA07::InitializeScene() {
	if (Game_Flag_Query(kFlagUG19toMA07)) {
		Setup_Scene_Information(  6.75f, -172.43f, 356.0f, 997);
		Game_Flag_Reset(kFlagUG19toMA07);
		Game_Flag_Set(kFlagUG19Available);
	} else if (Game_Flag_Query(kFlagPS14toMA07)) {
		Setup_Scene_Information(-312.0f,  -162.8f, 180.0f,   0);
	} else {
		Setup_Scene_Information( 104.0f, -162.16f,  56.0f, 519);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxRAIN10, 100, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxCOLONY,  100, 300, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,   60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,   60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2,  60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3,  50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4,  50, 180, 50, 100, 0, 0, -101, -101, 0, 0);

	if (Global_Variable_Query(kVariableChapter) > 1) {
		Scene_Exit_Add_2D_Exit(1, 0, 200, 50, 479, 3);
	}
	if (Game_Flag_Query(kFlagUG19Available)) {
		Scene_Exit_Add_2D_Exit(2, 176, 386, 230, 426, 2);
	}
	Scene_Exit_Add_2D_Exit(0, 270, 216, 382, 306, 0);
}

void SceneScriptMA07::SceneLoaded() {
	Obstacle_Object("BARRICADE", true);
	if (_vm->_cutContent) {
		if (Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelShouldBeOutsideMcCoysAct3) {
			Actor_Set_Goal_Number(kActorRachael, kGoalRachaelIsOutsideMcCoysBuildingAct3);
		}
	}
}

bool SceneScriptMA07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptMA07::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptMA07::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptMA07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptMA07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 104.0f, -162.0f, 56.0f, 12, true, false, false)) {
			if (Global_Variable_Query(kVariableChapter) == 4
			 && Game_Flag_Query(kFlagUG18GuzzaScene)
			) {
				Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyStartChapter5);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(kFlagMA07toMA06);
				Set_Enter(kSetMA06, kSceneMA06);
			}
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -400.0f, -162.8f, 185.08f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagMA07toPS14);
			Game_Flag_Reset(kFlagMcCoyInMcCoyApartment);
			Game_Flag_Set(kFlagMcCoyInPoliceStation);
			Set_Enter(kSetPS14, kScenePS14);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 8.0f, -172.43f, 356.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagMA07toUG19);
			Set_Enter(kSetUG19, kSceneUG19);
		}
		return true;
	}
	return false;
}

bool SceneScriptMA07::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptMA07::SceneFrameAdvanced(int frame) {
}

void SceneScriptMA07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == kActorGaff
	 && newGoal == kGoalGaffMA07Left
	) {
		Scene_Exits_Enable();
	}
}

void SceneScriptMA07::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagPS14toMA07)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -268.0f, -162.8f, 188.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagPS14toMA07);
	}

	if (Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsOutsideMcCoysBuildingAct3) {
		Actor_Set_Goal_Number(kActorRachael, kGoalRachaelIsOutWalksToPoliceHQAct3);
	} else if (_vm->_cutContent && Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsOutsideMcCoysBuildingAct4) {
		Actor_Set_Goal_Number(kActorRachael, kGoalRachaelIsOutWalksToPoliceHQAct4);
    }

	if (Game_Flag_Query(kFlagMA06toMA07)) {
		Game_Flag_Reset(kFlagMA06toMA07);
	}

	if (!Game_Flag_Query(kFlagMA07GaffTalk)
	 &&  Game_Flag_Query(kFlagUG18GuzzaScene)
	 &&  Global_Variable_Query(kVariableChapter) == 4
	) {
		Scene_Exits_Disable();
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// don't have McCoy with his gun drawn out when talking to Gaff here
		Player_Set_Combat_Mode(false);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_Goal_Number(kActorGaff, kGoalGaffMA07Wait);
	}

	if (Game_Flag_Query(kFlagMcCoyFreedOfAccusations)) {
		Actor_Voice_Over(1360, kActorVoiceOver);
		Actor_Voice_Over(1370, kActorVoiceOver);
		Actor_Voice_Over(1380, kActorVoiceOver);
		Actor_Voice_Over(1390, kActorVoiceOver);
		Actor_Voice_Over(1400, kActorVoiceOver);
		Delay(1000);
		Game_Flag_Reset(kFlagMcCoyFreedOfAccusations);
		Game_Flag_Set(kFlagMA06ToMA02);
		Set_Enter(kSetMA02_MA04, kSceneMA02);
	}
	//return false;

}

void SceneScriptMA07::PlayerWalkedOut() {
	if (_vm->_cutContent) {
		if (Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsOutsideMcCoysBuildingAct3
			|| Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsOutWalksToPoliceHQAct3
			|| Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsOutFleeingToPoliceHQAct3
		) {
			Actor_Set_Goal_Number(kActorRachael, kGoalRachaelAtEndOfAct3IfNotMetWithMcCoy);
		} else if (Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsOutResumesWalkToPoliceHQAct3) {
			Actor_Set_Goal_Number(kActorRachael, kGoalRachaelAtEndOfAct3IfMetWithMcCoy);
		} else if (Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsOutsideMcCoysBuildingAct4
			|| Actor_Query_Goal_Number(kActorRachael) == kGoalRachaelIsOutWalksToPoliceHQAct4
		) {
			Actor_Set_Goal_Number(kActorRachael, kGoalRachaelAtEndOfAct4);
		}
	}
}

void SceneScriptMA07::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
