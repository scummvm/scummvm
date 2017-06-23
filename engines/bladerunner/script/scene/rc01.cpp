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

void SceneScriptRC01::InitializeScene() {
#if _DEBUG
	//TODO: not part of game, remove
	//Game_Flag_Set(24); // force skip intro
#endif

	if (!Game_Flag_Query(24)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(28, 1, -1); // WSTLGO_E.VQA
		Outtake_Play(41, 1, -1); // BRLOGO_E.VQA
		Outtake_Play(0, 0, -1); // INTRO_E.VQA
		Outtake_Play(33, 1, -1); // DSCENT_E.VQA
	}
	if (Game_Flag_Query(9)) {
		Setup_Scene_Information(-171.16f, 5.55f, 27.28f, 616);
	} else if (Game_Flag_Query(114)) {
		Setup_Scene_Information(-471.98f, -0.30f, 258.15f, 616);
	} else {
		Setup_Scene_Information(-10.98f, -0.30f, 318.15f, 616);
	}
	Scene_Exit_Add_2D_Exit(0, 314, 145, 340, 255, 0);
	if (Game_Flag_Query(249)) {
		Scene_Exit_Add_2D_Exit(1, 482, 226, 639, 280, 2);
	}
	if (Global_Variable_Query(1) > 1 && Game_Flag_Query(710)) {
		Scene_Exit_Add_2D_Exit(2, 0, 0, 10, 479, 3);
	}
	if (!Game_Flag_Query(186)) {
		Scene_2D_Region_Add(0, 0, 294, 296, 479);
	}

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(54, 30, 0, 1); // CTRAIN1.AUD

	if (!Game_Flag_Query(186)) {
		Ambient_Sounds_Add_Sound(181, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_0470R.AUD
		Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_0480R.AUD
		Ambient_Sounds_Add_Sound(183, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_0500R.AUD
		Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_0540R.AUD
		Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_0560R.AUD
		Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_0870R.AUD
		Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_0900R.AUD
		Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_0940R.AUD
		Ambient_Sounds_Add_Sound(190, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_0960R.AUD
		Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_1070R.AUD
		Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_1080R.AUD
		Ambient_Sounds_Add_Sound(193, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_1100R.AUD
		Ambient_Sounds_Add_Sound(194, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_1140R.AUD
		Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0); // 67_1160R.AUD
	}

	Ambient_Sounds_Add_Looping_Sound(81, 60, 100, 1); // RCAMBR1.AUD
	Ambient_Sounds_Add_Sound(82, 5, 30, 30, 50, -100, 100, -101, -101, 0, 0); // RCCARBY1.AUD
	Ambient_Sounds_Add_Sound(83, 5, 30, 30, 55, -100, 100, -101, -101, 0, 0); // RCCARBY2.AUD
	Ambient_Sounds_Add_Sound(84, 5, 30, 30, 50, -100, 100, -101, -101, 0, 0); // RCCARBY3.AUD
	Ambient_Sounds_Add_Sound(67, 10, 50, 30, 50, -100, 100, -101, -101, 0, 0); // SPIN2A.AUD
	Ambient_Sounds_Add_Sound(87, 20, 80, 20, 40, -100, 100, -101, -101, 0, 0); // SIREN2.AUD

	if (Game_Flag_Query(186)) {
		if (!Game_Flag_Query(9) && !Game_Flag_Query(114)) {
			Scene_Loop_Start_Special(0, 5, 0);
		}
		if (Game_Flag_Query(249)) {
			Scene_Loop_Set_Default(6);
		} else {
			Scene_Loop_Set_Default(10);
		}
	} else {
		if (!Game_Flag_Query(9) && !Game_Flag_Query(114)) {
			Scene_Loop_Start_Special(0, 0, 0);
		}
		Scene_Loop_Set_Default(1);
	}

	I_Sez("Blade Runner");
	I_Sez("");
	I_Sez("From the dark recesses of David Leary's imagination comes a game unlike any");
	I_Sez("other. Blade Runner immerses you in the underbelly of future Los Angeles. Right");
	I_Sez("from the start, the story pulls you in with graphic descriptions of a");
	I_Sez("grandmother doing the shimmy in her underwear, child molestation, brutal");
	I_Sez("cold-blooded slaying of innocent animals, vomiting on desks, staring at a");
	I_Sez("woman's ass, the list goes on. And when the game starts, the real fun begins -");
	I_Sez("shoot down-on-their-luck homeless people and toss them into a dumpster. Watch");
	I_Sez("with sadistic glee as a dog gets blown into chunky, bloody, bits by an");
	I_Sez("explosive, and even murder a shy little girl who loves you. If you think David");
	I_Sez("Leary is sick, and you like sick, this is THE game for you.");
	I_Sez("");
	I_Sez("JW: Don't forget the wasting of helpless mutated cripples in the underground.");
	I_Sez("It's such a beautiful thing!");
	I_Sez("");
	I_Sez("DL: Go ahead.  Just keep beating that snarling pit bull...ignore the foam");
	I_Sez("around his jaws. There's room on the top shelf of my fridge for at least one");
	I_Sez("more head... - Psychotic Dave");
	I_Sez("");
}

void SceneScriptRC01::SceneLoaded() {
	Obstacle_Object("HYDRANT02", true);
	Obstacle_Object("PARKING METER 04", true);
	Obstacle_Object("CHEVY PROP", true);
	Obstacle_Object("PARKING METER 01", true);
	Obstacle_Object("T-CAN01", true);
	Obstacle_Object("BARICADE01", true);
	Obstacle_Object("BARICADE02", true);
	Obstacle_Object("DOOR LEFT", true);
	Unobstacle_Object("BOX06", true);
	Clickable_Object("DOORWAY01");
	Clickable_Object("DOOR LEFT");
	Clickable_Object("HYDRANT02");
	Clickable_Object("T-CAN01");
	Clickable_Object("BARICADE01");
	Clickable_Object("70_1");
	Clickable_Object("70_2");
	Clickable_Object("70_3");
	Clickable_Object("70_5");
	Clickable_Object("70_6");
	Unclickable_Object("BARICADE02");
	Unclickable_Object("BARICADE05");
	Unclickable_Object("SPINNER BODY");
	Unclickable_Object("HORSE01");
	Unclickable_Object("DOORWAY01");
	Unobstacle_Object("DOORWAY01", true);

	if (Game_Flag_Query(186)) {
		Unclickable_Object("70_1");
		Unclickable_Object("70_2");
		Unclickable_Object("70_3");
		Unclickable_Object("70_5");
		Unclickable_Object("70_6");
		Unclickable_Object("BARICADE01");
		Unclickable_Object("BARICADE03");
		Unclickable_Object("BARICADE04");
		Unobstacle_Object("70_1", true);
		Unobstacle_Object("70_2", true);
		Unobstacle_Object("70_3", true);
		Unobstacle_Object("70_5", true);
		Unobstacle_Object("70_6", true);
		Unobstacle_Object("BARICADE01", true);
		Unobstacle_Object("BARICADE02", true);
		Unobstacle_Object("BARICADE03", true);
		Unobstacle_Object("BARICADE04", true);
		Unobstacle_Object("BARICADE05", true);
	}

	if (!Game_Flag_Query(186)) {
		Preload(13);
		Preload(14);
		Preload(19);
		Preload(582);
		Preload(589);
	}

	if (!Game_Flag_Query(163)) {
		Item_Add_To_World(66, 938, 69, -148.60f, -0.30f, 225.15f, 256, 24, 24, false, true, false, true);
	}

	if (!Game_Flag_Query(24)) {
		ADQ_Flush();
		Actor_Voice_Over(1830, kActorVoiceOver);
		Actor_Voice_Over(1850, kActorVoiceOver);
		if (!Game_Flag_Query(378)) {
			Actor_Voice_Over(1860, kActorVoiceOver);
			I_Sez("MG: Is David Leary a self-respecting human or is he powered by rechargeable");
			I_Sez("batteries?\n");
		}
		Game_Flag_Set(24);
	}
}

bool SceneScriptRC01::MouseClick(int x, int y) {
	return y >= 430;
}

bool SceneScriptRC01::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("BARICADE01", objectName)
		|| Object_Query_Click("BARICADE03", objectName)
		|| Object_Query_Click("BARICADE04", objectName)
		|| Object_Query_Click("70_1", objectName)
		|| Object_Query_Click("70_2", objectName)
		|| Object_Query_Click("70_3", objectName)
		|| Object_Query_Click("70_5", objectName)
		|| Object_Query_Click("70_6", objectName)) {
		sub_403850();
		return true;
	}

	if (Object_Query_Click("HYDRANT02", objectName)) {
		if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "HYDRANT02", 60, true, false)) {
			if (Actor_Clue_Query(kActorMcCoy, kCluePaintTransfer)) {
				Actor_Says(kActorMcCoy, 6975, 3);
			} else {
				Actor_Face_Object(kActorMcCoy, "HYDRANT02", true);
				Actor_Voice_Over(1880, kActorVoiceOver);
				Actor_Voice_Over(1890, kActorVoiceOver);
				I_Sez("JM: That McCoy--he's one funny guy! Jet-black fire truck, hehehehe...");
				Actor_Clue_Acquire(kActorMcCoy, kCluePaintTransfer, true, -1);
			}
		}
		return true;
	}

	if (Object_Query_Click("DOOR LEFT", objectName)) {
		if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "DOOR LEFT", 48, true, false)) {
			Actor_Face_Object(kActorMcCoy, "DOOR LEFT", true);
			if (!Actor_Clue_Query(kActorMcCoy, kClueDoorForced2) && Actor_Query_In_Set(kActorOfficerLeary, 69) && Global_Variable_Query(1)) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 0);
				Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
				Actor_Says(kActorOfficerLeary, 0, 12);
				Actor_Says(kActorMcCoy, 4495, 13);
				Actor_Clue_Acquire(kActorMcCoy, kClueDoorForced2, true, kActorOfficerLeary);
			} else {
				Actor_Says(kActorMcCoy, 8570, 14);
			}
			Actor_Clue_Acquire(kActorMcCoy, kClueDoorForced1, true, -1);
		}
		return true;
	}

	if (Object_Query_Click("T-CAN01", objectName)) {
		if (!Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "T-CAN01", 24, true, false)) {
			Actor_Face_Object(kActorMcCoy, "T-CAN01", true);
			Actor_Voice_Over(1810, kActorVoiceOver);
			Actor_Voice_Over(1820, kActorVoiceOver);
		}
		return true;
	}

	return false;
}

bool SceneScriptRC01::ClickedOnActor(int actorId) {
	if (actorId == kActorOfficerLeary && Global_Variable_Query(1) == 1) {
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorOfficerLeary, 36, 1, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorOfficerLeary, true);
			Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
			if (Actor_Query_Goal_Number(kActorOfficerLeary) == 1) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 0);
			}
			if (Game_Flag_Query(3)) {
				Actor_Says(kActorMcCoy, 4535, 13);
				Game_Flag_Set(392);
				if (Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewA) && !Actor_Clue_Query(kActorMcCoy, kClueCrowdInterviewA)) {
					Actor_Face_Object(kActorOfficerLeary, "70_1", true);
					Actor_Says(kActorOfficerLeary, 100, 15);
					Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
					Actor_Clue_Acquire(kActorMcCoy, kClueCrowdInterviewA, true, kActorOfficerLeary);
					Game_Flag_Reset(392);
				} else if (Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewB) && !Actor_Clue_Query(kActorMcCoy, kClueCrowdInterviewB)) {
					Actor_Face_Object(kActorOfficerLeary, "70_5", true);
					Actor_Says(kActorOfficerLeary, 120, 19);
					Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
					Actor_Says(kActorOfficerLeary, 130, 14);
					I_Sez("JM: Did it have a huge, ugly piece of chrome on it?");
					Actor_Clue_Acquire(kActorMcCoy, kClueCrowdInterviewB, true, kActorOfficerLeary);
					Game_Flag_Reset(392);
				} else {
					Actor_Says(kActorOfficerLeary, 90, 16);
					I_Sez("JM: This officer has a talent for vivid metaphors.");
					if (!Game_Flag_Query(397)) {
						I_Sez("DL: What is that supposed to mean?  I didn't write this line...");
						Actor_Says(kActorMcCoy, 4540, 16);
						Game_Flag_Set(397);
					}
					Game_Flag_Reset(392);
				}
			} else {
				I_Sez("MG: Hey, leave that officer alone. Can't you see he's busy?");
				I_Sez("JM: (...mmm, donuts...)");
				Game_Flag_Set(3);
				Actor_Clue_Acquire(kActorMcCoy, kClueOfficersStatement, true, kActorOfficerLeary);
				Actor_Says(kActorMcCoy, 4515, 13);
				Game_Flag_Set(392);
				Actor_Says(kActorOfficerLeary, 40, 13);
				if (!Game_Flag_Query(1)) {
					Actor_Says(kActorOfficerLeary, 50, 14);
					Actor_Says(kActorOfficerLeary, 60, 15);
					I_Sez("MG: It's all fun and games until someone loses a tiger cub.");
					Actor_Says(kActorMcCoy, 4520, 18);
					Actor_Says(kActorOfficerLeary, 70, 16);
					Actor_Says(kActorMcCoy, 4525, 14);
					Actor_Says(kActorOfficerLeary, 80, 18);
					Actor_Says(kActorMcCoy, 4530, 15);
				}
				Game_Flag_Reset(392);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptRC01::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 66) {
		Actor_Set_Goal_Number(kActorOfficerLeary, 0);
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, 66, 36, 1, false)) {
			Actor_Face_Item(kActorMcCoy, 66, true);
			Actor_Clue_Acquire(kActorMcCoy, kClueChromeDebris, true, -1);
			Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
			Actor_Says(kActorOfficerLeary, 20, 12);
			Game_Flag_Set(163);
			Item_Remove_From_World(66);
			Item_Pickup_Spin_Effect(938, 426, 316);
			I_Sez("JM: Chrome...is that what that is?");
			Actor_Says(kActorMcCoy, 4505, 13);
			ADQ_Flush();
			ADQ_Add(kActorOfficerLeary, 30, -1);
			ADQ_Add(kActorMcCoy, 4510, 13);
			I_Sez("JM: It's hard to imagine that thing on either a car or a horse.");
			I_Sez("MG: McCoy! What a witty chap...");
			I_Sez("JM: He keeps me chuckling non-stop!\n");
			Loop_Actor_Walk_To_Actor(kActorOfficerLeary, kActorMcCoy, 36, 0, false);
		}
		return true;
	}
	return false;

}

void SceneScriptRC01::sub_4037AC() {
	Player_Loses_Control();
	Game_Flag_Set(182);
	Actor_Set_Immunity_To_Obstacles(kActorMcCoy, true);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -151.98f, -0.3f, 318.15f, 0, 0, false, 0);
	Actor_Set_Immunity_To_Obstacles(kActorMcCoy, false);
	Player_Gains_Control();
}

bool SceneScriptRC01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -174.77f, 5.55f, 25.95f, 12, 1, false, 0)) {
			if (Game_Flag_Query(705)) {
				Actor_Says(kActorMcCoy, 8522, 14);
			} else {
				switch (Global_Variable_Query(1)) {
				case 1:
				case 4:
					Game_Flag_Set(8);
					Set_Enter(16, 79);
					break;
				case 2:
				case 3:
				case 5:
					Actor_Says(kActorMcCoy, 8522, 12);
					break;
				default:
					return true;
				}
			}
		}
		return true;
	}
	if (exitId == 1) {
		if (Game_Flag_Query(486)) {
			Spinner_Set_Selectable_Destination_Flag(6, 1);
		}
		I_Sez("MG: Leaving already?  The fun is just beginning!");
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -151.98f, -0.3f, 318.15f, 0, 1, false, 0)) {
			Player_Loses_Control();
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, true);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -10.98f, -0.3f, 318.15f, 0, 0, false, 0);
			if (Game_Flag_Query(486) && !Game_Flag_Query(660)) {
				Actor_Voice_Over(4310, kActorVoiceOver);
				Actor_Voice_Over(4320, kActorVoiceOver);
				Actor_Voice_Over(4330, kActorVoiceOver);
				Actor_Voice_Over(4340, kActorVoiceOver);
				Actor_Voice_Over(4350, kActorVoiceOver);
				Game_Flag_Set(660);
			}
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, false);
			Player_Gains_Control();
			Game_Flag_Reset(176);
			Game_Flag_Reset(182);
			Game_Flag_Reset(179);
			Game_Flag_Reset(178);
			int spinnerDest;
			if (Game_Flag_Query(186)) {
				spinnerDest = Spinner_Interface_Choose_Dest(8, 1);
			} else {
				spinnerDest = Spinner_Interface_Choose_Dest(3, 1);
			}
			if (spinnerDest) {
				switch (spinnerDest) {
				case 1:
					Game_Flag_Set(179);
					Game_Flag_Reset(249);
					Game_Flag_Set(250);
					Set_Enter(49, 48);
					if (Game_Flag_Query(186)) {
						Scene_Loop_Start_Special(1, 9, 1);
					} else {
						Scene_Loop_Start_Special(1, 4, 1);
					}
					break;
				case 3:
					Game_Flag_Set(176);
					Game_Flag_Reset(249);
					Game_Flag_Set(248);
					Set_Enter(4, 13);
					if (Game_Flag_Query(186)) {
						Scene_Loop_Start_Special(1, 9, 1);
					} else {
						Scene_Loop_Start_Special(1, 4, 1);
					}
					break;
				case 5:
					Game_Flag_Set(261);
					Game_Flag_Reset(249);
					Game_Flag_Set(307);
					Set_Enter(17, 82);
					if (Game_Flag_Query(186)) {
						Scene_Loop_Start_Special(1, 9, 1);
					} else {
						Scene_Loop_Start_Special(1, 4, 1);
					}
					break;
				case 4:
					Game_Flag_Set(180);
					Game_Flag_Reset(249);
					Game_Flag_Set(252);
					Set_Enter(0, 0);
					if (Game_Flag_Query(186)) {
						Scene_Loop_Start_Special(1, 9, 1);
					} else {
						Scene_Loop_Start_Special(1, 4, 1);
					}
					break;
				case 6:
					Game_Flag_Set(177);
					Game_Flag_Reset(249);
					Game_Flag_Set(253);
					Set_Enter(7, 25);
					if (Game_Flag_Query(186)) {
						Scene_Loop_Start_Special(1, 9, 1);
					} else {
						Scene_Loop_Start_Special(1, 4, 1);
					}
					break;
				case 7:
					Game_Flag_Set(258);
					Game_Flag_Reset(249);
					Game_Flag_Set(254);
					Set_Enter(20, 2);
					if (Game_Flag_Query(186)) {
						Scene_Loop_Start_Special(1, 9, 1);
					} else {
						Scene_Loop_Start_Special(1, 4, 1);
					}
					break;
				case 8:
					Game_Flag_Set(181);
					Game_Flag_Reset(249);
					Game_Flag_Set(255);
					Set_Enter(54, 54);
					if (Game_Flag_Query(186)) {
						Scene_Loop_Start_Special(1, 9, 1);
					} else {
						Scene_Loop_Start_Special(1, 4, 1);
					}
					break;
				case 9:
					Game_Flag_Set(257);
					Game_Flag_Reset(249);
					Game_Flag_Set(256);
					Set_Enter(37, 34);
					if (Game_Flag_Query(186)) {
						Scene_Loop_Start_Special(1, 9, 1);
					} else {
						Scene_Loop_Start_Special(1, 4, 1);
					}
					break;
				default:
					sub_4037AC();
					break;
				}
			} else {
				Game_Flag_Set(178);
				Game_Flag_Reset(249);
				Game_Flag_Set(251);
				Set_Enter(61, 65);
				if (Game_Flag_Query(186)) {
					Scene_Loop_Start_Special(1, 9, 1);
				} else {
					Scene_Loop_Start_Special(1, 4, 1);
				}
			}
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -471.98f, -0.3f, 258.15f, 4, 1, false, 0)) {
			Game_Flag_Set(115);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(70, 80);
		}
		return true;
	}
	return false;
}

void SceneScriptRC01::sub_403850() {
	if (!Game_Flag_Query(186) && !Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "BARICADE03", 36, true, false)) {
		Actor_Set_Goal_Number(kActorOfficerLeary, 0);
		Actor_Face_Object(kActorMcCoy, "BARICADE03", true);
		Loop_Actor_Walk_To_Actor(kActorOfficerLeary, kActorMcCoy, 36, 1, false);
		Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
		Actor_Says(kActorMcCoy, 4500, 14);
		I_Sez("MG: We don't want any of that abstract art oozing out onto the street.");
		Actor_Says(kActorOfficerLeary, 10, 14);
		Actor_Set_Goal_Number(kActorOfficerLeary, 1);
	}
}

bool SceneScriptRC01::ClickedOn2DRegion(int region) {
	if (region == 0) {
		sub_403850();
		return true;
	}
	return false;
}

void SceneScriptRC01::SceneFrameAdvanced(int frame) {
	if (frame == 1) {
		Sound_Play(118, 40, 0, 0, 50); // CARDOWN3.AUD
	}
	if (frame == 61 || frame == 362) {
		Sound_Play(116, 100, 80, 80, 50); // SPINOPN4.AUD
	}
	if (frame == 108 || frame == 409) {
		Sound_Play(119, 100, 80, 80, 50); // SPINCLS1.AUD
	}
	if (frame == 183 || frame == 484) {
		Sound_Play(116, 100, 80, 80, 50); // SPINOPN4.AUD
	}
	if (frame == 228 || frame == 523) {
		Sound_Play(119, 100, 80, 80, 50); // SPINCLS1.AUD
	}
	if (frame == 243 || frame == 545) {
		Sound_Play(117, 40, 80, 80, 50); // CARUP3.AUD
	}
	if (frame == 315) {
		Sound_Play(118, 40, 80, 80, 50); // CARDOWN3.AUD
	}
}

void SceneScriptRC01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptRC01::PlayerWalkedIn() {
	if (Game_Flag_Query(249) && !Game_Flag_Query(9) && !Game_Flag_Query(114)) {
		sub_4037AC();
	}

	if (Game_Flag_Query(114)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -415.98f, -0.30f, 262.15f, 0, 0, false, 0);
		Player_Gains_Control();
		Game_Flag_Reset(114);
	}

	if (Game_Flag_Query(9)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -203.45f, 5.55f, 85.05f, 0, 0, false, 0);
		Player_Gains_Control();
		Game_Flag_Reset(9);

		if (Game_Flag_Query(1) && !Game_Flag_Query(4)) {
			Actor_Voice_Over(1910, kActorVoiceOver);
			Actor_Voice_Over(1920, kActorVoiceOver);
			Actor_Voice_Over(1930, kActorVoiceOver);
			Game_Flag_Set(4);
		}
		//return true;
	}
	//return false;
}

void SceneScriptRC01::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(8) && !Game_Flag_Query(115) && Global_Variable_Query(1)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(31, 1, -1);
	}
	//	return 1;
}

void SceneScriptRC01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
