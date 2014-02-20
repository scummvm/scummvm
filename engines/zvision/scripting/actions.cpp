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

#include "common/scummsys.h"

#include "zvision/scripting/actions.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/sound/zork_raw.h"
#include "zvision/video/zork_avi_decoder.h"
#include "zvision/scripting/controls/timer_node.h"
#include "zvision/scripting/controls/animation_control.h"

#include "common/file.h"

#include "audio/decoders/wave.h"


namespace ZVision {

//////////////////////////////////////////////////////////////////////////////
// ActionAdd
//////////////////////////////////////////////////////////////////////////////

ActionAdd::ActionAdd(const Common::String &line) {
	sscanf(line.c_str(), "%*[^(](%u,%u)", &_key, &_value);
}

bool ActionAdd::execute(ZVision *engine) {
	engine->getScriptManager()->addToStateValue(_key, _value);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionAssign
//////////////////////////////////////////////////////////////////////////////

ActionAssign::ActionAssign(const Common::String &line) {
	sscanf(line.c_str(), "%*[^(](%u, %u)", &_key, &_value);
}

bool ActionAssign::execute(ZVision *engine) {
	engine->getScriptManager()->setStateValue(_key, _value);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionAttenuate
//////////////////////////////////////////////////////////////////////////////

ActionAttenuate::ActionAttenuate(const Common::String &line) {
	sscanf(line.c_str(), "%*[^(](%u, %d)", &_key, &_attenuation);
}

bool ActionAttenuate::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionChangeLocation
//////////////////////////////////////////////////////////////////////////////

ActionChangeLocation::ActionChangeLocation(const Common::String &line) {
	sscanf(line.c_str(), "%*[^(](%c,%c,%c%c,%u)", &_world, &_room, &_node, &_view, &_offset);
}

bool ActionChangeLocation::execute(ZVision *engine) {
	// We can't directly call ScriptManager::ChangeLocationIntern() because doing so clears all the Puzzles, and thus would corrupt the current puzzle checking
	engine->getScriptManager()->changeLocation(_world, _room, _node, _view, _offset);
	// Tell the puzzle system to stop checking any more puzzles
	return false;
}


//////////////////////////////////////////////////////////////////////////////
// ActionCrossfade
//////////////////////////////////////////////////////////////////////////////

ActionCrossfade::ActionCrossfade(const Common::String &line) {
	sscanf(line.c_str(),
           "%*[^(](%u %u %u %u %u %u %u)",
           &_keyOne, &_keyTwo, &_oneStartVolume, &_twoStartVolume, &_oneEndVolume, &_twoEndVolume, &_timeInMillis);
}

bool ActionCrossfade::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionDisableControl
//////////////////////////////////////////////////////////////////////////////

ActionDisableControl::ActionDisableControl(const Common::String &line) {
	sscanf(line.c_str(), "%*[^(](%u)", &_key);
}

bool ActionDisableControl::execute(ZVision *engine) {
	debug("Disabling control %u", _key);
	
	ScriptManager *scriptManager = engine->getScriptManager();
	scriptManager->setStateFlags(_key, scriptManager->getStateFlags(_key) | ScriptManager::DISABLED);

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionEnableControl
//////////////////////////////////////////////////////////////////////////////

ActionEnableControl::ActionEnableControl(const Common::String &line) {
	sscanf(line.c_str(), "%*[^(](%u)", &_key);
}

bool ActionEnableControl::execute(ZVision *engine) {
	debug("Enabling control %u", _key);

	ScriptManager *scriptManager = engine->getScriptManager();
	scriptManager->setStateFlags(_key, scriptManager->getStateFlags(_key) & ~ScriptManager::DISABLED);

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionMusic
//////////////////////////////////////////////////////////////////////////////

ActionMusic::ActionMusic(const Common::String &line) : _volume(255) {
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

bool ActionMusic::execute(ZVision *engine) {
	Audio::RewindableAudioStream *audioStream;

	if (_fileName.contains(".wav")) {
		Common::File *file = new Common::File();
		if (file->open(_fileName)) {
			audioStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
		} else {
			warning("Unable to open %s", _fileName.c_str());
			return false;
		}
	} else {
		audioStream = makeRawZorkStream(_fileName, engine);
	}
	
	if (_loop) {
		Audio::LoopingAudioStream *loopingAudioStream = new Audio::LoopingAudioStream(audioStream, 0, DisposeAfterUse::YES);
		engine->_mixer->playStream(_soundType, 0, loopingAudioStream, -1, _volume);
	} else {
		engine->_mixer->playStream(_soundType, 0, audioStream, -1, _volume);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPreloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPreloadAnimation::ActionPreloadAnimation(const Common::String &line) {
	char fileName[25];

	// The two %*u are always 0 and dont seem to have a use
	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u(%25s %*u %*u %u %u)", &_key, fileName, &_mask, &_framerate);

	_fileName = Common::String(fileName);
}

bool ActionPreloadAnimation::execute(ZVision *engine) {
	// TODO: We ignore the mask and framerate atm. Mask refers to a key color used for binary alpha. We assume the framerate is the default framerate embedded in the videos

	// TODO: Check if the Control already exists

	// Create the control, but disable it until PlayPreload is called
	ScriptManager *scriptManager = engine->getScriptManager();
	scriptManager->addControl(new AnimationControl(engine, _key, _fileName));
	scriptManager->setStateFlags(_key, scriptManager->getStateFlags(_key) | ScriptManager::DISABLED);

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPlayAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayAnimation::ActionPlayAnimation(const Common::String &line) {
	char fileName[25];

	// The two %*u are always 0 and dont seem to have a use
	sscanf(line.c_str(),
           "%*[^:]:%*[^:]:%u(%25s %u %u %u %u %u %u %u %*u %*u %u %u)",
           &_key, fileName, &_x, &_y, &_width, &_height, &_start, &_end, &_loopCount, &_mask, &_framerate);

	_fileName = Common::String(fileName);
}

bool ActionPlayAnimation::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPlayPreloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayPreloadAnimation::ActionPlayPreloadAnimation(const Common::String &line) {
	sscanf(line.c_str(),
		   "%*[^:]:%*[^:]:%u(%u %u %u %u %u %u %u %u)",
		   &_animationKey, &_controlKey, &_x1, &_y1, &_x2, &_y2, &_startFrame, &_endFrame, &_loopCount);
}

bool ActionPlayPreloadAnimation::execute(ZVision *engine) {
	// Find the control
	AnimationControl *control = (AnimationControl *)engine->getScriptManager()->getControl(_controlKey);

	// Set the needed values within the control
	control->setAnimationKey(_animationKey);
	control->setLoopCount(_loopCount);
	control->setXPos(_x1);
	control->setYPost(_y1);

	// Enable the control. ScriptManager will take care of the rest
	control->enable();

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionQuit
//////////////////////////////////////////////////////////////////////////////

bool ActionQuit::execute(ZVision *engine) {
	engine->quitGame();

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionRandom
//////////////////////////////////////////////////////////////////////////////

ActionRandom::ActionRandom(const Common::String &line) {
	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u, %u)", &_key, &_max);
}

bool ActionRandom::execute(ZVision *engine) {
	uint randNumber = engine->getRandomSource()->getRandomNumber(_max);
	engine->getScriptManager()->setStateValue(_key, randNumber);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionSetPartialScreen
//////////////////////////////////////////////////////////////////////////////

ActionSetPartialScreen::ActionSetPartialScreen(const Common::String &line) {
	char fileName[25];
	uint color;

	sscanf(line.c_str(), "%*[^(](%u %u %25s %*u %u)", &_x, &_y, fileName, &color);

	_fileName = Common::String(fileName);

	if (color > 0xFFFF) {
		warning("Background color for ActionSetPartialScreen is bigger than a uint16");
	}
	_backgroundColor = color;
}

bool ActionSetPartialScreen::execute(ZVision *engine) {
	RenderManager *renderManager = engine->getRenderManager();
	
	if (_backgroundColor > 0) {
		renderManager->clearWorkingWindowTo555Color(_backgroundColor);
	}
	renderManager->renderImageToScreen(_fileName, _x, _y);

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionSetScreen
//////////////////////////////////////////////////////////////////////////////

ActionSetScreen::ActionSetScreen(const Common::String &line) {
	char fileName[25];
	sscanf(line.c_str(), "%*[^(](%25[^)])", fileName);

	_fileName = Common::String(fileName);
}

bool ActionSetScreen::execute(ZVision *engine) {
	engine->getRenderManager()->setBackgroundImage(_fileName);

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionStreamVideo
//////////////////////////////////////////////////////////////////////////////

ActionStreamVideo::ActionStreamVideo(const Common::String &line) {
	char fileName[25];
	uint skippable;

	sscanf(line.c_str(), "%*[^(](%25s %u %u %u %u %u %u)", fileName, &_x1, &_y1, &_x2, &_y2, &_flags, &skippable);

	_fileName = Common::String(fileName);
	_skippable = (skippable == 0) ? false : true;
}

bool ActionStreamVideo::execute(ZVision *engine) {
	ZorkAVIDecoder decoder;
	if (!decoder.loadFile(_fileName)) {
		return true;
	}

	Common::Rect destRect;
	if ((_flags & DIFFERENT_DIMENSIONS) == DIFFERENT_DIMENSIONS) {
		destRect = Common::Rect(_x1, _y1, _x2, _y2);
	}

	engine->playVideo(decoder, destRect, _skippable);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionTimer
//////////////////////////////////////////////////////////////////////////////

ActionTimer::ActionTimer(const Common::String &line) {
	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u(%u)", &_key, &_time);
}

bool ActionTimer::execute(ZVision *engine) {
	engine->getScriptManager()->addControl(new TimerNode(engine, _key, _time));
	return true;
}

} // End of namespace ZVision
