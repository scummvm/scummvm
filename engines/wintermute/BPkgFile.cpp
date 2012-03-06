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
#include "BPackage.h"
#include "BPkgFile.h"
#include "BGame.h"
#include "BFileManager.h"

#if _DEBUG
#pragma comment(lib, "zlib_d.lib")
#else
#pragma comment(lib, "zlib.lib")
#endif


extern "C" {
#include "zlib.h"
}

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBPkgFile::CBPkgFile(CBGame *inGame): CBFile(inGame) {
	m_FileEntry = NULL;
	m_File = NULL;
	m_Compressed = false;

	m_Stream.zalloc = (alloc_func)0;
	m_Stream.zfree = (free_func)0;
	m_Stream.opaque = (voidpf)0;

	m_InflateInit = false;
}


//////////////////////////////////////////////////////////////////////////
CBPkgFile::~CBPkgFile() {
	Close();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPkgFile::Open(Common::String Filename) {
	Close();

	char fileName[MAX_PATH];
	strcpy(fileName, Filename.c_str());

	// correct slashes
	for (int i = 0; i < strlen(fileName); i++) {
		if (fileName[i] == '/') fileName[i] = '\\';
	}

	m_FileEntry = Game->m_FileManager->GetPackageEntry(fileName);
	if (!m_FileEntry) return E_FAIL;

	m_File = m_FileEntry->m_Package->GetFilePointer();
	if (!m_File) return E_FAIL;


	m_Compressed = (m_FileEntry->m_CompressedLength != 0);
	m_Size = m_FileEntry->m_Length;

	SeekToPos(0);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPkgFile::Close() {
	if (m_FileEntry) {
		m_FileEntry->m_Package->CloseFilePointer(m_File);
		m_FileEntry = NULL;
	}
	m_File = NULL;

	m_Pos = 0;
	m_Size = 0;

	if (m_InflateInit) inflateEnd(&m_Stream);
	m_InflateInit = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPkgFile::Read(void *Buffer, uint32 Size) {
	if (!m_FileEntry) return E_FAIL;

	HRESULT ret = S_OK;

	if (m_Pos + Size > m_Size) {
		Size = m_Size - m_Pos;
		if (Size == 0) return E_FAIL;
	}

	if (m_Compressed) {
		uint32 InitOut = m_Stream.total_out;

		m_Stream.avail_out = Size;
		m_Stream.next_out = (byte  *)Buffer;

		while (m_Stream.total_out - InitOut < Size && m_Stream.total_in < m_FileEntry->m_CompressedLength) {
			// needs to read more data?
			if (m_Stream.avail_in == 0) {
				m_Stream.avail_in = MIN(COMPRESSED_BUFFER_SIZE, m_FileEntry->m_CompressedLength - m_Stream.total_in);
				m_FileEntry->m_Package->Read(m_File, m_FileEntry->m_Offset + m_Stream.total_in, m_CompBuffer, m_Stream.avail_in);
				m_Stream.next_in = m_CompBuffer;
			}

			int res = inflate(&m_Stream, Z_SYNC_FLUSH);
			if (res != Z_OK && res != Z_STREAM_END) {
				Game->LOG(0, "zlib error: %d", res);
				ret = E_FAIL;
				break;
			}
		}


	} else {
		ret = m_FileEntry->m_Package->Read(m_File, m_FileEntry->m_Offset + m_Pos, (byte  *)Buffer, Size);
	}

	m_Pos += Size;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPkgFile::Seek(uint32 Pos, TSeek Origin) {
	if (!m_FileEntry) return E_FAIL;

	uint32 NewPos = 0;

	switch (Origin) {
	case SEEK_TO_BEGIN:
		NewPos = Pos;
		break;
	case SEEK_TO_END:
		NewPos = m_Size + Pos;
		break;
	case SEEK_TO_CURRENT:
		NewPos = m_Pos + Pos;
		break;
	}

	if (NewPos < 0 || NewPos > m_Size) return E_FAIL;

	return SeekToPos(NewPos);
}


#define STREAM_BUFFER_SIZE 4096
//////////////////////////////////////////////////////////////////////////
HRESULT CBPkgFile::SeekToPos(uint32 NewPos) {
	HRESULT ret = S_OK;

	// seek compressed stream to NewPos
	if (m_Compressed) {
		byte StreamBuffer[STREAM_BUFFER_SIZE];
		if (m_InflateInit) inflateEnd(&m_Stream);
		m_InflateInit = false;

		m_Stream.avail_in = 0;
		m_Stream.next_in = m_CompBuffer;
		m_Stream.avail_out = MIN(STREAM_BUFFER_SIZE, NewPos);
		m_Stream.next_out = StreamBuffer;
		inflateInit(&m_Stream);
		m_InflateInit = true;

		while (m_Stream.total_out < NewPos && m_Stream.total_in < m_FileEntry->m_CompressedLength) {
			// needs to read more data?
			if (m_Stream.avail_in == 0) {
				m_Stream.avail_in = MIN(COMPRESSED_BUFFER_SIZE, m_FileEntry->m_CompressedLength - m_Stream.total_in);
				m_FileEntry->m_Package->Read(m_File, m_FileEntry->m_Offset + m_Stream.total_in, m_CompBuffer, m_Stream.avail_in);
				m_Stream.next_in = m_CompBuffer;
			}

			// needs more space?
			if (m_Stream.avail_out == 0) {
				m_Stream.next_out = StreamBuffer;
				m_Stream.avail_out = MIN(STREAM_BUFFER_SIZE, NewPos - m_Stream.total_out);
			}

			// stream on!
			int res = inflate(&m_Stream, Z_SYNC_FLUSH);
			if (res != Z_OK && res != Z_STREAM_END) {
				ret = E_FAIL;
				break;
			}
		}

	}

	m_Pos = NewPos;
	return ret;
}

} // end of namespace WinterMute
