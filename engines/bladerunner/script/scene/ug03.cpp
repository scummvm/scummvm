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

void SceneScriptUG03::InitializeScene() {
	if (Game_Flag_Query(kFlagUG04toUG03)) {
		Setup_Scene_Information(  -51.0f, 0.03f,  255.0f, 780);
		Game_Flag_Reset(kFlagUG04toUG03);
	} else if (Game_Flag_Query(kFlagUG10toUG03)) {
		Setup_Scene_Information( -139.0f, 0.03f,  -13.0f, 540);
		Game_Flag_Reset(kFlagUG10toUG03);
	} else {
		Setup_Scene_Information(-121.88f, 0.03f, 213.35f, 540);
	}
	Scene_Exit_Add_2D_Exit(0,  46, 137, 131, 296, 0);
	Scene_Exit_Add_2D_Exit(1, 559, 141, 639, 380, 1);

	Ambient_Sounds_Add_Looping_Sound(kSfxSTMLOOP7, 15, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED2,   40, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxYELL1M3,  2, 120, 10, 11,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT2M1, 2, 120, 10, 11,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M3, 2, 120, 10, 11,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxGRUNT1M2, 2, 120, 10, 11,    0, 100,    0,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPIPER1,   2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK1,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK2,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK3,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK4,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSQUEAK5,  2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG1,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG2,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG3,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG4,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG5,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBANG6,    2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
}

void SceneScriptUG03::SceneLoaded() {
	Obstacle_Object("=WALL_RIGHT_HOLE", true);
	Unobstacle_Object("=HOLERUBBLE1", true);
	Clickable_Object("CHAIR_HEADZAPPER");
	Clickable_Object("CHAIR_BACK");
	Clickable_Object("CHAIR_SEAT");
	Clickable_Object("CHAIR_STRAPLEGLEFT");
	Clickable_Object("CHAIR_STRAPLEGRIGHT");
#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (Actor_Clue_Query(kActorMcCoy, kClueScaryChair)) {
		Unclickable_Object("CHAIR_HEADZAPPER");
		Unclickable_Object("CHAIR_BACK");
		Unclickable_Object("CHAIR_SEAT");
		Unclickable_Object("CHAIR_STRAPLEGLEFT");
		Unclickable_Object("CHAIR_STRAPLEGRIGHT");
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

bool SceneScriptUG03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG03::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("CHAIR_BACK", objectName)
	 || Object_Query_Click("CHAIR_SEAT", objectName)
	 || Object_Query_Click("CHAIR_HEADZAPPER", objectName)
#if BLADERUNNER_ORIGINAL_BUGS
#else
	 || Object_Query_Click("CHAIR_STRAPLEGLEFT", objectName)
	 || Object_Query_Click("CHAIR_STRAPLEGRIGHT", objectName)
#endif // BLADERUNNER_ORIGINAL_BUGS
	) {
		if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "CHAIR_BACK", 36, true, false)) {
			Actor_Face_Object(kActorMcCoy, "CHAIR_BACK", true);
			if (!Actor_Clue_Query(kActorMcCoy, kClueScaryChair)) {
				Actor_Voice_Over(2550, kActorVoiceOver);
				Actor_Voice_Over(2560, kActorVoiceOver);
				Actor_Voice_Over(2570, kActorVoiceOver);
				Actor_Voice_Over(2580, kActorVoiceOver);
				Actor_Voice_Over(2590, kActorVoiceOver);
				Actor_Clue_Acquire(kActorMcCoy, kClueScaryChair, true, -1);
#if BLADERUNNER_ORIGINAL_BUGS
#else
				Unclickable_Object("CHAIR_HEADZAPPER");
				Unclickable_Object("CHAIR_BACK");
				Unclickable_Object("CHAIR_SEAT");
				Unclickable_Object("CHAIR_STRAPLEGLEFT");
				Unclickable_Object("CHAIR_STRAPLEGRIGHT");
#endif // BLADERUNNER_ORIGINAL_BUGS
			}
		}
	}
	return false;
}

bool SceneScriptUG03::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -139.0f, 0.0f, -13.0f, 0, true, false, false)) {
			if (Global_Variable_Query(kVariableChapter) < 4) {
				Actor_Says(kActorMcCoy, 8522, 14);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(kFlagUG03toUG10);
				Set_Enter(kSetUG10, kSceneUG10);
			}
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -51.0f, 0.0f, 255.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG03toUG04);
			Set_Enter(kSetUG04, kSceneUG04);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG03::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG03::PlayerWalkedIn() {
}

void SceneScriptUG03::PlayerWalkedOut() {
}

void SceneScriptUG03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
