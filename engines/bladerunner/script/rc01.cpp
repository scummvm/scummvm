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

#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"

#include "common/debug.h"

namespace BladeRunner {

void ScriptRC01::InitializeScene() {
	// Game_Flag_Set(24);
	if (!Game_Flag_Query(24)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(28, 1); // WSTLGO_E.VQA
		Outtake_Play(41, 1); // BRLOGO_E.VQA
		Outtake_Play( 0, 0); // INTRO_E.VQA
		Outtake_Play(33, 1); // DSCENT_E.VQA
	}

	// Game_Flag_Set(9); // Force flag 9 so McCoy will be in view
	if (Game_Flag_Query(9)) {
		Setup_Scene_Information(-171.16,  5.55,  27.28, 616);
	} else if (Game_Flag_Query(114)) {
		Setup_Scene_Information(-471.98, -0.30, 258.15, 616);
	} else {
		Setup_Scene_Information( -10.98, -0.30, 318.15, 616);
	}
	// Setup_Scene_Information(-151.98, -0.30, 318.15, 616);

	Scene_Exit_Add_2D_Exit(0, 314, 145, 340, 255, 0);
	if (Game_Flag_Query(249))
		Scene_Exit_Add_2D_Exit(1, 482, 226, 639, 280, 2);
	if (Global_Variable_Query(1) > 1 && Game_Flag_Query(710))
		Scene_Exit_Add_2D_Exit(2, 0, 0, 10, 479, 3);
	if (!Game_Flag_Query(186))
		Scene_2D_Region_Add(0, 0, 294, 296, 479);

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
	Ambient_Sounds_Add_Sound(82, 5, 30, 30, 50, -100, 100, -101, -101, 0, 0);  // RCCARBY1.AUD
	Ambient_Sounds_Add_Sound(83, 5, 30, 30, 55, -100, 100, -101, -101, 0, 0);  // RCCARBY2.AUD
	Ambient_Sounds_Add_Sound(84, 5, 30, 30, 50, -100, 100, -101, -101, 0, 0);  // RCCARBY3.AUD
	Ambient_Sounds_Add_Sound(67, 10, 50, 30, 50, -100, 100, -101, -101, 0, 0); // SPIN2A.AUD
	Ambient_Sounds_Add_Sound(87, 20, 80, 20, 40, -100, 100, -101, -101, 0, 0); // SIREN2.AUD

	if (Game_Flag_Query(186)) {
		if (!Game_Flag_Query(9) && !Game_Flag_Query(114))
			Scene_Loop_Start_Special(0, 5, 0);
		if (Game_Flag_Query(249))
			Scene_Loop_Set_Default(6);
		else
			Scene_Loop_Set_Default(10);
	} else {
		if (!Game_Flag_Query(9) && !Game_Flag_Query(114))
			Scene_Loop_Start_Special(0, 0, 0);
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

void ScriptRC01::SceneLoaded() {
	Obstacle_Object("HYDRANT02", 1);
	Obstacle_Object("PARKING METER 04", 1);
	Obstacle_Object("CHEVY PROP", 1);
	Obstacle_Object("PARKING METER 01", 1);
	Obstacle_Object("T-CAN01", 1);
	Obstacle_Object("BARICADE01", 1);
	Obstacle_Object("BARICADE02", 1);
	Obstacle_Object("DOOR LEFT", 1);
	Unobstacle_Object("BOX06", 1);
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
	Unobstacle_Object("DOORWAY01", 1);

	if (Game_Flag_Query(186)) {
		Unclickable_Object("70_1");
		Unclickable_Object("70_2");
		Unclickable_Object("70_3");
		Unclickable_Object("70_5");
		Unclickable_Object("70_6");
		Unclickable_Object("BARICADE01");
		Unclickable_Object("BARICADE03");
		Unclickable_Object("BARICADE04");
		Unobstacle_Object("70_1", 1);
		Unobstacle_Object("70_2", 1);
		Unobstacle_Object("70_3", 1);
		Unobstacle_Object("70_5", 1);
		Unobstacle_Object("70_6", 1);
		Unobstacle_Object("BARICADE01", 1);
		Unobstacle_Object("BARICADE02", 1);
		Unobstacle_Object("BARICADE03", 1);
		Unobstacle_Object("BARICADE04", 1);
		Unobstacle_Object("BARICADE05", 1);
	}

	if (!Game_Flag_Query(186)) {
		Preload(13);
		Preload(14);
		Preload(19);
		Preload(582);
		Preload(589);
	}

	/*
	if (!Game_Flag_Query(163))
		Item_Add_To_World(66, 938, 69, -148.60f, -0.30f, 225.15f, 256, 24, 24, 0, 1, 0, 1);
	*/

	if (!Game_Flag_Query(24)) {
		// ADQ_Flush();
		Actor_Voice_Over(1830, 99);
		Actor_Voice_Over(1850, 99);
		if (!Game_Flag_Query(378)) {
			Actor_Voice_Over(1860, 99);
			I_Sez("MG: Is David Leary a self-respecting human or is he powered by rechargeable");
			I_Sez("batteries?\n");
		}
		Game_Flag_Set(24);
	}
}

void ScriptRC01::sub_403850()
{
	if (Game_Flag_Query(186))
		return;

	if (Loop_Actor_Walk_To_Scene_Object(0, "BARICADE03", 36, 1, 0))
		return;

	Actor_Set_Goal_Number(23, 0);
	Actor_Face_Object(0, "BARICADE03", 1);
	// Loop_Actor_Walk_To_Actor(23, 0, 36, 1, 0);
	Actor_Face_Actor(23, 0, 1);
	Actor_Says(0, 4500, 14);
	I_Sez("MG: We don't want any of that abstract art oozing out onto the street.");
	Actor_Says(23, 10, 14);
	Actor_Set_Goal_Number(23, 1);
}

bool ScriptRC01::ClickedOn3DObject(const char *objectName) {
	if (Object_Query_Click("BARICADE01", objectName)
	 || Object_Query_Click("BARICADE03", objectName)
	 || Object_Query_Click("BARICADE04", objectName)
	 || Object_Query_Click("70_1", objectName)
	 || Object_Query_Click("70_2", objectName)
	 || Object_Query_Click("70_3", objectName)
	 || Object_Query_Click("70_5", objectName)
	 || Object_Query_Click("70_6", objectName))
	{
		sub_403850();
		return true;
	}

	if (Object_Query_Click("HYDRANT02", objectName)) {
		if (Loop_Actor_Walk_To_Scene_Object(0, "HYDRANT02", 60, 1, 0) == 0) {
			if (Actor_Clue_Query(0, 26)) {
				Actor_Says(0, 6975, 3);
			} else {
				Actor_Face_Object(0, "HYDRANT02", 1);
				Actor_Voice_Over(1880, 99);
				Actor_Voice_Over(1890, 99);
				I_Sez("JM: That McCoy--he's one funny guy! Jet-black fire truck, hehehehe...");
				Actor_Clue_Acquire(0, 26, 1, -1);
			}
		}
		return true;
	}

	if (Object_Query_Click("DOOR LEFT", objectName))
	{
		if (!Loop_Actor_Walk_To_Scene_Object(0, "DOOR LEFT", 48, 1, 0))
		{
			Actor_Face_Object(0, "DOOR LEFT", 1);
			if (Actor_Clue_Query(0, 2) || !Actor_Query_In_Set(23, 69) || Global_Variable_Query(1) != 1)
			{
				Actor_Says(0, 8570, 14);
			}
			else
			{
				Actor_Set_Goal_Number(23, 0);
				Actor_Face_Actor(23, 0, 1);
				Actor_Says(23, 0, 12);
				Actor_Says(0, 4495, 13);
				Actor_Clue_Acquire(0, 2, 1, 23);
			}
			Actor_Clue_Acquire(0, 1, 1, -1);
		}
		return true;
	}

	if (Object_Query_Click("T-CAN01", objectName))
	{
		if (!Loop_Actor_Walk_To_Scene_Object(0, "T-CAN01", 24, 1, 0))
		{
			Actor_Face_Object(0, "T-CAN01", 1);
			Actor_Voice_Over(1810, 99);
			Actor_Voice_Over(1820, 99);
		}
		return true;
	}

	return false;
}

bool ScriptRC01::ClickedOn2DRegion(int region) {
	if (region == 0) {
		sub_403850();
		return 1;
	}

	return 0;
}


void ScriptRC01::SceneFrameAdvanced(int frame) {
	if (frame == 1)
		Sound_Play(118, 40, 0, 0, 50);    // CARDOWN3.AUD

	if (frame == 61 || frame == 362)
		Sound_Play(116, 100, 80, 80, 50); // SPINOPN4.AUD

	if (frame == 108 || frame == 409)
		Sound_Play(119, 100, 80, 80, 50); // SPINCLS1.AUD

	if (frame == 183 || frame == 484)
		Sound_Play(116, 100, 80, 80, 50); // SPINOPN4.AUD

	if (frame == 228 || frame == 523)
		Sound_Play(119, 100, 80, 80, 50); // SPINCLS1.AUD

	if (frame == 243 || frame == 545)
		Sound_Play(117, 40, 80, 80, 50);  // CARUP3.AUD

	if (frame == 315)
		Sound_Play(118, 40, 80, 80, 50);  // CARDOWN3.AUD
}

void ScriptRC01::SceneActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptRC01::PlayerWalkedIn()
{
	if (Game_Flag_Query(249) && !Game_Flag_Query(9) && !Game_Flag_Query(114)) {
		// Extract to sub_4037AC():
		Player_Loses_Control();
		Game_Flag_Set(182);
		Actor_Set_Immunity_To_Obstacles(0, true);
		Loop_Actor_Walk_To_XYZ(0, -151.98, -0.30, 318.15, 0, 0, 0, 0);
		Actor_Set_Immunity_To_Obstacles(0, false);
		Player_Gains_Control();
	}

	if (Game_Flag_Query(114)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(0, -415.98, -0.30, 262.15, 0, 0, 0, 0);
		Player_Gains_Control();
		Game_Flag_Reset(114);
	}

	if (Game_Flag_Query(9)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(0, -203.45, 5.55, 85.05, 0, 0, 0, 0);
		Player_Gains_Control();
		Game_Flag_Reset(9);

		if (Game_Flag_Query(1) && !Game_Flag_Query(4)) {
			Actor_Voice_Over(1910, 99);
			Actor_Voice_Over(1920, 99);
			Actor_Voice_Over(1930, 99);
			Game_Flag_Set(4);
		}
	}
}

} // End of namespace BladeRunner
