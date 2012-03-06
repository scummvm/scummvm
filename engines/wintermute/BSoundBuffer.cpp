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

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAX_NONSTREAMED_FILE_SIZE 1024*1024

//////////////////////////////////////////////////////////////////////////
CBSoundBuffer::CBSoundBuffer(CBGame *inGame): CBBase(inGame) {
#if 0
	m_Stream = NULL;
	m_Sync = NULL;

	m_Streamed = false;
	m_Filename = NULL;
	m_File = NULL;
	m_PrivateVolume = 100;

	m_Looping = false;
	m_LoopStart = 0;

	m_Type = SOUND_SFX;

	m_FreezePaused = false;
#endif
}


//////////////////////////////////////////////////////////////////////////
CBSoundBuffer::~CBSoundBuffer() {
#if 0
	Stop();

	if (m_Stream) {
		BASS_StreamFree(m_Stream);
		m_Stream = NULL;
	}

	if (m_File) {
		Game->m_FileManager->CloseFile(m_File);
		m_File = NULL;
	}

	SAFE_DELETE_ARRAY(m_Filename);
#endif
}


//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::SetStreaming(bool Streamed, uint32 NumBlocks, uint32 BlockSize) {
	m_Streamed = Streamed;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::LoadFromFile(const char *Filename, bool ForceReload) {
#if 0
	if (m_Stream) {
		BASS_StreamFree(m_Stream);
		m_Stream = NULL;
	}

	if (m_File) Game->m_FileManager->CloseFile(m_File);

	m_File = Game->m_FileManager->OpenFile(Filename);
	if (!m_File) {
		Game->LOG(0, "Error opening sound file '%s'", Filename);
		return E_FAIL;
	}

	BASS_FILEPROCS fileProc;
	fileProc.close = CBSoundBuffer::FileCloseProc;
	fileProc.read = CBSoundBuffer::FileReadProc;
	fileProc.seek = CBSoundBuffer::FileSeekProc;
	fileProc.length = CBSoundBuffer::FileLenProc;

	m_Stream = BASS_StreamCreateFileUser(STREAMFILE_NOBUFFER, 0, &fileProc, (void *)m_File);
	if (!m_Stream) {
		Game->LOG(0, "BASS error: %d while loading '%s'", BASS_ErrorGetCode(), Filename);
		return E_FAIL;
	}

	CBUtils::SetString(&m_Filename, Filename);

	/*
	HRESULT res;
	bool NewlyCreated = false;

	if(!m_SoundBuffer || ForceReload || m_Streamed){
	    if(!m_File) m_File = Game->m_FileManager->OpenFile(Filename);
	    if(!m_File){
	        Game->LOG(0, "Error opening sound file '%s'", Filename);
	        return E_FAIL;
	    }
	    // switch to streamed for big files
	    if(!m_Streamed && (m_File->GetSize() > MAX_NONSTREAMED_FILE_SIZE && !Game->m_ForceNonStreamedSounds)) SetStreaming(true);
	}

	// create buffer
	if(!m_SoundBuffer){
	    NewlyCreated = true;

	    res = InitializeBuffer(m_File);
	    if(FAILED(res)){
	        Game->LOG(res, "Error creating sound buffer for file '%s'", Filename);
	        return res;
	    }
	}



	// store filename
	if(!m_Filename){
	    m_Filename = new char[strlen(Filename)+1];
	    strcpy(m_Filename, Filename);
	}

	// close file (if not streaming)
	if(!m_Streamed && m_File){
	    Game->m_FileManager->CloseFile(m_File);
	    m_File = NULL;
	}
	*/

	return S_OK;
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::Play(bool Looping, uint32 StartSample) {
#if 0
	if (m_Stream) {
		SetLooping(Looping);
		BASS_ChannelPlay(m_Stream, TRUE);
	}
	return S_OK;
#endif
}

//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::SetLooping(bool looping) {
#if 0
	m_Looping = looping;

	if (m_Stream) {
		BASS_ChannelFlags(m_Stream, looping ? BASS_SAMPLE_LOOP : 0, BASS_SAMPLE_LOOP);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::Resume() {
#if 0
	if (m_Stream) {
		BASS_ChannelPlay(m_Stream, FALSE);
	}
	return S_OK;
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::Stop() {
#if 0
	if (m_Stream) {
		BASS_ChannelStop(m_Stream);
	}
#endif
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::Pause() {
#if 0
	if (m_Stream) {
		BASS_ChannelPause(m_Stream);
	}
#endif
	return S_OK;

}

//////////////////////////////////////////////////////////////////////////
uint32 CBSoundBuffer::GetLength() {
#if 0
	QWORD len = BASS_ChannelGetLength(m_Stream, BASS_POS_BYTE);
	return 1000 * BASS_ChannelBytes2Seconds(m_Stream, len);
#endif
}


//////////////////////////////////////////////////////////////////////////
void CBSoundBuffer::SetType(TSoundType Type) {
	m_Type = Type;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::SetVolume(int Volume) {
#if 0
	if (m_Stream) {
		BASS_ChannelSetAttribute(m_Stream, BASS_ATTRIB_VOL, (float)Volume / 100.0f);
	}
#endif
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::SetPrivateVolume(int Volume) {
#if 0
	m_PrivateVolume = Volume;

	switch (m_Type) {
	case SOUND_SFX:
		Volume = Game->m_SoundMgr->m_VolumeSFX;
		break;
	case SOUND_SPEECH:
		Volume = Game->m_SoundMgr->m_VolumeSpeech;
		break;
	case SOUND_MUSIC:
		Volume = Game->m_SoundMgr->m_VolumeMusic;
		break;
	}
#endif
	return SetVolume(Volume);
}


//////////////////////////////////////////////////////////////////////////
bool CBSoundBuffer::IsPlaying() {
#if 0
	return m_FreezePaused || BASS_ChannelIsActive(m_Stream) == BASS_ACTIVE_PLAYING;
#endif
	return false;
}


//////////////////////////////////////////////////////////////////////////
uint32 CBSoundBuffer::GetPosition() {
#if 0
	if (m_Stream) {
		QWORD len = BASS_ChannelGetPosition(m_Stream, BASS_POS_BYTE);
		return 1000 * BASS_ChannelBytes2Seconds(m_Stream, len);
	} else return 0;
#endif
	return 0;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::SetPosition(uint32 Pos) {
#if 0
	if (m_Stream) {
		QWORD pos = BASS_ChannelSeconds2Bytes(m_Stream, (float)Pos / 1000.0f);
		BASS_ChannelSetPosition(m_Stream, pos, BASS_POS_BYTE);
	}
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSoundBuffer::SetLoopStart(uint32 Pos) {
	m_LoopStart = Pos;
#if 0
	if (m_Stream) {
		if (m_Sync) {
			BASS_ChannelRemoveSync(m_Stream, m_Sync);
			m_Sync = NULL;
		}
		if (m_LoopStart > 0) {
			QWORD len = BASS_ChannelGetLength(m_Stream, BASS_POS_BYTE);
			m_Sync = BASS_ChannelSetSync(m_Stream, BASS_SYNC_POS | BASS_SYNC_MIXTIME, len, CBSoundBuffer::LoopSyncProc, (void *)this);
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
	if (m_Stream) {
		BASS_ChannelSetAttribute(m_Stream, BASS_ATTRIB_PAN, Pan);
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
	file->Game->m_FileManager->CloseFile(file);
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
