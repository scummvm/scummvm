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

void SceneScriptBB06::InitializeScene() {
	if (Game_Flag_Query(kFlagBB51toBB06a)) {
		Setup_Scene_Information(  76.0f, 0.0f,   79.0f, 622);
	} else if (Game_Flag_Query(kFlagBB51toBB06b)) {
		Setup_Scene_Information(  55.0f, 0.0f,  -96.0f, 761);
	} else if (Game_Flag_Query(kFlagBB07toBB06)) {
		Setup_Scene_Information(-115.0f, 0.0f, -103.0f, 375);
		Game_Flag_Reset(kFlagBB07toBB06);
	} else {
		Setup_Scene_Information( -37.0f, 0.0f,  178.0f,   0);
	}

	Scene_Exit_Add_2D_Exit(0,   0,  43,  14, 478, 3);
	Scene_Exit_Add_2D_Exit(1, 425,   0, 639, 361, 0);
	Scene_Exit_Add_2D_Exit(3, 195, 164, 239, 280, 3);

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

	if (Game_Flag_Query(kFlagBB51toBB06a)
	 || Game_Flag_Query(kFlagBB51toBB06b)
	) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
		Game_Flag_Reset(kFlagBB51toBB06a);
		Game_Flag_Reset(kFlagBB51toBB06b);
	} else {
		Scene_Loop_Set_Default(1);
#if BLADERUNNER_ORIGINAL_BUGS // Sebastian's Doll Fix
#else
		// bugfix: case of not transitioning from BB51: chess/ egg boiler sub-space
		if (Game_Flag_Query(kFlagBB06AndroidDestroyed)) {
			Overlay_Play("BB06OVER", 1, true, true, 0);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
	}

#if BLADERUNNER_ORIGINAL_BUGS // Sebastian's Doll Fix
	if (Game_Flag_Query(kFlagBB06AndroidDestroyed)) {
		Overlay_Play("BB06OVER", 1, true, false, 0); // Original bug: it is playing while the background is still panning so it looks pretty weird
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptBB06::SceneLoaded() {
	Obstacle_Object("V2CHESSTBL01", true);
	Clickable_Object("BOX31");
#if BLADERUNNER_ORIGINAL_BUGS // Sebastian's Doll Fix
	// This Item_Add_To_World call is only ok for the transition from BB51 to BB06,
	// otherwise the "doll" item (actually the badge item) is not placed in the current set
	Item_Add_To_World(kItemBB06ControlBox, kModelAnimationBadge, kSetBB06_BB07, -127.0f, 68.42f, 57.0f, 0, 8, 8, true, true, false, true);
#else
	if (!Game_Flag_Query(kFlagBB06AndroidDestroyed)) {
		Combat_Target_Object("BOX31"); //
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptBB06::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB06::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("BOX31", objectName)) {
#if BLADERUNNER_ORIGINAL_BUGS // Sebastian's Doll Fix
		if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "BOX31", 24, true, false)) {
			Actor_Face_Object(kActorMcCoy, "BOX31", true);
			if (Game_Flag_Query(kFlagBB06AndroidDestroyed)) {
				Actor_Voice_Over(60, kActorVoiceOver);
				Actor_Voice_Over(70, kActorVoiceOver);
			} else {
				Actor_Voice_Over(50, kActorVoiceOver);
			}
		}
#else
		if (Player_Query_Combat_Mode()) {
			// Doll Explosion case:
			// We need to use enqueued overlays for this.
			// Note: Queuing only works on top of a video that is repeating itself.
			// First we load the "exploding animation state" as a forever loop (even though it will only play once)
			// Then we enqueue the final exploded state loop, also as a forever loop.
			// This (along with some fixes in the Overlays class will ensure
			// that the second overlay will play after the first has completed one loop
			// and it will persist (across save games too).
			Game_Flag_Set(kFlagBB06AndroidDestroyed);
			Overlay_Play("BB06OVER", 0, true, true,  0);
			Overlay_Play("BB06OVER", 1, true, false, 0);
			Un_Combat_Target_Object("BOX31");
			return true;
		} else {
			if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "BOX31", 24, true, false)) {
				Actor_Face_Object(kActorMcCoy, "BOX31", true);
				if (Game_Flag_Query(kFlagBB06AndroidDestroyed)) {
					Actor_Voice_Over(60, kActorVoiceOver);
					Actor_Voice_Over(70, kActorVoiceOver);
				} else {
					Actor_Voice_Over(50, kActorVoiceOver);
				}
			}
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	return false;
}

bool SceneScriptBB06::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB06::ClickedOnItem(int itemId, bool a2) {
#if BLADERUNNER_ORIGINAL_BUGS // Sebastian's Doll Fix
	if (itemId == kItemBB06ControlBox) {
		if (Player_Query_Combat_Mode()) {
			Overlay_Play("BB06OVER", 1, true, true, 0);
			Game_Flag_Set(kFlagBB06AndroidDestroyed);
			Item_Remove_From_World(kItemBB06ControlBox);
			return true;
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
	return false;
}

bool SceneScriptBB06::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -37.0f, 0.0f, 178.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB06toBB05);
			Set_Enter(kSetBB05, kSceneBB05);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 101.0f, 0.0f, -25.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB06toBB51);
			Set_Enter(kSetBB02_BB04_BB06_BB51, kSceneBB51);
		}
		return true;
	}

	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -115.0f, 0.0f, -103.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB06toBB07);
			Set_Enter(kSetBB06_BB07, kSceneBB07);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB06::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB06::SceneFrameAdvanced(int frame) {
	if (frame == 34) {
		Ambient_Sounds_Play_Sound(kSfxPNEUM5,  40, -50, -50, 10);
	}
#if BLADERUNNER_ORIGINAL_BUGS // Sebastian's Doll Fix
#else
	// last frame of transition is 15, try 13 for better transition - minimize weird effect
	if (frame == 13) { // executed once during transition FROM bb51 (chess sub space)
		if (Game_Flag_Query(kFlagBB06AndroidDestroyed)) {
			Overlay_Play("BB06OVER", 1, true, true, 0);
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
	if (frame == 16) {
		Ambient_Sounds_Play_Sound(kSfxROBOTMV1, 20, -50, -50, 10);
	}
	if (frame == 20) {
		Ambient_Sounds_Play_Sound(kSfxROBOTMV1, 20, -50, -50, 10);
	}
	if (frame == 25) {
		Ambient_Sounds_Play_Sound(kSfxROBOTMV1, 20, -50, -50, 10);
	}
	if (frame == 29) {
		Ambient_Sounds_Play_Sound(kSfxROBOTMV1, 20, -50, -50, 10);
	}
}

void SceneScriptBB06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB06::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagBB05toBB06)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -36.0f, 0.0f, 145.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagBB05toBB06);
	}
}

void SceneScriptBB06::PlayerWalkedOut() {
}

void SceneScriptBB06::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
