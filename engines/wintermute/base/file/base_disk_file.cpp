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
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/wintypes.h"
#include "engines/wintermute/base/file/BPkgFile.h"
#include "engines/wintermute/base/file/base_disk_file.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/file.h"
#include "common/zlib.h"
#include "common/archive.h"

namespace WinterMute {

void correctSlashes(char *fileName) {
	for (size_t i = 0; i < strlen(fileName); i++) {
		if (fileName[i] == '\\') fileName[i] = '/';
	}
}

Common::SeekableReadStream *openDiskFile(const Common::String &filename, BaseFileManager *fileManager) {
	char fullPath[MAX_PATH_LENGTH];
	uint32 prefixSize = 0;
	Common::SeekableReadStream *file = NULL;

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, filename);
	
	for (Common::ArchiveMemberList::iterator it = files.begin(); it != files.end(); it++) {
		if ((*it)->getName() == filename) {
			file = (*it)->createReadStream();
			break;
		}
	}

	if (file) {
		uint32 magic1, magic2;
		magic1 = file->readUint32LE();
		magic2 = file->readUint32LE();

		bool compressed = false;
		if (magic1 == DCGF_MAGIC && magic2 == COMPRESSED_FILE_MAGIC) compressed = true;

		if (compressed) {
			uint32 dataOffset, compSize, uncompSize;
			dataOffset = file->readUint32LE();
			compSize = file->readUint32LE();
			uncompSize = file->readUint32LE();

			byte *compBuffer = new byte[compSize];
			if (!compBuffer) {
				error("Error allocating memory for compressed file '%s'", filename.c_str());
				delete file;
				return NULL;
			}

			byte *data = new byte[uncompSize];
			if (!data) {
				error("Error allocating buffer for file '%s'", filename.c_str());
				delete [] compBuffer;
				delete file;
				return NULL;
			}
			file->seek(dataOffset + prefixSize, SEEK_SET);
			file->read(compBuffer, compSize);

			if (Common::uncompress(data, (unsigned long *)&uncompSize, compBuffer, compSize) != true) {
				error("Error uncompressing file '%s'", filename.c_str());
				delete [] compBuffer;
				delete file;
				return NULL;
			}

			delete [] compBuffer;

			return new Common::MemoryReadStream(data, uncompSize, DisposeAfterUse::YES);
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
