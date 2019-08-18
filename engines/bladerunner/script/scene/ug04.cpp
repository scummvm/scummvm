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

enum kUG04Loops {
	kUG04LoopTrainLoop = 0,
	kUG04LoopMainLoop  = 1
};

void SceneScriptUG04::InitializeScene() {
	if (Game_Flag_Query(kFlagUG05toUG04)) {
		Setup_Scene_Information(   0.0f, -1.74f, -2400.0f, 496);
		Game_Flag_Reset(kFlagUG05toUG04);
	} else if (Game_Flag_Query(kFlagUG06toUG04)) {
		Setup_Scene_Information( 164.0f, 11.87f, -1013.0f,  83);
	} else {
		Setup_Scene_Information(-172.0f, 16.29f,  -735.0f, 380);
		Game_Flag_Reset(kFlagUG03toUG04);
	}

	Scene_Exit_Add_2D_Exit(0, 123, 308, 159, 413, 3);
	if (Global_Variable_Query(kVariableChapter) > 3) {
		Scene_Exit_Add_2D_Exit(1, 256, 333, 290, 373, 0);
	}
	Scene_Exit_Add_2D_Exit(2, 344, 298, 451, 390, 1);

	Ambient_Sounds_Add_Looping_Sound(kSfxSTMLOOP7, 25, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED2,   40, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxPIPER1,   2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG1,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG2,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG4,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG6,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M1,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M1, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M1, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK1,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK3,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK5,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);

#if BLADERUNNER_ORIGINAL_BUGS
	Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kUG04LoopTrainLoop, false);
#endif // BLADERUNNER_ORIGINAL_BUGS
	Scene_Loop_Set_Default(kUG04LoopMainLoop);
}

void SceneScriptUG04::SceneLoaded() {
#if BLADERUNNER_ORIGINAL_BUGS
	Obstacle_Object("NAV", true); // a bug? this object does not exist in the scene
#else
	Obstacle_Object("VAN", true);
#endif // BLADERUNNER_ORIGINAL_BUGS
	Unobstacle_Object("RUBBLE", true);
	Unobstacle_Object("FLOOR DEBRIS WADS", true);
	Unobstacle_Object("FLOOR DEBRIS WADS01", true);
	Unobstacle_Object("FLOOR DEBRIS WADS02", true);

#if BLADERUNNER_ORIGINAL_BUGS
#else
	if ((Global_Variable_Query(kVariableChapter) == 3)
	    || (Global_Variable_Query(kVariableChapter) > 3 && Random_Query(1, 4) == 1)
	) {
		// Enhancement: don't always play the overground train after chapter 3
		// Bug fix: don't remove control from player. There is no chance to glitch into the scenery
		// while the video is playing and rats may attack!
		// Moved in SceneLoaded because the same code in InitializeScene
		// resulted in a infinite loop of the special loop, when mode is set to kSceneLoopModeOnce instead of kSceneLoopModeLoseControl
		Scene_Loop_Set_Default(kUG04LoopMainLoop);
		Scene_Loop_Start_Special(kSceneLoopModeOnce, kUG04LoopTrainLoop, false);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptUG04::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG04::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -172.0f, 16.29f, -735.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagUG04toUG03);
			Set_Enter(kSetUG03, kSceneUG03);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 0.0f, -1.74f, -2400.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagUG04toUG05);
			Set_Enter(kSetUG05, kSceneUG05);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 164.0f, 11.87f, -1013.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagUG04toUG06);
			Set_Enter(kSetUG06, kSceneUG06);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG04::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG04::SceneFrameAdvanced(int frame) {
	if (frame == 1) {
		Ambient_Sounds_Play_Sound(kSfxSUBWAY1, 90, -100, 100, 100);
	}
}

void SceneScriptUG04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG04::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagUG06toUG04)) {
#if BLADERUNNER_ORIGINAL_BUGS
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 60.0f, -1.74f, -976.0f, 6, false, false, false);
#else
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 60.0f, -1.74f, -976.0f, 6, true, false, false);
#endif
		Game_Flag_Reset(kFlagUG06toUG04);
	}
	if (_vm->_cutContent
	    && !Game_Flag_Query(kFlagUG04DispatchOnHoodooRats)
	    && !Game_Flag_Query(kFlagMcCoyCommentsOnHoodooRats)
	) {
		if (Random_Query(0, 2) == 1) {
			Game_Flag_Set(kFlagUG04DispatchOnHoodooRats);
			ADQ_Add_Pause(Random_Query(0, 1) * 1000);
			ADQ_Add(kActorDispatcher, 340, kAnimationModeTalk); // Southern Kipple Unit 2 LA.
			if (Random_Query(0, 1) == 0) {
				// Leary responds
				ADQ_Add(kActorOfficerLeary, 240, kAnimationModeTalk); // LA, South Kipple Unit 2. Go ahead.
				ADQ_Add(kActorDispatcher, 350, kAnimationModeTalk);
				ADQ_Add(kActorDispatcher, 360, kAnimationModeTalk);
				ADQ_Add(kActorOfficerLeary, 250, kAnimationModeTalk);
			} else {
				// Grayford responds
				ADQ_Add(kActorOfficerGrayford, 520, kAnimationModeTalk); // LA, South Kipple Unit 2. Go ahead.
				ADQ_Add(kActorDispatcher, 350, kAnimationModeTalk);
				ADQ_Add(kActorDispatcher, 360, kAnimationModeTalk);
				ADQ_Add(kActorOfficerGrayford, 530, kAnimationModeTalk);
			}
		}
	}
}

void SceneScriptUG04::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptUG04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
