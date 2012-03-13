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
#include "BGame.h"
#include "wintypes.h"
#include "BPkgFile.h"
#include "BDiskFile.h"
#include "common/stream.h"
#include "common/file.h"
#include "BFileManager.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBDiskFile::CBDiskFile(CBGame *inGame): CBFile(inGame) {
	m_File = NULL;
	m_Data = NULL;
	m_Compressed = false;
	m_PrefixSize = 0;
}


//////////////////////////////////////////////////////////////////////////
CBDiskFile::~CBDiskFile() {
	Close();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDiskFile::Open(Common::String Filename) {
	Close();

	char FullPath[MAX_PATH];

	for (int i = 0; i < Game->m_FileManager->m_SinglePaths.GetSize(); i++) {
		sprintf(FullPath, "%s%s", Game->m_FileManager->m_SinglePaths[i], Filename.c_str());
		CorrectSlashes(FullPath);
		
		//m_File = Common::createFileStream(FullPath);
		Common::File *tempFile = new Common::File();
		if(tempFile->open(FullPath)) {
			m_File = tempFile;
		} else {
			delete tempFile;
		}
/*		if (m_File != NULL) {
			error("Tried to open %s, but failed", Filename.c_str());
			break;
		}*/
	}

	// if we didn't find it in search paths, try to open directly
	if (!m_File) {
		strcpy(FullPath, Filename.c_str());
		CorrectSlashes(FullPath);
		//error("Tried to open %s, TODO: add SearchMan-support", Filename.c_str());
		//m_File = Common::createFileStream(FullPath);
		Common::File *tempFile = new Common::File();
		if (tempFile->open(FullPath)) {
			m_File = tempFile;
		} else {
			delete tempFile;
		}
	}

	if (!m_File) {
		warning("Couldn't load %s", Filename.c_str());
	}
	
	if (m_File) {
		uint32 magic1, magic2;
		magic1 = m_File->readUint32LE();
		magic2 = m_File->readUint32LE();

		if (magic1 == DCGF_MAGIC && magic2 == COMPRESSED_FILE_MAGIC) m_Compressed = true;

		if (m_Compressed) {
			uint32 DataOffset, CompSize, UncompSize;
			DataOffset = m_File->readUint32LE();
			CompSize = m_File->readUint32LE();
			UncompSize = m_File->readUint32LE();

			byte *CompBuffer = new byte[CompSize];
			if (!CompBuffer) {
				Game->LOG(0, "Error allocating memory for compressed file '%s'", Filename.c_str());
				Close();
				return E_FAIL;
			}

			m_Data = new byte[UncompSize];
			if (!m_Data) {
				Game->LOG(0, "Error allocating buffer for file '%s'", Filename.c_str());
				delete [] CompBuffer;
				Close();
				return E_FAIL;
			}
			m_File->seek(DataOffset + m_PrefixSize, SEEK_SET);
			m_File->read(CompBuffer, CompSize);

			if (uncompress(m_Data, (uLongf *)&UncompSize, CompBuffer, CompSize) != Z_OK) {
				Game->LOG(0, "Error uncompressing file '%s'", Filename.c_str());
				delete [] CompBuffer;
				Close();
				return E_FAIL;
			}

			delete [] CompBuffer;
			m_Size = UncompSize;
			m_Pos = 0;
			delete m_File;
			m_File = NULL;
		} else {
			m_Pos = 0;
			m_File->seek(0, SEEK_END);
			m_Size = m_File->pos() - m_PrefixSize;
			m_File->seek(m_PrefixSize,SEEK_SET);
		}

		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDiskFile::Close() {
	if (m_File) {
		delete m_File;
	}	
	m_File = NULL;
	m_Pos = 0;
	m_Size = 0;

	delete[] m_Data;
	m_Data = NULL;

	m_Compressed = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDiskFile::Read(void *Buffer, uint32 Size) {
	if (m_Compressed) {
		memcpy(Buffer, m_Data + m_Pos, Size);
		m_Pos += Size;
		return S_OK;
	} else {

		if (m_File) {
			size_t count = m_File->read(Buffer, Size);
			m_Pos += count;
			return S_OK;
		} else return E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDiskFile::Seek(uint32 Pos, TSeek Origin) {
	if (m_Compressed) {
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
		else m_Pos = NewPos;
		return S_OK;
	} else {
		if (!m_File) return E_FAIL;
		int ret = 1;

		switch (Origin) {
		case SEEK_TO_BEGIN:
			ret = m_File->seek(m_PrefixSize + Pos, SEEK_SET);
			break;
		case SEEK_TO_END:
			ret = m_File->seek(Pos, SEEK_END);
			break;
		case SEEK_TO_CURRENT:
			ret = m_File->seek(Pos, SEEK_CUR);
			break;
		}
		if (ret == 0) {
			m_Pos = m_File->pos() - m_PrefixSize;
			return S_OK;
		} else return E_FAIL;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBDiskFile::CorrectSlashes(char *fileName) {
	for (size_t i = 0; i < strlen(fileName); i++) {
		if (fileName[i] == '\\') fileName[i] = '/';
	}
}

} // end of namespace WinterMute
