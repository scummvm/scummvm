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

void SceneScriptNR04::InitializeScene() {
	Music_Adjust(30, 80, 2);
	Setup_Scene_Information(53.0f, 0.0f, -110.0f, 569);
	Scene_Exit_Add_2D_Exit(0, 498, 126, 560, 238, 0);
	Scene_2D_Region_Add(0, 0, 259, 61, 479);
	Scene_2D_Region_Add(1, 62, 327, 92, 479);
	Scene_2D_Region_Add(2, 93, 343, 239, 479);
	Ambient_Sounds_Add_Looping_Sound(408, 16, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(384, 16, 0, 1);
	Ambient_Sounds_Add_Sound(259, 3, 60, 9, 9, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(260, 3, 60, 9, 9, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(261, 3, 60, 9, 9, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(262, 3, 60, 9, 9, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 8, 8, -100, 100, -101, -101, 0, 0);
	Scene_Loop_Set_Default(0);
}

void SceneScriptNR04::SceneLoaded() {
	Clickable_Object("B.TV01");
	Clickable_Object("B.TV02");
	Clickable_Object("B.TV03");
	Clickable_Object("B.TV05");
	Clickable_Object("DESK");
	if (!Game_Flag_Query(605)) {
		Clickable_Object("TORUS01");
	}
	Clickable_Object("BOX12");
}

bool SceneScriptNR04::MouseClick(int x, int y) {
	if (Actor_Query_Animation_Mode(kActorMcCoy) == 85 || Actor_Query_Animation_Mode(kActorMcCoy) == 29) {
		return true;
	}
	if (Actor_Query_Animation_Mode(kActorMcCoy) == 53) {
		Actor_Change_Animation_Mode(kActorMcCoy, 29);
		return true;
	}
	return false;
}

bool SceneScriptNR04::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("B.TV01", objectName) || Object_Query_Click("B.TV02", objectName) || Object_Query_Click("B.TV03", objectName) || Object_Query_Click("B.TV05", objectName) || Object_Query_Click("DESK", objectName)) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 546, 0, 1, false)) {
			if (!Object_Query_Click("DESK", objectName)) {
				Actor_Face_Object(kActorMcCoy, "B.TV01", true);
				Actor_Voice_Over(1530, kActorVoiceOver);
				Actor_Voice_Over(1540, kActorVoiceOver);
				Actor_Voice_Over(1550, kActorVoiceOver);
			} else {
				Actor_Face_Object(kActorMcCoy, "DESK", true);
				if (!Actor_Clue_Query(kActorMcCoy, kCluePeruvianLadyInterview)) {
					Actor_Voice_Over(1600, kActorVoiceOver);
					Actor_Voice_Over(1610, kActorVoiceOver);
				} else if (Actor_Clue_Query(kActorMcCoy, kClueCollectionReceipt)) {
					Actor_Says(kActorMcCoy, 8580, 3);
				} else {
					Actor_Clue_Acquire(kActorMcCoy, kClueCollectionReceipt, 0, -1);
					Item_Pickup_Spin_Effect(961, 247, 141);
					Actor_Voice_Over(1560, kActorVoiceOver);
					Actor_Voice_Over(1570, kActorVoiceOver);
					Actor_Voice_Over(1580, kActorVoiceOver);
					Actor_Voice_Over(1590, kActorVoiceOver);
				}

			}
		}
	} else if (Object_Query_Click("TORUS01", objectName)
		&& !Loop_Actor_Walk_To_XYZ(kActorMcCoy, 18.56f, 0.0f, 38.86f, 0, 1, false, 0)
		&& !Game_Flag_Query(605)) {
		Unclickable_Object("TORUS01");
		Scene_Exits_Disable();
		Player_Loses_Control();
		Game_Flag_Set(605);
		Actor_Face_Object(kActorMcCoy, "TORUS01", true);
		Item_Pickup_Spin_Effect(975, 358, 160);
		Actor_Voice_Over(1620, kActorVoiceOver);
		Actor_Voice_Over(1630, kActorVoiceOver);
		Actor_Clue_Acquire(kActorMcCoy, kClueEarlyQsClub, 0, -1);
		Actor_Set_Goal_Number(kActorEarlyQ, 201);
	}
	return false;
}

bool SceneScriptNR04::ClickedOnActor(int actorId) {
	if (actorId == 18 && Game_Flag_Query(606)) {
		Actor_Voice_Over(1640, kActorVoiceOver);
		Actor_Voice_Over(1650, kActorVoiceOver);
		Actor_Voice_Over(1660, kActorVoiceOver);
		Actor_Voice_Over(1670, kActorVoiceOver);
		Actor_Voice_Over(1680, kActorVoiceOver);
		return true;
	}
	return false;
}

bool SceneScriptNR04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 45.0f, 0.0f, -106.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(437);
			Set_Enter(kSetNR03, kSceneNR03);
		}
		return true;
	}
	return false;
}

bool SceneScriptNR04::ClickedOn2DRegion(int region) {
	if ((region == 0 || region == 1 || region == 2) && Actor_Query_Which_Set_In(kActorEarlyQ) != 12 && Actor_Query_Animation_Mode(kActorMcCoy) != 53 && !Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 445, 0, 1, false)) {
		Actor_Face_Heading(kActorMcCoy, 49, false);
		Actor_Change_Animation_Mode(kActorMcCoy, 85);
		Delay(2500);
		if (Game_Flag_Query(606) == 1) {
			return true;
		}
		if (Game_Flag_Query(374)) {
			Player_Loses_Control();
			Actor_Voice_Over(4180, kActorVoiceOver);
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
			Ambient_Sounds_Play_Sound(555, 90, 99, 0, 0);
			Delay(350);
			Actor_Set_At_XYZ(kActorMcCoy, 109.0f, 0.0f, 374.0f, 0);
			Actor_Retired_Here(kActorMcCoy, 12, 12, 1, -1);
		}
		return true;
	}
	return false;
}

void SceneScriptNR04::SceneFrameAdvanced(int frame) {
	if (frame == 1 && !Music_Is_Playing()) {
		sub_402960();
	}
	if (frame > 60 && frame < 120) {
		sub_402860(frame);
	} else if (frame == 120) {
		Set_Fade_Color(1.0f, 1.0f, 1.0f);
		Set_Fade_Density(0.0f);
	}
	//return false;
}

void SceneScriptNR04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == 18) {
		switch (newGoal) {
		case 214:
			Actor_Change_Animation_Mode(kActorEarlyQ, 29);
			Delay(2500);
			Actor_Says(kActorEarlyQ, 290, 3);
			sub_401DB0();
			//return true;
			break;
		case 213:
			Actor_Clue_Acquire(kActorMcCoy, kClueDektorasDressingRoom, 0, kActorEarlyQ);
			Item_Pickup_Spin_Effect(984, 200, 160);
			Actor_Says(kActorEarlyQ, 200, 30);
			Actor_Says(kActorEarlyQ, 210, 30);
			Actor_Says(kActorEarlyQ, 220, 30);
			Actor_Says_With_Pause(kActorMcCoy, 3425, 1.5f, 23);
			Actor_Says(kActorMcCoy, 3430, 3);
			Actor_Says(kActorEarlyQ, 240, 30);
			Actor_Says(kActorMcCoy, 3435, 3);
			Actor_Says(kActorEarlyQ, 250, 30);
			Actor_Says(kActorMcCoy, 3440, 3);
			Actor_Says(kActorEarlyQ, 280, 30);
			Actor_Says(kActorMcCoy, 3445, 3);
			Actor_Set_Goal_Number(kActorEarlyQ, 214);
			//return true;
			break;
		case 209:
			Actor_Face_Actor(kActorMcCoy, kActorEarlyQ, true);
			Delay(3000);
			Actor_Says(kActorEarlyQ, 170, 30);
			Actor_Says(kActorMcCoy, 3415, 3);
			Actor_Says(kActorEarlyQ, 180, 30);
			Actor_Says_With_Pause(kActorMcCoy, 3420, 1.5f, 3);
			Actor_Says(kActorEarlyQ, 190, 30);
			Actor_Set_Goal_Number(kActorEarlyQ, 211);
			//return true;
			break;
		case 207:
			Loop_Actor_Walk_To_Waypoint(kActorEarlyQ, 445, 0, 1, false);
			Actor_Face_Heading(kActorEarlyQ, 49, false);
			Actor_Change_Animation_Mode(kActorEarlyQ, 85);
			Actor_Face_Actor(kActorMcCoy, kActorEarlyQ, true);
			Actor_Set_Goal_Number(kActorEarlyQ, 208);
			Actor_Clue_Acquire(kActorMcCoy, kClueEarlyQInterview, 0, kActorEarlyQ);
			//return true;
			break;
		case 204:
			Actor_Face_Actor(kActorMcCoy, kActorEarlyQ, true);
			Actor_Says(kActorEarlyQ, 90, 73);
			Actor_Says(kActorMcCoy, 3390, 3);
			Actor_Face_Actor(kActorEarlyQ, kActorMcCoy, true);
			Actor_Says(kActorEarlyQ, 110, 74);
			Actor_Says(kActorMcCoy, 3385, 3);
			Actor_Says(kActorEarlyQ, 120, 74);
			Actor_Face_Actor(kActorEarlyQ, kActorMcCoy, true);
			Actor_Set_Goal_Number(kActorEarlyQ, 205);
			//return true;
			break;
		case 202:
			Actor_Face_Actor(kActorEarlyQ, kActorMcCoy, true);
			Actor_Face_Actor(kActorMcCoy, kActorEarlyQ, true);
			Actor_Says(kActorEarlyQ, 30, 3);
			Actor_Says(kActorMcCoy, 3375, 3);
			Actor_Says_With_Pause(kActorEarlyQ, 50, 1.5f, 3);
			Actor_Says(kActorEarlyQ, 60, 3);
			Actor_Says_With_Pause(kActorMcCoy, 3380, 1.0f, 3);
			Actor_Says(kActorEarlyQ, 70, 3);
			Actor_Says(kActorMcCoy, 3415, 3);
			Actor_Says(kActorEarlyQ, 80, 3);
			Player_Gains_Control();
			Actor_Set_Goal_Number(kActorEarlyQ, 203);
			//return true;
			break;
		}
	}
	//return false;
}

void SceneScriptNR04::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 53.0f, 0.0f, -26.0f, 0, 0, false, 0);
	if (Game_Flag_Query(374)) {
		Overlay_Play("nr04over", 0, 1, 0, 0);
		Delay(4000);
		Overlay_Remove("nr04over");
	}
	//return false;
}

void SceneScriptNR04::PlayerWalkedOut() {
}

void SceneScriptNR04::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR04::sub_401DB0() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List(1530, 10, 5, 3);
	DM_Add_To_List(1540, 3, 5, 10);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	if (answer == 1530) {
		Loop_Actor_Walk_To_Actor(kActorEarlyQ, kActorMcCoy, 36, 0, false);
		Actor_Change_Animation_Mode(kActorMcCoy, 23);
		Actor_Change_Animation_Mode(kActorEarlyQ, 23);
		Delay(1500);
		Actor_Says(kActorEarlyQ, 300, 3);
		Actor_Change_Animation_Mode(kActorMcCoy, 0);
		Actor_Change_Animation_Mode(kActorEarlyQ, 0);
		Actor_Says(kActorEarlyQ, 310, 3);
		ADQ_Add(kActorMcCoy, 3450, 3);
		Actor_Set_Targetable(kActorEarlyQ, false);
		Actor_Set_Goal_Number(kActorEarlyQ, 217);
		Actor_Clue_Lose(kActorMcCoy, kClueEarlyQsClub);
		Scene_Exits_Enable();
	} else if (answer == 1540) {
		Actor_Says(kActorMcCoy, 8512, 15);
		Actor_Says(kActorEarlyQ, 320, 12);
		Actor_Says(kActorMcCoy, 3455, 13);
		Actor_Says(kActorEarlyQ, 330, 15);
		Actor_Says(kActorMcCoy, 3460, 12);
		Actor_Says(kActorEarlyQ, 340, 12);
		Actor_Says(kActorMcCoy, 3465, 12);
		Actor_Says(kActorEarlyQ, 350, 16);
		Actor_Set_Targetable(kActorEarlyQ, false);
		Actor_Set_Goal_Number(kActorEarlyQ, 217);
		Scene_Exits_Enable();
	}
}

void SceneScriptNR04::sub_402860(int frame) {
	float colorMap[] = {
		1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
		0.8f, 0.4f, 0.0f,
		0.7f, 0.7f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, 0.8f};

	float v3 = (frame - 60) * 0.1f;
	float v4 = (frame % 10) * 0.1f;
	float coef = 1.0f;
	if (frame > 100) {
		coef = 1.0f - (frame - 100) / 20.0f;
	}
	int index = 3 * v3;
	int nextIndex = 3 * v3 + 3;
	float r = ((colorMap[nextIndex + 0] - colorMap[index + 0]) * v4 + colorMap[index + 0]) * coef;
	float g = ((colorMap[nextIndex + 1] - colorMap[index + 1]) * v4 + colorMap[index + 1]) * coef;
	float b = ((colorMap[nextIndex + 2] - colorMap[index + 2]) * v4 + colorMap[index + 2]) * coef;
	Set_Fade_Color(r, g, b);
	if (frame >= 90) {
		Set_Fade_Density(0.75f);
	} else {
		Set_Fade_Density((frame - 60) / 45.0f);
	}
}

void SceneScriptNR04::sub_402960() {
	int v0 = Global_Variable_Query(53);
	if (!v0) {
		Music_Play(14, 11, 80, 2, -1, 0, 0);
	} else if (v0 == 1) {
		Music_Play(13, 11, 80, 2, -1, 0, 0);
	} else if (v0 == 2) {
		Music_Play(5, 11, 80, 2, -1, 0, 0);
	}
	v0++;
	if (v0 > 2) {
		v0 = 0;
	}
	Global_Variable_Set(53, v0);
}

} // End of namespace BladeRunner
