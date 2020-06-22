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

#include "engines/wintermute/base/base_game_music.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/sound/base_sound.h"

namespace Wintermute {

BaseGameMusic::BaseGameMusic(BaseGame *gameRef) : _gameRef(gameRef) {
	for (int i = 0; i < NUM_MUSIC_CHANNELS; i++) {
		_music[i] = nullptr;
		_musicStartTime[i] = 0;
	}

	_musicCrossfadeRunning = false;
	_musicCrossfadeStartTime = 0;
	_musicCrossfadeLength = 0;
	_musicCrossfadeChannel1 = -1;
	_musicCrossfadeChannel2 = -1;
	_musicCrossfadeSwap = false;
	_musicCrossfadeVolume1 = 0;
	_musicCrossfadeVolume2 = 100;
}

void BaseGameMusic::cleanup() {
	for (int i = 0; i < NUM_MUSIC_CHANNELS; i++) {
		delete _music[i];
		_music[i] = nullptr;
		_musicStartTime[i] = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseGameMusic::playMusic(int channel, const char *filename, bool looping, uint32 loopStart) {
	if (channel >= NUM_MUSIC_CHANNELS) {
		BaseEngine::LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", channel, NUM_MUSIC_CHANNELS);
		return STATUS_FAILED;
	}

	delete _music[channel];
	_music[channel] = nullptr;

	_music[channel] = new BaseSound(_gameRef);
	if (_music[channel] && DID_SUCCEED(_music[channel]->setSound(filename, Audio::Mixer::kMusicSoundType, true))) {
		if (_musicStartTime[channel]) {
			_music[channel]->setPositionTime(_musicStartTime[channel]);
			_musicStartTime[channel] = 0;
		}
		if (loopStart) {
			_music[channel]->setLoopStart(loopStart);
		}
		return _music[channel]->play(looping);
	} else {
		delete _music[channel];
		_music[channel] = nullptr;
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseGameMusic::stopMusic(int channel) {
	if (channel >= NUM_MUSIC_CHANNELS) {
		BaseEngine::LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", channel, NUM_MUSIC_CHANNELS);
		return STATUS_FAILED;
	}

	if (_music[channel]) {
		_music[channel]->stop();
		delete _music[channel];
		_music[channel] = nullptr;
		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseGameMusic::pauseMusic(int channel) {
	if (channel >= NUM_MUSIC_CHANNELS) {
		BaseEngine::LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", channel, NUM_MUSIC_CHANNELS);
		return STATUS_FAILED;
	}

	if (_music[channel]) {
		return _music[channel]->pause();
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseGameMusic::resumeMusic(int channel) {
	if (channel >= NUM_MUSIC_CHANNELS) {
		BaseEngine::LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", channel, NUM_MUSIC_CHANNELS);
		return STATUS_FAILED;
	}

	if (_music[channel]) {
		return _music[channel]->resume();
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseGameMusic::setMusicStartTime(int channel, uint32 time) {
	if (channel >= NUM_MUSIC_CHANNELS) {
		BaseEngine::LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", channel, NUM_MUSIC_CHANNELS);
		return STATUS_FAILED;
	}

	_musicStartTime[channel] = time;
	if (_music[channel] && _music[channel]->isPlaying()) {
		return _music[channel]->setPositionTime(time);
	} else {
		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseGameMusic::updateMusicCrossfade() {
	if (!_musicCrossfadeRunning) {
		return STATUS_OK;
	}
	if (_gameRef->_state == GAME_FROZEN) {
		return STATUS_OK;
	}

	if (_musicCrossfadeChannel1 < 0 || _musicCrossfadeChannel1 >= NUM_MUSIC_CHANNELS || !_music[_musicCrossfadeChannel1]) {
		_musicCrossfadeRunning = false;
		return STATUS_OK;
	}
	if (_musicCrossfadeChannel2 < 0 || _musicCrossfadeChannel2 >= NUM_MUSIC_CHANNELS || !_music[_musicCrossfadeChannel2]) {
		_musicCrossfadeRunning = false;
		return STATUS_OK;
	}

	if (!_music[_musicCrossfadeChannel1]->isPlaying()) {
		_music[_musicCrossfadeChannel1]->play();
	}
	if (!_music[_musicCrossfadeChannel2]->isPlaying()) {
		_music[_musicCrossfadeChannel2]->play();
	}

	uint32 currentTime = _gameRef->getLiveTimer()->getTime() - _musicCrossfadeStartTime;

	if (currentTime >= _musicCrossfadeLength) {
		_musicCrossfadeRunning = false;

		if (_musicCrossfadeVolume2 == 0) {
			_music[_musicCrossfadeChannel2]->stop();
			_music[_musicCrossfadeChannel2]->setVolumePercent(100);
		} else {
			_music[_musicCrossfadeChannel2]->setVolumePercent(_musicCrossfadeVolume2);
		}

		if (_musicCrossfadeChannel1 != _musicCrossfadeChannel2) {
			if (_musicCrossfadeVolume1 == 0) {
				_music[_musicCrossfadeChannel1]->stop();
				_music[_musicCrossfadeChannel1]->setVolumePercent(100);
			} else {
				_music[_musicCrossfadeChannel1]->setVolumePercent(_musicCrossfadeVolume1);
			}
		}

		if (_musicCrossfadeSwap) {
			// swap channels
			BaseSound *dummy = _music[_musicCrossfadeChannel1];
			int dummyInt = _musicStartTime[_musicCrossfadeChannel1];

			_music[_musicCrossfadeChannel1] = _music[_musicCrossfadeChannel2];
			_musicStartTime[_musicCrossfadeChannel1] = _musicStartTime[_musicCrossfadeChannel2];

			_music[_musicCrossfadeChannel2] = dummy;
			_musicStartTime[_musicCrossfadeChannel2] = dummyInt;
		}
	} else {
		float progress = (float)currentTime / (float)_musicCrossfadeLength;
		int volumeDelta = (int)((_musicCrossfadeVolume1 - _musicCrossfadeVolume2)*progress);
		_music[_musicCrossfadeChannel2]->setVolumePercent(_musicCrossfadeVolume1 - volumeDelta);
		BaseEngine::LOG(0, "Setting music channel %d volume to %d", _musicCrossfadeChannel2, _musicCrossfadeVolume1 - volumeDelta);

		if (_musicCrossfadeChannel1 != _musicCrossfadeChannel2) {
			_music[_musicCrossfadeChannel1]->setVolumePercent(_musicCrossfadeVolume2 + volumeDelta);
			BaseEngine::LOG(0, "Setting music channel %d volume to %d", _musicCrossfadeChannel1, _musicCrossfadeVolume2 + volumeDelta);
		}
	}

	return STATUS_OK;
}

bool BaseGameMusic::persistChannels(BasePersistenceManager *persistMgr) {
	for (int i = 0; i < NUM_MUSIC_CHANNELS; i++) {
		persistMgr->transferPtr(TMEMBER_PTR(_music[i]));
		persistMgr->transferUint32(TMEMBER(_musicStartTime[i]));
	}
	return true;
}

bool BaseGameMusic::persistCrossfadeSettings(BasePersistenceManager *persistMgr) {
	persistMgr->transferBool(TMEMBER(_musicCrossfadeRunning));
	persistMgr->transferUint32(TMEMBER(_musicCrossfadeStartTime));
	persistMgr->transferUint32(TMEMBER(_musicCrossfadeLength));
	persistMgr->transferSint32(TMEMBER(_musicCrossfadeChannel1));
	persistMgr->transferSint32(TMEMBER(_musicCrossfadeChannel2));
	persistMgr->transferBool(TMEMBER(_musicCrossfadeSwap));

	// let's keep savegame compatibility for the price of small possibility of wrong volume at game load
	if (!persistMgr->getIsSaving()) {
		_musicCrossfadeVolume1 = 0;
		_musicCrossfadeVolume2 = 100;
	}
	return true;
}

bool BaseGameMusic::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// PlayMusic / PlayMusicChannel
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "PlayMusic") == 0 || strcmp(name, "PlayMusicChannel") == 0) {
		int channel = 0;
		if (strcmp(name, "PlayMusic") == 0) {
			stack->correctParams(3);
		} else {
			stack->correctParams(4);
			channel = stack->pop()->getInt();
		}

		const char *filename = stack->pop()->getString();
		ScValue *valLooping = stack->pop();
		bool looping = valLooping->isNULL() ? true : valLooping->getBool();

		ScValue *valLoopStart = stack->pop();
		uint32 loopStart = (uint32)(valLoopStart->isNULL() ? 0 : valLoopStart->getInt());


		if (DID_FAIL(playMusic(channel, filename, looping, loopStart))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopMusic / StopMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopMusic") == 0 || strcmp(name, "StopMusicChannel") == 0) {
		int channel = 0;

		if (strcmp(name, "StopMusic") == 0) {
			stack->correctParams(0);
		} else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (DID_FAIL(stopMusic(channel))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseMusic / PauseMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PauseMusic") == 0 || strcmp(name, "PauseMusicChannel") == 0) {
		int channel = 0;

		if (strcmp(name, "PauseMusic") == 0) {
			stack->correctParams(0);
		} else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (DID_FAIL(pauseMusic(channel))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResumeMusic / ResumeMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ResumeMusic") == 0 || strcmp(name, "ResumeMusicChannel") == 0) {
		int channel = 0;
		if (strcmp(name, "ResumeMusic") == 0) {
			stack->correctParams(0);
		} else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (DID_FAIL(resumeMusic(channel))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetMusic / GetMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetMusic") == 0 || strcmp(name, "GetMusicChannel") == 0) {
		int channel = 0;
		if (strcmp(name, "GetMusic") == 0) {
			stack->correctParams(0);
		} else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}
		if (channel < 0 || channel >= NUM_MUSIC_CHANNELS) {
			stack->pushNULL();
		} else {
			if (!_music[channel] || !_music[channel]->getFilename()) {
				stack->pushNULL();
			} else {
				stack->pushString(_music[channel]->getFilename());
			}
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetMusicPosition / SetMusicChannelPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetMusicPosition") == 0 || strcmp(name, "SetMusicChannelPosition") == 0 || strcmp(name, "SetMusicPositionChannel") == 0) {
		int channel = 0;
		if (strcmp(name, "SetMusicPosition") == 0) {
			stack->correctParams(1);
		} else {
			stack->correctParams(2);
			channel = stack->pop()->getInt();
		}

		uint32 time = stack->pop()->getInt();

		if (DID_FAIL(setMusicStartTime(channel, time))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetMusicPosition / GetMusicChannelPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetMusicPosition") == 0 || strcmp(name, "GetMusicChannelPosition") == 0) {
		int channel = 0;
		if (strcmp(name, "GetMusicPosition") == 0) {
			stack->correctParams(0);
		} else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (channel < 0 || channel >= NUM_MUSIC_CHANNELS || !_music[channel]) {
			stack->pushInt(0);
		} else {
			stack->pushInt(_music[channel]->getPositionTime());
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsMusicPlaying / IsMusicChannelPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsMusicPlaying") == 0 || strcmp(name, "IsMusicChannelPlaying") == 0) {
		int channel = 0;
		if (strcmp(name, "IsMusicPlaying") == 0) {
			stack->correctParams(0);
		} else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (channel < 0 || channel >= NUM_MUSIC_CHANNELS || !_music[channel]) {
			stack->pushBool(false);
		} else {
			stack->pushBool(_music[channel]->isPlaying());
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetMusicVolume / SetMusicChannelVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetMusicVolume") == 0 || strcmp(name, "SetMusicChannelVolume") == 0) {
		int channel = 0;
		if (strcmp(name, "SetMusicVolume") == 0) {
			stack->correctParams(1);
		} else {
			stack->correctParams(2);
			channel = stack->pop()->getInt();
		}

		int volume = stack->pop()->getInt();
		if (channel < 0 || channel >= NUM_MUSIC_CHANNELS || !_music[channel]) {
			stack->pushBool(false);
		} else {
			if (DID_FAIL(_music[channel]->setVolumePercent(volume))) {
				stack->pushBool(false);
			} else {
				stack->pushBool(true);
			}
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetMusicVolume / GetMusicChannelVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetMusicVolume") == 0 || strcmp(name, "GetMusicChannelVolume") == 0) {
		int channel = 0;
		if (strcmp(name, "GetMusicVolume") == 0) {
			stack->correctParams(0);
		} else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (channel < 0 || channel >= NUM_MUSIC_CHANNELS || !_music[channel]) {
			stack->pushInt(0);
		} else {
			stack->pushInt(_music[channel]->getVolumePercent());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MusicCrossfade
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MusicCrossfade") == 0) {
		stack->correctParams(4);
		int channel1 = stack->pop()->getInt(0);
		int channel2 = stack->pop()->getInt(0);
		uint32 fadeLength = (uint32)stack->pop()->getInt(0);
		bool swap = stack->pop()->getBool(true);

		if (_musicCrossfadeRunning) {
			script->runtimeError("Game.MusicCrossfade: Music crossfade is already in progress.");
			stack->pushBool(false);
			return STATUS_OK;
		}

		_musicCrossfadeStartTime = _gameRef->getLiveTimer()->getTime();
		_musicCrossfadeChannel1 = channel1;
		_musicCrossfadeChannel2 = channel2;
		_musicCrossfadeVolume1 = 0;
		_musicCrossfadeVolume2 = 100;
		_musicCrossfadeLength = fadeLength;
		_musicCrossfadeSwap = swap;

		_musicCrossfadeRunning = true;

		stack->pushBool(true);
		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] MusicCrossfadeVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MusicCrossfadeVolume") == 0) {
		stack->correctParams(4);
		int channel = stack->pop()->getInt(0);
		int volume1 = stack->pop()->getInt(0);
		int volume2 = stack->pop()->getInt(0);
		uint32 fadeLength = (uint32)stack->pop()->getInt(0);

		if (_musicCrossfadeRunning) {
			script->runtimeError("Game.MusicCrossfade: Music crossfade is already in progress.");
			stack->pushBool(false);
			return STATUS_OK;
		}

		_musicCrossfadeStartTime = _gameRef->getLiveTimer()->getTime();
		_musicCrossfadeChannel1 = channel;
		_musicCrossfadeChannel2 = channel;
		_musicCrossfadeVolume1 = volume1;
		_musicCrossfadeVolume2 = volume2;
		_musicCrossfadeLength = fadeLength;
		_musicCrossfadeSwap = false;

		_musicCrossfadeRunning = true;

		stack->pushBool(true);
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// GetSoundLength
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSoundLength") == 0) {
		stack->correctParams(1);

		int length = 0;
		const char *filename = stack->pop()->getString();

		BaseSound *sound = new BaseSound(_gameRef);
		if (sound && DID_SUCCEED(sound->setSound(filename, Audio::Mixer::kMusicSoundType, true))) {
			length = sound->getLength();
			delete sound;
			sound = nullptr;
		}
		stack->pushInt(length);
		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}

} // End of namespace Wintermute
