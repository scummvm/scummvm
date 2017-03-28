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

void SceneScriptPS05::InitializeScene() {
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

void SceneScriptPS05::SceneLoaded() {
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

bool SceneScriptPS05::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS05::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("WATER FOUNTAIN", objectName) && !Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "WATER FOUNTAIN", 12, true, false)) {
		Actor_Face_Object(kActorMcCoy, "WATER FOUNTAIN", true);
		Actor_Says(kActorMcCoy, 3490, 18);
	}
	if (Object_Query_Click("ASHTRAY", objectName) && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, 662.0f, 0.37f, -180.0f, 0, 1, false, 0)) {
		Actor_Face_Object(kActorMcCoy, "ASHTRAY", true);
		Actor_Voice_Over(1770, kActorVoiceOver);
		Actor_Voice_Over(1780, kActorVoiceOver);
		Actor_Voice_Over(1790, kActorVoiceOver);
	}
	if (Object_Query_Click("WIRE BASKET", objectName) && !Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "WIRE BASKET", 12, true, false)) {
		Actor_Face_Object(kActorMcCoy, "WIRE BASKET", true);
		Actor_Voice_Over(1810, kActorVoiceOver);
		Actor_Voice_Over(1820, kActorVoiceOver);
	}
	if (Object_Query_Click("WANTED POSTERS", objectName) && !Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "WANTED POSTERS", 12, true, false)) {
		Actor_Face_Object(kActorMcCoy, "WANTED POSTERS", true);
		Actor_Voice_Over(1800, kActorVoiceOver);
	}
	return false;
}

bool SceneScriptPS05::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS05::ClickedOnItem(int itemId, bool a2) {
	if (Game_Flag_Query(23)) {
		Actor_Set_At_XYZ(kActorMcCoy, 718.72f, 0.37f, -461.26f, 600);
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

bool SceneScriptPS05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 2, 24, 1, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(101, 119);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 635.0f, 0.0f, -598.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(62, 66);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 742.52002f, 0.37f, -457.69f, 0, 1, false, 0)) {
			Game_Flag_Set(136);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(65, 70);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS05::ClickedOn2DRegion(int region) {
	if (region == 0 && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, 694.78f, 0.37f, -321.05f, 0, 1, false, 0)) {
		Actor_Face_Heading(kActorMcCoy, 130, false);
		View_Score_Board();
	}
	return false;
}

void SceneScriptPS05::SceneFrameAdvanced(int frame) {
	if (frame == 1 || frame == 16 || frame == 31 || frame == 46) {
		Sound_Play(149, Random_Query(10, 10), 70, 70, 50);
	}
	//return true;
}

void SceneScriptPS05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS05::PlayerWalkedIn() {
}

void SceneScriptPS05::PlayerWalkedOut() {
}

void SceneScriptPS05::DialogueQueueFlushed(int a1) {
	Overlay_Remove("PS05OVER");
}

void SceneScriptPS05::sub_401B34() {
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

void SceneScriptPS05::sub_401C30() {
	switch (Global_Variable_Query(52)) {
	case 4:
		if (!Game_Flag_Query(692)) {
			Overlay_Play("PS05OVER", 0, 1, 0, 0);
			ADQ_Add(kActorNewscaster, 230, 3);
			ADQ_Add(kActorNewscaster, 240, 3);
			Game_Flag_Set(692);
		}
		break;
	case 3:
		if (!Game_Flag_Query(691)) {
			Overlay_Play("PS05OVER", 0, 1, 0, 0);
			ADQ_Add(kActorNewscaster, 170, 3);
			ADQ_Add(kActorNewscaster, 180, 3);
			ADQ_Add(kActorNewscaster, 190, 3);
			ADQ_Add(kActorNewscaster, 200, 3);
			ADQ_Add(kActorNewscaster, 210, 3);
			ADQ_Add(kActorNewscaster, 220, 3);
			ADQ_Add(kActorGovernorKolvig, 80, 3);
			ADQ_Add(kActorGovernorKolvig, 90, 3);
			ADQ_Add(kActorGovernorKolvig, 100, 3);
			ADQ_Add(kActorGovernorKolvig, 110, 3);
			ADQ_Add(kActorGovernorKolvig, 120, 3);
			ADQ_Add(kActorGovernorKolvig, 130, 3);
			Game_Flag_Set(691);
		}
		break;
	case 2:
		if (!Game_Flag_Query(690)) {
			Overlay_Play("PS05OVER", 0, 1, 0, 0);
			if (Actor_Query_Friendliness_To_Other(kActorClovis, kActorMcCoy) > Actor_Query_Friendliness_To_Other(kActorSteele, kActorMcCoy)) {
				ADQ_Add(kActorNewscaster, 120, 3);
				ADQ_Add(kActorNewscaster, 130, 3);
				ADQ_Add(kActorNewscaster, 140, 3);
				ADQ_Add(kActorNewscaster, 150, 3);
				ADQ_Add(kActorGuzza, 1570, 3);
				ADQ_Add(kActorGuzza, 1580, 3);
				ADQ_Add(kActorGuzza, 1590, 3);
			} else {
				ADQ_Add(kActorNewscaster, 90, 3);
				ADQ_Add(kActorNewscaster, 100, 3);
				ADQ_Add(kActorNewscaster, 110, 3);
				ADQ_Add(kActorGuzza, 1540, 3);
				ADQ_Add(kActorGuzza, 1550, 3);
				ADQ_Add(kActorGuzza, 1560, 3);
			}
			Game_Flag_Set(690);
		}
		break;
	case 1:
		if (!Game_Flag_Query(689)) {
			Overlay_Play("PS05OVER", 0, 1, 0, 0);
			ADQ_Add(kActorNewscaster, 40, 3);
			ADQ_Add(kActorNewscaster, 50, 3);
			ADQ_Add(kActorNewscaster, 60, 3);
			ADQ_Add(kActorNewscaster, 70, 3);
			ADQ_Add(kActorNewscaster, 80, 3);
			Game_Flag_Set(689);
		}
		break;
	case 0:
		if (!Game_Flag_Query(688)) {
			Overlay_Play("PS05OVER", 0, 1, 0, 0);
			ADQ_Add(kActorNewscaster, 0, 3);
			ADQ_Add(kActorNewscaster, 10, 3);
			ADQ_Add(kActorNewscaster, 20, 3);
			ADQ_Add(kActorNewscaster, 30, 3);
			ADQ_Add(kActorTyrell, 430, 3);
			ADQ_Add(kActorTyrell, 440, 3);
			ADQ_Add(kActorTyrell, 450, 3);
			ADQ_Add(kActorTyrell, 460, 3);
			Game_Flag_Set(688);
		}
		break;
	default:
		return;
	}
}

} // End of namespace BladeRunner
