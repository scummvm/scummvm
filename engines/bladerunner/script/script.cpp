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

#include "bladerunner/bladerunner.h"

#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/gameflags.h"
#include "bladerunner/scene.h"
#include "bladerunner/vector.h"

namespace BladeRunner {

bool Script::open(const Common::String &name) {
	delete _currentScript;

	if (name == "RC01") { _currentScript = new ScriptRC01(_vm); return true; }

	return false;
}

Script::~Script() {
	delete _currentScript;
}

void Script::InitializeScene() {
	_inScriptCounter++;
	_currentScript->InitializeScene();
	_inScriptCounter--;
}

void Script::SceneLoaded() {
	_inScriptCounter++;
	_currentScript->SceneLoaded();
	_inScriptCounter--;
}

void Script::SceneFrameAdvanced(int frame) {
	_inScriptCounter++;
	_currentScript->SceneFrameAdvanced(frame);
	_inScriptCounter--;
}

void ScriptBase::Game_Flag_Set(int flag) {
	_vm->_gameFlags->set(flag);
}

void ScriptBase::Game_Flag_Reset(int flag) {
	_vm->_gameFlags->reset(flag);
}

bool ScriptBase::Game_Flag_Query(int flag) {
	return _vm->_gameFlags->query(flag);
}

int ScriptBase::Global_Variable_Set(int var, int value) {
	return _vm->_gameVars[var] = value;
}

int ScriptBase::Global_Variable_Reset(int var) {
	return _vm->_gameVars[var] = 0;
}

int ScriptBase::Global_Variable_Query(int var) {
	return _vm->_gameVars[var];
}

int ScriptBase::Global_Variable_Increment(int var, int inc) {
	return _vm->_gameVars[var] += inc;
}

int ScriptBase::Global_Variable_Decrement(int var, int dec) {
	return _vm->_gameVars[var] -= dec;
}

void ScriptBase::Sound_Play(int id, int volume, int panFrom, int panTo, int priority) {
	const char *name = _vm->_gameInfo->getSfxTrack(id);
	_vm->_audioPlayer->playAud(name, volume, panFrom, panTo, priority);
}

void ScriptBase::Scene_Loop_Set_Default(int a) {
	debug("Scene_Loop_Set_Default(%d)", a);
}

void ScriptBase::Scene_Loop_Start_Special(int a, int b, int c) {
	debug("Scene_Loop_Start_Special(%d, %d, %d)", a, b, c);
}

void ScriptBase::Outtake_Play(int id, int noLocalization, int container) {
	_vm->outtakePlay(id, noLocalization, container);
}

void ScriptBase::Ambient_Sounds_Add_Sound(int id, int time1, int time2, int volume1, int volume2, int pan1begin, int pan1end, int pan2begin, int pan2end, int priority, int unk) {
	_vm->_ambientSounds->addSound(id, time1, time2, volume1, volume2, pan1begin, pan1end, pan2begin, pan2end, priority, unk);
}

void ScriptBase::Ambient_Sounds_Remove_All_Non_Looping_Sounds(int time) {
	// _vm->_ambientSounds->removeAllNonLoopingSounds(time);
}

void ScriptBase::Ambient_Sounds_Add_Looping_Sound(int id, int volume, int pan, int fadeInTime) {
	_vm->_ambientSounds->addLoopingSound(id, volume, pan, fadeInTime);
}

void ScriptBase::Ambient_Sounds_Remove_All_Looping_Sounds(int time) {
	// _vm->_ambientSounds->removeAllLoopingSounds(time);
}

void ScriptBase::Setup_Scene_Information(float actorX, float actorY, float actorZ, int actorFacing) {
	_vm->_scene->setActorStart(Vector3(actorX, actorY, actorZ), actorFacing);
}

void ScriptBase::Scene_Exit_Add_2D_Exit(int a, int b, int c, int d, int e, int f) {
	debug("Scene_Exit_Add_2D_Exit(%d, %d, %d, %d, %d, %d)", a, b, c, d, e, f);
}

void ScriptBase::Scene_2D_Region_Add(int a, int b, int c, int d, int e) {
	debug("Scene_2D_Region_Add(%d, %d, %d, %d, %d)", a, b, c, d, e);
}

void ScriptBase::I_Sez(const char *str) {
	_vm->ISez(str);
}


} // End of namespace BladeRunner
