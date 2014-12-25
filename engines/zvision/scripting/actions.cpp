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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/scripting/actions.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/sound/zork_raw.h"
#include "zvision/video/zork_avi_decoder.h"
#include "zvision/scripting/sidefx/timer_node.h"
#include "zvision/scripting/sidefx/music_node.h"
#include "zvision/scripting/sidefx/syncsound_node.h"
#include "zvision/scripting/sidefx/animation_node.h"
#include "zvision/scripting/sidefx/distort_node.h"
#include "zvision/scripting/sidefx/ttytext_node.h"
#include "zvision/scripting/sidefx/region_node.h"
#include "zvision/scripting/controls/titler_control.h"
#include "zvision/graphics/render_table.h"
#include "zvision/graphics/effect.h"
#include "zvision/graphics/effects/fog.h"
#include "zvision/graphics/effects/light.h"
#include "zvision/graphics/effects/wave.h"
#include "zvision/core/save_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"

#include "common/file.h"

#include "audio/decoders/wave.h"

namespace ZVision {

//////////////////////////////////////////////////////////////////////////////
// ActionAdd
//////////////////////////////////////////////////////////////////////////////

ActionAdd::ActionAdd(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_key = 0;
	_value = 0;

	sscanf(line.c_str(), "%u,%d", &_key, &_value);
}

bool ActionAdd::execute() {
	_engine->getScriptManager()->setStateValue(_key, _engine->getScriptManager()->getStateValue(_key) + _value);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionAssign
//////////////////////////////////////////////////////////////////////////////

ActionAssign::ActionAssign(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_key = 0;

	char buf[64];
	memset(buf, 0, 64);
	sscanf(line.c_str(), "%u, %s", &_key, buf);
	_value = new ValueSlot(_engine->getScriptManager(), buf);
}

ActionAssign::~ActionAssign() {
	if (_value)
		delete _value;
}

bool ActionAssign::execute() {
	_engine->getScriptManager()->setStateValue(_key, _value->getValue());
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionAttenuate
//////////////////////////////////////////////////////////////////////////////

ActionAttenuate::ActionAttenuate(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_key = 0;
	_attenuation = 0;

	sscanf(line.c_str(), "%u, %d", &_key, &_attenuation);
}

bool ActionAttenuate::execute() {
	SideFX *fx = _engine->getScriptManager()->getSideFX(_key);
	if (fx && fx->getType() == SideFX::SIDEFX_AUDIO) {
		MusicNode *mus = (MusicNode *)fx;
		mus->setVolume(255 - (abs(_attenuation) >> 7));
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionChangeLocation
//////////////////////////////////////////////////////////////////////////////

ActionChangeLocation::ActionChangeLocation(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_world = 'g';
	_room = 'a';
	_node = 'r';
	_view = 'y';
	_offset = 0;

	sscanf(line.c_str(), "%c, %c, %c%c, %u", &_world, &_room, &_node, &_view, &_offset);
}

bool ActionChangeLocation::execute() {
	// We can't directly call ScriptManager::ChangeLocationIntern() because doing so clears all the Puzzles, and thus would corrupt the current puzzle checking
	_engine->getScriptManager()->changeLocation(_world, _room, _node, _view, _offset);
	// Tell the puzzle system to stop checking any more puzzles
	return false;
}

//////////////////////////////////////////////////////////////////////////////
// ActionCrossfade
//////////////////////////////////////////////////////////////////////////////

ActionCrossfade::ActionCrossfade(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_keyOne = 0;
	_keyTwo = 0;
	_oneStartVolume = 0;
	_twoStartVolume = 0;
	_oneEndVolume = 0;
	_twoEndVolume = 0;
	_timeInMillis = 0;

	sscanf(line.c_str(),
	       "%u %u %d %d %d %d %d",
	       &_keyOne, &_keyTwo, &_oneStartVolume, &_twoStartVolume, &_oneEndVolume, &_twoEndVolume, &_timeInMillis);
}

bool ActionCrossfade::execute() {
	if (_keyOne) {
		SideFX *fx = _engine->getScriptManager()->getSideFX(_keyOne);
		if (fx && fx->getType() == SideFX::SIDEFX_AUDIO) {
			MusicNode *mus = (MusicNode *)fx;
			if (_oneStartVolume >= 0)
				mus->setVolume((_oneStartVolume * 255) / 100);

			mus->setFade(_timeInMillis, (_oneEndVolume * 255) / 100);
		}
	}

	if (_keyTwo) {
		SideFX *fx = _engine->getScriptManager()->getSideFX(_keyTwo);
		if (fx && fx->getType() == SideFX::SIDEFX_AUDIO) {
			MusicNode *mus = (MusicNode *)fx;
			if (_twoStartVolume >= 0)
				mus->setVolume((_twoStartVolume * 255) / 100);

			mus->setFade(_timeInMillis, (_twoEndVolume * 255) / 100);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionCursor
//////////////////////////////////////////////////////////////////////////////

ActionCursor::ActionCursor(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
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

ActionDelayRender::ActionDelayRender(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_framesToDelay = 0;
	sscanf(line.c_str(), "%u", &_framesToDelay);
}

bool ActionDelayRender::execute() {
	_engine->setRenderDelay(_framesToDelay);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionDisableControl
//////////////////////////////////////////////////////////////////////////////

ActionDisableControl::ActionDisableControl(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_key = 0;

	sscanf(line.c_str(), "%u", &_key);
}

bool ActionDisableControl::execute() {
	_engine->getScriptManager()->setStateFlag(_key, Puzzle::DISABLED);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionDisableVenus
//////////////////////////////////////////////////////////////////////////////

ActionDisableVenus::ActionDisableVenus(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_key = 0;

	sscanf(line.c_str(), "%d", &_key);
}

bool ActionDisableVenus::execute() {
	_engine->getScriptManager()->setStateValue(_key, 0);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionDisplayMessage
//////////////////////////////////////////////////////////////////////////////

ActionDisplayMessage::ActionDisplayMessage(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_control = 0;
	_msgid = 0;

	sscanf(line.c_str(), "%hd %hd", &_control, &_msgid);
}

bool ActionDisplayMessage::execute() {
	Control *ctrl = _engine->getScriptManager()->getControl(_control);
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
}

bool ActionDissolve::execute() {
	// Cause black screen flick
	// _engine->getRenderManager()->bkgFill(0, 0, 0);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionDistort
//////////////////////////////////////////////////////////////////////////////

ActionDistort::ActionDistort(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_distSlot = 0;
	_speed = 0;
	_startAngle = 60.0;
	_endAngle = 60.0;
	_startLineScale = 1.0;
	_endLineScale = 1.0;

	sscanf(line.c_str(), "%hd %hd %f %f %f %f", &_distSlot, &_speed, &_startAngle, &_endAngle, &_startLineScale, &_endLineScale);
}

ActionDistort::~ActionDistort() {
	_engine->getScriptManager()->killSideFx(_distSlot);
}

bool ActionDistort::execute() {
	if (_engine->getScriptManager()->getSideFX(_distSlot))
		return true;

	_engine->getScriptManager()->addSideFX(new DistortNode(_engine, _distSlot, _speed, _startAngle, _endAngle, _startLineScale, _endLineScale));

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionEnableControl
//////////////////////////////////////////////////////////////////////////////

ActionEnableControl::ActionEnableControl(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_key = 0;

	sscanf(line.c_str(), "%u", &_key);
}

bool ActionEnableControl::execute() {
	_engine->getScriptManager()->unsetStateFlag(_key, Puzzle::DISABLED);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionFlushMouseEvents
//////////////////////////////////////////////////////////////////////////////

ActionFlushMouseEvents::ActionFlushMouseEvents(ZVision *engine, int32 slotkey) :
	ResultAction(engine, slotkey) {
}

bool ActionFlushMouseEvents::execute() {
	_engine->getScriptManager()->flushEvent(Common::EVENT_LBUTTONUP);
	_engine->getScriptManager()->flushEvent(Common::EVENT_LBUTTONDOWN);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionInventory
//////////////////////////////////////////////////////////////////////////////

ActionInventory::ActionInventory(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_type = -1;
	_key = 0;

	char buf[25];
	sscanf(line.c_str(), "%24s %d", buf, &_key);

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
		_engine->getScriptManager()->inventoryAdd(_key);
		break;
	case 1: // addi
		_engine->getScriptManager()->inventoryAdd(_engine->getScriptManager()->getStateValue(_key));
		break;
	case 2: // drop
		if (_key >= 0)
			_engine->getScriptManager()->inventoryDrop(_key);
		else
			_engine->getScriptManager()->inventoryDrop(_engine->getScriptManager()->getStateValue(StateKey_InventoryItem));
		break;
	case 3: // dropi
		_engine->getScriptManager()->inventoryDrop(_engine->getScriptManager()->getStateValue(_key));
		break;
	case 4: // cycle
		_engine->getScriptManager()->inventoryCycle();
		break;
	default:
		break;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionKill
//////////////////////////////////////////////////////////////////////////////

ActionKill::ActionKill(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_key = 0;
	_type = 0;
	char keytype[25];
	sscanf(line.c_str(), "%24s", keytype);
	if (keytype[0] == '"') {
		if (!scumm_stricmp(keytype, "\"ANIM\""))
			_type = SideFX::SIDEFX_ANIM;
		else if (!scumm_stricmp(keytype, "\"AUDIO\""))
			_type = SideFX::SIDEFX_AUDIO;
		else if (!scumm_stricmp(keytype, "\"DISTORT\""))
			_type = SideFX::SIDEFX_DISTORT;
		else if (!scumm_stricmp(keytype, "\"PANTRACK\""))
			_type = SideFX::SIDEFX_PANTRACK;
		else if (!scumm_stricmp(keytype, "\"REGION\""))
			_type = SideFX::SIDEFX_REGION;
		else if (!scumm_stricmp(keytype, "\"TIMER\""))
			_type = SideFX::SIDEFX_TIMER;
		else if (!scumm_stricmp(keytype, "\"TTYTEXT\""))
			_type = SideFX::SIDEFX_TTYTXT;
		else if (!scumm_stricmp(keytype, "\"ALL\""))
			_type = SideFX::SIDEFX_ALL;
	} else
		_key = atoi(keytype);
}

bool ActionKill::execute() {
	if (_type)
		_engine->getScriptManager()->killSideFxType((SideFX::SideFXType)_type);
	else
		_engine->getScriptManager()->killSideFx(_key);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionMenuBarEnable
//////////////////////////////////////////////////////////////////////////////

ActionMenuBarEnable::ActionMenuBarEnable(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_menus = 0xFFFF;

	sscanf(line.c_str(), "%hu", &_menus);
}

bool ActionMenuBarEnable::execute() {
	_engine->menuBarEnable(_menus);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionMusic
//////////////////////////////////////////////////////////////////////////////

ActionMusic::ActionMusic(ZVision *engine, int32 slotkey, const Common::String &line, bool global) :
	ResultAction(engine, slotkey),
	_volume(255),
	_note(0),
	_prog(0),
	_universe(global) {
	uint type = 0;
	char fileNameBuffer[25];
	uint loop = 0;
	uint volume = 255;

	sscanf(line.c_str(), "%u %24s %u %u", &type, fileNameBuffer, &loop, &volume);

	// Type 4 actions are MIDI commands, not files. These are only used by
	// Zork: Nemesis, for the flute and piano puzzles (tj4e and ve6f, as well
	// as vr)
	if (type == 4) {
		_midi = true;
		int note;
		int prog;
		sscanf(line.c_str(), "%u %d %d %u", &type, &prog, &note, &volume);
		_volume = volume;
		_note = note;
		_prog = prog;
	} else {
		_midi = false;
		_fileName = Common::String(fileNameBuffer);
		_loop = loop == 1 ? true : false;

		// Volume is optional. If it doesn't appear, assume full volume
		if (volume != 255) {
			// Volume in the script files is mapped to [0, 100], but the ScummVM mixer uses [0, 255]
			_volume = volume * 255 / 100;
		}
	}
}

ActionMusic::~ActionMusic() {
	if (!_universe)
		_engine->getScriptManager()->killSideFx(_slotKey);
}

bool ActionMusic::execute() {
	if (_engine->getScriptManager()->getSideFX(_slotKey))
		return true;

	if (_midi) {
		_engine->getScriptManager()->addSideFX(new MusicMidiNode(_engine, _slotKey, _prog, _note, _volume));
	} else {
		if (!_engine->getSearchManager()->hasFile(_fileName))
			return true;

		_engine->getScriptManager()->addSideFX(new MusicNode(_engine, _slotKey, _fileName, _loop, _volume));
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionPanTrack
//////////////////////////////////////////////////////////////////////////////

ActionPanTrack::ActionPanTrack(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey),
	_pos(0),
	_musicSlot(0) {

	sscanf(line.c_str(), "%u %d", &_musicSlot, &_pos);
}

ActionPanTrack::~ActionPanTrack() {
	_engine->getScriptManager()->killSideFx(_slotKey);
}

bool ActionPanTrack::execute() {
	if (_engine->getScriptManager()->getSideFX(_slotKey))
		return true;

	_engine->getScriptManager()->addSideFX(new PanTrackNode(_engine, _slotKey, _musicSlot, _pos));

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionPreferences
//////////////////////////////////////////////////////////////////////////////

ActionPreferences::ActionPreferences(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
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

ActionPreloadAnimation::ActionPreloadAnimation(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_mask = 0;
	_framerate = 0;

	char fileName[25];

	// The two %*u are usually 0 and dont seem to have a use
	sscanf(line.c_str(), "%24s %*u %*u %d %d", fileName, &_mask, &_framerate);

	_fileName = Common::String(fileName);
}

ActionPreloadAnimation::~ActionPreloadAnimation() {
	_engine->getScriptManager()->deleteSideFx(_slotKey);
}

bool ActionPreloadAnimation::execute() {
	AnimationNode *nod = (AnimationNode *)_engine->getScriptManager()->getSideFX(_slotKey);

	if (!nod) {
		nod = new AnimationNode(_engine, _slotKey, _fileName, _mask, _framerate, false);
		_engine->getScriptManager()->addSideFX(nod);
	} else
		nod->stop();
	_engine->getScriptManager()->setStateValue(_slotKey, 2);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionUnloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionUnloadAnimation::ActionUnloadAnimation(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_key = 0;

	sscanf(line.c_str(), "%u", &_key);
}

bool ActionUnloadAnimation::execute() {
	AnimationNode *nod = (AnimationNode *)_engine->getScriptManager()->getSideFX(_key);

	if (nod && nod->getType() == SideFX::SIDEFX_ANIM)
		_engine->getScriptManager()->deleteSideFx(_key);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionPlayAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayAnimation::ActionPlayAnimation(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
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
	sscanf(line.c_str(),
	       "%24s %u %u %u %u %u %u %d %*u %*u %d %d",
	       fileName, &_x, &_y, &_x2, &_y2, &_start, &_end, &_loopCount, &_mask, &_framerate);

	_fileName = Common::String(fileName);
}

ActionPlayAnimation::~ActionPlayAnimation() {
	_engine->getScriptManager()->deleteSideFx(_slotKey);
}

bool ActionPlayAnimation::execute() {
	AnimationNode *nod = (AnimationNode *)_engine->getScriptManager()->getSideFX(_slotKey);

	if (!nod) {
		nod = new AnimationNode(_engine, _slotKey, _fileName, _mask, _framerate);
		_engine->getScriptManager()->addSideFX(nod);
	} else
		nod->stop();

	if (nod)
		nod->addPlayNode(_slotKey, _x, _y, _x2, _y2, _start, _end, _loopCount);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionPlayPreloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayPreloadAnimation::ActionPlayPreloadAnimation(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_controlKey = 0;
	_x1 = 0;
	_y1 = 0;
	_x2 = 0;
	_y2 = 0;
	_startFrame = 0;
	_endFrame = 0;
	_loopCount = 0;

	sscanf(line.c_str(),
	       "%u %u %u %u %u %u %u %u",
	       &_controlKey, &_x1, &_y1, &_x2, &_y2, &_startFrame, &_endFrame, &_loopCount);
}

bool ActionPlayPreloadAnimation::execute() {
	AnimationNode *nod = (AnimationNode *)_engine->getScriptManager()->getSideFX(_controlKey);

	if (nod)
		nod->addPlayNode(_slotKey, _x1, _y1, _x2, _y2, _startFrame, _endFrame, _loopCount);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionQuit
//////////////////////////////////////////////////////////////////////////////

bool ActionQuit::execute() {
	_engine->quitGame();

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionRegion - only used by Zork: Nemesis
//////////////////////////////////////////////////////////////////////////////

ActionRegion::ActionRegion(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_delay = 0;
	_type = 0;
	_unk1 = 0;
	_unk2 = 0;

	char art[64];
	char custom[64];

	int32 x1 = 0, x2 = 0, y1 = 0, y2 = 0;

	sscanf(line.c_str(), "%s %d %d %d %d %hu %hu %hu %hu %s", art, &x1, &y1, &x2, &y2, &_delay, &_type, &_unk1, &_unk2, custom);
	_art = Common::String(art);
	_custom = Common::String(custom);
	_rect = Common::Rect(x1, y1, x2 + 1, y2 + 1);
}

ActionRegion::~ActionRegion() {
	_engine->getScriptManager()->killSideFx(_slotKey);
}

bool ActionRegion::execute() {
	if (_engine->getScriptManager()->getSideFX(_slotKey))
		return true;

	Effect *effct = NULL;
	switch (_type) {
	case 0: {
		uint16 centerX, centerY, frames;
		double amplitude, waveln, speed;
		sscanf(_custom.c_str(), "%hu,%hu,%hu,%lf,%lf,%lf,", &centerX, &centerY, &frames, &amplitude, &waveln, &speed);
		effct = new WaveFx(_engine, _slotKey, _rect, _unk1, frames, centerX, centerY, amplitude, waveln, speed);
	}
	break;
	case 1: {
		uint16 aX, aY, aD;
		if (_engine->getRenderManager()->getRenderTable()->getRenderState() == RenderTable::PANORAMA)
			sscanf(_art.c_str(), "useart[%hu,%hu,%hu]", &aY, &aX, &aD);
		else
			sscanf(_art.c_str(), "useart[%hu,%hu,%hu]", &aX, &aY, &aD);
		int8 minD;
		int8 maxD;
		EffectMap *_map = _engine->getRenderManager()->makeEffectMap(Common::Point(aX, aY), aD, _rect, &minD, &maxD);
		effct = new LightFx(_engine, _slotKey, _rect, _unk1, _map, atoi(_custom.c_str()), minD, maxD);
	}
	break;
	case 9: {
		int16 dum1;
		int32 dum2;
		char buf[64];
		sscanf(_custom.c_str(), "%hd,%d,%s", &dum1, &dum2, buf);
		Graphics::Surface tempMask;
		_engine->getRenderManager()->readImageToSurface(_art, tempMask);
		if (_rect.width() != tempMask.w)
			_rect.setWidth(tempMask.w);
		if (_rect.height() != tempMask.h)
			_rect.setHeight(tempMask.h);

		EffectMap *_map = _engine->getRenderManager()->makeEffectMap(tempMask, 0);
		effct = new FogFx(_engine, _slotKey, _rect, _unk1, _map, Common::String(buf));
	}
	break;
	default:
		break;
	}

	if (effct) {
		_engine->getScriptManager()->addSideFX(new RegionNode(_engine, _slotKey, effct, _delay));
		_engine->getRenderManager()->addEffect(effct);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionRandom
//////////////////////////////////////////////////////////////////////////////

ActionRandom::ActionRandom(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	char maxBuffer[64];
	memset(maxBuffer, 0, 64);
	sscanf(line.c_str(), "%s", maxBuffer);
	_max = new ValueSlot(_engine->getScriptManager(), maxBuffer);
}

ActionRandom::~ActionRandom() {
	if (_max)
		delete _max;
}

bool ActionRandom::execute() {
	uint randNumber = _engine->getRandomSource()->getRandomNumber(_max->getValue());
	_engine->getScriptManager()->setStateValue(_slotKey, randNumber);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionRestoreGame
//////////////////////////////////////////////////////////////////////////////

ActionRestoreGame::ActionRestoreGame(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	char buf[128];
	sscanf(line.c_str(), "%s", buf);
	_fileName = Common::String(buf);
}

bool ActionRestoreGame::execute() {
	_engine->getSaveManager()->loadGame(_fileName);
	return false;
}

//////////////////////////////////////////////////////////////////////////////
// ActionRotateTo
//////////////////////////////////////////////////////////////////////////////

ActionRotateTo::ActionRotateTo(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_time = 0;
	_toPos = 0;

	sscanf(line.c_str(), "%d, %d", &_toPos, &_time);
}

bool ActionRotateTo::execute() {
	_engine->rotateTo(_toPos, _time);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionSetPartialScreen
//////////////////////////////////////////////////////////////////////////////

ActionSetPartialScreen::ActionSetPartialScreen(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_x = 0;
	_y = 0;

	char fileName[25];

	sscanf(line.c_str(), "%u %u %24s %*u %d", &_x, &_y, fileName, &_backgroundColor);

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

ActionSetScreen::ActionSetScreen(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	char fileName[25];
	sscanf(line.c_str(), "%24s", fileName);

	_fileName = Common::String(fileName);
}

bool ActionSetScreen::execute() {
	_engine->getRenderManager()->setBackgroundImage(_fileName);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionSetVenus
//////////////////////////////////////////////////////////////////////////////

ActionSetVenus::ActionSetVenus(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_key = 0;

	sscanf(line.c_str(), "%d", &_key);
}

bool ActionSetVenus::execute() {
	if (_engine->getScriptManager()->getStateValue(_key))
		_engine->getScriptManager()->setStateValue(StateKey_Venus, _key);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionStop
//////////////////////////////////////////////////////////////////////////////

ActionStop::ActionStop(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_key = 0;
	sscanf(line.c_str(), "%u", &_key);
}

bool ActionStop::execute() {
	_engine->getScriptManager()->stopSideFx(_key);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionStreamVideo
//////////////////////////////////////////////////////////////////////////////

ActionStreamVideo::ActionStreamVideo(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_x1 = 0;
	_x2 = 0;
	_y1 = 0;
	_y2 = 0;
	_flags = 0;

	char fileName[25];
	uint skipline = 0;    //skipline - render video with skip every second line, not skippable.

	sscanf(line.c_str(), "%24s %u %u %u %u %u %u", fileName, &_x1, &_y1, &_x2, &_y2, &_flags, &skipline);

	_fileName = Common::String(fileName);
	_skippable = true;
}

bool ActionStreamVideo::execute() {
	ZorkAVIDecoder decoder;
	Common::File *_file = _engine->getSearchManager()->openFile(_fileName);

	if (_file) {
		if (!decoder.loadStream(_file)) {
			return true;
		}

		_engine->getCursorManager()->showMouse(false);

		Common::Rect destRect = Common::Rect(_x1, _y1, _x2 + 1, _y2 + 1);

		Common::String subname = _fileName;
		subname.setChar('s', subname.size() - 3);
		subname.setChar('u', subname.size() - 2);
		subname.setChar('b', subname.size() - 1);

		Subtitle *sub = NULL;

		if (_engine->getSearchManager()->hasFile(subname))
			sub = new Subtitle(_engine, subname);

		_engine->playVideo(decoder, destRect, _skippable, sub);

		_engine->getCursorManager()->showMouse(true);

		if (sub)
			delete sub;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionSyncSound
//////////////////////////////////////////////////////////////////////////////

ActionSyncSound::ActionSyncSound(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_syncto = 0;

	char fileName[25];
	int notUsed = 0;

	sscanf(line.c_str(), "%d %d %24s", &_syncto, &notUsed, fileName);

	_fileName = Common::String(fileName);
}

bool ActionSyncSound::execute() {
	SideFX *fx = _engine->getScriptManager()->getSideFX(_syncto);
	if (!fx)
		return true;

	if (!(fx->getType() & SideFX::SIDEFX_ANIM))
		return true;

	_engine->getScriptManager()->addSideFX(new SyncSoundNode(_engine, _slotKey, _fileName, _syncto));
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionTimer
//////////////////////////////////////////////////////////////////////////////

ActionTimer::ActionTimer(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	char timeBuffer[64];
	memset(timeBuffer, 0, 64);
	sscanf(line.c_str(), "%s", timeBuffer);
	_time = new ValueSlot(_engine->getScriptManager(), timeBuffer);
}

ActionTimer::~ActionTimer() {
	if (_time)
		delete _time;
	_engine->getScriptManager()->killSideFx(_slotKey);
}

bool ActionTimer::execute() {
	if (_engine->getScriptManager()->getSideFX(_slotKey))
		return true;
	_engine->getScriptManager()->addSideFX(new TimerNode(_engine, _slotKey, _time->getValue()));
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionTtyText
//////////////////////////////////////////////////////////////////////////////

ActionTtyText::ActionTtyText(ZVision *engine, int32 slotkey, const Common::String &line) :
	ResultAction(engine, slotkey) {
	_delay = 0;

	char filename[64];
	int32 x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	sscanf(line.c_str(), "%d %d %d %d %64s %u", &x1, &y1, &x2, &y2, filename, &_delay);
	_r = Common::Rect(x1, y1, x2, y2);
	_filename = Common::String(filename);
}

ActionTtyText::~ActionTtyText() {
	_engine->getScriptManager()->killSideFx(_slotKey);
}

bool ActionTtyText::execute() {
	if (_engine->getScriptManager()->getSideFX(_slotKey))
		return true;
	_engine->getScriptManager()->addSideFX(new ttyTextNode(_engine, _slotKey, _filename, _r, _delay));
	return true;
}

} // End of namespace ZVision
