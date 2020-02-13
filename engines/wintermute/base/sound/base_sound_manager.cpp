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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/sound/base_sound_buffer.h"
#include "engines/wintermute/wintermute.h"
#include "common/config-manager.h"
#include "audio/mixer.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//IMPLEMENT_PERSISTENT(BaseSoundMgr, true);

//////////////////////////////////////////////////////////////////////////
BaseSoundMgr::BaseSoundMgr(BaseGame *inGame) : BaseClass(inGame) {
	_soundAvailable = false;
	_volumeMaster = 255;
	_volumeMasterPercent = 100;
}


//////////////////////////////////////////////////////////////////////////
BaseSoundMgr::~BaseSoundMgr() {
	saveSettings();
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::cleanup() {
	for (uint32 i = 0; i < _sounds.size(); i++) {
		delete _sounds[i];
	}
	_sounds.clear();
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseSoundMgr::saveSettings() {
	if (_soundAvailable) {
		ConfMan.setInt("master_volume_percent", _volumeMasterPercent);
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::initialize() {
	_soundAvailable = false;

	if (!g_system->getMixer()->isReady()) {
		return STATUS_FAILED;
	}
	byte volumeMasterPercent = (ConfMan.hasKey("master_volume_percent") ? ConfMan.getInt("master_volume_percent") : 100);
	setMasterVolumePercent(volumeMasterPercent);
	_soundAvailable = true;

	g_engine->syncSoundSettings();
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
BaseSoundBuffer *BaseSoundMgr::addSound(const Common::String &filename, Audio::Mixer::SoundType type, bool streamed) {
	if (!_soundAvailable) {
		return nullptr;
	}

	if (filename.empty()) {
		// At least one game, Bickadoodle, calls playSound with an empty filename, see #6594
		BaseEngine::LOG(0, "addSound called with empty filename");
	}

	BaseSoundBuffer *sound;

	Common::String useFilename = filename;
	useFilename.toLowercase();
	// try to switch WAV to OGG file (if available)
	if (useFilename.hasSuffix(".wav")) {
		Common::String oggFilename = useFilename;
		oggFilename.erase(oggFilename.size() - 4);
		oggFilename = oggFilename + ".ogg";
		if (BaseFileManager::getEngineInstance()->hasFile(oggFilename)) {
			useFilename = oggFilename;
		}
	}

	sound = new BaseSoundBuffer(_gameRef);
	if (!sound) {
		return nullptr;
	}

	sound->setStreaming(streamed);
	sound->setType(type);


	bool res = sound->loadFromFile(useFilename);
	if (DID_FAIL(res)) {
		BaseEngine::LOG(res, "Error loading sound '%s'", useFilename.c_str());
		delete sound;
		return nullptr;
	}

	// Make sure the master-volume is applied to the sound.
	sound->updateVolume();

	// register sound
	_sounds.push_back(sound);

	return sound;

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::addSound(BaseSoundBuffer *sound, Audio::Mixer::SoundType type) {
	if (!sound) {
		return STATUS_FAILED;
	}

	// Make sure the master-volume is applied to the sound.
	sound->updateVolume();

	// register sound
	_sounds.push_back(sound);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::removeSound(BaseSoundBuffer *sound) {
	for (uint32 i = 0; i < _sounds.size(); i++) {
		if (_sounds[i] == sound) {
			delete _sounds[i];
			_sounds.remove_at(i);
			return STATUS_OK;
		}
	}

	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setVolume(Audio::Mixer::SoundType type, int volume) {
	if (!_soundAvailable) {
		return STATUS_OK;
	}

	switch (type) {
	case Audio::Mixer::kSFXSoundType:
		ConfMan.setInt("sfx_volume", volume);
		break;
	case Audio::Mixer::kSpeechSoundType:
		ConfMan.setInt("speech_volume", volume);
		break;
	case Audio::Mixer::kMusicSoundType:
		ConfMan.setInt("music_volume", volume);
		break;
	case Audio::Mixer::kPlainSoundType:
		error("Plain sound type shouldn't be used in WME");
		break;
	default:
		break;
	}
	g_engine->syncSoundSettings();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setVolumePercent(Audio::Mixer::SoundType type, byte percent) {
	return setVolume(type, percent * 255 / 100);
}


//////////////////////////////////////////////////////////////////////////
byte BaseSoundMgr::getVolumePercent(Audio::Mixer::SoundType type) {
	int volume = 0;

	switch (type) {
	case Audio::Mixer::kSFXSoundType:
	case Audio::Mixer::kSpeechSoundType:
	case Audio::Mixer::kMusicSoundType:
		volume = g_system->getMixer()->getVolumeForSoundType(type);
		break;
	default:
		error("Sound-type not set");
		break;
	}

	return (byte)(volume * 100 / 255);
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setMasterVolume(byte value) {
	// This function intentionally doesn't touch _volumeMasterPercent,
	// as that variable keeps track of what the game actually wanted,
	// and this gives a close approximation, while letting the game
	// be none the wiser about round-off-errors. This function should thus
	// ONLY be called by setMasterVolumePercent.
	_volumeMaster = value;
	for (uint32 i = 0; i < _sounds.size(); i++) {
		_sounds[i]->updateVolume();
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setMasterVolumePercent(byte percent) {
	_volumeMasterPercent = percent;
	setMasterVolume((int)ceil(percent * 255.0 / 100.0));
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
byte BaseSoundMgr::getMasterVolumePercent() {
	return _volumeMasterPercent;
}

//////////////////////////////////////////////////////////////////////////
byte BaseSoundMgr::getMasterVolume() {
	return (byte)_volumeMaster;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::pauseAll(bool includingMusic) {

	for (uint32 i = 0; i < _sounds.size(); i++) {
		if (_sounds[i]->isPlaying() && (_sounds[i]->getType() != Audio::Mixer::kMusicSoundType || includingMusic)) {
			_sounds[i]->pause();
			_sounds[i]->setFreezePaused(true);
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::resumeAll() {

	for (uint32 i = 0; i < _sounds.size(); i++) {
		if (_sounds[i]->isFreezePaused()) {
			_sounds[i]->resume();
			_sounds[i]->setFreezePaused(false);
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
float BaseSoundMgr::posToPan(int x, int y) {
	float relPos = (float)x / ((float)BaseEngine::getRenderer()->getWidth());

	float minPan = -0.7f;
	float maxPan = 0.7f;

	return minPan + relPos * (maxPan - minPan);
}

} // End of namespace Wintermute
