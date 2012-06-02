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
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/wintypes.h"
#include "engines/wintermute/Base/file/BPkgFile.h"
#include "engines/wintermute/Base/file/BDiskFile.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "common/stream.h"
#include "common/file.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBDiskFile::CBDiskFile(CBGame *inGame): CBFile(inGame) {
	_file = NULL;
	_data = NULL;
	_compressed = false;
	_prefixSize = 0;
}


//////////////////////////////////////////////////////////////////////////
CBDiskFile::~CBDiskFile() {
	Close();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDiskFile::Open(const Common::String &Filename) {
	Close();

	char FullPath[MAX_PATH];

	for (int i = 0; i < Game->_fileManager->_singlePaths.GetSize(); i++) {
		sprintf(FullPath, "%s%s", Game->_fileManager->_singlePaths[i], Filename.c_str());
		correctSlashes(FullPath);
		//_file = Common::createFileStream(FullPath);
		Common::File *tempFile = new Common::File();
		if (tempFile->open(FullPath)) {
			_file = tempFile;
		} else {
			delete tempFile;
		}
		/*      if (_file != NULL) {
		            error("Tried to open %s, but failed", Filename.c_str());
		            break;
		        }*/
	}

	// if we didn't find it in search paths, try to open directly
	if (!_file) {
		strcpy(FullPath, Filename.c_str());
		correctSlashes(FullPath);
		//error("Tried to open %s, TODO: add SearchMan-support", Filename.c_str());
		//_file = Common::createFileStream(FullPath);
		Common::File *tempFile = new Common::File();
		if (tempFile->open(FullPath)) {
			_file = tempFile;
		} else {
			delete tempFile;
		}
	}

	if (_file) {
		uint32 magic1, magic2;
		magic1 = _file->readUint32LE();
		magic2 = _file->readUint32LE();

		if (magic1 == DCGF_MAGIC && magic2 == COMPRESSED_FILE_MAGIC) _compressed = true;

		if (_compressed) {
			uint32 DataOffset, CompSize, UncompSize;
			DataOffset = _file->readUint32LE();
			CompSize = _file->readUint32LE();
			UncompSize = _file->readUint32LE();

			byte *CompBuffer = new byte[CompSize];
			if (!CompBuffer) {
				Game->LOG(0, "Error allocating memory for compressed file '%s'", Filename.c_str());
				Close();
				return E_FAIL;
			}

			_data = new byte[UncompSize];
			if (!_data) {
				Game->LOG(0, "Error allocating buffer for file '%s'", Filename.c_str());
				delete [] CompBuffer;
				Close();
				return E_FAIL;
			}
			_file->seek(DataOffset + _prefixSize, SEEK_SET);
			_file->read(CompBuffer, CompSize);

			if (uncompress(_data, (uLongf *)&UncompSize, CompBuffer, CompSize) != Z_OK) {
				Game->LOG(0, "Error uncompressing file '%s'", Filename.c_str());
				delete [] CompBuffer;
				Close();
				return E_FAIL;
			}

			delete [] CompBuffer;
			_size = UncompSize;
			_pos = 0;
			delete _file;
			_file = NULL;
		} else {
			_pos = 0;
			_file->seek(0, SEEK_END);
			_size = _file->pos() - _prefixSize;
			_file->seek(_prefixSize, SEEK_SET);
		}

		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDiskFile::Close() {
	if (_file) {
		delete _file;
	}
	_file = NULL;
	_pos = 0;
	_size = 0;

	delete[] _data;
	_data = NULL;

	_compressed = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDiskFile::Read(void *buffer, uint32 size) {
	if (_compressed) {
		memcpy(buffer, _data + _pos, size);
		_pos += size;
		return S_OK;
	} else {

		if (_file) {
			size_t count = _file->read(buffer, size);
			_pos += count;
			return S_OK;
		} else return E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDiskFile::Seek(uint32 pos, TSeek origin) {
	// TODO: Should this really need to use uint32?
	if (_compressed) {
		uint32 newPos = 0;

		switch (origin) {
		case SEEK_TO_BEGIN:
			newPos = pos;
			break;
		case SEEK_TO_END:
			newPos = _size + pos;
			break;
		case SEEK_TO_CURRENT:
			newPos = _pos + pos;
			break;
		}

		if (newPos < 0 || newPos > _size) return E_FAIL;
		else _pos = newPos;
		return S_OK;
	} else {
		if (!_file) return E_FAIL;
		int ret = 1;

		switch (origin) {
		case SEEK_TO_BEGIN:
			ret = _file->seek(_prefixSize + pos, SEEK_SET);
			break;
		case SEEK_TO_END:
			ret = _file->seek(pos, SEEK_END);
			break;
		case SEEK_TO_CURRENT:
			ret = _file->seek(pos, SEEK_CUR);
			break;
		}
		if (ret == 0) {
			_pos = _file->pos() - _prefixSize;
			return S_OK;
		} else return E_FAIL;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBDiskFile::correctSlashes(char *fileName) {
	for (size_t i = 0; i < strlen(fileName); i++) {
		if (fileName[i] == '\\') fileName[i] = '/';
	}
}

} // end of namespace WinterMute
