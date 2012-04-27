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
	_fileEntry = NULL;
	_file = NULL;
	_compressed = false;

	_stream.zalloc = (alloc_func)0;
	_stream.zfree = (free_func)0;
	_stream.opaque = (voidpf)0;

	_inflateInit = false;
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


	_compressed = (_fileEntry->_compressedLength != 0);
	_size = _fileEntry->_length;

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

	_pos = 0;
	_size = 0;

	if (_inflateInit) inflateEnd(&_stream);
	_inflateInit = false;

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

	if (_compressed) {
		uint32 InitOut = _stream.total_out;

		_stream.avail_out = Size;
		_stream.next_out = (byte  *)Buffer;

		while (_stream.total_out - InitOut < Size && _stream.total_in < _fileEntry->_compressedLength) {
			// needs to read more data?
			if (_stream.avail_in == 0) {
				_stream.avail_in = MIN((long unsigned int)COMPRESSED_BUFFER_SIZE, _fileEntry->_compressedLength - _stream.total_in); // TODO: long unsigned int????
				_fileEntry->_package->Read(_file, _fileEntry->_offset + _stream.total_in, _compBuffer, _stream.avail_in);
				_stream.next_in = _compBuffer;
			}

			int res = inflate(&_stream, Z_SYNC_FLUSH);
			if (res != Z_OK && res != Z_STREAM_END) {
				Game->LOG(0, "zlib error: %d", res);
				ret = E_FAIL;
				break;
			}
		}


	} else {
		ret = _fileEntry->_package->Read(_file, _fileEntry->_offset + _pos, (byte  *)Buffer, Size);
	}

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
	if (_compressed) {
		byte StreamBuffer[STREAM_BUFFER_SIZE];
		if (_inflateInit) inflateEnd(&_stream);
		_inflateInit = false;

		_stream.avail_in = 0;
		_stream.next_in = _compBuffer;
		_stream.avail_out = MIN((uint32)STREAM_BUFFER_SIZE, NewPos); //TODO: remove cast.
		_stream.next_out = StreamBuffer;
		inflateInit(&_stream);
		_inflateInit = true;

		while (_stream.total_out < NewPos && _stream.total_in < _fileEntry->_compressedLength) {
			// needs to read more data?
			if (_stream.avail_in == 0) {
				_stream.avail_in = MIN((long unsigned int)COMPRESSED_BUFFER_SIZE, _fileEntry->_compressedLength - _stream.total_in); // TODO: long unsigned int???
				_fileEntry->_package->Read(_file, _fileEntry->_offset + _stream.total_in, _compBuffer, _stream.avail_in);
				_stream.next_in = _compBuffer;
			}

			// needs more space?
			if (_stream.avail_out == 0) {
				_stream.next_out = StreamBuffer;
				_stream.avail_out = MIN((long unsigned int)STREAM_BUFFER_SIZE, NewPos - _stream.total_out); // TODO: long unsigned int???.
			}

			// stream on!
			int res = inflate(&_stream, Z_SYNC_FLUSH);
			if (res != Z_OK && res != Z_STREAM_END) {
				ret = E_FAIL;
				break;
			}
		}

	}

	_pos = NewPos;
	return ret;
}

} // end of namespace WinterMute
