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

void SceneScriptPS15::InitializeScene() {
	Setup_Scene_Information(-360.0f, -113.43f, 50.0f, 0);
	Scene_Exit_Add_2D_Exit(0, 0, 0, 20, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 620, 0, 639, 479, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxRUMLOOP1, 20, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxLABAMB3,  80, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxPHONE1, 5, 50,  8,  8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPAGE1,  5, 20, 30, 30,  -70,  70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPAGE2,  5, 20, 30, 30,  -70,  70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPAGE3,  5, 20, 30, 30,  -70,  70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP3, 5, 20,  5,  9,  -70,  70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP4, 5, 20,  5,  9,  -70,  70, -101, -101, 0, 0);
	Actor_Put_In_Set(kActorSergeantWalls, kSetPS15);
	Actor_Set_At_XYZ(kActorSergeantWalls, -265.4f, -113.43f, -31.29f, 623);
}

void SceneScriptPS15::SceneLoaded() {
	Obstacle_Object("E.ARCH", true);
	if (Global_Variable_Query(kVariableChapter) == 2) {
		Item_Add_To_World(kItemWeaponsCrate, kModelAnimationWeaponsCrate, kSetPS15, -208.0f, -113.43f, 30.28f, 750, 16, 12, false, true, false, true);
	}
}

bool SceneScriptPS15::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS15::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS15::ClickedOnActor(int actorId) {
	if (actorId == kActorSergeantWalls) {
		if (
#if BLADERUNNER_ORIGINAL_BUGS
		    (Actor_Clue_Query(kActorMcCoy, kClueWeaponsOrderForm)
		     || Actor_Clue_Query(kActorMcCoy, kCluePoliceIssueWeapons))
		    && !Actor_Clue_Query(kActorMcCoy, kClueShippingForm)
#else
		    (Actor_Clue_Query(kActorMcCoy, kClueShippingForm)
		     || Actor_Clue_Query(kActorMcCoy, kCluePoliceIssueWeapons))
		    && !Actor_Clue_Query(kActorMcCoy, kClueWeaponsOrderForm)
#endif // BLADERUNNER_ORIGINAL_BUGS
		) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -256.0f, -113.43f, 43.51f, 0, true, false, false)) {
				Actor_Face_Actor(kActorMcCoy, kActorSergeantWalls, true);
				Actor_Face_Actor(kActorSergeantWalls, kActorMcCoy, true);
				Actor_Says(kActorMcCoy, 4470, 17);
				Actor_Says(kActorSergeantWalls, 130, 12);
				Actor_Says(kActorMcCoy, 4475, 18);
				Actor_Says(kActorMcCoy, 4480, 13);
				Actor_Says(kActorSergeantWalls, 140, 16);
				Item_Pickup_Spin_Effect(kModelAnimationWeaponsOrderForm, 211, 239);
				Actor_Says(kActorSergeantWalls, 150, 14);
#if BLADERUNNER_ORIGINAL_BUGS
				// This code makes no sense (why remove the order form from the world,
				// when it's not added (it only gets added when kFlagPS04WeaponsOrderForm is set)
				// Also, why remove the kItemWeaponsOrderForm when McCoy acquires the kClueShippingForm from Walls?
				Actor_Clue_Acquire(kActorMcCoy, kClueShippingForm, true, kActorSergeantWalls);
				if (!Game_Flag_Query(kFlagPS04WeaponsOrderForm)) {
					Item_Remove_From_World(kItemWeaponsOrderForm);
				}
#else
				// BugFix: McCoy here gets the kClueWeaponsOrderForm form from Sgt Walls
				//         (not the shipping form)
				//         and if the order form was added to Guzza's office (PS04),
				//         then the kFlagPS04WeaponsOrderForm would be set
				//         so in that case we remove the item from the game world (ie. from PS04)
				Actor_Clue_Acquire(kActorMcCoy, kClueWeaponsOrderForm, true, kActorSergeantWalls);
				if (Game_Flag_Query(kFlagPS04WeaponsOrderForm)) {
					Item_Remove_From_World(kItemWeaponsOrderForm);
				}
#endif // BLADERUNNER_ORIGINAL_BUGS
			}
		} else {
			Actor_Face_Actor(kActorMcCoy, kActorSergeantWalls, true);
			Actor_Says(kActorMcCoy, 8600, 15);
			if (_vm->_cutContent) {
				switch (Random_Query(1, 2)) {
				case 1:
					Actor_Says(kActorSergeantWalls, 190, 12);
					break;
				case 2:
					Actor_Says(kActorSergeantWalls, 200, 12);
					break;
				}
			} else {
				Actor_Says(kActorSergeantWalls, 190, 12);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptPS15::ClickedOnItem(int itemId, bool a2) {
	if (itemId == kItemWeaponsCrate) {

		if (
#if BLADERUNNER_ORIGINAL_BUGS
			// the check here ideally should be about kClueShippingForm
			// although it suffices that we also check for kCluePoliceIssueWeapons (logic AND)
			// and the additional check for kClueWeaponsOrderForm does not affect anything
		    Actor_Clue_Query(kActorMcCoy, kClueWeaponsOrderForm)
#else
		    Actor_Clue_Query(kActorMcCoy, kClueShippingForm)
#endif // BLADERUNNER_ORIGINAL_BUGS
		    && Actor_Clue_Query(kActorMcCoy, kCluePoliceIssueWeapons)
		) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
			// McCoy should face the crate when saying "I've got all I can from that."
			Actor_Face_Item(kActorMcCoy, kItemWeaponsCrate, true);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorMcCoy, 8570, 14);
		} else {
#if BLADERUNNER_ORIGINAL_BUGS
#else
			// A form is added to McCoy's KIA from examining the crate
			// but no item pickup effect was playing in the original
			Item_Pickup_Spin_Effect(kModelAnimationWeaponsOrderForm, 411, 333);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Face_Actor(kActorMcCoy, kActorSergeantWalls, true);
			Actor_Face_Actor(kActorSergeantWalls, kActorMcCoy, true);
			Actor_Says(kActorMcCoy, 4485, 17);
			Actor_Says(kActorSergeantWalls, 160, 14);
			Actor_Says(kActorMcCoy, 4490, 12);
			Actor_Says(kActorSergeantWalls, 170, 13);
#if BLADERUNNER_ORIGINAL_BUGS
			// if the player did not get the weapons order form from Guzza's office, they get it here
			Actor_Clue_Acquire(kActorMcCoy, kClueWeaponsOrderForm,   true, kActorMcCoy);
			// A bug? Shouldn't the last argument be -1 or kActorSergeantWalls here?
			Actor_Clue_Acquire(kActorMcCoy, kCluePoliceIssueWeapons, true, kActorMcCoy);
#else
			// Bugfix: Shipping form makes more sense to be attached to the box of weapons
			//         Order form is now acquired from Walls or from Guzza's office
			if (!Actor_Clue_Query(kActorMcCoy, kClueShippingForm)) {
				// (McCoy apparently finds it attached to the weapon's shipment crate)
				// It's not given by Sgt Walls, so McCoy is credited for the clue find
				Actor_Clue_Acquire(kActorMcCoy, kClueShippingForm, true, kActorMcCoy);
			}
			Actor_Clue_Acquire(kActorMcCoy, kCluePoliceIssueWeapons, true, kActorSergeantWalls);
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		return true;
	}
	return false;
}

bool SceneScriptPS15::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -360.0f, -113.43f, 50.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagPS15toPS05);
			Set_Enter(kSetPS05, kScenePS05);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -183.58f, -113.43f, 91.7f, 0, true, false, false)) {
			Actor_Says(kActorMcCoy, 4440, 18);
			Actor_Says(kActorSergeantWalls, 150, 17);
#if BLADERUNNER_ORIGINAL_BUGS
			// Sometimes the scene transition code (or the Ambient_Sounds_Remove_All_Non_Looping_Sounds)
			// would stop this from playing (rare occasions)
			// Solution: moved into PS10 code
			Sound_Play(kSfxLABBUZZ1, 90, 0, 0, 50);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS10);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS15::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS15::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS15::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS15::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -326.93f, -113.43f, 101.42f, 0, false, false, false);
	if (!Game_Flag_Query(kFlagPS15Entered)) {
		Actor_Face_Actor(kActorMcCoy, kActorSergeantWalls, true);
		Actor_Face_Actor(kActorSergeantWalls, kActorMcCoy, true);
		Actor_Says(kActorSergeantWalls, 0, 12);
		Actor_Says(kActorMcCoy, 4445, 18);
		Actor_Says(kActorSergeantWalls, 10, 12);
		Actor_Says(kActorMcCoy, 4450, 18);
		if (_vm->_cutContent && !Game_Flag_Query(kFlagSergeantWallsMazeInstructions)) {
			// TODO: These instructions don't make much sense... Is it better to not restore this at all?
			//       Could there have been a system with a bell-tone that was removed from the original version?
			Game_Flag_Set(kFlagSergeantWallsMazeInstructions);
			Actor_Says(kActorSergeantWalls, 20, 13);
			Actor_Says(kActorSergeantWalls, 30, 12);
			Actor_Says(kActorMcCoy, 4455, 12);
			Actor_Says(kActorSergeantWalls, 40, 12);
			Actor_Says(kActorSergeantWalls, 50, 12);
		}
		Actor_Says(kActorSergeantWalls, 60, 13);
		Actor_Says(kActorSergeantWalls, 70, 12);
		Actor_Says(kActorMcCoy, 4460, 15);
		Actor_Says(kActorSergeantWalls, 80, 13);
		Actor_Says(kActorMcCoy, 4465, 16);
		Actor_Says(kActorSergeantWalls, 90, 13);
		Actor_Says(kActorSergeantWalls, 100, 14);
		Actor_Says(kActorSergeantWalls, 110, 15);
		Actor_Says(kActorSergeantWalls, 120, 15);
		Actor_Says(kActorMcCoy, 4555, 14);
		Game_Flag_Set(kFlagPS15Entered);
		//return true;
		return;
	} else {
		//return false;
		return;
	}
}

void SceneScriptPS15::PlayerWalkedOut() {
}

void SceneScriptPS15::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
