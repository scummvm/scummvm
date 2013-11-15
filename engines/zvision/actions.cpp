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

#include "zvision/actions.h"

#include "zvision/zvision.h"
#include "zvision/script_manager.h"
#include "zvision/render_manager.h"
#include "zvision/zork_raw.h"
#include "zvision/zork_avi_decoder.h"
#include "zvision/timer_node.h"
#include "zvision/music_node.h"
#include "zvision/animation_node.h"

#include "common/file.h"

#include "audio/decoders/wave.h"


namespace ZVision {

//////////////////////////////////////////////////////////////////////////////
// ActionAdd
//////////////////////////////////////////////////////////////////////////////

ActionAdd::ActionAdd(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	sscanf(line.c_str(), "%*[^(](%u,%d)", &_key, &_value);
}

bool ActionAdd::execute() {
	_engine->getScriptManager()->setStateValue(_key, _engine->getScriptManager()->getStateValue(_key) + _value);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionAssign
//////////////////////////////////////////////////////////////////////////////

ActionAssign::ActionAssign(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	char buf[64];
	memset(buf, 0, 64);
	sscanf(line.c_str(), "%*[^(](%u, %s)", &_key, buf);
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

ActionAttenuate::ActionAttenuate(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	sscanf(line.c_str(), "%*[^(](%u, %d)", &_key, &_attenuation);
}

bool ActionAttenuate::execute() {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionChangeLocation
//////////////////////////////////////////////////////////////////////////////

ActionChangeLocation::ActionChangeLocation(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	sscanf(line.c_str(), "%*[^(](%c, %c, %c%c, %u)", &_world, &_room, &_node, &_view, &_offset);
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

ActionCrossfade::ActionCrossfade(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	sscanf(line.c_str(),
	       "%*[^(](%u %u %u %u %u %u %u)",
	       &_keyOne, &_keyTwo, &_oneStartVolume, &_twoStartVolume, &_oneEndVolume, &_twoEndVolume, &_timeInMillis);
}

bool ActionCrossfade::execute() {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionDisableControl
//////////////////////////////////////////////////////////////////////////////

ActionDisableControl::ActionDisableControl(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	sscanf(line.c_str(), "%*[^(](%u)", &_key);
}

bool ActionDisableControl::execute() {
	_engine->getScriptManager()->setStateFlag(_key, Puzzle::DISABLED);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionEnableControl
//////////////////////////////////////////////////////////////////////////////

ActionEnableControl::ActionEnableControl(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	sscanf(line.c_str(), "%*[^(](%u)", &_key);
}

bool ActionEnableControl::execute() {
	_engine->getScriptManager()->unsetStateFlag(_key, Puzzle::DISABLED);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionKill
//////////////////////////////////////////////////////////////////////////////

ActionKill::ActionKill(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	_key = 0;
	_type = 0;
	char keytype[25];
	sscanf(line.c_str(), "%*[^(](%25s)", keytype);
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
// ActionMusic
//////////////////////////////////////////////////////////////////////////////

ActionMusic::ActionMusic(ZVision *engine, const Common::String &line, bool global) :
	ResultAction(engine),
	_volume(255),
	_universe(global) {
	uint type;
	char fileNameBuffer[25];
	uint loop;
	uint volume = 255;

	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u(%u %25s %u %u)", &_key, &type, fileNameBuffer, &loop, &volume);

	// type 4 are midi sound effect files
	if (type == 4) {
		_soundType = Audio::Mixer::kSFXSoundType;
		_fileName = Common::String::format("midi/%s/%u.wav", fileNameBuffer, loop);
		_loop = false;
	} else {
		// TODO: See what the other types are so we can specify the correct Mixer::SoundType. In the meantime use kPlainSoundType
		_soundType = Audio::Mixer::kPlainSoundType;
		_fileName = Common::String(fileNameBuffer);
		_loop = loop == 1 ? true : false;
	}

	// Volume is optional. If it doesn't appear, assume full volume
	if (volume != 255) {
		// Volume in the script files is mapped to [0, 100], but the ScummVM mixer uses [0, 255]
		_volume = volume * 255 / 100;
	}
}

ActionMusic::~ActionMusic() {
	if (!_universe)
		_engine->getScriptManager()->killSideFx(_key);
}

bool ActionMusic::execute() {
	if (_engine->getScriptManager()->getSideFX(_key))
		return true;
	Common::File *file = new Common::File();
	if (!file->exists(_fileName) && _fileName.size() >= 12) {
		_fileName.setChar('r', 9);
		_fileName.setChar('a', 10);
		_fileName.setChar('w', 11);
		if (!file->exists(_fileName)) {
			_fileName.setChar('i', 9);
			_fileName.setChar('f', 10);
			_fileName.setChar('p', 11);
			if (!file->exists(_fileName)) {
				_fileName.setChar('s', 9);
				_fileName.setChar('r', 10);
				_fileName.setChar('c', 11);
				if (!file->exists(_fileName))
					return true;
			}
		}
	}
	if (file->exists(_fileName))
		_engine->getScriptManager()->addSideFX(new MusicNode(_engine, _key, _fileName, _loop, _volume));
	delete file;

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPreloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPreloadAnimation::ActionPreloadAnimation(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	char fileName[25];

	// The two %*u are always 0 and dont seem to have a use
	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u(%25s %*u %*u %d %d)", &_key, fileName, &_mask, &_framerate);

	if (_mask > 0) {
		byte r, g, b;
		Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0).colorToRGB(_mask, r, g, b);
		_mask = _engine->_pixelFormat.RGBToColor(r, g, b);
	}

	_fileName = Common::String(fileName);
}

ActionPreloadAnimation::~ActionPreloadAnimation() {
	_engine->getScriptManager()->deleteSideFx(_key);
}

bool ActionPreloadAnimation::execute() {
	AnimationNode *nod = (AnimationNode *)_engine->getScriptManager()->getSideFX(_key);

	if (!nod) {
		nod = new AnimationNode(_engine, _key, _fileName, _mask, _framerate, false);
		_engine->getScriptManager()->addSideFX(nod);
	} else
		nod->stop();
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPlayAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayAnimation::ActionPlayAnimation(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	char fileName[25];

	// The two %*u are always 0 and dont seem to have a use
	sscanf(line.c_str(),
	       "%*[^:]:%*[^:]:%u(%25s %u %u %u %u %u %u %d %*u %*u %d %d)",
	       &_key, fileName, &_x, &_y, &_width, &_height, &_start, &_end, &_loopCount, &_mask, &_framerate);

	if (_mask > 0) {
		byte r, g, b;
		Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0).colorToRGB(_mask, r, g, b);
		_mask = _engine->_pixelFormat.RGBToColor(r, g, b);
	}

	_fileName = Common::String(fileName);
}

ActionPlayAnimation::~ActionPlayAnimation() {
	_engine->getScriptManager()->deleteSideFx(_key);
}

bool ActionPlayAnimation::execute() {
	AnimationNode *nod = (AnimationNode *)_engine->getScriptManager()->getSideFX(_key);

	if (!nod) {
		nod = new AnimationNode(_engine, _key, _fileName, _mask, _framerate);
		_engine->getScriptManager()->addSideFX(nod);
	} else
		nod->stop();

	if (nod)
		nod->addPlayNode(_key, _x, _y, _width, _height, _start, _end, _loopCount);

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPlayPreloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayPreloadAnimation::ActionPlayPreloadAnimation(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	sscanf(line.c_str(),
	       "%*[^:]:%*[^:]:%u(%u %u %u %u %u %u %u %u)",
	       &_animationKey, &_controlKey, &_x1, &_y1, &_x2, &_y2, &_startFrame, &_endFrame, &_loopCount);
}

bool ActionPlayPreloadAnimation::execute() {
	AnimationNode *nod = (AnimationNode *)_engine->getScriptManager()->getSideFX(_animationKey);

	if (nod)
		nod->addPlayNode(_controlKey, _x1, _y1, _x2, _y2, _startFrame, _endFrame, _loopCount);

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
// ActionRandom
//////////////////////////////////////////////////////////////////////////////

ActionRandom::ActionRandom(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	char max_buf[64];
	memset(max_buf, 0, 64);
	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u(%s)", &_key, max_buf);
	_max = new ValueSlot(_engine->getScriptManager(), max_buf);
}

ActionRandom::~ActionRandom() {
	if (_max)
		delete _max;
}

bool ActionRandom::execute() {
	uint randNumber = _engine->getRandomSource()->getRandomNumber(_max->getValue());
	_engine->getScriptManager()->setStateValue(_key, randNumber);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionSetPartialScreen
//////////////////////////////////////////////////////////////////////////////

ActionSetPartialScreen::ActionSetPartialScreen(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	char fileName[25];
	int color;

	sscanf(line.c_str(), "%*[^(](%u %u %25s %*u %d)", &_x, &_y, fileName, &color);

	_fileName = Common::String(fileName);

	if (color >= 0) {
		byte r, g, b;
		Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0).colorToRGB(color, r, g, b);
		_backgroundColor = _engine->_pixelFormat.RGBToColor(r, g, b);
	} else {
		_backgroundColor = color;
	}

	if (color > 65535) {
		warning("Background color for ActionSetPartialScreen is bigger than a uint16");
	}
}

bool ActionSetPartialScreen::execute() {
	RenderManager *renderManager = _engine->getRenderManager();
	if (_backgroundColor >= 0)
		renderManager->renderImageToBackground(_fileName, _x, _y, _backgroundColor);
	else
		renderManager->renderImageToBackground(_fileName, _x, _y);

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionSetScreen
//////////////////////////////////////////////////////////////////////////////

ActionSetScreen::ActionSetScreen(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	char fileName[25];
	sscanf(line.c_str(), "%*[^(](%25[^)])", fileName);

	_fileName = Common::String(fileName);
}

bool ActionSetScreen::execute() {
	_engine->getRenderManager()->setBackgroundImage(_fileName);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// ActionStop
//////////////////////////////////////////////////////////////////////////////

ActionStop::ActionStop(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	_key = 0;
	sscanf(line.c_str(), "%*[^(](%u)", &_key);
}

bool ActionStop::execute() {
	_engine->getScriptManager()->stopSideFx(_key);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionStreamVideo
//////////////////////////////////////////////////////////////////////////////

ActionStreamVideo::ActionStreamVideo(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	char fileName[25];
	uint skipline;    //skipline - render video with skip every second line, not skippable.

	sscanf(line.c_str(), "%*[^(](%25s %u %u %u %u %u %u)", fileName, &_x1, &_y1, &_x2, &_y2, &_flags, &skipline);

	_fileName = Common::String(fileName);
	_skippable = true;
}

bool ActionStreamVideo::execute() {
	ZorkAVIDecoder decoder;
	if (!decoder.loadFile(_fileName)) {
		return true;
	}

	Common::Rect destRect;
	if ((_flags & DIFFERENT_DIMENSIONS) == DIFFERENT_DIMENSIONS) {
		destRect = Common::Rect(_x1, _y1, _x2, _y2);
	}

	_engine->playVideo(decoder, destRect, _skippable);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionTimer
//////////////////////////////////////////////////////////////////////////////

ActionTimer::ActionTimer(ZVision *engine, const Common::String &line) :
	ResultAction(engine) {
	char time_buf[64];
	memset(time_buf, 0, 64);
	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u(%s)", &_key, time_buf);
	_time = new ValueSlot(_engine->getScriptManager(), time_buf);
}

ActionTimer::~ActionTimer() {
	if (_time)
		delete _time;
	_engine->getScriptManager()->killSideFx(_key);
}

bool ActionTimer::execute() {
	_engine->getScriptManager()->addSideFX(new TimerNode(_engine, _key, _time->getValue()));
	return true;
}

} // End of namespace ZVision
