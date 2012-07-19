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
#include "engines/wintermute/base/BPackage.h"
#include "engines/wintermute/base/file/BPkgFile.h"
#include "engines/wintermute/base/BGame.h"
#include "engines/wintermute/base/BFileManager.h"
#include "common/util.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/zlib.h"

namespace WinterMute {

// HACK: wrapCompressedStream might set the size to 0, so we need a way to override it.
class CBPkgFile : public Common::SeekableReadStream {
	uint32 _size;
	Common::SeekableReadStream *_stream;
public:
	CBPkgFile(Common::SeekableReadStream *stream, uint32 knownLength) : _size(knownLength), _stream(stream) {}
	virtual ~CBPkgFile() {
		delete _stream;
	}
	virtual uint32 read(void *dataPtr, uint32 dataSize) {
		return _stream->read(dataPtr, dataSize);
	}
	virtual bool eos() const {
		return _stream->eos();
	}
	virtual int32 pos() const {
		return _stream->pos();
	}
	virtual int32 size() const {
		return _size;
	}
	virtual bool seek(int32 offset, int whence = SEEK_SET) {
		return _stream->seek(offset, whence);
	}
};

Common::SeekableReadStream *openPkgFile(const Common::String &filename, CBFileManager *fileManager) {
	CBFileEntry *fileEntry;
	Common::SeekableReadStream *file = NULL;
	char fileName[MAX_PATH_LENGTH];
	strcpy(fileName, filename.c_str());

	// correct slashes
	for (uint32 i = 0; i < strlen(fileName); i++) {
		if (fileName[i] == '/') fileName[i] = '\\';
	}

	fileEntry = fileManager->getPackageEntry(fileName);
	if (!fileEntry) return NULL;

	file = fileEntry->_package->getFilePointer();
	if (!file) return NULL;

	// TODO: Cleanup
	bool compressed = (fileEntry->_compressedLength != 0);
	/* _size = fileEntry->_length; */

	if (compressed) {
		// TODO: Really, most of this logic might be doable directly in the fileEntry?
		// But for now, this should get us rolling atleast.
		file = Common::wrapCompressedReadStream(new Common::SeekableSubReadStream(file, fileEntry->_offset, fileEntry->_offset + fileEntry->_length, DisposeAfterUse::YES));
	} else {
		file = new Common::SeekableSubReadStream(file, fileEntry->_offset, fileEntry->_offset + fileEntry->_length, DisposeAfterUse::YES);
	}
	if (file->size() == 0) {
		file = new CBPkgFile(file, fileEntry->_length);
	}

	file->seek(0);

	return file;
}

} // end of namespace WinterMute
