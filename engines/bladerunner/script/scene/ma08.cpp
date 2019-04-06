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

void SceneScriptMA08::InitializeScene() {
	Setup_Scene_Information(0, 0, 0, 0);
	Ambient_Sounds_Add_Looping_Sound(kSfxRAIN10, 100, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
}

void SceneScriptMA08::SceneLoaded() {
	Obstacle_Object("(undefined)", true);
	Clickable_Object("(undefined)");
}

bool SceneScriptMA08::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptMA08::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptMA08::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptMA08::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptMA08::ClickedOnExit(int exitId) {
	return false;
}

bool SceneScriptMA08::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptMA08::SceneFrameAdvanced(int frame) {
}

void SceneScriptMA08::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptMA08::PlayerWalkedIn() {
}

void SceneScriptMA08::PlayerWalkedOut() {
}

void SceneScriptMA08::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
