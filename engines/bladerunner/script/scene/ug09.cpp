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

void SceneScriptUG09::InitializeScene() {
	if (Game_Flag_Query(kFlagCT12ToUG09)) {
		Setup_Scene_Information(-67.0f, 156.94f, -425.0f,  500);
		Game_Flag_Reset(kFlagCT12ToUG09);
	} else {
		Setup_Scene_Information(-53.0f, 156.94f,  174.0f, 1000);
		Game_Flag_Reset(kFlagUG07toUG09);
	}

	Scene_Exit_Add_2D_Exit(0, 204, 159, 392, 360, 0);
	Scene_Exit_Add_2D_Exit(1,   0, 455, 639, 479, 2);

	Ambient_Sounds_Add_Looping_Sound(kSfxCTRUNOFF, 71, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxBOILPOT2, 45, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   76, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP1,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP2,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP4,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M2,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxYELL1M3,  2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M1, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M2, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M3, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBDRIP5,  2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAM1,   5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAM3,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAM6A,  5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxZUBWLK3,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxZUBWLK4,  5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
}

void SceneScriptUG09::SceneLoaded() {
	Obstacle_Object("FACADE_CANOPY", true);
	Obstacle_Object("VW PROP SLAB", true);
	Obstacle_Object("WALL_LEFT FRONT", true);
	Unobstacle_Object("PATH_FRAGMENT 1", true);
	Unobstacle_Object("BOXS FOR ARCHWAY 02", true);
	Unobstacle_Object("BOXS FOR ARCHWAY 04", true);
	Unobstacle_Object("BOXS FOR ARCHWAY 05", true);
	Unobstacle_Object("BOX45", true);
	Unobstacle_Object("BOX44", true);
	Unobstacle_Object("BOX43", true);
	Unobstacle_Object("BOX42", true);
	Unobstacle_Object("BOX41", true);
	Unobstacle_Object("BOX40", true);
	Unobstacle_Object("BOX39", true);
	Unobstacle_Object("BOX38", true);
	Unobstacle_Object("BOX37", true);
	Unobstacle_Object("BOX36", true);
	Unobstacle_Object("BOX35", true);
	Unobstacle_Object("BOX34", true);
	Unobstacle_Object("BOX32", true);
	Clickable_Object("FACADE_CANOPY");
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Unclickable_Object("FACADE_CANOPY");
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptUG09::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG09::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG09::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG09::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG09::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -67.0f, 156.94f, -425.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG09toCT12);
			Set_Enter(kSetCT01_CT12, kSceneCT12);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -53.0f, 156.94f, 206.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG09ToUG07);
			Set_Enter(kSetUG07, kSceneUG07);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG09::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG09::SceneFrameAdvanced(int frame) {
	//return false;
}

void SceneScriptUG09::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG09::PlayerWalkedIn() {
	if (Global_Variable_Query(kVariableChapter) == 4
	 && Game_Flag_Query(kFlagUG07Empty)
	) {
		Game_Flag_Set(kFlagMutantsPaused);
	}
}

void SceneScriptUG09::PlayerWalkedOut() {
	if (Global_Variable_Query(kVariableChapter) == 4) {
		Game_Flag_Reset(kFlagMutantsPaused);
	}

	if (Game_Flag_Query(kFlagUG09toCT12)) {
		Game_Flag_Set(kFlagMcCoyInChinaTown);
		Game_Flag_Reset(kFlagMcCoyInUnderground);
	}
}

void SceneScriptUG09::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
