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

void SceneScriptBB51::InitializeScene() {
	Setup_Scene_Information(101.0f, 0.0f, -25.0f, 152);
	Game_Flag_Reset(kFlagBB06toBB51);

	Scene_Exit_Add_2D_Exit(0, 615,   0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1,   0, 323, 241, 479, 2);

	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 28, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY7,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT1,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT2,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);

	Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
	Scene_Loop_Set_Default(1);

#if BLADERUNNER_ORIGINAL_BUGS // Sebastian's Doll Fix
#else
	if (Game_Flag_Query(kFlagBB06AndroidDestroyed)) {
		Overlay_Play("BB06OVER", 1, true, true, 0);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

}

void SceneScriptBB51::SceneLoaded() {
	Obstacle_Object("V2CHESSTBL01", true);
	Clickable_Object("V2CHESSTBL01");
	Clickable_Object("TOP02");
}

bool SceneScriptBB51::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB51::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("V2CHESSTBL01", objectName)) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// acquire chess clue
		if (!Actor_Clue_Query(kActorMcCoy, kClueChessTable)) {
			Actor_Clue_Acquire(kActorMcCoy, kClueChessTable, true, -1);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Face_Object(kActorMcCoy, "V2CHESSTBL01", true);
		Actor_Voice_Over(80, kActorVoiceOver);
		Actor_Voice_Over(90, kActorVoiceOver);
	}

	if (Object_Query_Click("TOP02", objectName)) {
		Actor_Face_Object(kActorMcCoy, "TOP02", true);
		Actor_Voice_Over(100, kActorVoiceOver);
		Actor_Voice_Over(110, kActorVoiceOver);
		Actor_Voice_Over(120, kActorVoiceOver);
	}
	return false;
}

bool SceneScriptBB51::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB51::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB51::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 76.0f, 0.0f, 79.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB51toBB06a);
			Set_Enter(kSetBB02_BB04_BB06_BB51, kSceneBB06);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 55.0f, 0.0f, -96.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB51toBB06b);
			Set_Enter(kSetBB02_BB04_BB06_BB51, kSceneBB06);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB51::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB51::SceneFrameAdvanced(int frame) {
#if BLADERUNNER_ORIGINAL_BUGS // Sebastian's Doll Fix
#else
	// Scene Transition loop frames range from 0 to 14
	// keep destroyedDoll overlay for 2-3 frames - to minimize weird effect
	if (frame == 2) { // executed once during transition to BB51 (chess sub space)
		if (Game_Flag_Query(kFlagBB06AndroidDestroyed)) {
			Overlay_Remove("BB06OVER");
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptBB51::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB51::PlayerWalkedIn() {
}

void SceneScriptBB51::PlayerWalkedOut() {
}

void SceneScriptBB51::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
