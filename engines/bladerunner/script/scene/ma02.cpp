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

enum kMA02Exits {
	kMA02ExitMA06 = 0,
	kMA02ExitMA04 = 1
};

void SceneScriptMA02::InitializeScene() {
	if (Game_Flag_Query(kFlagMA04ToMA02)) {
		Setup_Scene_Information(-172.0f, -144.13f,   6.27f, 500);
	} else { // From MA06
		Setup_Scene_Information( 23.19f, -144.12f, 378.27f, 750);
		if (Global_Variable_Query(kVariableChapter) == 4) {
			Actor_Set_Goal_Number(kActorRajif, 300);
		}
		Game_Flag_Reset(kFlagMA04WatchedTV);
	}
	Scene_Exit_Add_2D_Exit(kMA02ExitMA06, 538, 84, 639, 327, 1);
	Scene_Exit_Add_2D_Exit(kMA02ExitMA04,  56, 98, 150, 260, 0);

	if (Global_Variable_Query(kVariableChapter) == 5
	 && Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
	) {
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieDead);
		Actor_Change_Animation_Mode(kActorMaggie, 88);
		Actor_Put_In_Set(kActorMaggie, kSetMA02_MA04);
		Actor_Set_At_XYZ(kActorMaggie, -35.51f, -144.12f, 428.0f, 0);
		Actor_Retired_Here(kActorMaggie, 24, 24, 1, -1);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxAPRTAMB5, 12, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxBRBED5,   25, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxSWEEP2,   5,  30,  5,  5, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP3,   5,  30,  5,  5, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP4,   5,  30,  5,  5, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 10,  60, 20, 20,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 10,  60, 20, 20,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSIREN2,  10,  60, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  60, 180, 14, 14,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  60, 180, 14, 14,    0,   0, -101, -101, 0, 0);
	if (isPhoneRinging()) {
		Ambient_Sounds_Add_Sound(kSfxVIDFONE1, 3, 3, 27, 27, -100, -100, -100, -100, 99, 0);
	}

	if ( Global_Variable_Query(kVariableChapter) == 5
	 &&  Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
	 && !Actor_Clue_Query(kActorMcCoy, kClueCrystalsCigarette)
	) {
		Overlay_Play("MA02OVER", 0, true, false, 0);
	}
}

void SceneScriptMA02::SceneLoaded() {
	Obstacle_Object("COUCH1", true);
	Unobstacle_Object("COUCH1", true);
	Clickable_Object("BAR-MAIN");
	Clickable_Object("E-ESPER");
}

bool SceneScriptMA02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptMA02::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("E-ESPER", objectName)) {
		Actor_Face_Object(kActorMcCoy, "E-ESPER", true);
		Delay(1000);
		ESPER_Flag_To_Activate();
		return true;
	}

	if (Object_Query_Click("BAR-MAIN", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -29.0f, -140.4f, 298.0f, 36, true, false, false)) {
			Actor_Face_Object(kActorMcCoy, "BAR-MAIN", true);
			if (Global_Variable_Query(kVariableChapter) < 4) {
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02GetFed);
			} else if ( Global_Variable_Query(kVariableChapter) == 5
			        &&  Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
			        && !Actor_Clue_Query(kActorMcCoy, kClueCrystalsCigarette)
			) {
				Overlay_Remove("MA02OVER");
				Item_Pickup_Spin_Effect(kModelAnimationCrystalsCigarette, 480, 240);
				Actor_Voice_Over(1150, kActorVoiceOver);
				Actor_Voice_Over(1160, kActorVoiceOver);
				Actor_Voice_Over(1170, kActorVoiceOver);
				Actor_Voice_Over(1180, kActorVoiceOver);
				Actor_Voice_Over(1190, kActorVoiceOver);
				Actor_Voice_Over(1200, kActorVoiceOver);
				Actor_Clue_Acquire(kActorMcCoy, kClueCrystalsCigarette, true, -1);
			} else {
				Actor_Says(kActorMcCoy, 8526, 0);
			}
			return true;
		}
	}
	return false;
}

bool SceneScriptMA02::ClickedOnActor(int actorId) {
	if (actorId == kActorMaggie
	 && Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieDead
	) {
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorMaggie, 30, true, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
			Actor_Voice_Over(1140, kActorVoiceOver);
		}
	}
	return false;
}

bool SceneScriptMA02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptMA02::ClickedOnExit(int exitId) {
	if (exitId == kMA02ExitMA06) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 23.19f, -144.12f, 378.27f, 0, true, false, false)) {
			Music_Stop(10);
			Game_Flag_Set(kFlagMA02toMA06);
			Set_Enter(kSetMA06, kSceneMA06);
		}
		return true;
	}

	if (exitId == kMA02ExitMA04) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -168.0f, -144.13f, 10.27f, 0, true, false, false)) {
			Game_Flag_Set(kFlagMA02ToMA04);
			Set_Enter(kSetMA04, kSceneMA04);
		}
		return true;
	}
	return false;
}

bool SceneScriptMA02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptMA02::SceneFrameAdvanced(int frame) {
}

void SceneScriptMA02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptMA02::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagMA06ToMA02)) {
		selectNextTvNews();
	}

	if (Game_Flag_Query(kFlagMA04ToMA02)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -148.12f, -144.13f, 34.27f, 0, true, false, false);
	}

	if ( Global_Variable_Query(kVariableChapter) == 4
	 && !Game_Flag_Query(kFlagMA02RajifTalk)
	) {
		Game_Flag_Set(kFlagUG07Empty);
		Game_Flag_Set(kFlagMA02RajifTalk);
		talkWithRajif();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 23.19f, -144.12f, 378.27f, 0, false, false, false);
		Game_Flag_Set(kFlagMA02toMA06);
		Set_Enter(kSetMA06, kSceneMA06);
		//	return true;
		return;
	}

	if ( Global_Variable_Query(kVariableChapter) == 5
	 && !Game_Flag_Query(kFlagMA02Chapter5Started)
	) {
		if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
			Actor_Says(kActorMcCoy, 2390, kAnimationModeIdle);
			Music_Play(kMusicBRBlues, 25, 0, 3, -1, 0, 0);
		} else {
			Actor_Says(kActorMcCoy, 2385,  kAnimationModeTalk);
		}
		Game_Flag_Set(kFlagMA02Chapter5Started);
		Autosave_Game(3);
	}

	if ( Global_Variable_Query(kVariableChapter) < 4
	 && !Game_Flag_Query(kFlagMA04ToMA02)
	 &&  Actor_Query_Goal_Number(kActorMaggie) != 2
	) {
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02WalkToEntrance);

		if (!Game_Flag_Query(kFlagMA02MaggieIntroduced)) {
			Game_Flag_Set(kFlagMA02MaggieIntroduced);
			Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
			Actor_Voice_Over(1210, kActorVoiceOver);
			if (!Game_Flag_Query(kFlagDirectorsCut)) {
				Actor_Voice_Over(1220, kActorVoiceOver);
			}
			Actor_Voice_Over(1230, kActorVoiceOver);
			if (!Game_Flag_Query(kFlagDirectorsCut)) {
				Actor_Voice_Over(1240, kActorVoiceOver);
				Actor_Voice_Over(1250, kActorVoiceOver);
			}
		}
	}

	Game_Flag_Reset(kFlagMA04ToMA02);
	Game_Flag_Reset(kFlagMA06ToMA02);
	//return false;
	return;
}

void SceneScriptMA02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptMA02::DialogueQueueFlushed(int a1) {
}

void SceneScriptMA02::talkWithRajif() {
	Actor_Says(kActorMcCoy, 2365, 13);
	Actor_Says(kActorRajif, 0, 13);
	Actor_Says(kActorMcCoy, 2370, 13);
	Actor_Says(kActorRajif, 10, 13);
	Actor_Says(kActorMcCoy, 2375, 13);
	if (_vm->_cutContent) {
		Actor_Says_With_Pause(kActorRajif, 20, 0.0f, 13);
		Actor_Says(kActorRajif, 30, 13);
	} else {
		Actor_Says(kActorRajif, 20, 13);
	}
	Actor_Says(kActorMcCoy, 2380, 13);
	Sound_Play(kSfxSHOTCOK1, 100, 0, 100, 50);
	Actor_Says(kActorRajif, 40, 13);
	Delay(3000);
}

bool SceneScriptMA02::isPhoneRinging() {
	return  Global_Variable_Query(kVariableChapter) == 5
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallClovis)
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallCrystal)
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallDektora1)
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallDektora2)
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallLucy1)
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallLucy2);
}

void SceneScriptMA02::selectNextTvNews() {
	int i = 0;
	int arr[7];
	if (Global_Variable_Query(kVariableChapter) < 4 && Game_Flag_Query(kFlagGordoIsReplicant)) {
		arr[i++] = 0;
	}
	arr[i++] = 1;
	if (Global_Variable_Query(kVariableChapter) >= 3) {
		arr[i++] = 2;
	}
	if (Global_Variable_Query(kVariableChapter) >= 2
	 && Global_Variable_Query(kVariableChapter) <= 4
	) {
		arr[i++] = 3;
	}
	if (Game_Flag_Query(kFlagCT04HomelessBodyFound)
	 && Game_Flag_Query(kFlagCT04HomelessBodyInDumpster)
	) {
		arr[i++] = 4;
	}
	Global_Variable_Set(kVariableNextTvNews, arr[Random_Query(0, i - 1)]);
}

} // End of namespace BladeRunner
