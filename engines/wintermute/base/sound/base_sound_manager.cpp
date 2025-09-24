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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/base/base_game.h"
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

//IMPLEMENT_PERSISTENT(BaseSoundMgr, true)

//////////////////////////////////////////////////////////////////////////
BaseSoundMgr::BaseSoundMgr(BaseGame *inGame) : BaseClass(inGame) {
	_soundAvailable = false;

	_volumeMaster = 100;
}


//////////////////////////////////////////////////////////////////////////
BaseSoundMgr::~BaseSoundMgr() {
	saveSettings();
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::cleanup() {
	for (int32 i = 0; i < _sounds.getSize(); i++) {
		delete _sounds[i];
	}
	_sounds.removeAll();
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseSoundMgr::saveSettings() {
	if (_soundAvailable) {
		ConfMan.setInt("master_volume_percent", _volumeMaster);
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::initialize() {
	_soundAvailable = false;

	if (!g_system->getMixer()->isReady()) {
		return STATUS_FAILED;
	}

	g_engine->syncSoundSettings();

	_volumeMaster = (ConfMan.hasKey("master_volume_percent") ? ConfMan.getInt("master_volume_percent") : 100);

	_soundAvailable = true;

	setMasterVolumePercent(_volumeMaster);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::initLoop() {
	if (!_soundAvailable) {
		return STATUS_OK;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
BaseSoundBuffer *BaseSoundMgr::addSound(const char *filename, TSoundType type, bool streamed, uint32 initialPrivateVolume) {
	if (!_soundAvailable) {
		return nullptr;
	}

	if (!filename || filename[0] == '\0') {
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
		if (_game->_fileManager->hasFile(oggFilename)) {
			useFilename = oggFilename;
		}
	}

	sound = new BaseSoundBuffer(_game);
	if (!sound) {
		return nullptr;
	}

	sound->setStreaming(streamed);
	sound->setType(type);


	bool res = sound->loadFromFile(useFilename.c_str());
	if (DID_FAIL(res)) {
		_game->LOG(res, "Error loading sound '%s'", useFilename.c_str());
		delete sound;
		return nullptr;
	}

	// sound starts with user defined instead of 100% volume (of the global setting)
	sound->setPrivateVolume(initialPrivateVolume);

	// register sound
	_sounds.add(sound);

	return sound;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::addSound(BaseSoundBuffer *sound, TSoundType type) {
	if (!sound) {
		return STATUS_FAILED;
	}

	// register sound
	_sounds.add(sound);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::removeSound(BaseSoundBuffer *sound) {
	for (int32 i = 0; i < _sounds.getSize(); i++) {
		if (_sounds[i] == sound) {
			delete _sounds[i];
			_sounds.removeAt(i);
			return STATUS_OK;
		}
	}

	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setVolume(TSoundType type, int volume) {
	if (!_soundAvailable) {
		return STATUS_OK;
	}

	switch (type) {
	case TSoundType::SOUND_SFX:
		g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, (volume * Audio::Mixer::kMaxChannelVolume) / 100);
		ConfMan.setInt("sfx_volume", g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSFXSoundType));
		break;
	case TSoundType::SOUND_SPEECH:
		g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, (volume * Audio::Mixer::kMaxChannelVolume) / 100);
		ConfMan.setInt("speech_volume", g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType));
		break;
	case TSoundType::SOUND_MUSIC:
		g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, (volume * Audio::Mixer::kMaxChannelVolume) / 100);
		ConfMan.setInt("music_volume", g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType));
		break;
	default:
		break;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setVolumePercent(TSoundType type, byte percent) {
	return setVolume(type, percent);
}


//////////////////////////////////////////////////////////////////////////
byte BaseSoundMgr::getVolumePercent(TSoundType type) {
	int volume = 0;

	switch (type) {
	case TSoundType::SOUND_SFX:
		volume = ConfMan.getInt("sfx_volume") * 100 / Audio::Mixer::kMaxChannelVolume;
		break;
	case TSoundType::SOUND_SPEECH:
		volume = ConfMan.getInt("speech_volume") * 100 / Audio::Mixer::kMaxChannelVolume;
		break;
	case TSoundType::SOUND_MUSIC:
		volume = ConfMan.getInt("music_volume") * 100 / Audio::Mixer::kMaxChannelVolume;
		break;
	default:
		break;
	}

	return (byte)volume;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setMasterVolumePercent(byte percent) {
	_volumeMaster = percent;
	for (int32 i = 0; i < _sounds.getSize(); i++) {
		_sounds[i]->setVolume(percent);
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
byte BaseSoundMgr::getMasterVolumePercent() {
	return _volumeMaster;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::pauseAll(bool includingMusic) {
	for (int32 i = 0; i < _sounds.getSize(); i++) {
		if (_sounds[i]->isPlaying() && (_sounds[i]->_type != TSoundType::SOUND_MUSIC || includingMusic)) {
			_sounds[i]->pause();
			_sounds[i]->_freezePaused = true;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::resumeAll() {

	for (int32 i = 0; i < _sounds.getSize(); i++) {
		if (_sounds[i]->_freezePaused) {
			_sounds[i]->resume();
			_sounds[i]->_freezePaused = false;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
float BaseSoundMgr::posToPan(int x, int y) {
	/*
	 * This is tricky to do right. Scenes could be scrolling (thus bigger than rendering width)
	 * and even then objects that emit sound could be "outside" the scene.
	 *
	 * As a compromise, the range where panning is applied is defined from
	 * (-0.5 * width) .. 0 .. (+1.5 * width).
	 *
	 * Because the sound library might simply ignore values out of range, extreme
	 * values are truncated.
	 */
	float width = (float)_game->_renderer->getWidth();
	float relPos = ((float)x + (0.5f * width)) / (width * 2.0f);

	// saturate
	if (relPos < 0.0f) {
		relPos = 0.0f;
	}
	if (relPos > 1.0f) {
		relPos = 1.0f;
	}

	float minPan = -1.0f;
	float maxPan = 1.0f;

	return minPan + relPos * (maxPan - minPan);
}

} // End of namespace Wintermute
