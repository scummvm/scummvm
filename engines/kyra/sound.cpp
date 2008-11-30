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
 * $URL$
 * $Id$
 *
 */


#include "common/system.h"
#include "common/config-manager.h"

#include "kyra/resource.h"
#include "kyra/sound.h"

#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiostream.h"

#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/flac.h"

namespace Kyra {

Sound::Sound(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: _vm(vm), _mixer(mixer), _soundChannels(), _musicEnabled(1),
	_sfxEnabled(true), _soundDataList(0) {
}

Sound::~Sound() {
}

bool Sound::voiceFileIsPresent(const char *file) {
	char filenamebuffer[25];
	for (int i = 0; _supportedCodecs[i].fileext; ++i) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, _supportedCodecs[i].fileext);
		if (_vm->resource()->getFileSize(filenamebuffer) > 0)
			return true;
	}

	strcpy(filenamebuffer, file);
	strcat(filenamebuffer, ".VOC");

	if (_vm->resource()->getFileSize(filenamebuffer) > 0)
		return true;

	return false;
}

int32 Sound::voicePlay(const char *file, bool isSfx) {
	char filenamebuffer[25];

	int h = 0;
	while (_mixer->isSoundHandleActive(_soundChannels[h].channelHandle) && h < kNumChannelHandles)
		h++;
	if (h >= kNumChannelHandles)
		return 0;

	Audio::AudioStream *audioStream = 0;

	for (int i = 0; _supportedCodecs[i].fileext; ++i) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, _supportedCodecs[i].fileext);

		Common::SeekableReadStream *stream = _vm->resource()->getFileStream(filenamebuffer);
		if (!stream)
			continue;
		audioStream = _supportedCodecs[i].streamFunc(stream, true, 0, 0, 1);
		break;
	}

	if (!audioStream) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, ".VOC");

		uint32 fileSize = 0;
		byte *fileData = _vm->resource()->fileData(filenamebuffer, &fileSize);
		if (!fileData)
			return 0;

		Common::MemoryReadStream vocStream(fileData, fileSize);
		audioStream = Audio::makeVOCStream(vocStream);

		delete[] fileData;
		fileSize = 0;
	}

	if (!audioStream) {
		warning("Couldn't load sound file '%s'", file);
		return 0;
	}

	_soundChannels[h].file = file;
	_mixer->playInputStream(isSfx ? Audio::Mixer::kSFXSoundType : Audio::Mixer::kSpeechSoundType, &_soundChannels[h].channelHandle, audioStream);

	return audioStream->getTotalPlayTime();
}

void Sound::voiceStop(const char *file) {
	if (!file) {
		for (int h = 0; h < kNumChannelHandles; h++) {
			if (_mixer->isSoundHandleActive(_soundChannels[h].channelHandle))
				_mixer->stopHandle(_soundChannels[h].channelHandle);
		}
	} else {
		for (int i = 0; i < kNumChannelHandles; ++i) {
			if (_soundChannels[i].file == file)
				_mixer->stopHandle(_soundChannels[i].channelHandle);
		}
	}
}

bool Sound::voiceIsPlaying(const char *file) {
	bool res = false;
	if (!file) {
		for (int h = 0; h < kNumChannelHandles; h++) {
			if (_mixer->isSoundHandleActive(_soundChannels[h].channelHandle))
				res = true;
		}
	} else {
		for (int i = 0; i < kNumChannelHandles; ++i) {
			if (_soundChannels[i].file == file)
				res = _mixer->isSoundHandleActive(_soundChannels[i].channelHandle);
		}
	}
	return res;
}

uint32 Sound::voicePlayedTime(const char *file) {
	if (!file)
		return 0;

	for (int i = 0; i < kNumChannelHandles; ++i) {
		if (_soundChannels[i].file == file)
			return _mixer->getSoundElapsedTime(_soundChannels[i].channelHandle);
	}

	return 0;
}

#pragma mark -

void KyraEngine_v1::snd_playTheme(int file, int track) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playTheme(%d, %d)", file, track);
	if (_curMusicTheme == file)
		return;

	_curSfxFile = _curMusicTheme = file;
	_sound->loadSoundFile(_curMusicTheme);

	// Kyrandia 2 uses a special file for
	// MIDI sound effects, so we load
	// this here
	if (_flags.gameID == GI_KYRA2)
		_sound->loadSfxFile("K2SFX");

	if (track != -1)
		_sound->playTrack(track);
}

void KyraEngine_v1::snd_playSoundEffect(int track, int volume) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playSoundEffect(%d, %d)", track, volume);
	_sound->playSoundEffect(track);
}

void KyraEngine_v1::snd_playWanderScoreViaMap(int command, int restart) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playWanderScoreViaMap(%d, %d)", command, restart);
	if (restart)
		_lastMusicCommand = -1;

	// no track mapping given
	// so don't do anything here
	if (!_trackMap || !_trackMapSize)
		return;

	//if (!_disableSound) {
	//	XXX
	//}

	if (_flags.platform == Common::kPlatformPC) {
		assert(command*2+1 < _trackMapSize);
		if (_curMusicTheme != _trackMap[command*2]) {
			if (_trackMap[command*2] != -1 && _trackMap[command*2] != -2)
				snd_playTheme(_trackMap[command*2], -1);
		}

		if (command != 1) {
			if (_lastMusicCommand != command) {
				_sound->haltTrack();
				_sound->playTrack(_trackMap[command*2+1]);
			}
		} else {
			_sound->beginFadeOut();
		}
	} else if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98) {
		if (command == -1) {
			_sound->haltTrack();
		} else {
			assert(command*2+1 < _trackMapSize);
			if (_trackMap[command*2] != -2 && command != _lastMusicCommand) {
				_sound->haltTrack();
				_sound->playTrack(command);
			}
		}
	}

	_lastMusicCommand = command;
}

void KyraEngine_v1::snd_stopVoice() {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_stopVoice()");
	if (!_speechFile.empty()) {
		_sound->voiceStop(_speechFile.c_str());
		_speechFile.clear();
	}
}

bool KyraEngine_v1::snd_voiceIsPlaying() {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_voiceIsPlaying()");
	return _speechFile.empty() ? false : _sound->voiceIsPlaying(_speechFile.c_str());
}

// static res

const Sound::SpeechCodecs Sound::_supportedCodecs[] = {
#ifdef USE_FLAC
	{ ".VOF", Audio::makeFlacStream },
#endif // USE_FLAC
#ifdef USE_VORBIS
	{ ".VOG", Audio::makeVorbisStream },
#endif // USE_VORBIS
#ifdef USE_MAD
	{ ".VO3", Audio::makeMP3Stream },
#endif // USE_MAD
	{ 0, 0 }
};

} // end of namespace Kyra


