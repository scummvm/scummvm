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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
#include "engines/wintermute/base/file/base_disk_file.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/utils/path_util.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/file.h"
#include "common/zlib.h"
#include "common/archive.h"
#include "common/tokenizer.h"
#include "common/config-manager.h"


namespace Wintermute {

void correctSlashes(Common::String &fileName) {
	for (size_t i = 0; i < fileName.size(); i++) {
		if (fileName[i] == '\\') {
			fileName.setChar('/', i);
		}
	}
}

// Parse a relative path in the game-folder, and if it exists, return a FSNode to it.
static Common::FSNode getNodeForRelativePath(const Common::String &filename) {
	// The filename can be an explicit path, thus we need to chop it up, expecting the path the game
	// specifies to follow the Windows-convention of folder\subfolder\file (absolute paths should not happen)

	// Absolute path: These should have been handled in openDiskFile.
	if (filename.contains(':')) {
		// So just return an invalid node.
		return Common::FSNode();
	}

	// Relative path:
	if (filename.contains('\\')) {
		Common::StringTokenizer path(filename, "\\");

		// Start traversing relative to the game-data-dir
		const Common::FSNode gameDataDir(ConfMan.get("path"));
		Common::FSNode curNode = gameDataDir;

		// Parse all path-elements
		while (!path.empty()) {
			// Get the next path-component by slicing on '\\'
			Common::String pathPart = path.nextToken();
			// Get the next FSNode in the chain, if it exists as a child from the previous.
			curNode = curNode.getChild(pathPart);
			if (!curNode.isReadable()) {
				// Return an invalid FSNode.
				return Common::FSNode();
			}
			// Following the comments in common/fs.h, anything not a directory is a file.
			if (!curNode.isDirectory()) {
				if (!path.empty()) {
					error("Relative path %s reached a file before the end of the path", filename.c_str());
				}
				return curNode;
			}
		}
	}
	// Return an invalid FSNode to mark that we didn't find the requested file.
	return Common::FSNode();
}

bool diskFileExists(const Common::String &filename) {
	// Try directly from SearchMan first
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, filename);

	for (Common::ArchiveMemberList::iterator it = files.begin(); it != files.end(); ++it) {
		if ((*it)->getName() == filename) {
			return true;
		}
	}
	// File wasn't found in SearchMan, try to parse the path as a relative path.
	Common::FSNode searchNode = getNodeForRelativePath(filename);
	if (searchNode.exists() && !searchNode.isDirectory() && searchNode.isReadable()) {
		return true;
	}
	return false;
}

Common::SeekableReadStream *openDiskFile(const Common::String &filename) {
	uint32 prefixSize = 0;
	Common::SeekableReadStream *file = nullptr;
	Common::String fixedFilename = filename;
	correctSlashes(fixedFilename);

	// HACK: There are a few games around which mistakenly refer to absolute paths in the scripts.
	// The original interpreter on Windows usually simply ignores them when it can't find them.
	// We try to turn the known ones into relative paths.
	if (fixedFilename.contains(':')) {
		const char* const knownPrefixes[] = { // Known absolute paths
				"c:/windows/fonts/", // East Side Story refers to "c:\windows\fonts\framd.ttf"
				"c:/carol6/svn/data/", // Carol Reed 6: Black Circle refers to "c:\carol6\svn\data\sprites\system\help.png"
				"d:/engine/\0xD2\0xC32/tg_ie_080128_1005/data/", // Tanya Grotter and the Disappearing Floor refers to "d:\engine\<\0xD2><\0xC3>2\tg_ie_080128_1005\data\interface\pixel\pixel.png"
				"f:/dokument/spel 5/demo/data/" // Carol Reed 5 (non-demo) refers to "f:\dokument\spel 5\demo\data\scenes\credits\op_cred_00\op_cred_00.jpg"
		};

		bool matched = false;

		for (uint i = 0; i < ARRAYSIZE(knownPrefixes); i++) {
			if (fixedFilename.hasPrefix(knownPrefixes[i])) {
				fixedFilename = fixedFilename.c_str() + strlen(knownPrefixes[i]);
				matched = true;
			}
		}

		if (!matched) {
			// fixedFilename is unchanged and thus still broken, none of the above workarounds worked.
			// We can only bail out
			error("openDiskFile::Absolute path or invalid filename used in %s", filename.c_str());
		}
	}
	// Try directly from SearchMan first
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, fixedFilename);

	for (Common::ArchiveMemberList::iterator it = files.begin(); it != files.end(); ++it) {
		if ((*it)->getName().equalsIgnoreCase(lastPathComponent(fixedFilename,'/'))) {
			file = (*it)->createReadStream();
			break;
		}
	}
	// File wasn't found in SearchMan, try to parse the path as a relative path.
	if (!file) {
		Common::FSNode searchNode = getNodeForRelativePath(PathUtil::normalizeFileName(filename));
		if (searchNode.exists() && !searchNode.isDirectory() && searchNode.isReadable()) {
			file = searchNode.createReadStream();
		}
	}
	if (file) {
		uint32 magic1, magic2;
		magic1 = file->readUint32LE();
		magic2 = file->readUint32LE();

		bool compressed = false;
		if (magic1 == DCGF_MAGIC && magic2 == COMPRESSED_FILE_MAGIC) {
			compressed = true;
		}

		if (compressed) {
			uint32 dataOffset, compSize;
			unsigned long uncompSize;
			dataOffset = file->readUint32LE();
			compSize = file->readUint32LE();
			uncompSize = file->readUint32LE();

			byte *compBuffer = new byte[compSize];
			if (!compBuffer) {
				error("Error allocating memory for compressed file '%s'", filename.c_str());
				delete file;
				return nullptr;
			}

			byte *data = new byte[uncompSize];
			if (!data) {
				error("Error allocating buffer for file '%s'", filename.c_str());
				delete[] compBuffer;
				delete file;
				return nullptr;
			}
			file->seek(dataOffset + prefixSize, SEEK_SET);
			file->read(compBuffer, compSize);

			if (Common::uncompress(data, &uncompSize, compBuffer, compSize) != true) {
				error("Error uncompressing file '%s'", filename.c_str());
				delete[] compBuffer;
				delete file;
				return nullptr;
			}

			delete[] compBuffer;
			delete file;
			return new Common::MemoryReadStream(data, uncompSize, DisposeAfterUse::YES);
		} else {
			file->seek(0, SEEK_SET);
			return file;
		}

		return file;

	}
	return nullptr;
}

} // End of namespace Wintermute
