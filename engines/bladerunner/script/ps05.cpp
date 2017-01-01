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

void ScriptPS05::InitializeScene() {
	if (Game_Flag_Query(21)) {
		Setup_Scene_Information(547.59f, 0.18f, -216.84f, 334);
	} else if (Game_Flag_Query(22)) {
		Setup_Scene_Information(635.0f, 0.0f, -598.0f, 475);
	} else {
		Setup_Scene_Information(630.72f, 0.38f, -469.26f, 400);
	}
	Scene_Exit_Add_2D_Exit(0, 218, 98, 280, 246, 3);
	Scene_Exit_Add_2D_Exit(1, 330, 90, 436, 198, 0);
	Scene_Exit_Add_2D_Exit(2, 476, 96, 524, 240, 1);
	Scene_2D_Region_Add(0, 519, 107, 537, 122);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(384, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(141, 80, 0, 1);
	Ambient_Sounds_Add_Sound(385, 5, 50, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(156, 5, 20, 30, 30, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(157, 5, 20, 30, 30, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(158, 5, 20, 30, 30, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(73, 5, 20, 5, 9, -70, 70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 20, 5, 9, -70, 70, -101, -101, 0, 0);
}

void ScriptPS05::SceneLoaded() {
	Obstacle_Object("WATER FOUNTAIN", true);
	Clickable_Object("WATER FOUNTAIN");
	Clickable_Object("ASHTRAY");
	Clickable_Object("FIRE EXTINGISHER");
	Clickable_Object("CUP");
	Clickable_Object("WIRE BASKET");
	Clickable_Object("WANTED POSTERS");
	Unclickable_Object("WATER FOUNTAIN");
	Unclickable_Object("CUP");
}

bool ScriptPS05::MouseClick(int x, int y) {
	return false;
}

bool ScriptPS05::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("WATER FOUNTAIN", objectName) && !Loop_Actor_Walk_To_Scene_Object(0, "WATER FOUNTAIN", 12, 1, false)) {
		Actor_Face_Object(0, "WATER FOUNTAIN", true);
		Actor_Says(0, 3490, 18);
	}
	if (Object_Query_Click("ASHTRAY", objectName) && !Loop_Actor_Walk_To_XYZ(0, 662.0f, 0.37f, -180.0f, 0, 1, false, 0)) {
		Actor_Face_Object(0, "ASHTRAY", true);
		Actor_Voice_Over(1770, 99);
		Actor_Voice_Over(1780, 99);
		Actor_Voice_Over(1790, 99);
	}
	if (Object_Query_Click("WIRE BASKET", objectName) && !Loop_Actor_Walk_To_Scene_Object(0, "WIRE BASKET", 12, 1, false)) {
		Actor_Face_Object(0, "WIRE BASKET", true);
		Actor_Voice_Over(1810, 99);
		Actor_Voice_Over(1820, 99);
	}
	if (Object_Query_Click("WANTED POSTERS", objectName) && !Loop_Actor_Walk_To_Scene_Object(0, "WANTED POSTERS", 12, 1, false)) {
		Actor_Face_Object(0, "WANTED POSTERS", true);
		Actor_Voice_Over(1800, 99);
	}
	return false;
}

bool ScriptPS05::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptPS05::ClickedOnItem(int itemId, bool a2) {
	if (Game_Flag_Query(23)) {
		Actor_Set_At_XYZ(0, 718.72f, 0.37f, -461.26f, 600);
	} else if (Game_Flag_Query(22)) {
		sub_401B34();
		sub_401C30();
	}
	Game_Flag_Reset(22);
	Game_Flag_Reset(23);
	Game_Flag_Reset(21);
	Game_Flag_Reset(204);
	return false;
}

bool ScriptPS05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_Waypoint(0, 2, 24, 1, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(101, 119);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 635.0f, 0.0f, -598.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(62, 66);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 742.52002f, 0.37f, -457.69f, 0, 1, false, 0)) {
			Game_Flag_Set(136);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(65, 70);
		}
		return true;
	}
	return false;
}

bool ScriptPS05::ClickedOn2DRegion(int region) {
	if (region == 0 && !Loop_Actor_Walk_To_XYZ(0, 694.78f, 0.37f, -321.05f, 0, 1, false, 0)) {
		Actor_Face_Heading(0, 130, false);
		View_Score_Board();
	}
	return false;
}

void ScriptPS05::SceneFrameAdvanced(int frame) {
	if (frame == 1 || frame == 16 || frame == 31 || frame == 46) {
		Sound_Play(149, Random_Query(10, 10), 70, 70, 50);
	}
	//return true;
}

void ScriptPS05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptPS05::PlayerWalkedIn() {
}

void ScriptPS05::PlayerWalkedOut() {
}

void ScriptPS05::DialogueQueueFlushed(int a1) {
	Overlay_Remove("PS05OVER");
}

void ScriptPS05::sub_401B34() {
	int v0;
	int v1;
	int v3[7];

	v0 = 0;
	if (Global_Variable_Query(1) < 4 && Game_Flag_Query(45)) {
		v0 = 1;
		v3[0] = 0;
	}
	v1 = v0 + 1;
	v3[v0] = 1;
	if (Global_Variable_Query(1) >= 3) {
		v3[v1] = 2;
		v1 = v0 + 2;
	}
	if (Global_Variable_Query(1) >= 2 && Global_Variable_Query(1) <= 4) {
		v3[v1++] = 3;
	}
	if (Game_Flag_Query(171) && Game_Flag_Query(170)) {
		v3[v1++] = 4;
	}
	if (v1 <= 0) {
		Global_Variable_Set(52, -1);
	} else {
		Global_Variable_Set(52, v3[Random_Query(0, v1 - 1)]);
	}
}

void ScriptPS05::sub_401C30() {
	switch (Global_Variable_Query(52)) {
	case 4:
		if (!Game_Flag_Query(692)) {
			Overlay_Play("PS05OVER", 0, 1, 0, 0);
			ADQ_Add(61, 230, 3);
			ADQ_Add(61, 240, 3);
			Game_Flag_Set(692);
		}
		break;
	case 3:
		if (!Game_Flag_Query(691)) {
			Overlay_Play("PS05OVER", 0, 1, 0, 0);
			ADQ_Add(61, 170, 3);
			ADQ_Add(61, 180, 3);
			ADQ_Add(61, 190, 3);
			ADQ_Add(61, 200, 3);
			ADQ_Add(61, 210, 3);
			ADQ_Add(61, 220, 3);
			ADQ_Add(41, 80, 3);
			ADQ_Add(41, 90, 3);
			ADQ_Add(41, 100, 3);
			ADQ_Add(41, 110, 3);
			ADQ_Add(41, 120, 3);
			ADQ_Add(41, 130, 3);
			Game_Flag_Set(691);
		}
		break;
	case 2:
		if (!Game_Flag_Query(690)) {
			Overlay_Play("PS05OVER", 0, 1, 0, 0);
			if (Actor_Query_Friendliness_To_Other(5, 0) > Actor_Query_Friendliness_To_Other(1, 0)) {
				ADQ_Add(61, 120, 3);
				ADQ_Add(61, 130, 3);
				ADQ_Add(61, 140, 3);
				ADQ_Add(61, 150, 3);
				ADQ_Add(4, 1570, 3);
				ADQ_Add(4, 1580, 3);
				ADQ_Add(4, 1590, 3);
			} else {
				ADQ_Add(61, 90, 3);
				ADQ_Add(61, 100, 3);
				ADQ_Add(61, 110, 3);
				ADQ_Add(4, 1540, 3);
				ADQ_Add(4, 1550, 3);
				ADQ_Add(4, 1560, 3);
			}
			Game_Flag_Set(690);
		}
		break;
	case 1:
		if (!Game_Flag_Query(689)) {
			Overlay_Play("PS05OVER", 0, 1, 0, 0);
			ADQ_Add(61, 40, 3);
			ADQ_Add(61, 50, 3);
			ADQ_Add(61, 60, 3);
			ADQ_Add(61, 70, 3);
			ADQ_Add(61, 80, 3);
			Game_Flag_Set(689);
		}
		break;
	case 0:
		if (!Game_Flag_Query(688)) {
			Overlay_Play("PS05OVER", 0, 1, 0, 0);
			ADQ_Add(61, 0, 3);
			ADQ_Add(61, 10, 3);
			ADQ_Add(61, 20, 3);
			ADQ_Add(61, 30, 3);
			ADQ_Add(51, 430, 3);
			ADQ_Add(51, 440, 3);
			ADQ_Add(51, 450, 3);
			ADQ_Add(51, 460, 3);
			Game_Flag_Set(688);
		}
		break;
	default:
		return;
	}
}

} // End of namespace BladeRunner
