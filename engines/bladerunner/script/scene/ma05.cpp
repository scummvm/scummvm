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

enum kMA05Loops {
	kMA05LoopInshot   = 0, //  0 -  29 // Frame 29 (in Acts 1 and 2 this ties in with Frame 30 of the Loop Main, in Act 5 it ties with frame 60?!)
	kMA05LoopMainLoop = 1, // 30 -  90
	kMA05LoopSpinner  = 3  // 91 - 150 // Frame 150 ties in with Frame 30 of Loop Main
};

enum kMA05Exits {
	kMA05ExitMA04 = 0
};

void SceneScriptMA05::InitializeScene() {
	if (Global_Variable_Query(kVariableChapter) != 2 && Global_Variable_Query(kVariableChapter) != 3) {
		Setup_Scene_Information(-7204.0f, 953.97f, 1651.0f, 0);
	} else {
#if BLADERUNNER_ORIGINAL_BUGS
		Setup_Scene_Information(-7207.0f, 956.17f, 1564.0f, 0);
#else
		Setup_Scene_Information(-7199.0f, 953.97f, 1605.0f, 0);
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	Scene_Exit_Add_2D_Exit(kMA05ExitMA04, 432, 21, 471, 226, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxROOFRAN1, 90,    0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxROOFAIR1, 40, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 50,   60, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  10, 100, 25,  50,    0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  10, 100, 25,  50,    0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 10,  70, 50, 100,    0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 10,  70, 50, 100,    0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSIREN2,  10,  70, 25,  25, -100, 0, -101, -101, 0, 0);
	if (isPhoneRinging()) {
		Ambient_Sounds_Add_Sound(kSfxVIDFONE1, 3, 3, 32, 32, 100, 100, -101, -101, 0, 0);
	}
	if (Global_Variable_Query(kVariableChapter) != 2 && Global_Variable_Query(kVariableChapter) != 3) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kMA05LoopInshot, false);
	}
	Scene_Loop_Set_Default(kMA05LoopMainLoop);
}

void SceneScriptMA05::SceneLoaded() {
	Obstacle_Object("Z-BOX-RAIL03", true);
	Footstep_Sounds_Set(0, 0);
	Footstep_Sounds_Set(1, 3);
}

bool SceneScriptMA05::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptMA05::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptMA05::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptMA05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptMA05::ClickedOnExit(int exitId) {
	if (exitId == kMA05ExitMA04) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7199.0f, 956.17f, 1579.0f, 0, false, false, false)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7199.0f, 956.17f, 1579.0f, 0, false, false, false);
			Game_Flag_Set(kFlagMA05toMA04);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -7199.0f, 953.97f, 1685.0f, 0, false);
			Set_Enter(kSetMA04, kSceneMA04);
		}
		return true;
	}
	return false;
}

bool SceneScriptMA05::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptMA05::SceneFrameAdvanced(int frame) {
	if (frame == 20) {
		Sound_Play(kSfxROOFLIT1, 70, -100, 100, 50);
	}
	//return true;
}

void SceneScriptMA05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptMA05::PlayerWalkedIn() {
	Music_Play(kMusicBRBlues, 52, 0, 2, -1, 0, 0);
	if ((Random_Query(0, 4) == 1 || (Game_Flag_Query(kFlagChapter1Ending) && !Game_Flag_Query(kFlagChapter1Ended))) && Global_Variable_Query(kVariableChapter) == 1) {
		Scene_Loop_Set_Default(kMA05LoopMainLoop);
		Scene_Loop_Start_Special(kSceneLoopModeOnce, kMA05LoopSpinner, true);
		Sound_Play(kSfxSPIN3A, 100, 0, 0, 50);
	}
	if (Game_Flag_Query(kFlagChapter1Ending) && !Game_Flag_Query(kFlagChapter1Ended)) {
		if (!Game_Flag_Query(kFlagDirectorsCut)) {
			Actor_Voice_Over(1260, kActorVoiceOver);
			Actor_Voice_Over(1270, kActorVoiceOver);
			Actor_Voice_Over(1280, kActorVoiceOver);
			Actor_Voice_Over(1290, kActorVoiceOver);
		}
		Game_Flag_Set(kFlagChapter1Ended);
		Player_Gains_Control();
	}
#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (Global_Variable_Query(kVariableChapter) == 2 || Global_Variable_Query(kVariableChapter) == 3) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -7199.0f, 953.97f, 1564.0f, 0, false, false, true);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
	//return false;
}

void SceneScriptMA05::PlayerWalkedOut() {
}

void SceneScriptMA05::DialogueQueueFlushed(int a1) {
}

bool SceneScriptMA05::isPhoneRinging() {
	return Global_Variable_Query(kVariableChapter) == 5
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallClovis)
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallCrystal)
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallDektora1)
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallDektora2)
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallLucy1)
		&& !Actor_Clue_Query(kActorMcCoy, kCluePhoneCallLucy2);
}

} // End of namespace BladeRunner
