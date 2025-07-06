/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "video/video_decoder.h"
#include "zvision/detection.h"
#include "zvision/zvision.h"
#include "zvision/file/save_manager.h"
#include "zvision/graphics/graphics_effect.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/graphics/render_table.h"
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/graphics/effects/fog.h"
#include "zvision/graphics/effects/light.h"
#include "zvision/graphics/effects/wave.h"
#include "zvision/scripting/menu.h"
#include "zvision/scripting/actions.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/scripting/controls/titler_control.h"
#include "zvision/scripting/effects/animation_effect.h"
#include "zvision/scripting/effects/distort_effect.h"
#include "zvision/scripting/effects/music_effect.h"
#include "zvision/scripting/effects/region_effect.h"
#include "zvision/scripting/effects/syncsound_effect.h"
#include "zvision/scripting/effects/timer_effect.h"
#include "zvision/scripting/effects/ttytext_effect.h"
#include "zvision/sound/volume_manager.h"

namespace ZVision {

ResultAction::ResultAction(ZVision *engine, int32 slotKey) :
	_engine(engine),
	_slotKey(slotKey),
	_scriptManager(engine->getScriptManager()) {
}

//////////////////////////////////////////////////////////////////////////////
// ActionAdd
//////////////////////////////////////////////////////////////////////////////

ActionAdd::ActionAdd(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_key = 0;

	char buf[64];
	memset(buf, 0, 64);
	if (sscanf(line.c_str(), "%u,%s", &_key, buf) != 2) {
		debugC(kDebugScript, "Malformed ActionAdd line: %s", line.c_str());
		return;
	}
	_value = new ValueSlot(_scriptManager, buf);
}

ActionAdd::~ActionAdd() {
	delete _value;
}

bool ActionAdd::execute() {
	_scriptManager->setStateValue(_key, _scriptManager->getStateValue(_key) + _value->getValue());
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionAssign
//////////////////////////////////////////////////////////////////////////////

ActionAssign::ActionAssign(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_key = 0;

	char buf[64];
	memset(buf, 0, 64);
	if (sscanf(line.c_str(), "%u, %s", &_key, buf) != 2) {
		debugC(kDebugScript, "Malformed ActionAssign line: %s", line.c_str());
		return;
	}
	_value = new ValueSlot(_scriptManager, buf);
}

ActionAssign::~ActionAssign() {
	delete _value;
}

bool ActionAssign::execute() {
	debugC(kDebugAssign, "[%d] = %d", _key, _value->getValue());
	_scriptManager->setStateValue(_key, _value->getValue());
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionAttenuate
//////////////////////////////////////////////////////////////////////////////

ActionAttenuate::ActionAttenuate(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_key = 0;
	_attenuation = 0;

	if (sscanf(line.c_str(), "%u, %d", &_key, &_attenuation) != 2)
		debugC(kDebugScript, "Malformed ActionAttenuate line: %s", line.c_str());
	debugC(3, kDebugAction, "Created Action: Attenuate, slotKey %d", _slotKey);
	debugC(3, kDebugAction, "Attenuate script: %s", line.c_str());
	debugC(3, kDebugAction, "Attenuate parameters: key1 %d, attenuation %d", _key, _attenuation);
}

bool ActionAttenuate::execute() {
	debugC(3, kDebugAction, "Executing Action: Attenuate, slotkey %d", _slotKey);
	ScriptingEffect *fx = _scriptManager->getSideFX(_key);
	if (fx && fx->getType() == ScriptingEffect::SCRIPTING_EFFECT_AUDIO) {
		MusicNodeBASE *mus = (MusicNodeBASE *)fx;
		mus->setVolume((10000 - abs(_attenuation)) / 100);  // TODO - verify that this is working correctly
		// Given that the scripts specify this effect in negative values of several, thousand, it is possible the original system may have multiplied by a factor defined in milli-decibels or similar.  Further investigation & comparison with the audio output in various locations from the original executables could shed more light on this.
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionChangeLocation
//////////////////////////////////////////////////////////////////////////////

ActionChangeLocation::ActionChangeLocation(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_world = 'g';
	_room = 'a';
	_node = 'r';
	_view = 'y';
	_offset = 0;

	if (sscanf(line.c_str(), "%c, %c, %c%c, %u", &_world, &_room, &_node, &_view, &_offset) != 5)
		debugC(kDebugScript, "Malformed ActionChangeLocation line: %s", line.c_str());
}

bool ActionChangeLocation::execute() {
	// We can't directly call ScriptManager::ChangeLocationReal() because doing so clears all the Puzzles, and thus would corrupt the current puzzle checking
	_scriptManager->changeLocation(_world, _room, _node, _view, _offset);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionCrossfade
//////////////////////////////////////////////////////////////////////////////

ActionCrossfade::ActionCrossfade(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_keyOne = 0;
	_keyTwo = 0;
	_oneStartVolume = 0;
	_twoStartVolume = 0;
	_oneEndVolume = 0;
	_twoEndVolume = 0;
	_timeInMillis = 0;

	if (sscanf(line.c_str(),
	       "%u %u %d %d %d %d %d",
	       &_keyOne, &_keyTwo, &_oneStartVolume, &_twoStartVolume, &_oneEndVolume, &_twoEndVolume, &_timeInMillis) != 7)
		debugC(kDebugScript, "Malformed ActionCrossFade line: %s", line.c_str());
	debugC(3, kDebugAction, "Created Action: CrossFade, slotKey %d", _slotKey);
	debugC(3, kDebugAction, "Crossfade script: %s", line.c_str());
	debugC(3, kDebugAction, "Crossfade parameters: key1 %u, key2 %u, startVol1 %d, startVol2 %d, endVol1 %d, endVol2 %d, time %dms", _keyOne, _keyTwo, _oneStartVolume, _twoStartVolume, _oneEndVolume, _twoEndVolume, _timeInMillis);
}

bool ActionCrossfade::execute() {
	debugC(3, kDebugAction, "Executing Action: CrossFade, slotkey %d", _slotKey);
	if (_keyOne) {
		ScriptingEffect *fx = _scriptManager->getSideFX(_keyOne);
		if (fx && fx->getType() == ScriptingEffect::SCRIPTING_EFFECT_AUDIO) {
			MusicNodeBASE *mus = (MusicNodeBASE *)fx;
			if (_oneStartVolume >= 0)
				mus->setVolume(_oneStartVolume);
			mus->setFade(_timeInMillis, _oneEndVolume);
		}
	}

	if (_keyTwo) {
		ScriptingEffect *fx = _scriptManager->getSideFX(_keyTwo);
		if (fx && fx->getType() == ScriptingEffect::SCRIPTING_EFFECT_AUDIO) {
			MusicNodeBASE *mus = (MusicNodeBASE *)fx;
			if (_twoStartVolume >= 0)
				mus->setVolume(_twoStartVolume);

			mus->setFade(_timeInMillis, _twoEndVolume);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionCursor
//////////////////////////////////////////////////////////////////////////////

ActionCursor::ActionCursor(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	Common::String up = line;
	up.toUppercase();
	_action = 0;

	if (up[0] == 'B')
		_action = 2;
	else if (up[0] == 'I')
		_action = 3;
	else if (up[0] == 'U')
		_action = 0;
	else if (up[0] == 'H')
		_action = 1;
}

bool ActionCursor::execute() {
	switch (_action) {
	case 1:
		_engine->getCursorManager()->showMouse(false);
		break;
	default:
		_engine->getCursorManager()->showMouse(true);
		break;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionDelayRender
//////////////////////////////////////////////////////////////////////////////

ActionDelayRender::ActionDelayRender(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_framesToDelay = 0;
	if (sscanf(line.c_str(), "%u", &_framesToDelay) != 1)
		debugC(kDebugScript, "Malformed ActionDelayRender line: %s", line.c_str());
	// Limit to 10 frames maximum. This fixes the script bug in ZGI scene px10
	// (outside Frobozz Electric building), where this is set to 100 (bug #6791).
	_framesToDelay = MIN<uint32>(_framesToDelay, 10);
}

bool ActionDelayRender::execute() {
	debugC(3, kDebugAction, "Executing Action: DelayRender");
	_engine->setRenderDelay(_framesToDelay);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionDisableControl
//////////////////////////////////////////////////////////////////////////////

ActionDisableControl::ActionDisableControl(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_key = 0;

	if (sscanf(line.c_str(), "%u", &_key) != 1)
		debugC(kDebugScript, "Malformed ActionDisableControl line: %s", line.c_str());
}

bool ActionDisableControl::execute() {
	_scriptManager->setStateFlag(_key, Puzzle::DISABLED);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionDisplayMessage
//////////////////////////////////////////////////////////////////////////////

ActionDisplayMessage::ActionDisplayMessage(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_control = 0;
	_msgid = 0;

	if (sscanf(line.c_str(), "%hd %hd", &_control, &_msgid) != 2)
		debugC(kDebugScript, "Malformed ActionDisplayMessage line: %s", line.c_str());
}

bool ActionDisplayMessage::execute() {
	Control *ctrl = _scriptManager->getControl(_control);
	if (ctrl && ctrl->getType() == Control::CONTROL_TITLER) {
		TitlerControl *titler = (TitlerControl *)ctrl;
		titler->setString(_msgid);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionDissolve
//////////////////////////////////////////////////////////////////////////////

ActionDissolve::ActionDissolve(ZVision *engine) :
	ResultAction(engine, 0) {
	debugC(3, kDebugAction, "Created action: Dissolve");
}

bool ActionDissolve::execute() {
	debugC(3, kDebugAction, "Executing action: Dissolve");
	// Cause black screen flick
	// Not ideal.  Original engine used a softer dissolve effect; simply turning the screen black is jarring, so disabled for now.
	// _engine->getRenderManager()->bkgFill(0, 0, 0); // TODO - reimplement this?
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionDistort - only used by Zork: Nemesis for the "treatment" puzzle in the Sanitarium (aj30)
//////////////////////////////////////////////////////////////////////////////

ActionDistort::ActionDistort(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_distSlot = 0;
	_speed = 0;
	_startAngle = 60.0;
	_endAngle = 60.0;
	_startLineScale = 1.0;
	_endLineScale = 1.0;

	if (sscanf(line.c_str(), "%hd %hd %f %f %f %f", &_distSlot, &_speed, &_startAngle, &_endAngle, &_startLineScale, &_endLineScale) != 6)
		debugC(kDebugScript, "Malformed ActionDistort line: %s", line.c_str());
}

ActionDistort::~ActionDistort() {
	_scriptManager->killSideFx(_distSlot);
}

bool ActionDistort::execute() {
	debugC(3, kDebugAction, "Executing Action: Distort");
	if (_scriptManager->getSideFX(_distSlot))
		return true;

	_scriptManager->addSideFX(new DistortNode(_engine, _distSlot, _speed, _startAngle, _endAngle, _startLineScale, _endLineScale));

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionEnableControl
//////////////////////////////////////////////////////////////////////////////

ActionEnableControl::ActionEnableControl(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_key = 0;

	if (sscanf(line.c_str(), "%u", &_key) != 1)
		debugC(kDebugScript, "Malformed ActionEnableControl line: %s", line.c_str());
}

bool ActionEnableControl::execute() {
	_scriptManager->unsetStateFlag(_key, Puzzle::DISABLED);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionFlushMouseEvents
//////////////////////////////////////////////////////////////////////////////

ActionFlushMouseEvents::ActionFlushMouseEvents(ZVision *engine, int32 slotKey) :
	ResultAction(engine, slotKey) {
}

bool ActionFlushMouseEvents::execute() {
	_scriptManager->flushEvent(Common::EVENT_LBUTTONUP);
	_scriptManager->flushEvent(Common::EVENT_LBUTTONDOWN);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionInventory
//////////////////////////////////////////////////////////////////////////////

ActionInventory::ActionInventory(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_type = -1;
	_key = 0;

	char buf[25];
	if (sscanf(line.c_str(), "%24s %d", buf, &_key) != 2) {
		debugC(kDebugScript, "Malformed ActionInventory line: %s", line.c_str());
		return;
	}
	if (strcmp(buf, "add") == 0) {
		_type = 0;
	} else if (strcmp(buf, "addi") == 0) {
		_type = 1;
	} else if (strcmp(buf, "drop") == 0) {
		_type = 2;
	} else if (strcmp(buf, "dropi") == 0) {
		_type = 3;
	} else if (strcmp(buf, "cycle") == 0) {
		_type = 4;
	}
}

bool ActionInventory::execute() {
	switch (_type) {
	case 0: // add
		_scriptManager->inventoryAdd(_key);
		break;
	case 1: // addi
		_scriptManager->inventoryAdd(_scriptManager->getStateValue(_key));
		break;
	case 2: // drop
		if (_key >= 0)
			_scriptManager->inventoryDrop(_key);
		else
			_scriptManager->inventoryDrop(_scriptManager->getStateValue(StateKey_InventoryItem));
		break;
	case 3: // dropi
		_scriptManager->inventoryDrop(_scriptManager->getStateValue(_key));
		break;
	case 4: // cycle
		_scriptManager->inventoryCycle();
		break;
	default:
		break;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionKill - only used by ZGI
//////////////////////////////////////////////////////////////////////////////

ActionKill::ActionKill(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_key = 0;
	_type = 0;
	char keytype[25];
	if (sscanf(line.c_str(), "%24s", keytype) != 1) 
		debugC(kDebugScript, "Malformed ActionKill line: %s", line.c_str());
	else {
		if (keytype[0] == '"') {
			if (!scumm_stricmp(keytype, "\"ANIM\""))
				_type = ScriptingEffect::SCRIPTING_EFFECT_ANIM;
			else if (!scumm_stricmp(keytype, "\"AUDIO\""))
				_type = ScriptingEffect::SCRIPTING_EFFECT_AUDIO;
			else if (!scumm_stricmp(keytype, "\"DISTORT\""))
				_type = ScriptingEffect::SCRIPTING_EFFECT_DISTORT;
			else if (!scumm_stricmp(keytype, "\"PANTRACK\""))
				_type = ScriptingEffect::SCRIPTING_EFFECT_PANTRACK;
			else if (!scumm_stricmp(keytype, "\"REGION\""))
				_type = ScriptingEffect::SCRIPTING_EFFECT_REGION;
			else if (!scumm_stricmp(keytype, "\"TIMER\""))
				_type = ScriptingEffect::SCRIPTING_EFFECT_TIMER;
			else if (!scumm_stricmp(keytype, "\"TTYTEXT\""))
				_type = ScriptingEffect::SCRIPTING_EFFECT_TTYTXT;
			else if (!scumm_stricmp(keytype, "\"ALL\""))
				_type = ScriptingEffect::SCRIPTING_EFFECT_ALL;
		} else
			_key = atoi(keytype);
	}
	debugC(3, kDebugAction, "Created Action: Kill, slotKey %d, type %s, target slotKey %d", _slotKey, keytype, _key);
}

bool ActionKill::execute() {
	if (_type)
		_scriptManager->killSideFxType((ScriptingEffect::ScriptingEffectType)_type);
	else {
		debugC(2, kDebugAction, "Executing Action: Kill, slotKey %d, target slotKey %d", _slotKey, _key);
		_scriptManager->killSideFx(_key);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionMenuBarEnable
//////////////////////////////////////////////////////////////////////////////

ActionMenuBarEnable::ActionMenuBarEnable(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_menus = 0xFFFF;

	if (sscanf(line.c_str(), "%hu", &_menus) != 1)
		debugC(kDebugScript, "Malformed ActionMenuBarEnable line: %s", line.c_str());
}

bool ActionMenuBarEnable::execute() {
	_engine->getMenuManager()->setEnable(_menus);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionMusic
//////////////////////////////////////////////////////////////////////////////

ActionMusic::ActionMusic(ZVision *engine, int32 slotKey, const Common::String &line, bool global) :
	ResultAction(engine, slotKey),
	_note(0),
	_prog(0),
	_universe(global) {
	uint type = 0;
	char fileNameBuffer[25];
	uint loop = 0;
	char volumeBuffer[15];

	// Volume is optional. If it doesn't appear, assume full volume
	Common::strcpy_s(volumeBuffer, "100");

	if (sscanf(line.c_str(), "%u %24s %u %14s", &type, fileNameBuffer, &loop, volumeBuffer) < 3) {
		debugC(kDebugScript, "Malformed ActionMusic line: %s", line.c_str());
		return;
	}

	// Type 4 actions are MIDI commands, not files. These are only used by
	// Zork: Nemesis, for the flute and piano puzzles (tj4e and ve6f, as well
	// as vr)
	switch (type) {
	case 4:
		_midi = true;
		int note;
		int prog;
		if (sscanf(line.c_str(), "%u %d %d %14s", &type, &prog, &note, volumeBuffer) != 4) {
			debugC(kDebugScript, "Malformed ActionMusic MIDI line: %s", line.c_str());
			break;
		}
		_note = note;
		_prog = prog;
		_loop = false;
		break;
	default:
		_midi = false;
		_fileName = Common::String(fileNameBuffer);
		_loop = loop == 1 ? true : false;
		break;
	}
	if (volumeBuffer[0] != '[' && atoi(volumeBuffer) > 100) {
		// I thought I saw a case like this in Zork Nemesis, so
		// let's guard against it.
		debugC(kDebugSound, "\tActionMusic: Adjusting volume for %s from %s to 100", _fileName.toString().c_str(), volumeBuffer);
		Common::strcpy_s(volumeBuffer, "100");
	}
	_volume = new ValueSlot(_scriptManager, volumeBuffer);

	// WORKAROUND for a script bug in Zork Nemesis, rooms mq70/mq80.
	// Fixes an edge case where the player goes to the dark room with the grue
	// without holding a torch, and then quickly runs away before the grue's
	// sound effect finishes. Fixes script bug #6794.
	if (engine->getGameId() == GID_NEMESIS && _slotKey == 14822 && _scriptManager->getStateValue(_slotKey) == 2)
		_scriptManager->setStateValue(_slotKey, 0);

	// Ensure MusicNodes that were active when game was saved are recreated when it is loaded.
	// Certain game scripts can become locked-up if this is not the case.
	if (_engine->getScriptManager()->getStateValue(_slotKey) == 1)
		if (!_scriptManager->getSideFX(_slotKey)) {
			debugC(1, kDebugAction, "Recreating missing musicnode, slotkey %d", _slotKey);
			execute();
		}

	debugC(2, kDebugAction, "Created Action: Music, slotKey %d, type %u, file %24s, note %u, volume %d, %s", _slotKey, type, fileNameBuffer, _note, _volume->getValue(), _loop ? "looping" : "");
	debugC(4, kDebugAction, "Music script: %s", line.c_str());
}

ActionMusic::~ActionMusic() {
	if (!_universe)
		_scriptManager->killSideFx(_slotKey);
	delete _volume;
	debugC(2, kDebugAction, "Destroyed Action: %sMusic, slotkey %d", _universe ? "Universe_" : "", _slotKey);
}

bool ActionMusic::execute() {
	debugC(2, kDebugAction, "Executing Action: Music, slotKey %d, volume %d", _slotKey, _volume->getValue());
	if (_scriptManager->getSideFX(_slotKey)) {
		_scriptManager->killSideFx(_slotKey);
		_scriptManager->setStateValue(_slotKey, 2);
	}

	uint volume = _volume->getValue();
	if (_midi) {
		_scriptManager->addSideFX(new MusicMidiNode(_engine, _slotKey, _prog, _note, volume));
	} else {
		_scriptManager->addSideFX(new MusicNode(_engine, _slotKey, _fileName, _loop, volume));
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionPanTrack
//////////////////////////////////////////////////////////////////////////////

ActionPanTrack::ActionPanTrack(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey),
	_pos(0),
	_mag(255),
	_resetMusicNode(true),
	_resetMixerOnDelete(false),
	_staticScreen(false),
	_musicSlot(0) {
	uint mag = 255;  // Original game scripts do not specify this, but require it to be 255 to work correctly.
	uint resetMusicNode = 1;  // Original game scripts do not specify this, but require it to be true to work correctly.
	uint resetMixerOnDelete = 0;  // Original game scripts do not specify this, but require it to be false to work correctly.
	uint staticScreen = 0;  // Original game scripts do not specify this, but require it to be false to work correctly.
	if (sscanf(line.c_str(), "%u %d %u %u %u %u", &_musicSlot, &_pos, &mag, &resetMusicNode, &resetMixerOnDelete, &staticScreen) < 2)
		debugC(kDebugScript, "Malformed ActionPanTrack line: %s", line.c_str());
	_resetMusicNode = resetMusicNode > 0;
	_resetMixerOnDelete = resetMixerOnDelete > 0;
	_staticScreen = staticScreen > 0;
	_mag = mag;
	if (_resetMusicNode) {
		if (_scriptManager->getStateValue(_musicSlot) != 2) {
			debugC(3, kDebugAction, "Forcing musicSlot %d to 2", _musicSlot);
			_scriptManager->setStateValue(_musicSlot, 2); // Not all original game pan_track scripts trigger correctly unless this is set!
		} else
			debugC(3, kDebugAction, "musicSlot %d already set to 2", _musicSlot);
	} else
		debugC(3, kDebugAction, "NOT forcing musicSlot %d to 2", _musicSlot);
	debugC(3, kDebugAction, "Created Action: PanTrack, slotkey %d, musicSlot %u, pos %d, mag %d", _slotKey, _musicSlot, _pos, _mag);
}

ActionPanTrack::~ActionPanTrack() {
	_scriptManager->killSideFx(_slotKey);
	debugC(3, kDebugAction, "Destroyed Action: PanTrack, slotkey %d", _slotKey);
}

bool ActionPanTrack::execute() {
	debugC(3, kDebugAction, "Executing Action: PanTrack, slotkey %d, musicSlot %u, pos %d, mag %d", _slotKey, _musicSlot, _pos, _mag);
	if (_scriptManager->getSideFX(_slotKey))
		return true;

	_scriptManager->addSideFX(new PanTrackNode(_engine, _slotKey, _musicSlot, _pos, _mag, _resetMixerOnDelete, _staticScreen));

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionPreferences
//////////////////////////////////////////////////////////////////////////////

ActionPreferences::ActionPreferences(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	if (line.compareToIgnoreCase("save") == 0)
		_save = true;
	else
		_save = false;
}

bool ActionPreferences::execute() {
	if (_save)
		_engine->saveSettings();
	else
		_engine->loadSettings();

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionPreloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPreloadAnimation::ActionPreloadAnimation(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_mask = 0;
	_framerate = 0;

	char fileName[25];

	// The two %*u are usually 0 and dont seem to have a use
	if (sscanf(line.c_str(), "%24s %*u %*u %d %d", fileName, &_mask, &_framerate) != 3) {
		debugC(kDebugScript, "Malformed ActionPreloadAnimation line: %s", line.c_str());
		return;
	}
	// Mask 0 means "no transparency" in this case. Since we use a common blitting
	// code for images and animations, we set it to -1 to avoid confusion with
	// color 0, which is used as a mask in some images
	if (_mask == 0)
		_mask = -1;

	_fileName = Common::String(fileName);
}

ActionPreloadAnimation::~ActionPreloadAnimation() {
	_scriptManager->deleteSideFx(_slotKey);
}

bool ActionPreloadAnimation::execute() {
	AnimationEffect *nod = (AnimationEffect *)_scriptManager->getSideFX(_slotKey);

	if (!nod) {
		nod = new AnimationEffect(_engine, _slotKey, _fileName, _mask, _framerate, false);
		_scriptManager->addSideFX(nod);
	} else
		nod->stop();
	_scriptManager->setStateValue(_slotKey, 2);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionUnloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionUnloadAnimation::ActionUnloadAnimation(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_key = 0;

	if (sscanf(line.c_str(), "%u", &_key) != 1)
		debugC(kDebugScript, "Malformed ActionUnloadAnimation line: %s", line.c_str());
}

bool ActionUnloadAnimation::execute() {
	AnimationEffect *nod = (AnimationEffect *)_scriptManager->getSideFX(_key);

	if (nod && nod->getType() == ScriptingEffect::SCRIPTING_EFFECT_ANIM)
		_scriptManager->deleteSideFx(_key);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionPlayAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayAnimation::ActionPlayAnimation(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_x = 0;
	_y = 0;
	_x2 = 0;
	_y2 = 0;
	_start = 0;
	_end = 0;
	_loopCount = 0;
	_mask = 0;
	_framerate = 0;

	char fileName[25];

	// The two %*u are always 0 and dont seem to have a use
	if (sscanf(line.c_str(),
			"%24s %u %u %u %u %u %u %d %*u %*u %d %d",
			fileName, &_x, &_y, &_x2, &_y2, &_start, &_end, &_loopCount, &_mask, &_framerate) != 10) {
		debugC(kDebugScript, "Malformed ActionPlayAnimation line: %s", line.c_str());
		return;
	}

	// Mask 0 means "no transparency" in this case. Since we use a common blitting
	// code for images and animations, we set it to -1 to avoid confusion with
	// color 0, which is used as a mask in some images
	if (_mask == 0)
		_mask = -1;

	_fileName = Common::String(fileName);

	// WORKAROUND for bug #6769, location me1g.scr (the "Alchemical debacle"
	// video in ZGI). We only scale up by 2x, in AnimationEffect::process(),
	// but the dimensions of the target frame are off by 2 pixels. We fix that
	// here, so that the video can be scaled.
	if (_fileName == "me1ga011.avi" && _y2 == 213)
		_y2 = 215;
}

ActionPlayAnimation::~ActionPlayAnimation() {
	_scriptManager->deleteSideFx(_slotKey);
}

bool ActionPlayAnimation::execute() {
	AnimationEffect *nod = (AnimationEffect *)_scriptManager->getSideFX(_slotKey);

	if (!nod) {
		nod = new AnimationEffect(_engine, _slotKey, _fileName, _mask, _framerate);
		_scriptManager->addSideFX(nod);
	} else
		nod->stop();

	if (nod)
		nod->addPlayNode(_slotKey, _x, _y, _x2, _y2, _start, _end, _loopCount);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionPlayPreloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayPreloadAnimation::ActionPlayPreloadAnimation(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_controlKey = 0;
	_x1 = 0;
	_y1 = 0;
	_x2 = 0;
	_y2 = 0;
	_startFrame = 0;
	_endFrame = 0;
	_loopCount = 0;

	if (sscanf(line.c_str(),
			"%u %u %u %u %u %u %u %u",
			&_controlKey, &_x1, &_y1, &_x2, &_y2, &_startFrame, &_endFrame, &_loopCount) != 8)
		debugC(kDebugScript, "Malformed ActionPlayPreloadAnimation line: %s", line.c_str());
	       
	// WORKAROUND for script bug in Zork Nemesis, room tl9e
	// Original script gives wrong coordinates & frames
	if (engine->getGameId() == GID_NEMESIS)
		switch (_slotKey) {
		case 1282:
			_x1 = 146;
			_y1 = 142;
			_x2 = 298;
			_y2 = 236;
			_startFrame = 16;
			_endFrame = 31;
			break;
		case 1289:
			_x1 = 146;
			_y1 = 142;
			_x2 = 298;
			_y2 = 236;
			_endFrame = 15;
			break;
		default:
			break;
		}
}

bool ActionPlayPreloadAnimation::execute() {
	AnimationEffect *nod = (AnimationEffect *)_scriptManager->getSideFX(_controlKey);

	if (nod)
		nod->addPlayNode(_slotKey, _x1, _y1, _x2, _y2, _startFrame, _endFrame, _loopCount);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionQuit
//////////////////////////////////////////////////////////////////////////////

bool ActionQuit::execute() {
	_engine->quit(false);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionRegion - only used by Zork: Nemesis
//////////////////////////////////////////////////////////////////////////////

ActionRegion::ActionRegion(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_delay = 0;
	_type = 0;
	_unk1 = 0;
	_unk2 = 0;

	char art[64];
	char custom[64];

	int32 x1 = 0, x2 = 0, y1 = 0, y2 = 0;

	if (sscanf(line.c_str(), "%s %d %d %d %d %hu %hu %hu %hu %s", art, &x1, &y1, &x2, &y2, &_delay, &_type, &_unk1, &_unk2, custom) != 10) {
		debugC(kDebugScript, "Malformed ActionRegion line: %s", line.c_str());
		return;
	}
	_art = Common::String(art);
	_custom = Common::String(custom);
	_rect = Common::Rect(x1, y1, x2 + 1, y2 + 1);
}

ActionRegion::~ActionRegion() {
	_scriptManager->killSideFx(_slotKey);
}

bool ActionRegion::execute() {
	debugC(2, kDebugAction, "Executing Action: Region");
	if (_scriptManager->getSideFX(_slotKey))
		return true;

	GraphicsEffect *effect = NULL;
	switch (_type) {
	case 0: {
		uint16 centerX, centerY, frames;
		double amplitude, waveln, speed;
		if (sscanf(_custom.c_str(), "%hu,%hu,%hu,%lf,%lf,%lf,", &centerX, &centerY, &frames, &amplitude, &waveln, &speed) != 6) {
			debugC(kDebugScript, "Malformed ActionRegion custom string: %s", _custom.c_str());
			break;
		}
		effect = new WaveFx(_engine, _slotKey, _rect, _unk1, frames, centerX, centerY, amplitude, waveln, speed);
	}
	break;
	case 1: {
		uint16 aX, aY, aD;
		if (_engine->getRenderManager()->getRenderTable()->getRenderState() == RenderTable::PANORAMA) {
			if (sscanf(_art.c_str(), "useart[%hu,%hu,%hu]", &aY, &aX, &aD) != 3) {
				debugC(kDebugScript, "Malformed ActionRegion art string: %s", _art.c_str());
				break;
			}
		}
		else {
			if (sscanf(_art.c_str(), "useart[%hu,%hu,%hu]", &aX, &aY, &aD) != 3) {
				debugC(kDebugScript, "Malformed ActionRegion art string: %s", _art.c_str());
				break;
			}
		}
		int8 minD;
		int8 maxD;
		EffectMap *_map = _engine->getRenderManager()->makeEffectMap(Common::Point(aX, aY), aD, _rect, &minD, &maxD);
		effect = new LightFx(_engine, _slotKey, _rect, _unk1, _map, atoi(_custom.c_str()), minD, maxD);
	}
	break;
	case 9: {
		int16 dum1;
		int32 dum2;
		char buf[64];
		if (sscanf(_custom.c_str(), "%hd,%d,%s", &dum1, &dum2, buf) != 3) {
				debugC(kDebugScript, "Malformed ActionRegion custom string: %s", _custom.c_str());
			break;
		}
		Graphics::Surface tempMask;
		_engine->getRenderManager()->readImageToSurface(Common::Path(_art), tempMask);
		if (_rect.width() != tempMask.w)
			_rect.setWidth(tempMask.w);
		if (_rect.height() != tempMask.h)
			_rect.setHeight(tempMask.h);

		EffectMap *_map = _engine->getRenderManager()->makeEffectMap(tempMask, 0);
		effect = new FogFx(_engine, _slotKey, _rect, _unk1, _map, buf);
		tempMask.free();
	}
	break;
	default:
		break;
	}

	if (effect) {
		_scriptManager->addSideFX(new RegionNode(_engine, _slotKey, effect, _delay));
		_engine->getRenderManager()->addEffect(effect);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionRandom
//////////////////////////////////////////////////////////////////////////////

ActionRandom::ActionRandom(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	char maxBuffer[64];
	memset(maxBuffer, 0, 64);
	if (sscanf(line.c_str(), "%s", maxBuffer) != 1) {
		debugC(kDebugScript, "Malformed ActionRandom line: %s", line.c_str());
		return;
	}
	_max = new ValueSlot(_scriptManager, maxBuffer);
}

ActionRandom::~ActionRandom() {
	delete _max;
}

bool ActionRandom::execute() {
	uint randNumber = _engine->getRandomSource()->getRandomNumber(_max->getValue());
	_scriptManager->setStateValue(_slotKey, randNumber);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionRestoreGame
//////////////////////////////////////////////////////////////////////////////

ActionRestoreGame::ActionRestoreGame(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	char buf[128];
	if (sscanf(line.c_str(), "%s", buf) != 1) {
		debugC(kDebugScript, "Malformed ActionRestoreGame line: %s", line.c_str());
		return;
	}
	_fileName = Common::String(buf);
}

bool ActionRestoreGame::execute() {
	_engine->getSaveManager()->loadGame(-1);
	return false;
}

//////////////////////////////////////////////////////////////////////////////
// ActionRotateTo
//////////////////////////////////////////////////////////////////////////////

ActionRotateTo::ActionRotateTo(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_time = 0;
	_toPos = 0;

	if (sscanf(line.c_str(), "%d, %d", &_toPos, &_time) != 2)
		debugC(kDebugScript, "Malformed ActionRotateTo line: %s", line.c_str());
}

bool ActionRotateTo::execute() {
	_engine->getRenderManager()->rotateTo(_toPos, _time);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionSetPartialScreen
//////////////////////////////////////////////////////////////////////////////

ActionSetPartialScreen::ActionSetPartialScreen(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_x = 0;
	_y = 0;

	char fileName[25];

	if (sscanf(line.c_str(), "%u %u %24s %*u %d", &_x, &_y, fileName, &_backgroundColor) != 4) {
		debugC(kDebugScript, "Malformed ActionSetPartialScreen line: %s", line.c_str());
		return;
	}
	_fileName = Common::String(fileName);

	if (_backgroundColor > 65535) {
		warning("Background color for ActionSetPartialScreen is bigger than a uint16");
	}
}

bool ActionSetPartialScreen::execute() {
	RenderManager *renderManager = _engine->getRenderManager();

	if (_engine->getGameId() == GID_NEMESIS) {
		if (_backgroundColor)
			renderManager->renderImageToBackground(_fileName, _x, _y, 0, 0);
		else
			renderManager->renderImageToBackground(_fileName, _x, _y);
	} else {
		if (_backgroundColor >= 0)
			renderManager->renderImageToBackground(_fileName, _x, _y, _backgroundColor);
		else if (_backgroundColor == -2)
			renderManager->renderImageToBackground(_fileName, _x, _y, 0, 0);
		else
			renderManager->renderImageToBackground(_fileName, _x, _y);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionSetScreen
//////////////////////////////////////////////////////////////////////////////

ActionSetScreen::ActionSetScreen(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	char fileName[25];
	if (sscanf(line.c_str(), "%24s", fileName) != 1) {
		debugC(kDebugScript, "Malformed ActionSetScreen line: %s", line.c_str());
		return;
	}
	_fileName = Common::String(fileName);
}

bool ActionSetScreen::execute() {
	_engine->getRenderManager()->setBackgroundImage(_fileName);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionStop
//////////////////////////////////////////////////////////////////////////////

ActionStop::ActionStop(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_key = 0;
	if (sscanf(line.c_str(), "%u", &_key) != 1)
		debugC(kDebugScript, "Malformed ActionStop line: %s", line.c_str());
}

bool ActionStop::execute() {
	debugC(2, kDebugAction, "Executing Action: Stop, key %d", _key);
	_scriptManager->stopSideFx(_key);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionStreamVideo
//////////////////////////////////////////////////////////////////////////////

ActionStreamVideo::ActionStreamVideo(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_x1 = 0;
	_x2 = 0;
	_y1 = 0;
	_y2 = 0;
	_flags = 0;

	char fileName[25];
	uint skipline = 0;    // skipline - render video with skip every second line, not skippable.
	_skippable = true;
	
	if (sscanf(line.c_str(), "%24s %u %u %u %u %u %u", fileName, &_x1, &_y1, &_x2, &_y2, &_flags, &skipline) != 7) {
		debugC(kDebugScript, "Malformed ActionStreamVideo line: %s", line.c_str());
		return;
	}
	_fileName = Common::String(fileName);
}

bool ActionStreamVideo::execute() {
	debugC(3, kDebugAction, "Executing video stream");
	Video::VideoDecoder *decoder;
	Common::Rect destRect = Common::Rect(_x1, _y1, _x2 + 1, _y2 + 1);
	Common::Rect srcRect = Common::Rect(0, 0);
	debugC(3, kDebugAction, "Streaming video original scripted destination left=%d, top=%d, right=%d, bottom=%d", destRect.left, destRect.top, destRect.right, destRect.bottom);
	Common::String subname = _fileName.baseName();
	subname.setChar('s', subname.size() - 3);
	subname.setChar('u', subname.size() - 2);
	subname.setChar('b', subname.size() - 1);
	Common::Path subpath(_fileName.getParent().appendComponent(subname));
	bool subtitleExists = SearchMan.hasFile(subpath);
	bool switchToHires = false;

// NOTE: We only show the hires MPEG2 videos when libmpeg2 and liba52 are compiled in,
// otherwise we fall back to the lowres ones
#if defined(USE_MPEG2) && defined(USE_A52)
	Common::String hiresFileName = _fileName.baseName();
	hiresFileName.setChar('d', hiresFileName.size() - 8);
	hiresFileName.setChar('v', hiresFileName.size() - 3);
	hiresFileName.setChar('o', hiresFileName.size() - 2);
	hiresFileName.setChar('b', hiresFileName.size() - 1);

	Common::Path hiresPath(_fileName.getParent().appendComponent(hiresFileName));

	if (_scriptManager->getStateValue(StateKey_MPEGMovies) == 1 && SearchMan.hasFile(hiresPath)) {
		_fileName = hiresPath;
		switchToHires = true;
	} else if (!SearchMan.hasFile(_fileName))
		return true;
#else
	if (!SearchMan.hasFile(_fileName))
		return true;
#endif

	decoder = _engine->loadAnimation(_fileName);
	uint16 sub = (subtitleExists) ? _engine->getSubtitleManager()->create(subpath, switchToHires) : 0;

	_engine->getCursorManager()->showMouse(false);

	if (switchToHires) {
		_engine->getRenderManager()->initialize(true);
		srcRect = Common::Rect(Common::Point(0, 69), 720, 344);
		// ZGI hi-res video resolution = 720x480, with baked-in letterboxing around content at 720x344 (origin 0,69), interestingly conforming to playfield vertical resolution of 344
		destRect = _engine->getRenderManager()->getWorkingArea(); // Game scripts only give destRect for normal resolution; we must manually override them for HD videos
		destRect.moveTo(0, 0);
	}

	// WORKAROUND for what appears to be a script bug. When riding with
	// Charon in one direction, the game issues a command to kill the
	// universe_hades_sound_task. When going in the other direction (either
	// as yourself or as the two-headed beast) it does not. Since the
	// cutscene plays music, there may be two pieces of music playing
	// simultaneously during the ride.
	//
	// Rather than mucking about with killing and restarting the sound,
	// simply pause the ScummVM mixer during the ride.

	bool pauseBackgroundMusic = _engine->getGameId() == GID_GRANDINQUISITOR && (_fileName == "hp3ea021.avi" || _fileName == "hp4ea051.avi");

	if (pauseBackgroundMusic) {
		_engine->_mixer->pauseAll(true);
	}

	_engine->playVideo(*decoder, destRect, _skippable, sub, srcRect);

	if (pauseBackgroundMusic) {
		_engine->_mixer->pauseAll(false);
	}

	if (switchToHires) {
		_engine->getRenderManager()->initialize(false);
	}

	_engine->getCursorManager()->showMouse(true);
	_engine->getSubtitleManager()->destroy(sub);
	_engine->setRenderDelay(2); // Necessary for avoiding redraw of previous scene between sequential videos (eg totemization sequence in ZGI) & when changing location right after a video (e.g. opening temple door in Nemesis)
	debugC(3, kDebugAction, "Completed executing video stream");

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionSyncSound
//////////////////////////////////////////////////////////////////////////////

ActionSyncSound::ActionSyncSound(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_syncto = 0;

	char fileName[25];
	int notUsed = 0;

	if (sscanf(line.c_str(), "%d %d %24s", &_syncto, &notUsed, fileName) != 3) {
		debugC(kDebugScript, "Malformed ActionSyncSound line: %s", line.c_str());
		return;
	}
	_fileName = Common::String(fileName);
}

bool ActionSyncSound::execute() {
	debugC(3, kDebugAction, "Executing Action: SyncSound");
	ScriptingEffect *fx = _scriptManager->getSideFX(_syncto);
	if (!fx)
		return true;

	if (!(fx->getType() & ScriptingEffect::SCRIPTING_EFFECT_ANIM))
		return true;

	_scriptManager->addSideFX(new SyncSoundNode(_engine, _slotKey, _fileName, _syncto));
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionTimer
//////////////////////////////////////////////////////////////////////////////

ActionTimer::ActionTimer(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	char timeBuffer[64];
	memset(timeBuffer, 0, 64);
	if (sscanf(line.c_str(), "%s", timeBuffer) != 1) {
		debugC(kDebugScript, "Malformed ActionTimer line: %s", line.c_str());
		return;
	}
	_time = new ValueSlot(_scriptManager, timeBuffer);
}

ActionTimer::~ActionTimer() {
	delete _time;
	_scriptManager->killSideFx(_slotKey);
}

bool ActionTimer::execute() {
	if (_scriptManager->getSideFX(_slotKey))
		return true;
	_scriptManager->addSideFX(new TimerNode(_engine, _slotKey, _time->getValue()));
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionTtyText
//////////////////////////////////////////////////////////////////////////////

ActionTtyText::ActionTtyText(ZVision *engine, int32 slotKey, const Common::String &line) :
	ResultAction(engine, slotKey) {
	_delay = 0;

	char filename[64];
	int32 x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	if (sscanf(line.c_str(), "%d %d %d %d %63s %u", &x1, &y1, &x2, &y2, filename, &_delay) != 6) {
		debugC(kDebugScript, "Malformed ActionTtyText line: %s", line.c_str());
		return;
	}
	_r = Common::Rect(x1, y1, x2, y2);
	_filename = Common::String(filename);
}

ActionTtyText::~ActionTtyText() {
	_scriptManager->killSideFx(_slotKey);
}

bool ActionTtyText::execute() {
	if (_scriptManager->getSideFX(_slotKey))
		return true;
	_scriptManager->addSideFX(new ttyTextNode(_engine, _slotKey, _filename, _r, _delay));
	return true;
}

} // End of namespace ZVision
