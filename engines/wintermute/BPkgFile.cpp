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
#include "common/util.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/zlib.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBPkgFile::CBPkgFile(CBGame *inGame): CBFile(inGame) {
	_fileEntry = NULL;
	_file = NULL;
	_compressed = false;

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

	_fileEntry = Game->_fileManager->GetPackageEntry(fileName);
	if (!_fileEntry) return E_FAIL;

	_file = _fileEntry->_package->GetFilePointer();
	if (!_file) return E_FAIL;

	// TODO: Cleanup
	_compressed = (_fileEntry->_compressedLength != 0);
	_size = _fileEntry->_length;
	
	if (_compressed) {
		// TODO: Really, most of this logic might be doable directly in the fileEntry?
		// But for now, this should get us rolling atleast.
		_file = wrapCompressedReadStream(new Common::SeekableSubReadStream(_file, _fileEntry->_offset, _fileEntry->_offset + _fileEntry->_length, DisposeAfterUse::YES));
	} else {
		_file = new Common::SeekableSubReadStream(_file, _fileEntry->_offset, _fileEntry->_offset + _fileEntry->_length, DisposeAfterUse::YES);
	}
	
	SeekToPos(0);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPkgFile::Close() {
	if (_fileEntry) {
		_fileEntry->_package->CloseFilePointer(_file);
		_fileEntry = NULL;
	}
	_file = NULL;

	// TODO: Do we really need to take care of our position and size at all (or could (Safe)SubStreams fix that for us?
	_pos = 0;
	_size = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPkgFile::Read(void *Buffer, uint32 Size) {
	if (!_fileEntry) return E_FAIL;

	HRESULT ret = S_OK;

	if (_pos + Size > _size) {
		Size = _size - _pos;
		if (Size == 0) return E_FAIL;
	}

	ret = _file->read(Buffer, Size);

	_pos += Size;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPkgFile::Seek(uint32 Pos, TSeek Origin) {
	if (!_fileEntry) return E_FAIL;

	uint32 NewPos = 0;

	switch (Origin) {
	case SEEK_TO_BEGIN:
		NewPos = Pos;
		break;
	case SEEK_TO_END:
		NewPos = _size + Pos;
		break;
	case SEEK_TO_CURRENT:
		NewPos = _pos + Pos;
		break;
	}

	if (NewPos < 0 || NewPos > _size) return E_FAIL;

	return SeekToPos(NewPos);
}


#define STREAM_BUFFER_SIZE 4096
//////////////////////////////////////////////////////////////////////////
HRESULT CBPkgFile::SeekToPos(uint32 NewPos) {
	HRESULT ret = S_OK;

	// seek compressed stream to NewPos
	_pos = NewPos;
	return ret;
}

} // end of namespace WinterMute
