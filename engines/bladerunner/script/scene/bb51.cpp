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

#include "bladerunner/script/scene.h"

namespace BladeRunner {

void SceneScriptBB51::InitializeScene() {
	Setup_Scene_Information(101.0f, 0.0f, -25.0f, 152);
	Game_Flag_Reset(393);
	Scene_Exit_Add_2D_Exit(0, 615, 0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 0, 323, 241, 479, 2);
	Ambient_Sounds_Add_Looping_Sound(103, 28, 0, 1);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(443, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(446, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(309, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(310, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Scene_Loop_Start_Special(0, 0, 0);
	Scene_Loop_Set_Default(1);
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
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 76.0f, 0.0f, 79.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(394);
			Set_Enter(1, 7);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 55.0f, 0.0f, -96.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(395);
			Set_Enter(1, 7);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB51::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB51::SceneFrameAdvanced(int frame) {
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
