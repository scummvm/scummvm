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
#include "engines/wintermute/Base/file/BFile.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BSoundMgr.h"
#include "engines/wintermute/Base/BSoundBuffer.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/utils/utils.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/raw.h"
#include "common/system.h"
#include "common/substream.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAX_NONSTREAMED_FILE_SIZE 1024*1024

//////////////////////////////////////////////////////////////////////////
CBSoundBuffer::CBSoundBuffer(CBGame *inGame): CBBase(inGame) {
	_stream = NULL;
	_handle = NULL;
//	_sync = NULL;

	_streamed = false;
	_filename = NULL;
	_file = NULL;
	_privateVolume = 255;
	_volume = 255;

	_looping = false;
	_loopStart = 0;

	_type = Audio::Mixer::kSFXSoundType;

	_freezePaused = false;
}


//////////////////////////////////////////////////////////////////////////
CBSoundBuffer::~CBSoundBuffer() {
	stop();

	if (_handle) {
		g_system->getMixer()->stopHandle(*_handle);
		delete _handle;
		_handle = NULL;
	}
	delete _stream;
	_stream = NULL;

	delete[] _filename;
	_filename = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::setStreaming(bool Streamed, uint32 NumBlocks, uint32 BlockSize) {
	_streamed = Streamed;
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::loadFromFile(const char *filename, bool forceReload) {
	warning("BSoundBuffer::LoadFromFile(%s,%d)", filename, forceReload);
#if 0
	if (_stream) {
		BASS_StreamFree(_stream);
		_stream = NULL;
	}
#endif

	// Load a file, but avoid having the File-manager handle the disposal of it.
	_file = _gameRef->_fileManager->openFile(filename, true, false);
	if (!_file) {
		_gameRef->LOG(0, "Error opening sound file '%s'", filename);
		return STATUS_FAILED;
	}
	Common::String strFilename(filename);
	if (strFilename.hasSuffix(".ogg")) {
		_stream = Audio::makeVorbisStream(_file, DisposeAfterUse::YES);
	} else if (strFilename.hasSuffix(".wav")) {
		int waveSize, waveRate;
		byte waveFlags;
		uint16 waveType;
		
		if (Audio::loadWAVFromStream(*_file, waveSize, waveRate, waveFlags, &waveType)) {
			if (waveType == 1) {
				// We need to wrap the file in a substream to make sure the size is right.
				_file = new Common::SeekableSubReadStream(_file, 0, waveSize);
				_stream = Audio::makeRawStream(_file, waveRate, waveFlags, DisposeAfterUse::YES);
			} else {
				warning("BSoundBuffer::LoadFromFile - WAVE not supported yet for %s with type %d", filename, waveType);
			}
		}
	} else {
		warning("BSoundBuffer::LoadFromFile - Unknown filetype for %s", filename);
	}
	if (!_stream) {
		return STATUS_FAILED;
	}
	CBUtils::setString(&_filename, filename);

	return STATUS_OK;
#if 0
	BASS_FILEPROCS fileProc;
	fileProc.close = CBSoundBuffer::FileCloseProc;
	fileProc.read = CBSoundBuffer::FileReadProc;
	fileProc.seek = CBSoundBuffer::FileSeekProc;
	fileProc.length = CBSoundBuffer::FileLenProc;

	_stream = BASS_StreamCreateFileUser(STREAMFILE_NOBUFFER, 0, &fileProc, (void *)_file);
	if (!_stream) {
		_gameRef->LOG(0, "BASS error: %d while loading '%s'", BASS_ErrorGetCode(), filename);
		return STATUS_FAILED;
	}

	CBUtils::setString(&_filename, filename);

	/*
	bool res;
	bool NewlyCreated = false;

	if(!_soundBuffer || ForceReload || _streamed){
	    if(!_file) _file = _gameRef->_fileManager->openFile(filename);
	    if(!_file){
	        _gameRef->LOG(0, "Error opening sound file '%s'", filename);
	        return STATUS_FAILED;
	    }
	    // switch to streamed for big files
	    if(!_streamed && (_file->GetSize() > MAX_NONSTREAMED_FILE_SIZE && !_gameRef->_forceNonStreamedSounds)) SetStreaming(true);
	}

	// create buffer
	if(!_soundBuffer){
	    NewlyCreated = true;

	    res = InitializeBuffer(_file);
	    if(DID_FAIL(res)){
	        _gameRef->LOG(res, "Error creating sound buffer for file '%s'", filename);
	        return res;
	    }
	}



	// store filename
	if(!_filename){
	    _filename = new char[strlen(filename)+1];
	    strcpy(_filename, filename);
	}

	// close file (if not streaming)
	if(!_streamed && _file){
	    _gameRef->_fileManager->closeFile(_file);
	    _file = NULL;
	}
	*/

	return STATUS_OK;
#endif
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::play(bool looping, uint32 startSample) {
	if (startSample != 0) {
		warning("BSoundBuffer::Play - Should start playback at %d, but currently we don't", startSample);
	}
	if (_handle) {
		g_system->getMixer()->stopHandle(*_handle);
		delete _handle;
		_handle = NULL;
	}
	if (_stream) {
		_stream->seek(startSample);
		_handle = new Audio::SoundHandle;
		if (looping) {
			Audio::AudioStream *loopStream = new Audio::LoopingAudioStream(_stream, 0, DisposeAfterUse::NO);
			g_system->getMixer()->playStream(_type, _handle, loopStream, -1, _volume, 0, DisposeAfterUse::YES);
		} else {
			g_system->getMixer()->playStream(_type, _handle, _stream, -1, _volume, 0, DisposeAfterUse::NO);
		}
	} 

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::setLooping(bool looping) {
	warning("BSoundBuffer::SetLooping(%d) - won't change a playing sound", looping);
	_looping = looping;
#if 0


	if (_stream) {
		BASS_ChannelFlags(_stream, looping ? BASS_SAMPLE_LOOP : 0, BASS_SAMPLE_LOOP);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::resume() {
	if (_stream && _handle) {
		g_system->getMixer()->pauseHandle(*_handle, false);
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::stop() {
	if (_stream && _handle) {
		g_system->getMixer()->stopHandle(*_handle);
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::pause() {
	if (_stream && _handle) {
		g_system->getMixer()->pauseHandle(*_handle, true);
	}
	return STATUS_OK;

}

//////////////////////////////////////////////////////////////////////////
uint32 CBSoundBuffer::getLength() {
	if (_stream) {
		uint32 len = _stream->getLength().msecs();
		return len * 1000;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::setType(Audio::Mixer::SoundType type) {
	_type = type;
}

//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::updateVolume() {
	setVolume(_privateVolume);
}

//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::setVolume(int volume) {
	_volume = volume * _gameRef->_soundMgr->getMasterVolume() / 255;
	if (_stream && _handle) {
		byte vol = (byte)(_volume);
		g_system->getMixer()->setChannelVolume(*_handle, vol);
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::setPrivateVolume(int volume) {
	_privateVolume = volume;
	return setVolume(_privateVolume);
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::isPlaying() {
	if (_stream && _handle) {
		return _freezePaused || g_system->getMixer()->isSoundHandleActive(*_handle);
	} else {
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
uint32 CBSoundBuffer::getPosition() {
	if (_stream && _handle) {
		uint32 pos = g_system->getMixer()->getSoundElapsedTime(*_handle);
		return pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::setPosition(uint32 pos) {
	warning("CBSoundBuffer::SetPosition - not implemented yet");
#if 0
	if (_stream) {
		QWORD pos = BASS_ChannelSeconds2Bytes(_stream, (float)Pos / 1000.0f);
		BASS_ChannelSetPosition(_stream, pos, BASS_POS_BYTE);
	}
#endif
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::setLoopStart(uint32 pos) {
	_loopStart = pos;
#if 0
	if (_stream) {
		if (_sync) {
			BASS_ChannelRemoveSync(_stream, _sync);
			_sync = NULL;
		}
		if (_loopStart > 0) {
			QWORD len = BASS_ChannelGetLength(_stream, BASS_POS_BYTE);
			_sync = BASS_ChannelSetSync(_stream, BASS_SYNC_POS | BASS_SYNC_MIXTIME, len, CBSoundBuffer::LoopSyncProc, (void *)this);
		}
	}
#endif
	return STATUS_OK;
}
#if 0
//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::LoopSyncProc(HSYNC handle, uint32 channel, uint32 data, void *user) {
	CBSoundBuffer *soundBuf = static_cast<CBSoundBuffer *>(user);
	QWORD pos = BASS_ChannelSeconds2Bytes(channel, (float)soundBuf->GetLoopStart() / 1000.0f);

	if (!BASS_ChannelSetPosition(channel, pos, BASS_POS_BYTE))
		BASS_ChannelSetPosition(channel, 0, BASS_POS_BYTE);
}
#endif
//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::setPan(float pan) {
	if (_handle) {
		g_system->getMixer()->setChannelBalance(*_handle, (int8)(pan * 127));
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::applyFX(TSFXType type, float param1, float param2, float param3, float param4) {
	warning("CBSoundBuffer::ApplyFX - not implemented yet");
	switch (type) {
	case SFX_ECHO:
		break;

	case SFX_REVERB:
		break;

	default:
		break;
	}
	return STATUS_OK;
}

} // end of namespace WinterMute
