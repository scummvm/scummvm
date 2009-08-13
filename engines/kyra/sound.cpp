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
	for (int i = 0; _supportedCodecs[i].fileext; ++i) {
		Common::String f = file;
		f += _supportedCodecs[i].fileext;
		if (_vm->resource()->getFileSize(f.c_str()) > 0)
			return true;
	}

	return false;
}

bool Sound::isVoicePresent(const char *file) {
	char filenamebuffer[25];

	for (int i = 0; _supportedCodecs[i].fileext; ++i) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, _supportedCodecs[i].fileext);

		if (_vm->resource()->exists(filenamebuffer))
			return true;
	}

	return false;
}

int32 Sound::voicePlay(const char *file, Audio::SoundHandle *handle, uint8 volume, bool isSfx) {
	Audio::AudioStream *audioStream = getVoiceStream(file);

	if (!audioStream) {
		return 0;
	}

	int playTime = audioStream->getTotalPlayTime();
	playVoiceStream(audioStream, handle, volume, isSfx);
	return playTime;
}

Audio::AudioStream *Sound::getVoiceStream(const char *file) {
	char filenamebuffer[25];

	Audio::AudioStream *audioStream = 0;
	for (int i = 0; _supportedCodecs[i].fileext; ++i) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, _supportedCodecs[i].fileext);

		Common::SeekableReadStream *stream = _vm->resource()->createReadStream(filenamebuffer);
		if (!stream)
			continue;

		audioStream = _supportedCodecs[i].streamFunc(stream, true, 0, 0, 1);
		break;
	}

	if (!audioStream) {
		warning("Couldn't load sound file '%s'", file);
		return 0;
	} else {
		return audioStream;
	}
}

bool Sound::playVoiceStream(Audio::AudioStream *stream, Audio::SoundHandle *handle, uint8 volume, bool isSfx) {
	int h = 0;
	while (_mixer->isSoundHandleActive(_soundChannels[h]) && h < kNumChannelHandles)
		h++;
	if (h >= kNumChannelHandles)
		return false;

	_mixer->playInputStream(isSfx ? Audio::Mixer::kSFXSoundType : Audio::Mixer::kSpeechSoundType, &_soundChannels[h], stream, -1, volume);
	if (handle)
		*handle = _soundChannels[h];

	return true;
}

void Sound::voiceStop(const Audio::SoundHandle *handle) {
	if (!handle) {
		for (int h = 0; h < kNumChannelHandles; h++) {
			if (_mixer->isSoundHandleActive(_soundChannels[h]))
				_mixer->stopHandle(_soundChannels[h]);
		}
	} else {
		_mixer->stopHandle(*handle);
	}
}

bool Sound::voiceIsPlaying(const Audio::SoundHandle *handle) {
	if (!handle) {
		for (int h = 0; h < kNumChannelHandles; h++) {
			if (_mixer->isSoundHandleActive(_soundChannels[h]))
				return true;
		}
	} else {
		return _mixer->isSoundHandleActive(*handle);
	}

	return false;
}

bool Sound::allVoiceChannelsPlaying() {
	for (int i = 0; i < kNumChannelHandles; ++i)
		if (!_mixer->isSoundHandleActive(_soundChannels[i]))
			return false;
	return true;
}

#pragma mark -

void KyraEngine_v1::snd_playTheme(int file, int track) {
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
	_sound->playSoundEffect(track);
}

void KyraEngine_v1::snd_playWanderScoreViaMap(int command, int restart) {
	if (restart)
		_lastMusicCommand = -1;

	// no track mapping given
	// so don't do anything here
	if (!_trackMap || !_trackMapSize)
		return;

	//if (!_disableSound) {
	//	XXX
	//}

	if (_flags.platform == Common::kPlatformPC || _flags.platform == Common::kPlatformMacintosh) {
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
	_sound->voiceStop(&_speechHandle);
}

bool KyraEngine_v1::snd_voiceIsPlaying() {
	return _sound->voiceIsPlaying(&_speechHandle);
}

// static res

namespace {

// A simple wrapper to create VOC streams the way like creating MP3, OGG/Vorbis and FLAC streams.
// Possible TODO: Think of making this complete and moving it to sound/voc.cpp ?
Audio::AudioStream *makeVOCStream(Common::SeekableReadStream *stream, bool disposeAfterUse, uint32 startTime, uint32 duration, uint numLoops) {

#ifdef STREAM_AUDIO_FROM_DISK
	Audio::AudioStream *as = Audio::makeVOCStream(*stream, Audio::Mixer::FLAG_UNSIGNED, 0, 0, disposeAfterUse);
#else
	Audio::AudioStream *as = Audio::makeVOCStream(*stream, Audio::Mixer::FLAG_UNSIGNED);

	if (disposeAfterUse)
		delete stream;
#endif

	return as;
}

} // end of anonymous namespace

const Sound::SpeechCodecs Sound::_supportedCodecs[] = {
	{ ".VOC", makeVOCStream },
	{ "", makeVOCStream },

#ifdef USE_MAD
	{ ".VO3", Audio::makeMP3Stream },
	{ ".MP3", Audio::makeMP3Stream },
#endif // USE_MAD

#ifdef USE_VORBIS
	{ ".VOG", Audio::makeVorbisStream },
	{ ".OGG", Audio::makeVorbisStream },
#endif // USE_VORBIS

#ifdef USE_FLAC
	{ ".VOF", Audio::makeFlacStream },
	{ ".FLA", Audio::makeFlacStream },
#endif // USE_FLAC

	{ 0, 0 }
};

} // end of namespace Kyra


