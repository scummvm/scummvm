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
#include "common/memstream.h"
#include "common/file.h"
#include "common/zlib.h"

namespace WinterMute {

void correctSlashes(char *fileName) {
	for (size_t i = 0; i < strlen(fileName); i++) {
		if (fileName[i] == '\\') fileName[i] = '/';
	}
}

Common::SeekableReadStream *openDiskFile(const Common::String &filename, CBFileManager *fileManager) {
	char fullPath[MAX_PATH];
	uint32 prefixSize = 0;
	Common::SeekableReadStream *file = NULL;

	for (int i = 0; i < fileManager->_singlePaths.GetSize(); i++) {
		sprintf(fullPath, "%s%s", fileManager->_singlePaths[i], filename.c_str());
		correctSlashes(fullPath);
		Common::File *tempFile = new Common::File();
		if (tempFile->open(fullPath)) {
			file = tempFile;
		} else {
			delete tempFile;
		}
	}

	// if we didn't find it in search paths, try to open directly
	if (!file) {
		strcpy(fullPath, filename.c_str());
		correctSlashes(fullPath);

		Common::File *tempFile = new Common::File();
		if (tempFile->open(fullPath)) {
			file = tempFile;
		} else {
			delete tempFile;
		}
	}

	if (file) {
		uint32 magic1, magic2;
		magic1 = file->readUint32LE();
		magic2 = file->readUint32LE();

		bool compressed = false;
		if (magic1 == DCGF_MAGIC && magic2 == COMPRESSED_FILE_MAGIC) compressed = true;

		if (compressed) {
			uint32 DataOffset, CompSize, UncompSize;
			DataOffset = file->readUint32LE();
			CompSize = file->readUint32LE();
			UncompSize = file->readUint32LE();

			byte *CompBuffer = new byte[CompSize];
			if (!CompBuffer) {
				error("Error allocating memory for compressed file '%s'", filename.c_str());
				delete file;
				return NULL;
			}

			byte *data = new byte[UncompSize];
			if (!data) {
				error("Error allocating buffer for file '%s'", filename.c_str());
				delete [] CompBuffer;
				delete file;
				return NULL;
			}
			file->seek(DataOffset + prefixSize, SEEK_SET);
			file->read(CompBuffer, CompSize);

			if (Common::uncompress(data, (unsigned long *)&UncompSize, CompBuffer, CompSize) != true) {
				error("Error uncompressing file '%s'", filename.c_str());
				delete [] CompBuffer;
				delete file;
				return NULL;
			}

			delete [] CompBuffer;

			return new Common::MemoryReadStream(data, UncompSize, DisposeAfterUse::YES);
			delete file;
			file = NULL;
		} else {
			file->seek(0, SEEK_SET);
			return file;
		}

		return file;

	}
	return NULL;
}

} // end of namespace WinterMute
