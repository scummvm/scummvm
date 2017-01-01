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

void ScriptNR01::InitializeScene() {
	if (Game_Flag_Query(617)) {
		Setup_Scene_Information(-153.86f, 23.88f, -570.21f, 402);
	} else if (Game_Flag_Query(632)) {
		Setup_Scene_Information(-416.0f, 31.93f, -841.0f, 200);
		Actor_Set_Invisible(0, true);
		Preload(167);
	} else if (Game_Flag_Query(534)) {
		Setup_Scene_Information(-416.0f, 31.93f, -841.0f, 200);
	} else if (Game_Flag_Query(342)) {
		Setup_Scene_Information(-270.0f, 4.93f, -1096.0f, 500);
	} else if (Game_Flag_Query(533)) {
		Setup_Scene_Information(312.0f, 31.66f, -901.0f, 700);
	} else if (Game_Flag_Query(545)) {
		Setup_Scene_Information(-170.0f, 24.0f, -574.0f, 768);
	} else {
		Setup_Scene_Information(76.0f, 23.88f, -109.0f, 966);
	}
	Scene_Exit_Add_2D_Exit(0, 31, 270, 97, 373, 3);
	if (Global_Variable_Query(1) > 3) {
		Scene_Exit_Add_2D_Exit(1, 201, 320, 276, 357, 2);
	}
	Scene_Exit_Add_2D_Exit(2, 583, 262, 639, 365, 1);
	if (Game_Flag_Query(255)) {
		Scene_Exit_Add_2D_Exit(3, 320, 445, 639, 479, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(54, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(362, 22, 55, 1);
	Ambient_Sounds_Add_Sound(361, 10, 10, 20, 20, -70, -70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(68, 10, 80, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 80, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 10, 80, 33, 33, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 10, 80, 33, 33, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 10, 80, 33, 33, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(643) && Actor_Query_Goal_Number(1) == 230) {
		Game_Flag_Reset(255);
		Game_Flag_Reset(256);
	}
	if (Game_Flag_Query(255) && !Game_Flag_Query(247)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
		Game_Flag_Set(247);
	} else if (Game_Flag_Query(255) && Game_Flag_Query(247)) {
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(4);
	}
}

void ScriptNR01::SceneLoaded() {
	Obstacle_Object("LAMPBASE01", true);
	Unclickable_Object("LAMPBASE01");
}

bool ScriptNR01::MouseClick(int x, int y) {
	if (Actor_Query_Goal_Number(0) == 212) {
		Global_Variable_Increment(47, 4);
		return true;
	}
	return false;
}

bool ScriptNR01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptNR01::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptNR01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptNR01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -380.0f, 31.93f, -841.0f, 0, 1, false, 0)) {
			if (Global_Variable_Query(1) > 3) {
				Actor_Says(0, 8522, 12);
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(535);
				Set_Enter(55, 56);
			}
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -281.0f, 31.93f, -1061.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 45, false);
			Loop_Actor_Travel_Stairs(0, 3, 0, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(343);
			Set_Enter(79, 91);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 312.0f, 31.66f, -901.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(532);
			Set_Enter(11, 55);
		}
		return true;
	}
	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(0, 108.0f, 23.88f, -93.0f, 0, 1, false, 0)) {
			Game_Flag_Reset(176);
			Game_Flag_Reset(182);
			Game_Flag_Reset(179);
			Game_Flag_Reset(178);
			Game_Flag_Reset(258);
			Game_Flag_Reset(257);
			Game_Flag_Reset(261);
			Game_Flag_Reset(181);
			switch (Spinner_Interface_Choose_Dest(-1, 1)) {
			case 9:
				Game_Flag_Set(257);
				Game_Flag_Reset(255);
				Game_Flag_Set(256);
				Set_Enter(37, 34);
				Scene_Loop_Start_Special(1, 3, 1);
				break;
			case 7:
				Game_Flag_Set(258);
				Game_Flag_Reset(255);
				Game_Flag_Reset(247);
				Game_Flag_Set(254);
				Set_Enter(20, 2);
				Scene_Loop_Start_Special(1, 3, 1);
				break;
			case 6:
				Game_Flag_Set(177);
				Game_Flag_Reset(255);
				Game_Flag_Reset(247);
				Game_Flag_Set(253);
				Set_Enter(7, 25);
				Scene_Loop_Start_Special(1, 3, 1);
				break;
			case 5:
				Game_Flag_Set(261);
				Game_Flag_Reset(255);
				Game_Flag_Reset(247);
				Game_Flag_Set(307);
				Set_Enter(17, 82);
				Scene_Loop_Start_Special(1, 3, 1);
				break;
			case 4:
				Game_Flag_Set(180);
				Game_Flag_Reset(255);
				Game_Flag_Reset(247);
				Game_Flag_Set(252);
				Set_Enter(0, 0);
				Scene_Loop_Start_Special(1, 3, 1);
				break;
			case 3:
				Game_Flag_Set(176);
				Game_Flag_Reset(255);
				Game_Flag_Reset(247);
				Game_Flag_Set(248);
				Set_Enter(4, 13);
				Scene_Loop_Start_Special(1, 3, 1);
				break;
			case 2:
				Game_Flag_Set(182);
				Game_Flag_Reset(255);
				Game_Flag_Reset(247);
				Game_Flag_Set(249);
				Set_Enter(69, 78);
				Scene_Loop_Start_Special(1, 3, 1);
				break;
			case 1:
				Game_Flag_Set(179);
				Game_Flag_Reset(255);
				Game_Flag_Reset(247);
				Game_Flag_Set(250);
				Set_Enter(49, 48);
				Scene_Loop_Start_Special(1, 3, 1);
				break;
			case 0:
				Game_Flag_Set(178);
				Game_Flag_Reset(255);
				Game_Flag_Reset(247);
				Game_Flag_Set(251);
				Set_Enter(61, 65);
				Scene_Loop_Start_Special(1, 3, 1);
				break;
			default:
				Player_Loses_Control();
				Game_Flag_Set(181);
				Game_Flag_Set(247);
				Player_Gains_Control();
				break;
			}
		}
		return true;
	}
	return false;
}

bool ScriptNR01::ClickedOn2DRegion(int region) {
	if (region == 0 && Player_Query_Combat_Mode()) {
		Sound_Play(517, 100, 0, 0, 50);
		Actor_Set_Goal_Number(1, 260);
		Scene_2D_Region_Remove(0);
	}
	if (region == 1 && Player_Query_Combat_Mode()) {
		Sound_Play(517, 100, 0, 0, 50);
		Actor_Set_Goal_Number(2, 299);
		Actor_Set_Goal_Number(1, 258);
		Scene_2D_Region_Remove(1);
		return true;
	}
	return false;

}

void ScriptNR01::SceneFrameAdvanced(int frame) {
	if (frame == 61) {
		Sound_Play(118, 40, 0, 0, 50);
	}
	if (frame == 184) {
		Sound_Play(117, 40, 80, 80, 50);
	}
	//return 0;
}

void ScriptNR01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptNR01::PlayerWalkedIn() {
	if (Game_Flag_Query(617)) {
		Actor_Set_Goal_Number(1, 280);
		Game_Flag_Reset(617);
		//return true;
		return;
	}
	if (Actor_Query_Goal_Number(1) == 250) {
		Scene_Exits_Disable();
		ADQ_Flush();
		Actor_Set_Goal_Number(1, 251);
		Scene_2D_Region_Add(0, 450, 316, 464, 333);
		Scene_2D_Region_Add(1, 233, 321, 240, 362);
		ADQ_Add(2, 70, 81);
		ADQ_Add(1, 990, 3);
		ADQ_Add(2, 80, 82);
		ADQ_Add(2, 90, 81);
		ADQ_Add(1, 1010, 3);
		ADQ_Add(2, 100, 81);
		ADQ_Add(1, 1020, 3);
		ADQ_Add(2, 110, 82);
		ADQ_Add(1, 1030, 3);
		ADQ_Add(1, 1040, 3);
		ADQ_Add(2, 120, 82);
	}
	if (Game_Flag_Query(604)) {
		if (Game_Flag_Query(622)) {
			ADQ_Add(25, 150, 3);
			Game_Flag_Reset(622);
		}
		Game_Flag_Reset(604);
		Player_Gains_Control();
		//return true;
		return;
	}
	if (Game_Flag_Query(632)) {
		Delay(3500);
		Set_Enter(60, 64);
		//return true;
		return;
	}
	if (Game_Flag_Query(534)) {
		Loop_Actor_Walk_To_XYZ(0, -380.0f, 31.73f, -841.0f, 0, 0, false, 0);
		Game_Flag_Reset(534);
	} else {
		if (Game_Flag_Query(342)) {
			Loop_Actor_Travel_Stairs(0, 3, 1, 0);
			Game_Flag_Reset(342);
			if (Actor_Query_Goal_Number(1) == 230) {
				Actor_Face_Actor(1, 0, true);
				Actor_Says(1, 1440, 13);
				Loop_Actor_Walk_To_Actor(0, 1, 48, 0, true);
				Actor_Says(0, 3145, 13);
				if (Global_Variable_Query(40) != 3) {
					Actor_Says(1, 1450, 12);
					Actor_Says(1, 1460, 13);
				}
				Actor_Says(0, 3150, 14);
				Actor_Says(1, 1470, 12);
				Actor_Says(1, 1480, 13);
				Actor_Says(0, 3155, 15);
				Actor_Says(1, 1500, 16);
				Actor_Says(0, 3160, 12);
				if (Game_Flag_Query(643)) {
					Actor_Says(1, 1330, 12);
					Actor_Says(1, 1340, 12);
					Actor_Says(1, 1350, 12);
					Actor_Says(0, 3120, 15);
					Actor_Says(1, 1360, 12);
					Actor_Says(1, 1370, 12);
					Actor_Says(0, 3125, 15);
					Actor_Says(1, 1380, 12);
					Actor_Says(0, 3130, 15);
					Actor_Says(1, 1390, 12);
					Actor_Says(1, 1400, 12);
					Actor_Says(1, 1410, 12);
					Actor_Says(0, 3135, 15);
					Actor_Says(1, 1420, 12);
					Actor_Says(0, 3140, 15);
					Actor_Says(1, 1430, 12);
					Actor_Set_Goal_Number(1, 285);
				} else {
					int v0 = Global_Variable_Query(40) - 1;
					if (!v0) {
						Actor_Says(1, 1510, 15);
						Actor_Says(1, 1520, 14);
						Actor_Says(1, 1530, 13);
						Actor_Says(0, 3170, 13);
						Actor_Set_Goal_Number(1, 231);
					} else if (v0 == 1) {
						Actor_Says(1, 1590, 15);
						Actor_Says(0, 3195, 14);
						Actor_Says(1, 1600, 16);
						Actor_Says(0, 3200, 13);
						Actor_Says(1, 1610, 17);
						Actor_Says(1, 1620, 15);
						Actor_Says(1, 1630, 14);
						Actor_Says(0, 3205, 12);
						Actor_Set_Goal_Number(1, 232);
					} else if (v0 == 2) {
						Actor_Says(1, 1540, 15);
						Actor_Says(0, 3175, 13);
						Actor_Says(1, 1550, 13);
						Actor_Says(1, 1560, 16);
						Actor_Says(0, 3180, 15);
						Actor_Says(1, 1570, 12);
						Actor_Says(1, 1580, 14);
						Actor_Says(0, 3190, 12);
						Actor_Set_Goal_Number(1, 233);
					}
				}
			}
		} else if (Game_Flag_Query(533)) {
			Loop_Actor_Walk_To_XYZ(0, 239.0f, 31.66f, -901.0f, 0, 0, false, 0);
			Game_Flag_Reset(533);
			if (Actor_Query_Goal_Number(2) == 230) {
				Scene_Exits_Disable();
				Actor_Set_Goal_Number(2, 231);
				Non_Player_Actor_Combat_Mode_On(2, 0, 1, 0, 3, 4, 7, 8, -1, -1, -1, 20, 300, 0);
			}
		} else if (Game_Flag_Query(545)) {
			Game_Flag_Reset(545);
			Actor_Put_In_Set(25, 54);
			Actor_Set_At_XYZ(25, -202.0f, 24.0f, -574.0f, 0);
			Actor_Face_Heading(25, 256, false);
			Actor_Set_Goal_Number(25, 204);
			Player_Gains_Control();
		} else {
			Loop_Actor_Walk_To_XYZ(0, 48.0f, 23.88f, -189.0f, 0, 0, false, 0);
		}
	}
	if (Game_Flag_Query(652)) {
		Game_Flag_Reset(652);
		Actor_Voice_Over(950, 99);
		Actor_Voice_Over(960, 99);
		Actor_Voice_Over(970, 99);
		Actor_Voice_Over(980, 99);
	}
	if (Actor_Query_Goal_Number(2) == 240) {
		Scene_Exits_Disable();
		Actor_Set_Goal_Number(2, 241);
		if (!Player_Query_Combat_Mode()) {
			Player_Set_Combat_Mode(true);
		}
	}
	//return false;
	return;
}

void ScriptNR01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(343) && !Game_Flag_Query(532) && !Game_Flag_Query(535) && !Game_Flag_Query(632) && !Game_Flag_Query(722)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(30, 1, -1);
		Outtake_Play(35, 1, -1);
	}
	Game_Flag_Reset(722);
}

void ScriptNR01::DialogueQueueFlushed(int a1) {
	if (Actor_Query_Goal_Number(1) == 251 && Actor_Query_Goal_Number(2) != 299 && Actor_Query_Goal_Number(2) != 254 && Actor_Query_Goal_Number(2) != 255) {
		Actor_Set_Goal_Number(1, 252);
	}
}

} // End of namespace BladeRunner
