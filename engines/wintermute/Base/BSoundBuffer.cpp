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
#include "common/system.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAX_NONSTREAMED_FILE_SIZE 1024*1024

//////////////////////////////////////////////////////////////////////////
CBSoundBuffer::CBSoundBuffer(CBGame *inGame): CBBase(inGame) {
	_stream = NULL;
	_handle = new Audio::SoundHandle;
//	_sync = NULL;

	_streamed = false;
	_filename = NULL;
	_file = NULL;
	_privateVolume = 100;

	_looping = false;
	_loopStart = 0;

	_type = SOUND_SFX;

	_freezePaused = false;
}


//////////////////////////////////////////////////////////////////////////
CBSoundBuffer::~CBSoundBuffer() {
	stop();

	if (_handle) {
		delete _handle;
		_handle = NULL;
	}

	if (_file) {
		Game->_fileManager->closeFile(_file);
		_file = NULL;
	}

	delete[] _filename;
	_filename = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::setStreaming(bool Streamed, uint32 NumBlocks, uint32 BlockSize) {
	_streamed = Streamed;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::loadFromFile(const char *filename, bool forceReload) {
	warning("BSoundBuffer::LoadFromFile(%s,%d)", filename, forceReload);
#if 0
	if (_stream) {
		BASS_StreamFree(_stream);
		_stream = NULL;
	}
#endif
	// If we already had a file, delete it.
	delete _file;

	// Load a file, but avoid having the File-manager handle the disposal of it.
	_file = Game->_fileManager->openFile(filename, true, false);
	if (!_file) {
		Game->LOG(0, "Error opening sound file '%s'", filename);
		return E_FAIL;
	}
	Common::String strFilename(filename);
	if (strFilename.hasSuffix(".ogg")) {
		_stream = Audio::makeVorbisStream(_file, DisposeAfterUse::YES);
	} else if (strFilename.hasSuffix(".wav")) {
		warning("BSoundBuffer::LoadFromFile - WAVE not supported yet for %s", filename);
		//_stream = Audio::makeWAVStream(_file, DisposeAfterUse::NO);
	} else {
		warning("BSoundBuffer::LoadFromFile - Unknown filetype for %s", filename);
	}
	if (!_stream) {
		return E_FAIL;
	}
	CBUtils::setString(&_filename, filename);

	return S_OK;
#if 0
	BASS_FILEPROCS fileProc;
	fileProc.close = CBSoundBuffer::FileCloseProc;
	fileProc.read = CBSoundBuffer::FileReadProc;
	fileProc.seek = CBSoundBuffer::FileSeekProc;
	fileProc.length = CBSoundBuffer::FileLenProc;

	_stream = BASS_StreamCreateFileUser(STREAMFILE_NOBUFFER, 0, &fileProc, (void *)_file);
	if (!_stream) {
		Game->LOG(0, "BASS error: %d while loading '%s'", BASS_ErrorGetCode(), filename);
		return E_FAIL;
	}

	CBUtils::setString(&_filename, filename);

	/*
	HRESULT res;
	bool NewlyCreated = false;

	if(!_soundBuffer || ForceReload || _streamed){
	    if(!_file) _file = Game->_fileManager->openFile(filename);
	    if(!_file){
	        Game->LOG(0, "Error opening sound file '%s'", filename);
	        return E_FAIL;
	    }
	    // switch to streamed for big files
	    if(!_streamed && (_file->GetSize() > MAX_NONSTREAMED_FILE_SIZE && !Game->_forceNonStreamedSounds)) SetStreaming(true);
	}

	// create buffer
	if(!_soundBuffer){
	    NewlyCreated = true;

	    res = InitializeBuffer(_file);
	    if(FAILED(res)){
	        Game->LOG(res, "Error creating sound buffer for file '%s'", filename);
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
	    Game->_fileManager->closeFile(_file);
	    _file = NULL;
	}
	*/

	return S_OK;
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::play(bool looping, uint32 startSample) {
	if (startSample != 0) {
		warning("BSoundBuffer::Play - Should start playback at %d, but currently we don't", startSample);
	}
	if (_stream) {
		if (looping) {
			Audio::AudioStream *loopStream = new Audio::LoopingAudioStream(_stream, 0, DisposeAfterUse::YES);
			g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, _handle, loopStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
		} else {
			g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, _handle, _stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::setLooping(bool looping) {
	warning("BSoundBuffer::SetLooping(%d) - not implemented yet", looping);
#if 0
	_looping = looping;

	if (_stream) {
		BASS_ChannelFlags(_stream, looping ? BASS_SAMPLE_LOOP : 0, BASS_SAMPLE_LOOP);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::resume() {
	if (_stream && _handle) {
		g_system->getMixer()->pauseHandle(*_handle, false);
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::stop() {
	if (_stream && _handle) {
		g_system->getMixer()->stopHandle(*_handle);
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::pause() {
	if (_stream && _handle) {
		g_system->getMixer()->pauseHandle(*_handle, true);
	}
	return S_OK;

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
void CBSoundBuffer::setType(TSoundType Type) {
	_type = Type;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::setVolume(int Volume) {
	if (_stream && _handle) {
		byte vol = (byte)(Volume / 100.f * Audio::Mixer::kMaxChannelVolume);
		g_system->getMixer()->setChannelVolume(*_handle, vol);
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::setPrivateVolume(int Volume) {
#if 0
	_privateVolume = Volume;

	switch (_type) {
	case SOUND_SFX:
		Volume = Game->_soundMgr->_volumeSFX;
		break;
	case SOUND_SPEECH:
		Volume = Game->_soundMgr->_volumeSpeech;
		break;
	case SOUND_MUSIC:
		Volume = Game->_soundMgr->_volumeMusic;
		break;
	}
#endif
	return setVolume(Volume);
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
HRESULT CBSoundBuffer::setPosition(uint32 Pos) {
	warning("CBSoundBuffer::SetPosition - not implemented yet");
#if 0
	if (_stream) {
		QWORD pos = BASS_ChannelSeconds2Bytes(_stream, (float)Pos / 1000.0f);
		BASS_ChannelSetPosition(_stream, pos, BASS_POS_BYTE);
	}
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::setLoopStart(uint32 Pos) {
	_loopStart = Pos;
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
	return S_OK;
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
HRESULT CBSoundBuffer::setPan(float pan) {
	if (_stream) {
		g_system->getMixer()->setChannelBalance(*_handle, pan * 127);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::applyFX(TSFXType Type, float Param1, float Param2, float Param3, float Param4) {
	warning("CBSoundBuffer::ApplyFX - not implemented yet");
#if 0
	switch (Type) {
	case SFX_ECHO:
		break;

	case SFX_REVERB:
		break;

	default:
		break;
	}
#endif
	return S_OK;
}

} // end of namespace WinterMute
