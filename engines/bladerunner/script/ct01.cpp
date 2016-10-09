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

void ScriptCT01::InitializeScene() {
	Music_Play(3, 28, 0, 2, -1, 1, 0);
	Game_Flag_Reset(247);
	if (Game_Flag_Query(68)) {
		Game_Flag_Reset(68);
		Setup_Scene_Information(-35.2f, -6.5f, 352.28f, 603);
	} else if (Game_Flag_Query(71)) {
		Game_Flag_Reset(71);
		Setup_Scene_Information(-311.0f, -6.5f, 710.0f, 878);
	} else if (Game_Flag_Query(88)) {
		Game_Flag_Reset(88);
		Setup_Scene_Information(-419.0f, -6.5f, 696.0f, 28);
		if (Global_Variable_Query(1) != 2 && Global_Variable_Query(1) != 3) {
			if (Game_Flag_Query(248)) {
				Scene_Loop_Start_Special(0, 0, 0);
			} else {
				Scene_Loop_Start_Special(0, 6, 0);
			}
		}
	} else if (Game_Flag_Query(248)) {
		Setup_Scene_Information(-530.0f, -6.5f, 241.0f, 506);
		Game_Flag_Set(247);
	} else {
		Setup_Scene_Information(-397.0f, -6.5f, 471.0f, 250);
	}
	Scene_Exit_Add_2D_Exit(0, 290, 256, 360, 304, 1);
	if (Actor_Clue_Query(0, 18)) {
		Scene_Exit_Add_2D_Exit(1, 571, 233, 639, 367, 1);
	}
	if (Game_Flag_Query(94)) {
		Scene_Exit_Add_2D_Exit(2, 506, 400, 639, 479, 2);
	}
	if (Game_Flag_Query(248)) {
		Scene_Exit_Add_2D_Exit(3, 0, 286, 158, 350, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(54, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(55, 40, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(56, 40, 100, 1);
	Ambient_Sounds_Add_Sound(61, 10, 30, 16, 20, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(62, 10, 30, 16, 20, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(63, 10, 30, 16, 20, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(64, 10, 30, 16, 20, 0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 27, 47, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(68, 10, 40, 33, 50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 40, 33, 50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 20, 40, 33, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 20, 40, 33, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 20, 40, 33, 50, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(248)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(7);
	}
}

void ScriptCT01::SceneLoaded() {
	Obstacle_Object("HYDRANT02", true);
	Obstacle_Object("HOWWALLRT", true);
	Obstacle_Object("HOW-CHAIR1", true);
	Obstacle_Object("HOWWALLLFT", true);
	Obstacle_Object("HOWDOOR01", true);
	Unobstacle_Object("SPINNER BODY", true);
	Unobstacle_Object("HOWFLOOR", true);
	Unobstacle_Object("PAPER11", true);
	Unobstacle_Object("PAPER16", true);
	Unclickable_Object("HYDRANT02");
	Unclickable_Object("TURBINE");
	Unclickable_Object("SPINNER BODY");
	Unclickable_Object("OBJECT04");
}

bool ScriptCT01::MouseClick(int x, int y) {
	return false;
}

bool ScriptCT01::ClickedOn3DObject(const char *objectName, bool a2) {
//	if ("ASIANSITTINGANDEATI" == objectName) { //bug?
	if (Object_Query_Click("ASIANSITTINGANDEATI", objectName)) {
		Actor_Face_Object(0, "ASIANSITTINGANDEATI", true);
		Actor_Says(0, 365, 13);
		Actor_Says(28, 160, 13);
		return true;
	}
	return false;
}

bool ScriptCT01::ClickedOnActor(int actorId) {
	if (actorId == 28) {
		Actor_Set_Goal_Number(28, 50);
		if (!Loop_Actor_Walk_To_XYZ(0, -335.23f, -6.5f, 578.97f, 12, 1, false, 0)) {
			Actor_Face_Actor(0, 28, true);
			Actor_Face_Actor(28, 0, true);
			if (!Game_Flag_Query(26)) {
				Actor_Says(0, 260, 18);
				Actor_Says(28, 0, 14);
				Game_Flag_Set(26);
				Actor_Set_Goal_Number(28, 0);
			} else if (!Game_Flag_Query(30) && Actor_Query_Friendliness_To_Other(28, 0) >= 40) {
				sub_40269C();
				Actor_Set_Goal_Number(28, 0);
			} else {
				if (Game_Flag_Query(31)) {
					Actor_Says(0, 330, 17);
					Actor_Says(28, 130, 13);
					Actor_Says(28, 140, 14);
				} else if (Actor_Query_Friendliness_To_Other(28, 0) < 50) {
					Actor_Says(0, 330, 13);
					Actor_Says(28, 160, 15);
				} else {
					Actor_Says(0, 310, 11);
					Actor_Says(28, 10, 16);
				}
				Actor_Set_Goal_Number(28, 0);
			}
			return true;
		}
	}
	if (actorId == 19) {
		if (!Loop_Actor_Walk_To_XYZ(0, -335.23f, -6.5f, 578.97f, 12, 1, false, 0)) {
			Actor_Face_Actor(0, 19, true);
			Actor_Says(0, 355, 18);
			if (!Actor_Query_Goal_Number(19)) {
				Actor_Says(19, 10, 16);
				Actor_Face_Actor(28, 0, true);
				Actor_Says(28, 150, 3);
				Actor_Face_Actor(0, 28, true);
				Actor_Says(0, 360, 13);
				Actor_Modify_Friendliness_To_Other(28, 0, -5);
				Actor_Modify_Friendliness_To_Other(19, 0, -4);
			}
			return true;
		}
	}
	if (actorId == 2) {
		//todo: some weird code in assembly EBP is used but may not be initialized, loc_401C78
		if (!Actor_Query_Goal_Number(2)) {
			if (Loop_Actor_Walk_To_XYZ(0, -338.1f, -6.5f, 419.65f, 6, 1, false, 0)) {
				return false;
			}
		}

		Actor_Face_Actor(0, 2, true);
		if (!Game_Flag_Query(32)) {
			Actor_Says(0, 335, 18);
			Actor_Says(2, 20, 30);
			Game_Flag_Set(32);
			Actor_Clue_Acquire(2, 213, 1, 0);
			Actor_Clue_Acquire(0, 214, 1, 0);
			Actor_Modify_Friendliness_To_Other(2, 0, -1);
		} else if (Actor_Query_Goal_Number(2)) {
			Actor_Says(0, 365, 14);
		} else {
			Actor_Says(0, 340, 13);
			Actor_Says(0, 345, 11);
			Actor_Says(2, 30, 30);
			Actor_Says(0, 350, 13);
			Actor_Says(2, 40, 30);
			Actor_Modify_Friendliness_To_Other(2, 0, -5);
			Player_Loses_Control();
		}
		if (Actor_Query_Is_In_Current_Set(19)) {
			Actor_Modify_Friendliness_To_Other(19, 0, -2);
		}
		return true;
	}
	return false;
}

bool ScriptCT01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptCT01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -327.5f, -6.5f, 352.28f, 0, 1, false, 0)) {
			Player_Loses_Control();
			Loop_Actor_Walk_To_Waypoint(0, 106, 0, 0, false);
			Player_Gains_Control();
			Game_Flag_Reset(247);
			Set_Enter(27, 14);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -259.0f, -6.5f, 710.0f, 0, 1, false, 0)) {
			Game_Flag_Reset(247);
			Set_Enter(5, 15);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, -419.0f, -6.5f, 696.0f, 0, 1, false, 0)) {
			Game_Flag_Set(123);
			Game_Flag_Reset(247);
			Set_Enter(4, 24);
		}
		return true;
	}
	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(0, -314.0f, -6.5f, 326.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(0, -330.0f, -6.5f, 221.0f, 0, 0, true, 0);
			Loop_Actor_Walk_To_XYZ(0, -530.0f, -6.5f, 241.0f, 0, 0, true, 0);
			Game_Flag_Reset(176);
			Game_Flag_Reset(182);
			Game_Flag_Reset(179);
			Game_Flag_Reset(178);
			Game_Flag_Reset(180);
			Game_Flag_Reset(261);
			Game_Flag_Reset(177);
			Game_Flag_Reset(258);
			int spinnerDest = Spinner_Interface_Choose_Dest(-1, 0);

			switch (spinnerDest) {
			case 0:
				Game_Flag_Set(178);
				Game_Flag_Reset(247);
				Game_Flag_Reset(248);
				Game_Flag_Set(251);
				Set_Enter(61, 65);
				Scene_Loop_Start_Special(1, 5, 1);
				break;
			case 1:
				Game_Flag_Set(179);
				Game_Flag_Reset(247);
				Game_Flag_Reset(248);
				Game_Flag_Set(250);
				Set_Enter(49, 48);
				Scene_Loop_Start_Special(1, 5, 1);
				break;
			case 2:
				Game_Flag_Set(182);
				Game_Flag_Reset(247);
				Game_Flag_Reset(248);
				Game_Flag_Set(249);
				Set_Enter(69, 78);
				Scene_Loop_Start_Special(1, 5, 1);
				break;
			case 4:
				Game_Flag_Set(180);
				Game_Flag_Reset(247);
				Game_Flag_Reset(248);
				Game_Flag_Set(252);
				Set_Enter(0, 0);
				Scene_Loop_Start_Special(1, 5, 1);
				break;
			case 5:
				Game_Flag_Set(261);
				Game_Flag_Reset(248);
				Game_Flag_Reset(247);
				Game_Flag_Set(307);
				Set_Enter(17, 82);
				Scene_Loop_Start_Special(1, 5, 1);
				break;
			case 6:
				Game_Flag_Set(177);
				Game_Flag_Reset(247);
				Game_Flag_Reset(248);
				Game_Flag_Set(253);
				Set_Enter(7, 25);
				Scene_Loop_Start_Special(1, 5, 1);
				break;
			case 7:
				Game_Flag_Set(258);
				Game_Flag_Reset(247);
				Game_Flag_Reset(248);
				Game_Flag_Set(254);
				Set_Enter(20, 2);
				Scene_Loop_Start_Special(1, 5, 1);
				break;
			case 8:
				Game_Flag_Set(181);
				Game_Flag_Reset(247);
				Game_Flag_Reset(248);
				Game_Flag_Set(255);
				Set_Enter(54, 54);
				Scene_Loop_Start_Special(1, 5, 1);
				break;
			case 9:
				Game_Flag_Set(257);
				Game_Flag_Reset(247);
				Game_Flag_Reset(248);
				Game_Flag_Set(256);
				Set_Enter(37, 34);
				Scene_Loop_Start_Special(1, 5, 1);
				break;
			default:
				Game_Flag_Set(176);
				Player_Loses_Control();
				Loop_Actor_Walk_To_XYZ(0, -530.0f, -6.5f, 241.0f, 0, 0, true, 0);
				Loop_Actor_Walk_To_XYZ(0, -330.0f, -6.5f, 221.0f, 0, 0, true, 0);
				Loop_Actor_Walk_To_XYZ(0, -314.0f, -6.5f, 326.0f, 0, 0, false, 0);
				Player_Gains_Control();
				break;
			}
		}
		return true;
	}
	return false;
}

bool ScriptCT01::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptCT01::SceneFrameAdvanced(int frame) {
	if ((frame < 316 || frame > 435) && !((frame - 1) % 10)) {
		/*int v1;
		int v2 = Random_Query(0, 1);
		if (v2 <= 1) {
			if (v2) {
				v1 = 60;
			} else {
				v1 = 59;
			}
		}*/
		Ambient_Sounds_Play_Sound(/*v1*/Random_Query(59, 60), 25, 30, 30, 0);
	}
	if (frame == 23) {
		Ambient_Sounds_Play_Sound(118, 40, 99, 0, 0);
	}
	if (frame == 316) {
		Ambient_Sounds_Play_Sound(373, 50, -50, 100, 99);
	}
	if (frame == 196 || frame == 452) {
		int v3 = Random_Query(0, 6);
		if (v3 == 0) {
			Overlay_Play("ct01spnr", 0, 0, 1, 0);
			if (Random_Query(0, 1)) {
				Ambient_Sounds_Play_Sound(68, Random_Query(33, 50), 0, 0, 0);
			} else {
				Ambient_Sounds_Play_Sound(67, Random_Query(33, 50), 0, 0, 0);
			}
		} else if (v3 == 1) {
			Overlay_Play("ct01spnr", 1, 0, 1, 0);
			if (Random_Query(0, 1)) {
				Ambient_Sounds_Play_Sound(69, Random_Query(33, 50), 0, 0, 0);
			} else {
				Ambient_Sounds_Play_Sound(66, Random_Query(33, 50), 0, 0, 0);
			}

		}
	}
}

void ScriptCT01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptCT01::PlayerWalkedIn() {
	if (Game_Flag_Query(234)) {
		Loop_Actor_Walk_To_XYZ(0, -327.2f, -6.5f, 352.28f, 0, 0, false, 0);
		Game_Flag_Reset(234);
	} else {
		if (!Game_Flag_Query(247)) {
			Game_Flag_Reset(247);
		}
		Loop_Actor_Walk_To_XYZ(0, -330.0f, -6.5f, 221.0f, 0, 0, false, 0);
		Loop_Actor_Walk_To_XYZ(0, -314.0f, -6.5f, 326.0f, 0, 0, false, 0);
		if (!Game_Flag_Query(25)) {
			Game_Flag_Set(25);
			if (!Game_Flag_Query(378)) {
				Actor_Voice_Over(200, 99);
				Actor_Voice_Over(210, 99);
				Actor_Voice_Over(220, 99);
			}
		}
	}
}

void ScriptCT01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	if (Game_Flag_Query(123)) {
		Ambient_Sounds_Remove_Looping_Sound(55, true);
		Ambient_Sounds_Remove_Looping_Sound(56, true);
	} else {
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
	}
	Music_Stop(5);
	if (!Game_Flag_Query(176) && Global_Variable_Query(1)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(38, 1, -1);
	}
}

void ScriptCT01::DialogueQueueFlushed(int a1) {
}

void ScriptCT01::sub_40269C() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 13)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(40, 4, 5, 6);
	}
	if ((Actor_Clue_Query(0, 8) || Actor_Clue_Query(0, 9)) && !Game_Flag_Query(27)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(50, 5, 5, 4);
	}
	if (Actor_Clue_Query(0, 8) && Actor_Clue_Query(0, 9) && Game_Flag_Query(27) && !Game_Flag_Query(28)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(60, 3, 5, 5);
	}
	if (Game_Flag_Query(293)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(80, 9, 9, 9);
	} else if (Game_Flag_Query(29)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(80, 3, 4, 8);
	}
	if (Actor_Clue_Query(0, 30) && Actor_Clue_Query(0, 40) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(90, 5, 4, 5);
	}
	DM_Add_To_List_Never_Repeat_Once_Selected(70, 7, 3, -1);
	Dialogue_Menu_Add_DONE_To_List(100);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 40:
		Actor_Says(0, 265, 11);
		Actor_Says(28, 20, 14);
		if (Actor_Query_Is_In_Current_Set(19)) {
			if (!Actor_Query_Goal_Number(19)) {
				Actor_Face_Actor(28, 19, true);
				Actor_Says(28, 120, 14);
				Actor_Face_Actor(19, 28, true);
				Actor_Says(19, 40, 18);
				Actor_Face_Heading(19, 103, false);
				Actor_Face_Actor(28, 0, true);
				Actor_Modify_Friendliness_To_Other(19, 0, -2);
				if (Actor_Query_Is_In_Current_Set(2)) {
					Actor_Modify_Friendliness_To_Other(2, 0, -3);
					Actor_Clue_Acquire(2, 213, 1, 0);
				}
			}
		}
		break;
	case 50:
		if (Actor_Clue_Query(0, 8) == 1) {
			Actor_Says(0, 270, 11);
			Actor_Says(28, 30, 16);
		} else {
			Actor_Says(0, 280, 11);
			Actor_Says(28, 40, 14);
		}
		Game_Flag_Set(27);
		break;
	case 60:
		if (Actor_Clue_Query(0, 9) == 1) {
			Actor_Says(0, 270, 11);
			Actor_Says(28, 40, 15);
		} else {
			Actor_Says(0, 270, 11);
			Actor_Says(28, 30, 14);
		}
		Actor_Modify_Friendliness_To_Other(28, 0, 5);
		Game_Flag_Set(28);
		break;
	case 70:
		Actor_Says(0, 290, 13);
		if (Actor_Query_Friendliness_To_Other(28, 0) > 49 && (Global_Variable_Query(2) > 10 || Query_Difficulty_Level() == 0)) {
			Actor_Says(28, 50, 3);
			Actor_Says(28, 60, 3);
			Actor_Face_Actor(28, 0, true);
			Actor_Says(28, 70, 16);
			Actor_Says(0, 325, 13);
			if (Query_Difficulty_Level() != 0) {
				Global_Variable_Decrement(2, 10);
			}
			Game_Flag_Set(192);
		} else {
			Actor_Says(28, 130, 15);
		}
		break;
	case 80:
		Actor_Says(0, 295, 11);
		Actor_Says(28, 90, 14);
		Actor_Says(28, 100, 13);
		Actor_Clue_Acquire(0, 25, 1, 28);
		Actor_Modify_Friendliness_To_Other(28, 0, -3);
		break;
	case 90:
		Actor_Says(0, 300, 13);
		Actor_Says(28, 110, 16);
		break;
	case 100:
		Actor_Says(0, 305, 18);
		break;
	}
}

} // End of namespace BladeRunner
