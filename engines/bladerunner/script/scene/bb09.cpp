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

void SceneScriptBB09::InitializeScene() {
	Setup_Scene_Information(111.2f, -8.96f, 134.65f, 0);
	if (Game_Flag_Query(kFlagBB10toBB09)) {
		Game_Flag_Reset(kFlagBB10toBB09);
#if BLADERUNNER_ORIGINAL_BUGS
		Setup_Scene_Information(115.45f, -8.96f, 134.0f, 628);
#else
		Setup_Scene_Information(455.74f, -8.99f, 217.48f, 996);
#endif // BLADERUNNER_ORIGINAL_BUGS
	} else if (Game_Flag_Query(kFlagBB08toBB09)) {
		Game_Flag_Reset(kFlagBB08toBB09);
		Setup_Scene_Information(107.45f, -9.14f, 166.0f, 244);
	}

#if BLADERUNNER_ORIGINAL_BUGS
	Scene_Exit_Add_2D_Exit(0, 224, 213, 286, 353, 1);
#else
	// in the original McCoy could run ahead of Sadik to the next room (BB10)
	// if the player clicked quickly enough
	// kFlagBB11SadikFight is set after Sadik exits this room in Chapter 2
	// (and his goal is set to kGoalSadikBB11Wait)
	// this flag will be reset before Act 3, when McCoy is ambushed at the BB roof
	if ((Global_Variable_Query(kVariableChapter) == 2
	       && Game_Flag_Query(kFlagBB11SadikFight))
	    || (Global_Variable_Query(kVariableChapter) > 2)
	) {
		Scene_Exit_Add_2D_Exit(0, 224, 213, 286, 353, 1);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
	Scene_Exit_Add_2D_Exit(1,  75, 450, 480, 479, 2);

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1,  20, 100, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxRAINAWN1, 40,   0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRUNOFF, 50,  55, 1);
	Ambient_Sounds_Add_Sound(kSfxRADIATR2, 5,  20, 20, 25, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxRADIATR3, 5,  20, 20, 25, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxRADIATR4, 5,  20, 20, 25, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,   2, 180, 14, 16, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,   2, 180, 14, 16, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,   2, 180, 14, 16, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY7,   2, 180, 14, 16, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 17, 27, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 17, 27, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 17, 27, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1,  5,  50, 17, 27, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2,  5,  50, 17, 27, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3,  5,  50, 17, 27, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT1,   5,  50, 17, 27, -100,  100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT2,   5,  50, 17, 27, -100,  100, -101, -101, 0, 0);

	// Original: // Don't kill Sadik yet, game cannot continue.
	// This is an original bug - fixed in ScummVM in Sadik's AI script (method ShotAtAndHit() )
	// Also Sadik is also set as targetable (redundant) in his AI script (method Update() )
	Actor_Set_Targetable(kActorSadik, true);
}

void SceneScriptBB09::SceneLoaded() {
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Obstacle_Object("BACKWALL1", true);
	Obstacle_Object("BACKWALL2", true);
#endif // BLADERUNNER_ORIGINAL_BUGS
	Obstacle_Object("WICKER CHAIR ", true);
	Unobstacle_Object("ROOM03 RIGHT WALL", true);
	Unclickable_Object("WICKER CHAIR ");
}

bool SceneScriptBB09::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB09::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB09::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB09::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB09::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 454.56f, -9.0f, 190.31f, 0, true, false, false)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 450.56f, -9.0f, 250.31f, 0, false, false, false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB09toBB10a);
			Game_Flag_Set(kFlagBB09toBB10b);
			Set_Enter(kSetBB10, kSceneBB10);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 71.0f, -9.0f, 136.0f, 72, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB09toBB08);
			Set_Enter(kSetBB08, kSceneBB08);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB09::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB09::SceneFrameAdvanced(int frame) {
}

void SceneScriptBB09::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
	// in the original McCoy could run ahead of Sadik to the next room (BB10)
	// if the player clicked quickly enough
	if (actorId == kActorSadik && newGoal == kGoalSadikBB11Wait) {
		Scene_Exit_Add_2D_Exit(0, 224, 213, 286, 353, 1);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void SceneScriptBB09::PlayerWalkedIn() {
}

void SceneScriptBB09::PlayerWalkedOut() {
}

void SceneScriptBB09::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
