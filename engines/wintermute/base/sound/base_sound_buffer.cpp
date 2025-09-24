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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/base/sound/base_sound_buffer.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/dcgf.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#ifdef USE_VORBIS
#include "audio/decoders/vorbis.h"
#endif
#include "audio/decoders/wave.h"
#include "audio/decoders/raw.h"
#include "common/system.h"
#include "common/substream.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
BaseSoundBuffer::BaseSoundBuffer(BaseGame *inGame) : BaseClass(inGame) {
	_stream = nullptr;
	_handle = nullptr;

	_streamed = false;
	_filename = nullptr;
	_privateVolume = 100;
	_volume = 255;
	_pan = 0;

	_looping = false;
	_loopStart = 0;
	_startPos = 0;

	_type = TSoundType::SOUND_SFX;

	_freezePaused = false;
}


//////////////////////////////////////////////////////////////////////////
BaseSoundBuffer::~BaseSoundBuffer() {
	stop();

	if (_handle) {
		g_system->getMixer()->stopHandle(*_handle);
		SAFE_DELETE(_handle);
	}

	SAFE_DELETE(_stream);
	SAFE_DELETE_ARRAY(_filename);
}


//////////////////////////////////////////////////////////////////////////
void BaseSoundBuffer::setStreaming(bool streamed, uint32 numBlocks, uint32 blockSize) {
	_streamed = streamed;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::loadFromFile(const char *filename, bool forceReload) {
	if (_handle) {
		g_system->getMixer()->stopHandle(*_handle);
		SAFE_DELETE(_handle);
	}
	SAFE_DELETE(_stream);

	// Load a file, but avoid having the File-manager handle the disposal of it.
	Common::SeekableReadStream *file = _game->_fileManager->openFile(filename, true, false);
	if (!file) {
		_game->LOG(0, "Error opening sound file '%s'", filename);
		return STATUS_FAILED;
	}
	Common::String strFilename(filename);
	strFilename.toLowercase();
	if (strFilename.hasSuffix(".ogg")) {
#ifdef USE_VORBIS
		_stream = Audio::makeVorbisStream(file, DisposeAfterUse::YES);
#else
		error("BSoundBuffer::loadFromFile - Ogg Vorbis not supported by this version of ScummVM (please report as this shouldn't trigger)");
#endif
	} else if (strFilename.hasSuffix(".wav")) {
		int waveSize, waveRate;
		byte waveFlags;
		uint16 waveType;

		if (Audio::loadWAVFromStream(*file, waveSize, waveRate, waveFlags, &waveType)) {
			if (waveType == 1) {
				// We need to wrap the file in a substream to make sure the size is right.
				file = new Common::SeekableSubReadStream(file, file->pos(), waveSize + file->pos(), DisposeAfterUse::YES);
				_stream = Audio::makeRawStream(file, waveRate, waveFlags, DisposeAfterUse::YES);
			} else {
				error("BSoundBuffer::loadFromFile - WAVE not supported yet for %s with type %d", filename, waveType);
			}
		}
	} else {
		error("BSoundBuffer::loadFromFile - Unknown filetype for %s", filename);
	}
	if (!_stream) {
		_game->LOG(0, "BSoundBuffer::koadFromFile - Error while loading '%s'", filename);
		return STATUS_FAILED;
	}

	BaseUtils::setString(&_filename, filename);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::play(bool looping, uint32 startSample) {
	if (_handle) {
		g_system->getMixer()->stopHandle(*_handle);
		SAFE_DELETE(_handle);
	}
	// Store the loop-value for save-games.
	setLooping(looping);
	if (_stream) {
		_stream->seek(startSample);
		_handle = new Audio::SoundHandle;
		Audio::Mixer::SoundType type = Audio::Mixer::SoundType::kPlainSoundType;
		if (_type == TSoundType::SOUND_SFX) {
			type = Audio::Mixer::SoundType::kSFXSoundType;
		} else if (_type == TSoundType::SOUND_MUSIC) {
			type = Audio::Mixer::SoundType::kMusicSoundType;
		} else if (_type == TSoundType::SOUND_SPEECH) {
			type = Audio::Mixer::SoundType::kSpeechSoundType;
		}
		if (_looping) {
			if (_loopStart != 0) {
				Audio::AudioStream *loopStream = new Audio::SubLoopingAudioStream(_stream, 0, Audio::Timestamp(_loopStart, _stream->getRate()), _stream->getLength(), DisposeAfterUse::NO);
				g_system->getMixer()->playStream(type, _handle, loopStream, -1, _volume, _pan, DisposeAfterUse::YES);
			} else {
				Audio::AudioStream *loopStream = new Audio::LoopingAudioStream(_stream, 0, DisposeAfterUse::NO);
				g_system->getMixer()->playStream(type, _handle, loopStream, -1, _volume, _pan, DisposeAfterUse::YES);
			}
		} else {
			g_system->getMixer()->playStream(type, _handle, _stream, -1, _volume, _pan, DisposeAfterUse::NO);
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseSoundBuffer::setLooping(bool looping) {
	_looping = looping;

	if (isPlaying()) {
		// This warning is here, to see if this is ever the case.
		warning("BSoundBuffer::SetLooping(%d) - won't change a playing sound", looping); // TODO
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::resume() {
	// If the sound was paused while active:
	if (_stream && _handle) {
		g_system->getMixer()->pauseHandle(*_handle, false);
	} else if (_stream) { // Otherwise we come from a savegame, and thus have no handle
		play(_looping, _startPos);
	} else {
		warning("BaseSoundBuffer::resume - Called without a handle or a stream");
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::stop() {
	if (_stream && _handle) {
		g_system->getMixer()->stopHandle(*_handle);
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::pause() {
	if (_stream && _handle) {
		g_system->getMixer()->pauseHandle(*_handle, true);
	}
	return STATUS_OK;

}

//////////////////////////////////////////////////////////////////////////
uint32 BaseSoundBuffer::getLength() {
	if (_stream) {
		uint32 len = _stream->getLength().msecs();
		return len * 1000;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
void BaseSoundBuffer::setType(TSoundType type) {
	_type = type;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::setVolume(int volume) {
	volume = (float)volume / 100.0f * _privateVolume;
	_volume = (volume * Audio::Mixer::kMaxChannelVolume) / 100;
	if (_stream && _handle) {
		g_system->getMixer()->setChannelVolume(*_handle, (byte)(_volume));
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::setPrivateVolume(int volume) {
	_privateVolume = volume;
	return setVolume(volume);
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::isPlaying() {
	if (_stream && _handle) {
		return _freezePaused || g_system->getMixer()->isSoundHandleActive(*_handle);
	} else {
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
uint32 BaseSoundBuffer::getPosition() {
	if (_stream && _handle) {
		uint32 pos = g_system->getMixer()->getSoundElapsedTime(*_handle);
		return pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::setPosition(uint32 pos) {
	if (_stream) {
		_stream->seek(pos);
	} else {
		if (isPlaying()) {
			warning("BaseSoundBuffer::SetPosition - not implemented for playing sounds yet.");
		}
	}
	_startPos = pos;
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::setLoopStart(uint32 pos) {
	_loopStart = pos;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::setPan(float pan) {
	pan = MAX(pan, -1.0f);
	pan = MIN(pan, 1.0f);
	_pan = (int8)(pan * 127);
	if (_handle) {
		g_system->getMixer()->setChannelBalance(*_handle, _pan);
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundBuffer::applyFX(TSFXType type, float param1, float param2, float param3, float param4) {
	// TODO
	switch (type) {
	case SFX_ECHO:
		warning("BaseSoundBuffer::ApplyFX(SFX_ECHO, %f, %f, %f, %f)  - not implemented yet", param1, param2, param3, param4);
		break;

	case SFX_REVERB:
		warning("BaseSoundBuffer::ApplyFX(SFX_REVERB, %f, %f, %f, %f)  - not implemented yet", param1, param2, param3, param4);
		break;

	default:
		break;
	}
	return STATUS_OK;
}

} // End of namespace Wintermute
