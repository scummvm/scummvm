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

void ScriptNR08::InitializeScene() {
	if (Actor_Query_Goal_Number(1) == 231) {
		Setup_Scene_Information(-1174.1f, 0.32f, 303.9f, 435);
	} else if (Game_Flag_Query(546)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(-1102.88f, 0.0f, 107.43f, 0);
		if (Actor_Query_Goal_Number(3) == 210) {
			Music_Stop(1);
		}
	} else if (Game_Flag_Query(439)) {
		Setup_Scene_Information(-724.7f, 0.0f, 384.24f, 1000);
		Game_Flag_Reset(439);
	} else if (Game_Flag_Query(615)) {
		Setup_Scene_Information(-1663.33f, 0.65f, 342.84f, 330);
		Game_Flag_Reset(615);
	}
	Scene_Exit_Add_2D_Exit(0, 610, 0, 639, 479, 1);
	if (Actor_Query_Goal_Number(3) != 210) {
		Scene_Exit_Add_2D_Exit(1, 0, 309, 30, 398, 3);
		Scene_Exit_Add_2D_Exit(2, 520, 330, 556, 386, 0);
	}
	Ambient_Sounds_Add_Looping_Sound(280, 50, 38, 0);
	Ambient_Sounds_Add_Sound(252, 3, 60, 14, 14, 60, 90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(254, 3, 60, 14, 14, 60, 90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(255, 3, 60, 14, 14, 60, 90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(256, 3, 60, 14, 14, 60, 90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(257, 3, 60, 14, 14, 60, 90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(258, 3, 60, 14, 14, 60, 90, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(259, 3, 60, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(260, 3, 60, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(261, 3, 60, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(262, 3, 60, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Scene_Loop_Set_Default(1);
}

void ScriptNR08::SceneLoaded() {
	Obstacle_Object("BOX283", true);
	Unobstacle_Object("BOX283", true);
	if (Actor_Query_Goal_Number(3) == 210) {
		Actor_Change_Animation_Mode(3, 79);
	}
}

bool ScriptNR08::MouseClick(int x, int y) {
	return false;
}

bool ScriptNR08::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptNR08::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptNR08::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptNR08::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -1102.88f, 0.0f, 107.43f, 0, 1, false, 0)) {
			Game_Flag_Set(547);
			Set_Enter(13, 58);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -724.7f, 0.0f, 384.24f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 505, false);
			Loop_Actor_Travel_Stairs(0, 4, 1, 0);
			Game_Flag_Set(440);
			Set_Enter(56, 59);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, -1663.33f, 0.65f, 342.84f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 831, false);
			Footstep_Sound_Override_On(2);
			Loop_Actor_Travel_Stairs(0, 6, 1, 0);
			Footstep_Sound_Override_Off();
			Game_Flag_Set(614);
			Set_Enter(58, 62);
		}
	}
	return false;
}

bool ScriptNR08::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptNR08::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing()) {
		sub_4021B4();
	}
	Set_Fade_Color(0, 0, 0);
	if (frame >= 76 && frame < 91) {
		Set_Fade_Density((frame - 76) / 14.0f);
		Music_Stop(3);
		Ambient_Sounds_Play_Sound(566, 27, 0, 99, 0);
	} else if (frame >= 91 && frame < 120) {
		Actor_Set_Invisible(0, true);
		Set_Fade_Density(1.0f);
	} else if (frame >= 120 && frame < 135) {
		Set_Fade_Density((134 - frame) / 14.0f);
		Music_Play(7, 61, 0, 1, -1, 0, 0);
	} else {
		Actor_Set_Invisible(0, false);
		Set_Fade_Density(0.0f);
	}
	if (Game_Flag_Query(651) && !Game_Flag_Query(636)) {
		Game_Flag_Set(636);
		Scene_Exits_Disable();
		Scene_Loop_Set_Default(1);
		Scene_Loop_Start_Special(2, 3, 1);
	}
	if (frame == 95) {
		Actor_Put_In_Set(3, 91);
		Actor_Set_At_Waypoint(3, 33, 0);
		Actor_Change_Animation_Mode(3, 0);
		Actor_Set_Goal_Number(3, 200);
		Scene_Exit_Add_2D_Exit(1, 0, 309, 30, 398, 3);
		Scene_Exit_Add_2D_Exit(2, 520, 330, 556, 386, 0);
	}
	if (frame == 130) {
		Scene_Exits_Enable();
	}
	//return false;
}

void ScriptNR08::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptNR08::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(3) != 210 || Game_Flag_Query(729)) {
		Music_Adjust(51, 0, 2);
	} else {
		Game_Flag_Set(729);
		Ambient_Sounds_Play_Sound(566, 27, 0, 99, 0);
		Outtake_Play(40, 1, -1);
	}
	if (Actor_Query_Goal_Number(3) == 245) {
		Actor_Face_Heading(3, 790, false);
		Loop_Actor_Travel_Stairs(3, 8, 1, 0);
		Actor_Set_Goal_Number(3, 246);
	}
	if (Actor_Query_Goal_Number(1) == 231) {
		Actor_Says(1, 1640, 12);
		if (!Game_Flag_Query(378)) {
			Actor_Says(0, 3790, 13);
			Actor_Says(1, 1650, 14);
		}
		Actor_Says(1, 1660, 12);
		Actor_Says(0, 3795, 16);
		Actor_Says(1, 1670, 13);
		Actor_Says(1, 1680, 14);
		Actor_Says(1, 1690, 15);
		Actor_Set_Goal_Number(1, 235);
		//return true;
		return;
	} else {
		if (Game_Flag_Query(546)) {
			Loop_Actor_Walk_To_XYZ(0, -1090.88f, 0.0f, 147.43f, 0, 1, false, 0);
			Game_Flag_Reset(546);
		}
		//return false;
		return;
	}
}

void ScriptNR08::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(547)) {
		Music_Stop(2);
	}
}

void ScriptNR08::DialogueQueueFlushed(int a1) {
}

void ScriptNR08::sub_4021B4() {
	if (Music_Is_Playing()) {
		Music_Adjust(51, 0, 2);
	} else if (Actor_Query_Goal_Number(3) == 210) {
		Music_Play(6, 61, 0, 1, -1, 0, 0);
	} else {
		int v0 = Global_Variable_Query(54);
		if (v0 == 0) {
			Music_Play(16, 61, -80, 2, -1, 0, 0);
		} else if (v0 == 1) {
			Music_Play(15, 41, -80, 2, -1, 0, 0);
		} else if (v0 == 2) {
			Music_Play(7, 41, -80, 2, -1, 0, 0);
		}
		v0++;
		if (v0 > 2) {
			v0 = 0;
		}
		Global_Variable_Set(54, v0);
	}
}

} // End of namespace BladeRunner
