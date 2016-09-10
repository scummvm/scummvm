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

void ScriptMA08::InitializeScene() {
	Setup_Scene_Information(0, 0, 0, 0);
	Ambient_Sounds_Add_Looping_Sound(381, 100, 1, 1);
	Ambient_Sounds_Add_Sound(68, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
}

void ScriptMA08::SceneLoaded() {
	Obstacle_Object("(undefined)", true);
	Clickable_Object("(undefined)");
}

bool ScriptMA08::MouseClick(int x, int y) {
	return false;
}

bool ScriptMA08::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptMA08::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptMA08::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptMA08::ClickedOnExit(int exitId) {
	return false;
}

bool ScriptMA08::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptMA08::SceneFrameAdvanced(int frame) {
}

void ScriptMA08::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptMA08::PlayerWalkedIn() {
}

void ScriptMA08::PlayerWalkedOut() {
}

void ScriptMA08::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
