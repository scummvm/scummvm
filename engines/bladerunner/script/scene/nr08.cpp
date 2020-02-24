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

enum kNR08Loops {
	kNR08LoopPanFromNR05 = 0,
	kNR08LoopMainLoop = 1,
	kNR08LoopFadeLoop = 3
};

void SceneScriptNR08::InitializeScene() {
	if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleNR01GoToNR08) {
		Setup_Scene_Information(-1174.1f, 0.32f, 303.9f, 435);
	} else if (Game_Flag_Query(kFlagNR05toNR08)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kNR08LoopPanFromNR05, false);
		Scene_Loop_Set_Default(kNR08LoopMainLoop);
		Setup_Scene_Information(-1102.88f, 0.0f, 107.43f, 0);
		if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR08Dance) {
			Music_Stop(1);
		}
	} else if (Game_Flag_Query(kFlagNR06toNR08)) {
		Setup_Scene_Information(  -724.7f,  0.0f, 384.24f, 1000);
		Game_Flag_Reset(kFlagNR06toNR08);
	} else if (Game_Flag_Query(kFlagNR09toNR08)) {
		Setup_Scene_Information(-1663.33f, 0.65f, 342.84f,  330);
		Game_Flag_Reset(kFlagNR09toNR08);
	}

	Scene_Exit_Add_2D_Exit(0, 610, 0, 639, 479, 1);
	if (Actor_Query_Goal_Number(kActorDektora) != kGoalDektoraNR08Dance) {
		Scene_Exit_Add_2D_Exit(1,   0, 309,  30, 398, 3);
		Scene_Exit_Add_2D_Exit(2, 520, 330, 556, 386, 0);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxBARAMB1, 50, 38, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX1,  3, 60, 14, 14,   60,  90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX3,  3, 60, 14, 14,   60,  90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX4,  3, 60, 14, 14,   60,  90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX5,  3, 60, 14, 14,   60,  90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX6,  3, 60, 14, 14,   60,  90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX7,  3, 60, 14, 14,   60,  90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK1,   3, 60, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK2,   3, 60, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK3,   3, 60, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK4,   3, 60, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);

	Scene_Loop_Set_Default(kNR08LoopMainLoop);
}

void SceneScriptNR08::SceneLoaded() {
	Obstacle_Object("BOX283", true);
	Unobstacle_Object("BOX283", true);
	if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR08Dance) {
		Actor_Change_Animation_Mode(kActorDektora, 79);
	}
}

bool SceneScriptNR08::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR08::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptNR08::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptNR08::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR08::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1102.88f, 0.0f, 107.43f, 0, true, false, false)) {
			Game_Flag_Set(kFlagNR08toNR05);
			Set_Enter(kSetNR05_NR08, kSceneNR05);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -724.7f, 0.0f, 384.24f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 505, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 4, true, kAnimationModeIdle);
			Game_Flag_Set(kFlagNR08toNR06);
			Set_Enter(kSetNR06, kSceneNR06);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1663.33f, 0.65f, 342.84f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 831, false);
			Footstep_Sound_Override_On(2);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 6, true, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
			Game_Flag_Set(kFlagNR08toNR09);
			Set_Enter(kSetNR09, kSceneNR09);
		}
	}
	return false;
}

bool SceneScriptNR08::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptNR08::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing()) {
		playNextMusic();
	}

	Set_Fade_Color(0, 0, 0);

	if (frame >= 76
	 && frame < 91
	) {
		Set_Fade_Density((frame - 76) / 14.0f);
		Music_Stop(3);
		Ambient_Sounds_Play_Sound(kSfxDEKCLAP1, 27, 0, 99, 0);
	} else if (frame >= 91
	        && frame < 120
	) {
		Actor_Set_Invisible(kActorMcCoy, true);
		Set_Fade_Density(1.0f);
	} else if (frame >= 120
	        && frame < 135
	) {
		Set_Fade_Density((134 - frame) / 14.0f);
		Music_Play(kMusicArkDnce1, 61, 0, 1, -1, 0, 0);
	} else {
		Actor_Set_Invisible(kActorMcCoy, false);
		Set_Fade_Density(0.0f);
	}

	if ( Game_Flag_Query(kFlagNR08McCoyWatchingShow)
	 && !Game_Flag_Query(kFlagNR08Faded)
	) {
		Game_Flag_Set(kFlagNR08Faded);
		Scene_Exits_Disable();
		Scene_Loop_Set_Default(kNR08LoopMainLoop);
		Scene_Loop_Start_Special(kSceneLoopModeOnce, kNR08LoopFadeLoop, true);
	}

	if (frame == 95) {
		Actor_Put_In_Set(kActorDektora, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorDektora, 33, 0);
		Actor_Change_Animation_Mode(kActorDektora, kAnimationModeIdle);
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR07Sit);
		Scene_Exit_Add_2D_Exit(1,   0, 309,  30, 398, 3);
		Scene_Exit_Add_2D_Exit(2, 520, 330, 556, 386, 0);
	}

	if (frame == 130) {
		Scene_Exits_Enable();
	}
	//return false;
}

void SceneScriptNR08::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR08::PlayerWalkedIn() {
	if ( Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR08Dance
	 && !Game_Flag_Query(kFlagNR08DektoraShow)
	) {
		Game_Flag_Set(kFlagNR08DektoraShow);
		Ambient_Sounds_Play_Sound(kSfxDEKCLAP1, 27, 0, 99, 0);
		Music_Play(kMusicArkdFly1, 61, 0, 1, -1, 0, 0);
		Outtake_Play(kOuttakeDektora, true, -1);
	} else {
		Music_Adjust(51, 0, 2);
	}

	if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR08ReadyToRun) {
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Face_Heading(kActorDektora, 790, false);
		Loop_Actor_Travel_Stairs(kActorDektora, 8, true, kAnimationModeIdle);
#else
		Actor_Face_Heading(kActorDektora, 831, false);
		Loop_Actor_Travel_Stairs(kActorDektora, 7, true, kAnimationModeIdle);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR08GoToNR10);
	}

	if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleNR01GoToNR08) {
		Actor_Says(kActorSteele, 1640, 12);
		if (!Game_Flag_Query(kFlagDirectorsCut)) {
			Actor_Says(kActorMcCoy, 3790, 13);
			Actor_Says(kActorSteele, 1650, 14);
		}
		Actor_Says(kActorSteele, 1660, 12);
		Actor_Says(kActorMcCoy, 3795, 16);
		Actor_Says(kActorSteele, 1670, 13);
		Actor_Says(kActorSteele, 1680, 14);
		Actor_Says(kActorSteele, 1690, 15);
		Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNR08WalkOut);
		return; // true;
	}

	if (Game_Flag_Query(kFlagNR05toNR08)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1090.88f, 0.0f, 147.43f, 0, true, false, false);
		Game_Flag_Reset(kFlagNR05toNR08);
	}
	//return false;
}

void SceneScriptNR08::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(kFlagNR08toNR05)) {
		Music_Stop(2);
	}
}

void SceneScriptNR08::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR08::playNextMusic() {
	if (Music_Is_Playing()) {
		Music_Adjust(51, 0, 2);
	} else if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR08Dance) {
		Music_Play(kMusicArkdFly1, 61, 0, 1, -1, 0, 0);
	} else {
		int track = Global_Variable_Query(kVariableEarlyQBackMusic);
		if (track == 0) {
			Music_Play(kMusicDkoDnce1, 61, -80, 2, -1, 0, 0);
		} else if (track == 1) {
			Music_Play(kMusicStrip1, 41, -80, 2, -1, 0, 0);
		} else if (track == 2) {
			Music_Play(kMusicArkDnce1, 41, -80, 2, -1, 0, 0);
		}
		++track;
		if (track > 2) {
			track = 0;
		}
		Global_Variable_Set(kVariableEarlyQBackMusic, track);
	}
}

} // End of namespace BladeRunner
