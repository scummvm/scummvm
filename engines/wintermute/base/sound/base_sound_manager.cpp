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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/base/base_registry.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/sound/base_sound_buffer.h"
#include "engines/wintermute/wintermute.h"
#include "common/config-manager.h"
#include "audio/mixer.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//IMPLEMENT_PERSISTENT(CBSoundMgr, true);

//////////////////////////////////////////////////////////////////////////
CBSoundMgr::CBSoundMgr(CBGame *inGame): CBBase(inGame) {
	_soundAvailable = false;
	_volumeMaster = 255;
}


//////////////////////////////////////////////////////////////////////////
CBSoundMgr::~CBSoundMgr() {
	saveSettings();
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundMgr::cleanup() {
	for (uint32 i = 0; i < _sounds.size(); i++)
		delete _sounds[i];
	_sounds.clear();
#if 0
	BASS_Free();
#endif
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBSoundMgr::saveSettings() {
	if (_soundAvailable) {
		_gameRef->_registry->writeInt("Audio", "MasterVolume", _volumeMaster);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CBSoundMgr::initialize() {
	_soundAvailable = false;
	
	if (!g_system->getMixer()->isReady()) {
		return STATUS_FAILED;
	}
	_volumeMaster = _gameRef->_registry->readInt("Audio", "MasterVolume", 255);
	_soundAvailable = true;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundMgr::initLoop() {
	if (!_soundAvailable)
		return STATUS_OK;
#if 0

	BASS_Update(500);
#endif
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
CBSoundBuffer *CBSoundMgr::addSound(const char *filename, Audio::Mixer::SoundType type, bool streamed) {
	if (!_soundAvailable)
		return NULL;

	CBSoundBuffer *sound;

	// try to switch WAV to OGG file (if available)
	AnsiString ext = PathUtil::getExtension(filename);
	if (StringUtil::compareNoCase(ext, "wav")) {
		AnsiString path = PathUtil::getDirectoryName(filename);
		AnsiString name = PathUtil::getFileNameWithoutExtension(filename);

		AnsiString newFile = PathUtil::combine(path, name + "ogg");
		if (_gameRef->_fileManager->hasFile(newFile)) {
			filename = newFile.c_str();
		}
	}

	sound = new CBSoundBuffer(_gameRef);
	if (!sound) return NULL;

	sound->setStreaming(streamed);
	sound->setType(type);


	bool res = sound->loadFromFile(filename);
	if (DID_FAIL(res)) {
		_gameRef->LOG(res, "Error loading sound '%s'", filename);
		delete sound;
		return NULL;
	}

	// Make sure the master-volume is applied to the sound.
	sound->updateVolume();

	// register sound
	_sounds.push_back(sound);

	return sound;

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CBSoundMgr::addSound(CBSoundBuffer *sound, Audio::Mixer::SoundType type) {
	if (!sound)
		return STATUS_FAILED;

	// Make sure the master-volume is applied to the sound.
	sound->updateVolume();

	// register sound
	_sounds.push_back(sound);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBSoundMgr::removeSound(CBSoundBuffer *sound) {
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
bool CBSoundMgr::setVolume(Audio::Mixer::SoundType type, int volume) {
	if (!_soundAvailable)
		return STATUS_OK;

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
	}
	g_wintermute->syncSoundSettings();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBSoundMgr::setVolumePercent(Audio::Mixer::SoundType type, byte percent) {
	return setVolume(type, percent * 255 / 100);
}


//////////////////////////////////////////////////////////////////////////
byte CBSoundMgr::getVolumePercent(Audio::Mixer::SoundType type) {
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
bool CBSoundMgr::setMasterVolume(byte value) {
	_volumeMaster = value;
	for (uint32 i = 0; i < _sounds.size(); i++) {
		_sounds[i]->updateVolume();
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBSoundMgr::setMasterVolumePercent(byte percent) {
	setMasterVolume(percent * 255 / 100);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
byte CBSoundMgr::getMasterVolumePercent() {
	return getMasterVolume() * 100 / 255;
}

//////////////////////////////////////////////////////////////////////////
byte CBSoundMgr::getMasterVolume() {
	return (byte)_volumeMaster;
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundMgr::pauseAll(bool includingMusic) {

	for (uint32 i = 0; i < _sounds.size(); i++) {
		if (_sounds[i]->isPlaying() && (_sounds[i]->_type != Audio::Mixer::kMusicSoundType || includingMusic)) {
			_sounds[i]->pause();
			_sounds[i]->_freezePaused = true;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundMgr::resumeAll() {

	for (uint32 i = 0; i < _sounds.size(); i++) {
		if (_sounds[i]->_freezePaused) {
			_sounds[i]->resume();
			_sounds[i]->_freezePaused = false;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
float CBSoundMgr::posToPan(int x, int y) {
	float relPos = (float)x / ((float)_gameRef->_renderer->_width);

	float minPan = -0.7f;
	float maxPan = 0.7f;

	return minPan + relPos * (maxPan - minPan);
}

} // end of namespace WinterMute
