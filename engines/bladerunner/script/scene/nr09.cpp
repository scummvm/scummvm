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

void SceneScriptNR09::InitializeScene() {
	if (Game_Flag_Query(kFlagNR10toNR09)) {
		if (!Game_Flag_Query(kFlagNR10CameraDestroyed)) {
			Ambient_Sounds_Adjust_Looping_Sound(kSfx35MM, 22, 100, 2);
		}
		Game_Flag_Reset(kFlagNR10toNR09);
		Setup_Scene_Information(-556.07f, 0.35f, 399.04f, 440);
	} else {
		if (!Game_Flag_Query(kFlagNR10CameraDestroyed)) {
			Ambient_Sounds_Add_Looping_Sound(kSfx35MM, 22, 100, 1);
		}
		Setup_Scene_Information(-704.07f, 0.35f, 663.04f,   0);
	}

	Scene_Exit_Add_2D_Exit(0, 400, 100, 440, 280, 1);
	Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);

	Ambient_Sounds_Add_Looping_Sound(kSfxCTDRONE1, 22, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxBRBED5,   33, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,  2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,  2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,  2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
}

void SceneScriptNR09::SceneLoaded() {
	Obstacle_Object("X2NEWSPAPER", true);
	Unobstacle_Object("X2NEWSPAPER", true);
	Unclickable_Object("X2NEWSPAPER");
}

bool SceneScriptNR09::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR09::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptNR09::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptNR09::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR09::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -564.07f, 0.35f, 399.04f, 0, true, false, false)) {
			Game_Flag_Set(kFlagNR09toNR10);
			Set_Enter(kSetNR10, kSceneNR10);
			return true;
		}
	}

	if (exitId == 1) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		int v1 = Loop_Actor_Walk_To_XYZ(kActorMcCoy, -704.07f, 0.35f, 663.04f, 0, true, false, false);
		if (!v1) {
			Game_Flag_Set(kFlagNR09toNR08);
			Set_Enter(kSetNR05_NR08, kSceneNR08);
			return true;
		}
	}
	return false;
}

bool SceneScriptNR09::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptNR09::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing()) {
		playNextMusic();
	}
}

void SceneScriptNR09::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR09::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagNR08toNR09)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -704.07f, 0.35f, 623.04f, 0, false, false, false);
		Game_Flag_Reset(kFlagNR08toNR09);
	}
	//return false;
}

void SceneScriptNR09::PlayerWalkedOut() {
	if (Game_Flag_Query(kFlagNR09toNR10)) {
		Music_Stop(2);
	}
}

void SceneScriptNR09::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR09::playNextMusic() {
	if (Music_Is_Playing()) {
		Music_Adjust(31, -80, 2);
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
