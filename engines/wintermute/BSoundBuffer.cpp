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

#include "dcgf.h"
#include "BFile.h"
#include "BGame.h"
#include "BSoundMgr.h"
#include "BSoundBuffer.h"
#include "BFileManager.h"
#include "utils.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/vorbis.h"
#include "common/system.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAX_NONSTREAMED_FILE_SIZE 1024*1024

//////////////////////////////////////////////////////////////////////////
CBSoundBuffer::CBSoundBuffer(CBGame *inGame): CBBase(inGame) {
	_stream = NULL;
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
#if 0
	Stop();

	if (_stream) {
		BASS_StreamFree(_stream);
		_stream = NULL;
	}

	if (_file) {
		Game->_fileManager->CloseFile(_file);
		_file = NULL;
	}

	SAFE_DELETE_ARRAY(_filename);
#endif
}


//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::SetStreaming(bool Streamed, uint32 NumBlocks, uint32 BlockSize) {
	_streamed = Streamed;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::LoadFromFile(const char *Filename, bool ForceReload) {
	warning("BSoundBuffer::LoadFromFile(%s,%d)", Filename, ForceReload);
#if 0
	if (_stream) {
		BASS_StreamFree(_stream);
		_stream = NULL;
	}
#endif
	delete _stream;
	_stream = NULL;

	if (_file) Game->_fileManager->CloseFile(_file);

	_file = Game->_fileManager->OpenFile(Filename);
	if (!_file) {
		Game->LOG(0, "Error opening sound file '%s'", Filename);
		return E_FAIL;
	}
	
	_stream = Audio::makeVorbisStream(_file->getMemStream(), DisposeAfterUse::YES);
	CBUtils::SetString(&_filename, Filename);
	
	return S_OK;
#if 0
	BASS_FILEPROCS fileProc;
	fileProc.close = CBSoundBuffer::FileCloseProc;
	fileProc.read = CBSoundBuffer::FileReadProc;
	fileProc.seek = CBSoundBuffer::FileSeekProc;
	fileProc.length = CBSoundBuffer::FileLenProc;

	_stream = BASS_StreamCreateFileUser(STREAMFILE_NOBUFFER, 0, &fileProc, (void *)_file);
	if (!_stream) {
		Game->LOG(0, "BASS error: %d while loading '%s'", BASS_ErrorGetCode(), Filename);
		return E_FAIL;
	}

	CBUtils::SetString(&_filename, Filename);

	/*
	HRESULT res;
	bool NewlyCreated = false;

	if(!_soundBuffer || ForceReload || _streamed){
	    if(!_file) _file = Game->_fileManager->OpenFile(Filename);
	    if(!_file){
	        Game->LOG(0, "Error opening sound file '%s'", Filename);
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
	        Game->LOG(res, "Error creating sound buffer for file '%s'", Filename);
	        return res;
	    }
	}



	// store filename
	if(!_filename){
	    _filename = new char[strlen(Filename)+1];
	    strcpy(_filename, Filename);
	}

	// close file (if not streaming)
	if(!_streamed && _file){
	    Game->_fileManager->CloseFile(_file);
	    _file = NULL;
	}
	*/

	return S_OK;
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::Play(bool Looping, uint32 StartSample) {
	warning("Play: %s", _filename);
	if (_stream) {
		SetLooping(Looping);
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, _handle, _stream);
		//BASS_ChannelPlay(_stream, TRUE);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::SetLooping(bool looping) {
#if 0
	_looping = looping;

	if (_stream) {
		BASS_ChannelFlags(_stream, looping ? BASS_SAMPLE_LOOP : 0, BASS_SAMPLE_LOOP);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::Resume() {
#if 0
	if (_stream) {
		BASS_ChannelPlay(_stream, FALSE);
	}
	return S_OK;
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::Stop() {
#if 0
	if (_stream) {
		BASS_ChannelStop(_stream);
	}
#endif
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::Pause() {
#if 0
	if (_stream) {
		BASS_ChannelPause(_stream);
	}
#endif
	return S_OK;

}

//////////////////////////////////////////////////////////////////////////
uint32 CBSoundBuffer::GetLength() {
#if 0
	QWORD len = BASS_ChannelGetLength(_stream, BASS_POS_BYTE);
	return 1000 * BASS_ChannelBytes2Seconds(_stream, len);
#endif
}


//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::SetType(TSoundType Type) {
	_type = Type;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::SetVolume(int Volume) {
#if 0
	if (_stream) {
		BASS_ChannelSetAttribute(_stream, BASS_ATTRIB_VOL, (float)Volume / 100.0f);
	}
#endif
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::SetPrivateVolume(int Volume) {
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
	return SetVolume(Volume);
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::IsPlaying() {
#if 0
	return _freezePaused || BASS_ChannelIsActive(_stream) == BASS_ACTIVE_PLAYING;
#endif
	return false;
}


//////////////////////////////////////////////////////////////////////////
uint32 CBSoundBuffer::GetPosition() {
#if 0
	if (_stream) {
		QWORD len = BASS_ChannelGetPosition(_stream, BASS_POS_BYTE);
		return 1000 * BASS_ChannelBytes2Seconds(_stream, len);
	} else return 0;
#endif
	return 0;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::SetPosition(uint32 Pos) {
#if 0
	if (_stream) {
		QWORD pos = BASS_ChannelSeconds2Bytes(_stream, (float)Pos / 1000.0f);
		BASS_ChannelSetPosition(_stream, pos, BASS_POS_BYTE);
	}
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::SetLoopStart(uint32 Pos) {
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
HRESULT CBSoundBuffer::SetPan(float Pan) {
#if 0
	if (_stream) {
		BASS_ChannelSetAttribute(_stream, BASS_ATTRIB_PAN, Pan);
	}
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::ApplyFX(TSFXType Type, float Param1, float Param2, float Param3, float Param4) {
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

#if 0
//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::FileCloseProc(void *user) {
	/*
	CBFile* file = static_cast<CBFile*>(user);
	file->Game->_fileManager->CloseFile(file);
	*/
}

//////////////////////////////////////////////////////////////////////////
QWORD CBSoundBuffer::FileLenProc(void *user) {
	CBFile *file = static_cast<CBFile *>(user);
	return file->GetSize();
}

//////////////////////////////////////////////////////////////////////////
uint32 CBSoundBuffer::FileReadProc(void *buffer, uint32 length, void *user) {
	CBFile *file = static_cast<CBFile *>(user);
	uint32 oldPos = file->GetPos();
	file->Read(buffer, length);
	return file->GetPos() - oldPos;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBSoundBuffer::FileSeekProc(QWORD offset, void *user) {
	CBFile *file = static_cast<CBFile *>(user);
	return SUCCEEDED(file->Seek(offset));
}
#endif
} // end of namespace WinterMute
