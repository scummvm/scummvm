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

namespace BladeRunner {

void ScriptRC01::InitializeScene() {
	if (!Game_Flag_Query(24)) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(28, 1); // WSTLGO_E.VQA
		Outtake_Play(41, 1); // BRLOGO_E.VQA
		Outtake_Play( 0, 0); // INTRO_E.VQA
		Outtake_Play(33, 1); // DSCENT_E.VQA
	}

	Game_Flag_Set(9); // Force flag 9 so McCoy will be in view
	if (Game_Flag_Query(9)) {
		Setup_Scene_Information(-171.16,  5.55,  27.28, 616);
	} else if (Game_Flag_Query(114)) {
		Setup_Scene_Information(-471.98, -0.30, 258.15, 616);
	} else {
		Setup_Scene_Information( -10.98, -0.30, 318.15, 616);
	}

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

} // End of namespace BladeRunner
