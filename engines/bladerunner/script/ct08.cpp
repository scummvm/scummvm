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

void ScriptCT08::InitializeScene() {
	if (Game_Flag_Query(679)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(2, 0, -1);
		if (Game_Flag_Query(46)) {
			Outtake_Play(3, 0, -1);
		} else if (Game_Flag_Query(47)) {
			Outtake_Play(4, 0, -1);
		} else {
			Outtake_Play(5, 0, -1);
		}
		Outtake_Play(6, 0, -1);
		Game_Flag_Reset(679);
	}
	Actor_Force_Stop_Walking(0);
	if (Game_Flag_Query(380)) {
		Setup_Scene_Information(-11.0f, 0.0f, -156.0f, 769);
	} else if (Game_Flag_Query(79)) {
		Setup_Scene_Information(-143.0f, 0.0f, -92.0f, 420);
	} else {
		Setup_Scene_Information(-183.0f, 0.0f, 128.0f, 205);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 0, 30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 389, 0, 639, 303, 0);
	Scene_Exit_Add_2D_Exit(2, 115, 87, 137, 267, 3);
	if (Game_Flag_Query(550)) {
		Scene_2D_Region_Add(0, 185, 185, 230, 230);
	}
	Ambient_Sounds_Add_Looping_Sound(381, 100, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(205, 20, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(67, 80, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(68, 50, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(379, 5, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(380, 5, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 5, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(380)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(1);
	}
}

void ScriptCT08::SceneLoaded() {
	Obstacle_Object("ASHTRAY", 1);
	Unobstacle_Object("BLANKET03", 1);
	if (!Actor_Clue_Query(0, 85)) {
		Item_Add_To_World(85, 943, 6, 44.0f, 0.0f, -95.0f, 540, 12, 12, 0, 1, 0, 1);
	}
	if (!Actor_Clue_Query(0, 87)) {
		Item_Add_To_World(81, 936, 6, -102.0f, 2.0f, 41.0f, 432, 6, 6, 0, 1, 0, 1);
	}
}

bool ScriptCT08::MouseClick(int x, int y) {
	return false;
}

bool ScriptCT08::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptCT08::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptCT08::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 81) {
		if (!Loop_Actor_Walk_To_Item(0, 81, 36, 1, 0) && !Game_Flag_Query(550)) {
			Actor_Clue_Acquire(0, 87, 1, -1);
			Item_Pickup_Spin_Effect(936, 266, 328);
			Item_Remove_From_World(81);
			Actor_Voice_Over(480, 99);
			Actor_Voice_Over(490, 99);
			Actor_Voice_Over(500, 99);
		}
		return true;
	}
	return false;
}

bool ScriptCT08::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -183.0f, 0.0f, 128.0f, 0, 1, 0, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(81);
			Set_Enter(31, 21);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -11.0f, 0.0f, -156.0f, 0, 1, 0, 0)) {
			Loop_Actor_Walk_To_XYZ(0, 0.0f, 0.0f, -102.0f, 0, 0, 0, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(379);
			Set_Enter(6, 105);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, -143.0f, 0.0f, -92.0f, 0, 1, 0, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(80);
			Set_Enter(29, 18);
		}
		return true;
	}
	return false;
}

bool ScriptCT08::ClickedOn2DRegion(int region) {
	if (region == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -108.0f, 0.0f, -178.0f, 0, 1, 0, 0)) {
			Actor_Face_Heading(0, 512, 0);
			Game_Flag_Reset(550);
			Player_Set_Combat_Mode_Access(1);
			Scene_Exits_Enable();
			Ambient_Sounds_Play_Sound(564, 40, 99, 0, 0);
			Scene_2D_Region_Remove(0);
			Player_Loses_Control();
		}
		return true;
	}
	return false;
}

void ScriptCT08::SceneFrameAdvanced(int frame) {
}

void ScriptCT08::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptCT08::PlayerWalkedIn() {
	if (Game_Flag_Query(550)) {
		Actor_Change_Animation_Mode(0, 3);
		Actor_Change_Animation_Mode(0, 0);
		Actor_Set_At_XYZ(0, -148.0f, 0.0f, 4.0f, 256);
		Player_Set_Combat_Mode_Access(0);
		Scene_Exits_Disable();
		Game_Flag_Reset(380);
		Game_Flag_Reset(79);
		Autosave_Game(1);
	} else if (Game_Flag_Query(380)) {
		Game_Flag_Reset(380);
	} else if (Game_Flag_Query(79)) {
		Game_Flag_Reset(79);
	} else {
		Loop_Actor_Walk_To_XYZ(0, -156.0f, 0.0f, 128.0f, 0, 0, 0, 0);
		Game_Flag_Reset(84);
	}
}

void ScriptCT08::PlayerWalkedOut() {
	if (!Actor_Clue_Query(0, 85)) {
		Item_Remove_From_World(85);
	}
}

void ScriptCT08::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
