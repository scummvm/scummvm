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

	if (name == "AR01") { _currentScript = new SceneScriptAR01(_vm); return true; }
	if (name == "AR02") { _currentScript = new SceneScriptAR02(_vm); return true; }
	if (name == "BB01") { _currentScript = new SceneScriptBB01(_vm); return true; }
	if (name == "BB02") { _currentScript = new SceneScriptBB02(_vm); return true; }
	if (name == "BB03") { _currentScript = new SceneScriptBB03(_vm); return true; }
	if (name == "BB04") { _currentScript = new SceneScriptBB04(_vm); return true; }
	if (name == "BB05") { _currentScript = new SceneScriptBB05(_vm); return true; }
	if (name == "BB06") { _currentScript = new SceneScriptBB06(_vm); return true; }
	if (name == "BB07") { _currentScript = new SceneScriptBB07(_vm); return true; }
	if (name == "BB08") { _currentScript = new SceneScriptBB08(_vm); return true; }
	if (name == "BB09") { _currentScript = new SceneScriptBB09(_vm); return true; }
	if (name == "BB10") { _currentScript = new SceneScriptBB10(_vm); return true; }
	if (name == "BB11") { _currentScript = new SceneScriptBB11(_vm); return true; }
	if (name == "BB12") { _currentScript = new SceneScriptBB12(_vm); return true; }
	if (name == "BB51") { _currentScript = new SceneScriptBB51(_vm); return true; }
	if (name == "CT01") { _currentScript = new SceneScriptCT01(_vm); return true; }
	if (name == "CT02") { _currentScript = new SceneScriptCT02(_vm); return true; }
	if (name == "CT03") { _currentScript = new SceneScriptCT03(_vm); return true; }
	if (name == "CT04") { _currentScript = new SceneScriptCT04(_vm); return true; }
	if (name == "CT05") { _currentScript = new SceneScriptCT05(_vm); return true; }
	if (name == "CT06") { _currentScript = new SceneScriptCT06(_vm); return true; }
	if (name == "CT07") { _currentScript = new SceneScriptCT07(_vm); return true; }
	if (name == "CT08") { _currentScript = new SceneScriptCT08(_vm); return true; }
	if (name == "CT09") { _currentScript = new SceneScriptCT09(_vm); return true; }
	if (name == "CT10") { _currentScript = new SceneScriptCT10(_vm); return true; }
	if (name == "CT11") { _currentScript = new SceneScriptCT11(_vm); return true; }
	if (name == "CT12") { _currentScript = new SceneScriptCT12(_vm); return true; }
	if (name == "CT51") { _currentScript = new SceneScriptCT51(_vm); return true; }
	if (name == "DR01") { _currentScript = new SceneScriptDR01(_vm); return true; }
	if (name == "DR02") { _currentScript = new SceneScriptDR02(_vm); return true; }
	if (name == "DR03") { _currentScript = new SceneScriptDR03(_vm); return true; }
	if (name == "DR04") { _currentScript = new SceneScriptDR04(_vm); return true; }
	if (name == "DR05") { _currentScript = new SceneScriptDR05(_vm); return true; }
	if (name == "DR06") { _currentScript = new SceneScriptDR06(_vm); return true; }
	if (name == "HC01") { _currentScript = new SceneScriptHC01(_vm); return true; }
	if (name == "HC02") { _currentScript = new SceneScriptHC02(_vm); return true; }
	if (name == "HC03") { _currentScript = new SceneScriptHC03(_vm); return true; }
	if (name == "HC04") { _currentScript = new SceneScriptHC04(_vm); return true; }
	if (name == "HF01") { _currentScript = new SceneScriptHF01(_vm); return true; }
	if (name == "HF02") { _currentScript = new SceneScriptHF02(_vm); return true; }
	if (name == "HF03") { _currentScript = new SceneScriptHF03(_vm); return true; }
	if (name == "HF04") { _currentScript = new SceneScriptHF04(_vm); return true; }
	if (name == "HF05") { _currentScript = new SceneScriptHF05(_vm); return true; }
	if (name == "HF06") { _currentScript = new SceneScriptHF06(_vm); return true; }
	if (name == "HF07") { _currentScript = new SceneScriptHF07(_vm); return true; }
	if (name == "KP01") { _currentScript = new SceneScriptKP01(_vm); return true; }
	if (name == "KP02") { _currentScript = new SceneScriptKP02(_vm); return true; }
	if (name == "KP03") { _currentScript = new SceneScriptKP03(_vm); return true; }
	if (name == "KP04") { _currentScript = new SceneScriptKP04(_vm); return true; }
	if (name == "KP05") { _currentScript = new SceneScriptKP05(_vm); return true; }
	if (name == "KP06") { _currentScript = new SceneScriptKP06(_vm); return true; }
	if (name == "KP07") { _currentScript = new SceneScriptKP07(_vm); return true; }
	if (name == "MA01") { _currentScript = new SceneScriptMA01(_vm); return true; }
	if (name == "MA02") { _currentScript = new SceneScriptMA02(_vm); return true; }
	if (name == "MA04") { _currentScript = new SceneScriptMA04(_vm); return true; }
	if (name == "MA05") { _currentScript = new SceneScriptMA05(_vm); return true; }
	if (name == "MA06") { _currentScript = new SceneScriptMA06(_vm); return true; }
	if (name == "MA07") { _currentScript = new SceneScriptMA07(_vm); return true; }
	if (name == "MA08") { _currentScript = new SceneScriptMA08(_vm); return true; }
	if (name == "NR01") { _currentScript = new SceneScriptNR01(_vm); return true; }
	if (name == "NR02") { _currentScript = new SceneScriptNR02(_vm); return true; }
	if (name == "NR03") { _currentScript = new SceneScriptNR03(_vm); return true; }
	if (name == "NR04") { _currentScript = new SceneScriptNR04(_vm); return true; }
	if (name == "NR05") { _currentScript = new SceneScriptNR05(_vm); return true; }
	if (name == "NR06") { _currentScript = new SceneScriptNR06(_vm); return true; }
	if (name == "NR07") { _currentScript = new SceneScriptNR07(_vm); return true; }
	if (name == "NR08") { _currentScript = new SceneScriptNR08(_vm); return true; }
	if (name == "NR09") { _currentScript = new SceneScriptNR09(_vm); return true; }
	if (name == "NR10") { _currentScript = new SceneScriptNR10(_vm); return true; }
	if (name == "NR11") { _currentScript = new SceneScriptNR11(_vm); return true; }
	if (name == "PS01") { _currentScript = new SceneScriptPS01(_vm); return true; }
	if (name == "PS02") { _currentScript = new SceneScriptPS02(_vm); return true; }
	if (name == "PS03") { _currentScript = new SceneScriptPS03(_vm); return true; }
	if (name == "PS04") { _currentScript = new SceneScriptPS04(_vm); return true; }
	if (name == "PS05") { _currentScript = new SceneScriptPS05(_vm); return true; }
	if (name == "PS06") { _currentScript = new SceneScriptPS06(_vm); return true; }
	if (name == "PS07") { _currentScript = new SceneScriptPS07(_vm); return true; }
	if (name == "PS09") { _currentScript = new SceneScriptPS09(_vm); return true; }
	if (name == "PS10") { _currentScript = new SceneScriptPS10(_vm); return true; }
	if (name == "PS11") { _currentScript = new SceneScriptPS11(_vm); return true; }
	if (name == "PS12") { _currentScript = new SceneScriptPS12(_vm); return true; }
	if (name == "PS13") { _currentScript = new SceneScriptPS13(_vm); return true; }
	if (name == "PS14") { _currentScript = new SceneScriptPS14(_vm); return true; }
	if (name == "PS15") { _currentScript = new SceneScriptPS15(_vm); return true; }
	if (name == "RC01") { _currentScript = new SceneScriptRC01(_vm); return true; }
	if (name == "RC02") { _currentScript = new SceneScriptRC02(_vm); return true; }
	if (name == "RC03") { _currentScript = new SceneScriptRC03(_vm); return true; }
	if (name == "RC04") { _currentScript = new SceneScriptRC04(_vm); return true; }
	if (name == "RC51") { _currentScript = new SceneScriptRC51(_vm); return true; }
	if (name == "TB02") { _currentScript = new SceneScriptTB02(_vm); return true; }
	if (name == "TB03") { _currentScript = new SceneScriptTB03(_vm); return true; }
	if (name == "TB05") { _currentScript = new SceneScriptTB05(_vm); return true; }
	if (name == "TB06") { _currentScript = new SceneScriptTB06(_vm); return true; }
	if (name == "TB07") { _currentScript = new SceneScriptTB07(_vm); return true; }
	if (name == "UG01") { _currentScript = new SceneScriptUG01(_vm); return true; }
	if (name == "UG02") { _currentScript = new SceneScriptUG02(_vm); return true; }
	if (name == "UG03") { _currentScript = new SceneScriptUG03(_vm); return true; }
	if (name == "UG04") { _currentScript = new SceneScriptUG04(_vm); return true; }
	if (name == "UG05") { _currentScript = new SceneScriptUG05(_vm); return true; }
	if (name == "UG06") { _currentScript = new SceneScriptUG06(_vm); return true; }
	if (name == "UG07") { _currentScript = new SceneScriptUG07(_vm); return true; }
	if (name == "UG08") { _currentScript = new SceneScriptUG08(_vm); return true; }
	if (name == "UG09") { _currentScript = new SceneScriptUG09(_vm); return true; }
	if (name == "UG10") { _currentScript = new SceneScriptUG10(_vm); return true; }
	if (name == "UG12") { _currentScript = new SceneScriptUG12(_vm); return true; }
	if (name == "UG13") { _currentScript = new SceneScriptUG13(_vm); return true; }
	if (name == "UG14") { _currentScript = new SceneScriptUG14(_vm); return true; }
	if (name == "UG15") { _currentScript = new SceneScriptUG15(_vm); return true; }
	if (name == "UG16") { _currentScript = new SceneScriptUG16(_vm); return true; }
	if (name == "UG17") { _currentScript = new SceneScriptUG17(_vm); return true; }
	if (name == "UG18") { _currentScript = new SceneScriptUG18(_vm); return true; }
	if (name == "UG19") { _currentScript = new SceneScriptUG19(_vm); return true; }

	error("SceneScript::Open: Unknown script %s", name.c_str());

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
