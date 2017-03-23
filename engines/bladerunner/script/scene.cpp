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

bool SceneScript::Open(const Common::String &name) {
	delete _currentScript;

	if (name == "RC01") { _currentScript = new SceneScriptRC01(_vm); return true; }
	if (name == "RC02") { _currentScript = new SceneScriptRC02(_vm); return true; }
	if (name == "RC03") { _currentScript = new SceneScriptRC03(_vm); return true; }
	if (name == "RC04") { _currentScript = new SceneScriptRC04(_vm); return true; }
	if (name == "RC51") { _currentScript = new SceneScriptRC51(_vm); return true; }

	return false;
}

SceneScript::~SceneScript() {
	delete _currentScript;
}

void SceneScript::InitializeScene() {
	_inScriptCounter++;
	_currentScript->InitializeScene();
	_inScriptCounter--;
}

void SceneScript::SceneLoaded() {
	_inScriptCounter++;
	_currentScript->SceneLoaded();
	_inScriptCounter--;
}

bool SceneScript::MouseClick(int x, int y) {
	if (_inScriptCounter > 0)
		return true;

	_inScriptCounter++;
	//MouseX = x;
	//MouseY = y;
	bool result = _currentScript->MouseClick(x, y);
	//SelectedEntity = -1;
	_inScriptCounter--;
	//MouseX = -1;
	//MouseY = -1;
	return result;
}

bool SceneScript::ClickedOn3DObject(const char *objectName, bool a2) {
	if (_inScriptCounter > 0)
		return true;

	_inScriptCounter++;
	bool result = _currentScript->ClickedOn3DObject(objectName, a2);
	_inScriptCounter--;
	return result;
}

bool SceneScript::ClickedOnActor(int actorId) {
	if (_inScriptCounter > 0)
		return true;

	_inScriptCounter++;
	bool result = _currentScript->ClickedOnActor(actorId);
	_inScriptCounter--;
	return result;
}

bool SceneScript::ClickedOnItem(int itemId, bool a2) {
	if (_inScriptCounter > 0)
		return true;

	_inScriptCounter++;
	bool result = _currentScript->ClickedOnItem(itemId, a2);
	_inScriptCounter--;
	return result;
}

bool SceneScript::ClickedOnExit(int exitId) {
	if (_inScriptCounter > 0)
		return true;

	_inScriptCounter++;
	bool result = _currentScript->ClickedOnExit(exitId);
	_inScriptCounter--;
	return result;
}

bool SceneScript::ClickedOn2DRegion(int region) {
	if (_inScriptCounter > 0)
		return true;

	_inScriptCounter++;
	bool result = _currentScript->ClickedOn2DRegion(region);
	_inScriptCounter--;
	return result;
}

void SceneScript::SceneFrameAdvanced(int frame) {
	_inScriptCounter++;
	_currentScript->SceneFrameAdvanced(frame);
	_inScriptCounter--;
}

void SceneScript::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	_inScriptCounter++;
	//TODO remove this check
	if(_currentScript)
		_currentScript->ActorChangedGoal(actorId, newGoal, oldGoal, currentSet);
	_inScriptCounter--;
}

void SceneScript::PlayerWalkedIn() {
	_inScriptCounter++;
	_currentScript->PlayerWalkedIn();
	_inScriptCounter--;
}

void SceneScript::PlayerWalkedOut() {
	_inScriptCounter++;
	_currentScript->PlayerWalkedOut();
	_inScriptCounter--;
}

void SceneScript::DialogueQueueFlushed(int a1) {
	_inScriptCounter++;
	_currentScript->DialogueQueueFlushed(a1);
	_inScriptCounter--;
}

} // End of namespace BladeRunner
