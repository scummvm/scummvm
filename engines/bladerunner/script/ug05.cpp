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

void ScriptUG05::InitializeScene() {
	if (Game_Flag_Query(360)) {
		if (Game_Flag_Query(663) && !Game_Flag_Query(368)) {
			Setup_Scene_Information(-356.35f, 132.77f, -1092.36f, 389);
		} else {
			Setup_Scene_Information(-180.0f, 37.28f, -1124.0f, 296);
		}
	} else {
		Setup_Scene_Information(0.0f, -1.37f, 0.0f, 0);
		Game_Flag_Reset(338);
	}
	Scene_Exit_Add_2D_Exit(0, 215, 240, 254, 331, 3);
	if (!Game_Flag_Query(663)) {
		Scene_Exit_Add_2D_Exit(1, 303, 422, 639, 479, 2);
	}
	if (!Game_Flag_Query(663) || Game_Flag_Query(368)) {
		Scene_Exit_Add_2D_Exit(2, 352, 256, 393, 344, 0);
	}
	Ambient_Sounds_Add_Looping_Sound(105, 28, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	Ambient_Sounds_Add_Sound(234, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(225, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(226, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(227, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(235, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(391, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(368, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(402, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(395, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(398, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(224, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(228, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(392, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(229, 2, 190, 12, 16, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(368)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void ScriptUG05::SceneLoaded() {
	if (!Game_Flag_Query(368)) {
		Unobstacle_Object("DROPPED CAR OBSTACL", true);
	}
	Obstacle_Object("VANBODY", true);
}

bool ScriptUG05::MouseClick(int x, int y) {
	return false;
}

bool ScriptUG05::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptUG05::ClickedOnActor(int actorId) {
	if (!Loop_Actor_Walk_To_Actor(0, actorId, 30, 1, false)) {
		Actor_Face_Actor(0, actorId, true);
		int v1 = sub_4021B0();
		if (actorId == 24 && Game_Flag_Query(368) && !Game_Flag_Query(683)) {
			Actor_Says(24, 220, -1);
			Actor_Says(0, 5540, 14);
			Actor_Says(24, 230, -1);
			Actor_Says(0, 5545, 17);
			Actor_Says(24, 240, -1);
			Actor_Says(0, 5550, 3);
			Game_Flag_Set(683);
			return false;
		}
		if (actorId == v1) {
			sub_402218();
			return true;
		}
		return false;
	}
	return false;
}

bool ScriptUG05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptUG05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (Game_Flag_Query(663) && !Game_Flag_Query(368)) {
			Loop_Actor_Walk_To_XYZ(0, -356.35f, 132.77f, -1092.36f, 0, 0, false, 0);
			Game_Flag_Set(361);
			Set_Enter(43, 40);
		} else if (!Loop_Actor_Walk_To_XYZ(0, -156.72f, 3.03f, -1118.17f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 760, false);
			Loop_Actor_Travel_Stairs(0, 3, 1, 0);
			Game_Flag_Set(361);
			Set_Enter(43, 40);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 4.0f, -11.67f, -4.0f, 0, 1, false, 0)) {
			Game_Flag_Set(339);
			Set_Enter(77, 89);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Game_Flag_Query(662)) {
			if (!Loop_Actor_Walk_To_XYZ(0, 0.0f, -1.37f, -1500.0f, 0, 1, false, 0)) {
				if (!Game_Flag_Query(522)) {
					Actor_Voice_Over(2600, 99);
					Actor_Voice_Over(2610, 99);
					Game_Flag_Set(522);
				}
				return true;
			}
		} else {
			int v1 = sub_4021B0();
			bool v2;
			if (v1 == -1) {
				v2 = Loop_Actor_Walk_To_XYZ(0, 0.0f, -1.37f, -1500.0f, 0, 1, false, 0) != 0;
			} else {
				v2 = Loop_Actor_Walk_To_Actor(0, v1, 30, 1, false) != 0;
			}
			if (!v2) {
				sub_402218();
				return true;
			}
		}
	}
	return false;
}

bool ScriptUG05::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptUG05::SceneFrameAdvanced(int frame) {
}

void ScriptUG05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptUG05::PlayerWalkedIn() {
	if (Game_Flag_Query(663)) {
		if (Game_Flag_Query(368)) {
			Music_Stop(2);
			Actor_Put_In_Set(24, 78);
			Actor_Set_At_XYZ(24, 4.22f, -1.37f, -925.0f, 750);
			Actor_Set_Goal_Number(24, 599);
			Actor_Retired_Here(24, 70, 36, 1, -1);
			int v0 = sub_4021B0();
			if (v0 == 3) {
				Actor_Put_In_Set(3, 78);
				Actor_Set_At_XYZ(3, -100.0f, -10.31f, -906.0f, 866);
				Actor_Force_Stop_Walking(3);
			} else if (v0 == 6) {
				Actor_Put_In_Set(6, 78);
				Actor_Set_At_XYZ(6, -100.0f, -10.31f, -906.0f, 866);
				Actor_Force_Stop_Walking(6);
			}
		} else {
			if (!Actor_Query_In_Set(23, 78)) {
				Actor_Put_In_Set(23, 78);
				Actor_Set_At_XYZ(23, 0.0f, -1.37f, -1400.0f, 768);
			}
			if (!Actor_Query_In_Set(24, 78)) {
				ADQ_Flush();
				ADQ_Add(24, 280, 3);
				Actor_Put_In_Set(24, 78);
				Actor_Set_At_XYZ(24, -16.0f, -1.37f, -960.0f, 768);
			}
		}
	}
	if (Game_Flag_Query(360)) {
		if (Game_Flag_Query(663) && !Game_Flag_Query(368)) {
			Loop_Actor_Walk_To_XYZ(0, -288.35f, 132.77f, -1092.36f, 0, 1, false, 0);
		} else {
			Loop_Actor_Travel_Stairs(0, 2, 0, 0);
		}
	}
	if (Game_Flag_Query(663)) {
		Game_Flag_Query(368);
	}
	Game_Flag_Reset(360);
}

void ScriptUG05::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptUG05::DialogueQueueFlushed(int a1) {
}

int ScriptUG05::sub_4021B0() {
	if (Global_Variable_Query(45) == 2 && Actor_Query_Goal_Number(3) != 599) {
		return 3;
	}
	if (Global_Variable_Query(45) == 3 && Actor_Query_Goal_Number(6) != 599) {
		return 6;
	}
	return -1;
}

void ScriptUG05::sub_402218() {
	int v0 = sub_4021B0();
	if (v0 != -1) {
		Actor_Face_Actor(0, v0, true);
		Actor_Face_Actor(v0, 0, true);
		Actor_Says(0, 5535, 13);
		if (v0 == 3) {
			Actor_Says(3, 1110, 15);
		} else {
			Actor_Says(6, 670, 17);
		}
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (v0 == 6) {
		if (Game_Flag_Query(46)) {
			if (Global_Variable_Query(48) > 3) {
				Outtake_Play(13, 0, -1);
			} else {
				Outtake_Play(14, 0, -1);
			}
		} else {
			Outtake_Play(12, 0, -1);
		}
	} else if (v0 == 3) {
		if (Game_Flag_Query(47)) {
			if (Global_Variable_Query(48) > 3) {
				Outtake_Play(16, 0, -1);
			} else {
				Outtake_Play(17, 0, -1);
			}
		} else {
			Outtake_Play(15, 0, -1);
		}
	} else {
		Outtake_Play(19, 0, -1);
	}
	Outtake_Play(18, 0, -1);
	Game_Over();
}

} // End of namespace BladeRunner
