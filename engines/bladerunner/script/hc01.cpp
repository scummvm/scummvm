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

void ScriptHC01::InitializeScene() {
	Music_Play(0, 31, 0, 2, -1, 1, 2);
	if (Game_Flag_Query(385)) {
		Setup_Scene_Information(64.0f, 0.14f, 83.0f, 266);
	} else if (Game_Flag_Query(387)) {
		Setup_Scene_Information(607.0f, 0.14f, 9.0f, 530);
	} else {
		Setup_Scene_Information(780.0f, 0.14f, 153.0f, 815);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	if (Game_Flag_Query(402)) {
		Scene_Exit_Add_2D_Exit(1, 394, 229, 485, 371, 1);
	}
	Scene_Exit_Add_2D_Exit(2, 117, 0, 286, 319, 0);
	Ambient_Sounds_Add_Looping_Sound(103, 50, 50, 0);
	Ambient_Sounds_Add_Looping_Sound(241, 50, 50, 0);
	Ambient_Sounds_Add_Sound(242, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(243, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(244, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(245, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(246, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(247, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(248, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(249, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(181, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(183, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(190, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(193, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(194, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Scene_Loop_Set_Default(0);
}

void ScriptHC01::SceneLoaded() {
	Obstacle_Object("PILLAR", true);
	if (Game_Flag_Query(322)) {
		Preload(19);
		Preload(426);
		Preload(430);
		Preload(437);
		Preload(427);
		Preload(431);
		Preload(433);
		Preload(424);
		Preload(428);
		Preload(436);
		Preload(429);
		Preload(425);
		Preload(432);
	}
}

bool ScriptHC01::MouseClick(int x, int y) {
	return false;
}

bool ScriptHC01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptHC01::ClickedOnActor(int actorId) {
	if (actorId == 7 && (Actor_Query_Goal_Number(7) == 150 || Actor_Query_Goal_Number(7) == 0)) {
		AI_Movement_Track_Pause(7);
		if (!Loop_Actor_Walk_To_XYZ(0, 624.43f, 0.14f, 83.0f, 0, 1, false, 0)) {
			if (Game_Flag_Query(400)) {
				Actor_Face_Actor(0, 7, true);
				Actor_Face_Actor(7, 0, true);
				sub_402384();
			} else {
				Actor_Face_Actor(7, 0, true);
				Actor_Says_With_Pause(7, 10, 0.2f, 13);
				Actor_Face_Actor(0, 7, true);
				Actor_Says(7, 20, 17);
				Actor_Says(0, 1035, 18);
				Actor_Says_With_Pause(7, 30, 0.2f, 17);
				Actor_Says_With_Pause(7, 40, 0.0f, 13);
				Actor_Says(7, 50, 12);
				Actor_Says_With_Pause(0, 1040, 1.2f, 13);
				Actor_Says(7, 60, 16);
				Actor_Says_With_Pause(7, 70, 1.0f, 13);
				Actor_Says_With_Pause(0, 1045, 0.6f, 14);
				Actor_Says(7, 80, 18);
				Game_Flag_Set(400);
			}
		}
		AI_Movement_Track_Unpause(7);
	}
	return false;
}

bool ScriptHC01::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 107) {
		Item_Remove_From_World(107);
		Item_Pickup_Spin_Effect(977, 361, 381);
		Delay(1500);
		Item_Pickup_Spin_Effect(984, 377, 397);
		Delay(1500);
		Item_Pickup_Spin_Effect(984, 330, 384);
		if (Game_Flag_Query(374)) {
			Actor_Clue_Acquire(0, 246, 1, 7);
		} else {
			Actor_Clue_Acquire(0, 247, 1, 7);
		}
		Actor_Clue_Acquire(0, 260, 1, 7);
		return true;
	}
	return false;
}

bool ScriptHC01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 814.0f, 0.14f, 153.0f, 0, 1, false, 0)) {
			Music_Adjust(12, 0, 2);
			Game_Flag_Set(323);
			Set_Enter(0, 0);
			Game_Flag_Reset(479);
			Game_Flag_Set(180);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 607.0f, 0.14f, 9.0f, 0, 1, false, 0)) {
			Game_Flag_Set(386);
			Set_Enter(8, 33);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 105.0f, 0.14f, 103.0f, 0, 1, false, 0)) {
			Game_Flag_Set(384);
			Async_Actor_Walk_To_XYZ(0, -57.0f, 0.14f, 83.0f, 0, false);
			Set_Enter(8, 32);
		}
		return true;
	}
	return false;
}

bool ScriptHC01::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptHC01::SceneFrameAdvanced(int frame) {
	Set_Fade_Color(1.0f, 1.0f, 1.0f);
	if (frame >= 61 && frame < 65) {
		Set_Fade_Density((frame - 61) / 4.0f);
	} else if (frame >= 65 && frame < 93) {
		Set_Fade_Density(1.0f);
	} else if (frame >= 93 && frame < 106) {
		Set_Fade_Density((105 - frame) / 13.0f);
	} else {
		Set_Fade_Density(0.0f);
	}
	if (frame == 61) {
		Ambient_Sounds_Play_Sound(312, 90, 0, 0, 0);
	}
	if (frame == 65) {
		Ambient_Sounds_Play_Sound(315, 50, 0, 100, 0);
	}
	if (frame == 80) {
		Ambient_Sounds_Play_Sound(316, 40, 100, 100, 0);
		Item_Add_To_World(121, 931, 8, 582.0f, 27.0f, -41.0f, 0, 8, 8, true, true, false, true);
	}
}

void ScriptHC01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptHC01::PlayerWalkedIn() {
	if (Game_Flag_Query(385)) {
		Loop_Actor_Walk_To_XYZ(0, 105.0f, 0.14f, 103.0f, 0, 0, false, 0);
		Game_Flag_Reset(385);
	}
	if (Game_Flag_Query(387)) {
		Game_Flag_Reset(387);
	}
	if (Game_Flag_Query(322)) {
		Game_Flag_Reset(322);
	}
}

void ScriptHC01::PlayerWalkedOut() {
	Set_Fade_Density(0.0f);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptHC01::DialogueQueueFlushed(int a1) {
}

void ScriptHC01::sub_402384() {
	if (!Game_Flag_Query(401)) {
		Actor_Says(0, 1055, 13);
		Actor_Says(7, 130, 13);
		Actor_Says_With_Pause(0, 1060, 0.2f, 13);
		Actor_Says(7, 140, 13);
		Game_Flag_Set(401);
	}
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 56) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1020, 6, 7, 3);
	} else if (Actor_Clue_Query(0, 44) || Actor_Clue_Query(0, 47) || Actor_Clue_Query(0, 14)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1010, 6, 7, 3);
	}
	if (Actor_Clue_Query(0, 58) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1030, 1, 5, 7);
	} else if (Actor_Clue_Query(0, 5) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1040, 4, 4, 6);
	}
	if (Actor_Clue_Query(0, 181) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1050, -1, 3, 8);
	} else if (Actor_Clue_Query(0, 180) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1060, -1, 3, 8);
	}
	if (!Dialogue_Menu_Query_List_Size()) {
		Actor_Says_With_Pause(0, 1105, 1.2f, 13);
		if (Actor_Query_Friendliness_To_Other(7, 0) < 50) {
			Actor_Says(7, 550, 15);
		} else {
			Actor_Says(7, 250, 13);
			Actor_Modify_Friendliness_To_Other(7, 0, -1);
			if (Actor_Query_Friendliness_To_Other(7, 0) < 47 && Query_Difficulty_Level() == 0) {
				sub_40346C();
			}
		}
		return;
	}
	Dialogue_Menu_Add_DONE_To_List(100);
	bool end = false;
	do {
		Dialogue_Menu_Appear(320, 240);
		int answer = Dialogue_Menu_Query_Input();
		Dialogue_Menu_Disappear();
		if (answer == 1020) {
			Dialogue_Menu_Remove_From_List(1020);
			Actor_Says(0, 1065, 15);
			Actor_Says(7, 160, 3);
			Actor_Says(0, 1110, 16);
			Actor_Says(7, 170, 3);
			Actor_Says(7, 180, 3);
			Actor_Says(7, 190, 12);
			if (Query_Difficulty_Level() < 2) {
				Actor_Modify_Friendliness_To_Other(7, 0, -2);
			}
		}
		if (answer == 1010) {
			Dialogue_Menu_Remove_From_List(1010);
			Actor_Clue_Acquire(0, 60, 0, 7);
			Actor_Says(0, 1070, 13);
			Actor_Says(7, 200, 17);
			Actor_Says(7, 210, 12);
			Actor_Says(0, 1115, 12);
			Actor_Says(7, 220, 16);
			Actor_Says(7, 230, 3);
			Actor_Says(7, 240, 15);
			if (Query_Difficulty_Level() < 2) {
				Actor_Modify_Friendliness_To_Other(7, 0, -1);
			}
		}
		if (answer == 1010 || answer == 1020) {
			Actor_Says_With_Pause(0, 1120, 0.9f, 17);
			Actor_Says(7, 250, 13);
			Actor_Says(0, 1125, 14);
			if (Actor_Query_Friendliness_To_Other(7, 0) < 47) {
				Actor_Set_Goal_Number(7, 1);
				Player_Loses_Control();
				Actor_Says(7, 90, 3);
				Actor_Face_Actor(7, 0, true);
				Actor_Says(7, 100, 3);
				Actor_Says(7, 110, 3);
				Actor_Says_With_Pause(0, 1050, 0.2f, 3);
				Actor_Says(7, 120, 3);
				Actor_Set_Goal_Number(7, 2);
			}
			end = true;
		}
		if (answer == 1030) {
			Dialogue_Menu_Remove_From_List(1030);
			Actor_Says(0, 1075, 18);
			Actor_Says(7, 260, 12);
			Actor_Says(7, 270, 16);
			Actor_Says(0, 1130, 14);
			Actor_Says(7, 280, 17);
			Actor_Says(0, 1135, 15);
			Actor_Says(7, 290, 15);
			Actor_Says(7, 300, 12);
			Actor_Says(7, 310, 17);
			Actor_Says(0, 1140, 3);
			if (Query_Difficulty_Level() < 2) {
				Actor_Modify_Friendliness_To_Other(7, 0, -2);
			}
			if (Actor_Query_Friendliness_To_Other(7, 0) < 47) {
				Actor_Set_Goal_Number(7, 1);
				Player_Loses_Control();
				Actor_Says(7, 90, 3);
				Actor_Face_Actor(7, 0, true);
				Actor_Says(7, 100, 3);
				Actor_Says(7, 110, 3);
				Actor_Says_With_Pause(0, 1050, 0.2f, 3);
				Actor_Says(7, 120, 3);
				Actor_Set_Goal_Number(7, 2);
			}
			end = true;
		}
		if (answer == 1040) {
			Dialogue_Menu_Remove_From_List(1040);
			Actor_Says(0, 1080, 15);
			Actor_Says(0, 1085, 17);
			Actor_Says(7, 320, 17);
			Actor_Says(0, 1145, 13);
			Actor_Says(7, 330, 17);
			Actor_Says(7, 340, 13);
			Actor_Says(7, 350, 12);
			end = true;
		}
		if (answer == 1050) {
			Dialogue_Menu_Remove_From_List(1050);
			Actor_Says(0, 1090, 18);
			Actor_Says(7, 360, 14);
			Actor_Says(0, 1150, 17);
			Actor_Says(7, 370, 13);
			Actor_Says(0, 1155, 15);
			Actor_Says(7, 380, 12);
			Actor_Says(0, 1160, 14);
			Actor_Says(0, 1165, 18);
			Actor_Says(7, 390, 16);
			Actor_Says(0, 1170, 12);
			Actor_Says(7, 400, 13);
			Actor_Says(0, 1180, 14);
			Actor_Says(7, 410, 12);
			Actor_Says(7, 420, 16);
			Actor_Says(7, 430, 17);
			Actor_Says(7, 440, 13);
			Actor_Modify_Friendliness_To_Other(7, 0, -4);
			if (Actor_Query_Friendliness_To_Other(7, 0) < 47) {
				Actor_Set_Goal_Number(7, 1);
				Player_Loses_Control();
				Actor_Says(7, 90, 3);
				Actor_Face_Actor(7, 0, true);
				Actor_Says(7, 100, 3);
				Actor_Says(7, 110, 3);
				Actor_Says_With_Pause(0, 1050, 0.2f, 3);
				Actor_Says(7, 120, 3);
				Actor_Set_Goal_Number(7, 2);
			}
			end = true;
		}
		if (answer == 1060) {
			Dialogue_Menu_Remove_From_List(1060);
			Actor_Says(0, 1095, 15);
			Actor_Says_With_Pause(0, 1100, 1.2f, 18);
			Actor_Says(7, 450, 12);
			Actor_Says(7, 460, 13);
			Actor_Says(0, 1185, 18);
			Actor_Says(7, 470, 14);
			Actor_Says(0, 1190, 14);
			Actor_Says(7, 480, 13);
			Actor_Says(0, 1195, 16);
			Actor_Says(0, 1200, 18);
			Actor_Says(7, 490, 12);
			Actor_Says(0, 1205, 14);
			Actor_Says(7, 500, 14);
			Actor_Says(7, 510, 17);
			Actor_Says(7, 520, 16);
			Actor_Says(7, 530, 15);
			Actor_Says(0, 1210, 16);
			Actor_Modify_Friendliness_To_Other(7, 0, -4);
			if (Actor_Query_Friendliness_To_Other(7, 0) < 47) {
				Actor_Set_Goal_Number(7, 1);
				Player_Loses_Control();
				Actor_Says(7, 90, 3);
				Actor_Face_Actor(7, 0, true);
				Actor_Says(7, 100, 3);
				Actor_Says(7, 110, 3);
				Actor_Says_With_Pause(0, 1050, 0.2f, 3);
				Actor_Says(7, 120, 3);
				Actor_Set_Goal_Number(7, 2);
			}
			end = true;
		}
		if (answer == 100) {
			end = true;
		}
	} while (!end);
}

void ScriptHC01::sub_40346C() {
	Actor_Set_Goal_Number(7, 1);
	Player_Loses_Control();
	Actor_Says(7, 90, 3);
	Actor_Face_Actor(7, 0, true);
	Actor_Says(7, 100, 3);
	Actor_Says(7, 110, 3);
	Actor_Says_With_Pause(0, 1050, 0.2f, 3);
	Actor_Says(7, 120, 3);
	Actor_Set_Goal_Number(7, 2);
}

} // End of namespace BladeRunner
