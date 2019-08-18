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

void SceneScriptUG06::InitializeScene() {
	if (Game_Flag_Query(kFlagChapter4Intro)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeMovieC1, false, -1);
		if (Game_Flag_Query(kFlagLutherLanceIsReplicant)) {
			Outtake_Play(kOuttakeMovieC3, false, -1);
		} else {
			Outtake_Play(kOuttakeMovieC2, false, -1);
		}
		Game_Flag_Reset(kFlagChapter4Intro);
	}

	if (Game_Flag_Query(kFlagUG04toUG06)) {
		Setup_Scene_Information(  23.0f,   0.0f,  321.0f,   0);
	} else if (Game_Flag_Query(kFlagNR01toUG06)) {
		Setup_Scene_Information(  66.0f, 153.0f, -301.4f, 512);
	} else {
		Setup_Scene_Information(-165.0f,   1.0f,   89.0f, 990);
	}
	Scene_Exit_Add_2D_Exit(0,   0,  0,  30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 294, 68, 544, 236, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxFIREBD1, 18, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,  40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED2,  40, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxPIPER1,   2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK1,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M2,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M3,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M1, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M3, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK3,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK5,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG2,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG4,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG5,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG6,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
}

void SceneScriptUG06::SceneLoaded() {
	Obstacle_Object("BOX06", true);
	Unobstacle_Object("BOX06", true);
	Unobstacle_Object("BOX07", true);
	Unobstacle_Object("BOX16", true);
	Unobstacle_Object("BOX05", true);
}

bool SceneScriptUG06::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG06::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG06::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG06::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG06::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 23.0f, 0.0f, 321.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG06toUG04);
			Set_Enter(kSetUG04, kSceneUG04);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 66.0f, 0.0f, -90.0f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 0, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 17, true, kAnimationModeIdle);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 66.0f, 153.0f, -446.0f, 0, false, false, false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG06toNR01);
			Set_Enter(kSetNR01, kSceneNR01);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG06::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG06::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG06::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagUG04toUG06)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 23.0f, 0.0f, 273.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagUG04toUG06);
	}

	if (Game_Flag_Query(kFlagNR01toUG06)) {
		Loop_Actor_Travel_Stairs(kActorMcCoy, 17, false, kAnimationModeIdle);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 66.0f, 0.0f, -36.91f, 0, false, false, false);
		Game_Flag_Reset(kFlagNR01toUG06);
	}

	if ( Global_Variable_Query(kVariableChapter) == 4
	 && !Game_Flag_Query(kFlagUG06Chapter4Started)
	) {
		Player_Loses_Control();
		Actor_Voice_Over(2620, kActorVoiceOver);
		Actor_Voice_Over(2630, kActorVoiceOver);
		Actor_Voice_Over(2640, kActorVoiceOver);
		Actor_Voice_Over(2650, kActorVoiceOver);
		Actor_Voice_Over(2660, kActorVoiceOver);
		Actor_Voice_Over(2670, kActorVoiceOver);
		Actor_Voice_Over(2680, kActorVoiceOver);
		Actor_Voice_Over(2690, kActorVoiceOver);
		Actor_Voice_Over(2700, kActorVoiceOver);
		Player_Gains_Control();
		Game_Flag_Set(kFlagUG06Chapter4Started);
		Autosave_Game(2);
	}
	//return false;
}

void SceneScriptUG06::PlayerWalkedOut() {
}

void SceneScriptUG06::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
