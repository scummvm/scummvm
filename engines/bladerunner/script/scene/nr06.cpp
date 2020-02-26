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

void SceneScriptNR06::InitializeScene() {
	playNextMusic();
	if (Game_Flag_Query(kFlagNR07toNR06)) {
		Setup_Scene_Information( 48.0f, -71.88f,  -26.0f, 782);
	} else {
		Setup_Scene_Information(-36.0f,   0.37f, -373.0f, 592);
	}

	Scene_Exit_Add_2D_Exit(0, 533, 234, 592, 414, 1);
	Scene_Exit_Add_2D_Exit(1, 238, 137, 337, 322, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxDRAMB5, 25, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBARSFX1,  3, 60, 8, 12,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX3,  3, 60, 8,  8,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX4,  3, 60, 8,  8,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX5,  3, 60, 8,  8,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX6,  3, 60, 8,  8,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX7,  3, 60, 8,  8,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK1,   3, 60, 8,  8,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK2,   3, 60, 8,  8,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK3,   3, 60, 8,  8,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK4,   3, 60, 8,  8,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 8,  8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 8,  8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 8,  8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 8,  8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 8,  8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 8,  8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 8,  8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 8,  8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 8,  8, -100, 100, -101, -101, 0, 0);
}

void SceneScriptNR06::SceneLoaded() {
	Obstacle_Object("CHAIR01", true);
	Unobstacle_Object("LOFT04", true);
	Unobstacle_Object("LINE02", true);
	Unobstacle_Object("WALL01", true);
	Unclickable_Object("CHAIR01");
}

bool SceneScriptNR06::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR06::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptNR06::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptNR06::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR06::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 48.0f, -71.88f, -26.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Game_Flag_Set(kFlagNR06toNR07);
			Set_Enter(kSetNR07, kSceneNR07);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -137.0f, -71.88f, -243.0f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 95, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 8, true, kAnimationModeIdle);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -36.0f, 0.37f, -373.0f, 0, false, false, false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagNR06toNR08);
			Set_Enter(kSetNR05_NR08, kSceneNR08);
		}
		return true;
	}
	return false;
}

bool SceneScriptNR06::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptNR06::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing()) {
		playNextMusic();
	}
	//return false;
}

void SceneScriptNR06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR06::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagNR07toNR06)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -3.0f, -71.88f, -26.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagNR07toNR06);
	} else {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -81.72f, 0.12f, -323.49f, 0, false, false, false);
		Actor_Face_Heading(kActorMcCoy, 600, false);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 8, false, kAnimationModeIdle);
		Game_Flag_Reset(kFlagNR08toNR06);
	}
	//return false;
}

void SceneScriptNR06::PlayerWalkedOut() {
	if (Game_Flag_Query(kFlagNR06toNR07)) {
		Music_Stop(2);
	}
}

void SceneScriptNR06::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR06::playNextMusic() {
	if (Music_Is_Playing()) {
		Music_Adjust(31, 80, 2);
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
