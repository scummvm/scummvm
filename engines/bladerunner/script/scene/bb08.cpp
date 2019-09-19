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

void SceneScriptBB08::InitializeScene() {
	if (Game_Flag_Query(kFlagBB09toBB08)) {
#if BLADERUNNER_ORIGINAL_BUGS
		Setup_Scene_Information(204.0f, 0.0f, 92.0f, 875);
#else
		Setup_Scene_Information(204.0f, 96.1f, 94.0f, 256);
#endif
	} else {
		Setup_Scene_Information(247.0f, 0.0f, 27.0f, 790);
	}

	Scene_Exit_Add_2D_Exit(0, 307,  0, 361, 238, 0);
	Scene_Exit_Add_2D_Exit(1, 117, 38, 214, 245, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRUNOFF, 44, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP1, 1,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP2, 1,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP3, 1,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP4, 1,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP5, 1,  20, 20, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY7,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT1,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT2,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);

	if (!Game_Flag_Query(kFlagBB08Entered)) {
		Overlay_Play("BB08OVER", 0, false, false, 0); // Sadik's shadow going up
		Game_Flag_Set(kFlagBB08Entered);
	}
}

void SceneScriptBB08::SceneLoaded() {
	Obstacle_Object("BATHTUB", true);
	Unobstacle_Object("DOORWAY", true);
	Unclickable_Object("BATHTUB");
}

bool SceneScriptBB08::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB08::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB08::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB08::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB08::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 204.0f, 0.1f, 94.0f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 256, false);
			Footstep_Sound_Override_On(2);
			Loop_Actor_Travel_Ladder(kActorMcCoy, 8, true, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB08toBB09);
			Set_Enter(kSetBB09, kSceneBB09);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 247.0f, 0.1f, 27.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB08toBB12);
			Set_Enter(kSetBB12, kSceneBB12);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB08::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB08::SceneFrameAdvanced(int frame) {
}

void SceneScriptBB08::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB08::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagBB09toBB08)) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// prevents a sling-shot effect whereby McCoy reverts to the position vector
		// he had in the previous room (BB09), if he just started walking before exiting BB09
		// this bug would result in McCoy teleporting to the right of this scene (BB08)
		// and he would often then glitch through the bathtub.
		// Also he would skip the climbing down stairs animation altogether
		// The bug seems to be particular only to this case of McCoy climbing down a ladder
		Actor_Force_Stop_Walking(kActorMcCoy);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_At_XYZ(kActorMcCoy, 204.0f, 96.1f, 94.0f, 256);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Ladder(kActorMcCoy, 8, false, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
		Actor_Face_Heading(kActorMcCoy, 768, false);
		Game_Flag_Reset(kFlagBB09toBB08);
	} else {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 188.0f, 0.1f, 28.0f, 0, false, false, false);
	}
}

void SceneScriptBB08::PlayerWalkedOut() {
}

void SceneScriptBB08::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
