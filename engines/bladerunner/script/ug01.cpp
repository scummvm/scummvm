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

#include "bladerunner/script/script.h"

namespace BladeRunner {

void ScriptUG01::InitializeScene() {
	if (Game_Flag_Query(317)) {
		Setup_Scene_Information(34.47f, -50.13f, -924.11f, 500);
		Game_Flag_Reset(317);
	} else if (Game_Flag_Query(118)) {
		Setup_Scene_Information(-68.0f, -50.13f, -504.0f, 377);
	} else {
		Setup_Scene_Information(-126.0f, -50.13f, -286.0f, 0);
	}
	Scene_Exit_Add_2D_Exit(0, 280, 204, 330, 265, 0);
	Scene_Exit_Add_2D_Exit(1, 144, 0, 210, 104, 0);
	Scene_Exit_Add_2D_Exit(2, 0, 173, 139, 402, 3);
	Ambient_Sounds_Add_Looping_Sound(331, 28, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	Ambient_Sounds_Add_Sound(291, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(402, 2, 120, 10, 11, 20, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(370, 2, 120, 10, 11, 20, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(397, 2, 120, 10, 11, 20, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(396, 2, 120, 10, 11, 20, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(294, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(234, 2, 190, 12, 16, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(391, 2, 190, 12, 16, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(394, 2, 190, 12, 16, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(224, 2, 190, 12, 16, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(227, 2, 190, 12, 16, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(228, 2, 190, 12, 16, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(229, 2, 190, 12, 16, 0, 100, 0, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 37, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 37, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 37, 0, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(324)) {
		Scene_Loop_Set_Default(3);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void ScriptUG01::SceneLoaded() {
	Unobstacle_Object("BEAM02", true);
	Unobstacle_Object("BEAM03", true);
	Unobstacle_Object("BEAM04", true);
	Clickable_Object("PIPES_FG_LFT");
}

bool ScriptUG01::MouseClick(int x, int y) {
	return false;
}

bool ScriptUG01::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("PIPES_FG_LFT", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(0, -9.0f, -50.13f, -148.0f, 0, 1, false, 0) && !Game_Flag_Query(324)) {
			Actor_Says(0, 8525, 13);
			Scene_Loop_Set_Default(3);
			Scene_Loop_Start_Special(2, 2, 1);
			Game_Flag_Set(324);
		} else {
			Actor_Says(0, 8525, 13);
		}
	}
	return false;
}

bool ScriptUG01::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptUG01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptUG01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -32.0f, -50.13f, -1350.0f, 12, 1, false, 0)) {
			Game_Flag_Set(316);
			Set_Enter(83, 95);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -70.0f, -50.13f, -500.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 768, false);
			Loop_Actor_Travel_Ladder(0, 12, 1, 0);
			Game_Flag_Set(119);
			Game_Flag_Reset(259);
			Game_Flag_Set(182);
			Set_Enter(70, 80);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, -126.0f, -50.13f, -286.0f, 0, 1, false, 0)) {
			Game_Flag_Set(314);
			Set_Enter(75, 87);
		}
		return true;
	}
	return false;
}

bool ScriptUG01::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptUG01::SceneFrameAdvanced(int frame) {
	if (frame >= 61 && frame <= 120) {
		float v1 = (120 - frame) / 29500.0f;
		Set_Fog_Density("BoxFog01", v1);
		Set_Fog_Density("BoxFog02", v1);
		Set_Fog_Density("BoxFog03", v1);
		Set_Fog_Density("BoxFog04", v1);
	} else if (frame > 120) {
		Set_Fog_Density("BoxFog01", 0.0f);
		Set_Fog_Density("BoxFog02", 0.0f);
		Set_Fog_Density("BoxFog03", 0.0f);
		Set_Fog_Density("BoxFog04", 0.0f);
	}
	//return false;
}

void ScriptUG01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptUG01::PlayerWalkedIn() {
	if (Game_Flag_Query(315)) {
		Loop_Actor_Walk_To_XYZ(0, -55.0f, -50.13f, -288.0f, 12, 0, false, 0);
		Game_Flag_Reset(315);
	}
	if (Game_Flag_Query(118)) {
		Actor_Set_At_XYZ(0, -70.0f, 93.87f, -500.0f, 768);
		Loop_Actor_Travel_Ladder(0, 12, 0, 0);
		Loop_Actor_Walk_To_XYZ(0, -58.0f, -50.13f, -488.0f, 0, 0, false, 0);
		Game_Flag_Reset(118);
	}
	if (Actor_Query_Goal_Number(6) == 310) {
		Music_Play(21, 35, 0, 3, -1, 0, 0);
		Actor_Set_Goal_Number(6, 311);
	}
	//return false;
}

void ScriptUG01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptUG01::DialogueQueueFlushed(int a1) {
}


} // End of namespace BladeRunner
