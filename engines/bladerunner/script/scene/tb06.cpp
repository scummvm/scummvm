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

void SceneScriptTB06::InitializeScene() {
	Setup_Scene_Information(-16.0f, 149.0f, -466.0f, 990);
	Scene_Exit_Add_2D_Exit(0, 330, 195, 417, 334, 0);
	Ambient_Sounds_Add_Looping_Sound(kSfxTB5LOOP1, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxTB5LOOP2, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxTBLOOP1,  66, 0, 1);
	if (!Game_Flag_Query(kFlagNotUsed103)) {
		Actor_Put_In_Set(kActorMarcus, kSetTB06);
		Actor_Set_At_XYZ(kActorMarcus, 135.0f, 151.0f, -671.0f, 800);
		Actor_Retired_Here(kActorMarcus, 60, 32, 1, -1);
		//return true;
		return;
	}
	Scene_Loop_Set_Default(0);
	//return false;
}

void SceneScriptTB06::SceneLoaded() {
	Obstacle_Object("DOOR", true);
	Unobstacle_Object("GLASS01", true);
	Clickable_Object("DOOR");
	Unclickable_Object("SMUDGE_GLASS01");

	if (!Game_Flag_Query(kFlagTB06DogCollarTaken)
	 &&  Actor_Query_Goal_Number(kActorPhotographer) != 199
	) {
		Item_Add_To_World(kItemDogCollar, kModelAnimationDogCollar, kSetTB06, 36.54f, 149.48f, -565.67f, 0, 6, 6, false, true, false, true);
	}

	if (!Game_Flag_Query(kFlagTB06KitchenBoxTaken)) {
		Item_Add_To_World(kItemKitchenBox, kModelAnimationKingstonKitchenBox, kSetTB06, 18.0f, 149.65f, -599.0f, 0, 6, 6, false, true, false, true);
	}

	if (Actor_Query_Goal_Number(kActorPhotographer) != 199) {
		Item_Add_To_World(kItemDeadDogA, kModelAnimationDeadDogA, kSetTB06, -46.82f, 149.6f, -666.88f, 0, 12, 12, false, true, false, true);
		Item_Add_To_World(kItemDeadDogB, kModelAnimationDeadDogB, kSetTB06, -30.27f, 149.6f, -610.7f, 0, 15, 45, false, true, false, true);
		Item_Add_To_World(kItemDeadDogC, kModelAnimationDeadDogC, kSetTB06, 9.87f, 149.6f, -683.5f, 0, 12, 12, false, true, false, true);
	}
}

bool SceneScriptTB06::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptTB06::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptTB06::ClickedOnActor(int actorId) {
	if (actorId == kActorMarcus) {
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorMarcus, 24, true, false)) {
			if (!Actor_Clue_Query(kActorMcCoy, kClueDetonatorWire)) {
				Actor_Voice_Over(2300, kActorVoiceOver);
				Actor_Voice_Over(2310, kActorVoiceOver);
				Item_Pickup_Spin_Effect(kModelAnimationDetonatorWire, 66, 397);
				Actor_Voice_Over(2320, kActorVoiceOver);
				if (Game_Flag_Query(kFlagSadikIsReplicant)) {
					Actor_Voice_Over(2330, kActorVoiceOver);
					Actor_Voice_Over(2340, kActorVoiceOver);
				}
				Actor_Voice_Over(2350, kActorVoiceOver);
				Actor_Clue_Acquire(kActorMcCoy, kClueDetonatorWire, true, -1);
				return true;
			}
			Actor_Says(kActorMcCoy, 8665, 13);
			return false;
		}
	}
	return false;
}

bool SceneScriptTB06::ClickedOnItem(int itemId, bool a2) {
	if (itemId == kItemDogCollar) {
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemDogCollar, 12, true, false)) {
			Actor_Face_Item(kActorMcCoy, kItemDogCollar, true);
			Actor_Clue_Acquire(kActorMcCoy, kClueDogCollar1, true, -1);
			Item_Pickup_Spin_Effect(kModelAnimationDogCollar, 341, 368);
			Item_Remove_From_World(kItemDogCollar);
			Actor_Voice_Over(4160, kActorVoiceOver);
			Game_Flag_Set(kFlagTB06DogCollarTaken);
			return true;
		}
	}
	if (itemId == kItemKitchenBox) {
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemKitchenBox, 12, true, false)) {
			Actor_Face_Item(kActorMcCoy, kItemKitchenBox, true);
			Actor_Clue_Acquire(kActorMcCoy, kClueKingstonKitchenBox1, true, -1);
			Item_Remove_From_World(kItemKitchenBox);
			Item_Pickup_Spin_Effect(kModelAnimationKingstonKitchenBox, 390, 368);
			Actor_Says(kActorMcCoy, 8775, kAnimationModeTalk);
			Game_Flag_Set(kFlagTB06KitchenBoxTaken);
			return true;
		}
	}
	if (itemId == kItemChopstickWrapper) { // this item is not here, it is in RC51
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemChopstickWrapper, 12, true, false)) {
			Actor_Face_Item(kActorMcCoy, kItemChopstickWrapper, true);
			Actor_Says(kActorMcCoy, 5285, kAnimationModeTalk);
			return true;
		}
	}

	if (itemId == kItemDeadDogA
	 || itemId == kItemDeadDogB
	 || itemId == kItemDeadDogC
	) {
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemDeadDogA, 24, true, false)) {
			Actor_Face_Item(kActorMcCoy, kItemDeadDogA, true);
			Actor_Voice_Over(2380, kActorVoiceOver);
			Actor_Voice_Over(2390, kActorVoiceOver);
			Actor_Voice_Over(2400, kActorVoiceOver);
			if (_vm->_cutContent) {
				Actor_Says_With_Pause(kActorMcCoy, 8516, 0.70f, kAnimationModeTalk); // Any idea if they were real dogs? (gets no answer)
			}
			return true;
		}
	}
	return false;
}

bool SceneScriptTB06::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -16.0f, 149.0f, -427.0f, 12, true, false, false)) {
			Game_Flag_Set(kFlagTB06toTB05);
			Set_Enter(kSetTB05, kSceneTB05);
			Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 2, true);
		}
		return true;
	}
	return false;
}

bool SceneScriptTB06::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptTB06::SceneFrameAdvanced(int frame) {
	if (frame == 61) {
		Sound_Play(kSfxLABMISC5, Random_Query(52, 52), 0, 0, 50);
	}
	if (frame == 63) {
		Sound_Play(kSfxTBDOOR1,  Random_Query(55, 55), 0, 0, 50);
	}
	//return true;
}

void SceneScriptTB06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptTB06::PlayerWalkedIn() {
	if (!Game_Flag_Query(kFlagTB06Visited)
	 && !Game_Flag_Query(kFlagTB06Introduction)
	) {
		Actor_Face_Actor(kActorMcCoy, kActorMarcus, true);
		Actor_Says(kActorMcCoy, 5290, kAnimationModeTalk);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -10.0f, 149.0f, -631.0f, 0, false, false, false);

#if BLADERUNNER_ORIGINAL_BUGS
		bool talkWithPhotographer = true;
#else
		// It's possible for McCoy to go to AR01 (so the kActorPhotographer will go away from TB06)
		// without ever going into this scene (TB06)
		// so later on in the Act, if McCoy visits this scene the Photographer would be gone
		// but the dialogue would play as if he was there. This fixes that bug.
		bool talkWithPhotographer = false;
		if (Actor_Query_Is_In_Current_Set(kActorPhotographer)) {
			talkWithPhotographer = true;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		if (talkWithPhotographer) {
			AI_Movement_Track_Pause(kActorPhotographer);
			Actor_Face_Actor(kActorMcCoy, kActorPhotographer, true);
			Actor_Face_Actor(kActorPhotographer, kActorMcCoy, true);
			Actor_Says(kActorPhotographer, 0, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 5295, kAnimationModeTalk);
			Actor_Face_Actor(kActorPhotographer, kActorMarcus, true);
			Actor_Says(kActorPhotographer, 10, kAnimationModeTalk);
			AI_Movement_Track_Unpause(kActorPhotographer);
		}
		Game_Flag_Set(kFlagTB06Introduction);
		//return true;
		return;
	}
	if (Game_Flag_Query(kFlagNotUsed103)) {
		Item_Remove_From_World(kItemDogCollar);
		Item_Remove_From_World(kItemChopstickWrapper);
		Item_Remove_From_World(kItemToyDog); // why? some unused branch
		//return true;
		return;
	}
	//return false;
	return;
}

void SceneScriptTB06::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptTB06::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
