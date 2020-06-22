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
	if (filename.contains('/')) {
		Common::StringTokenizer path(filename, "/");

		// Start traversing relative to the game-data-dir
		const Common::FSNode gameDataDir(ConfMan.get("path"));
		Common::FSNode curNode = gameDataDir;

		// Parse all path-elements
		while (!path.empty()) {
			// Get the next path-component by slicing on '/'
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


int listMatchingDiskFileMembers(Common::ArchiveMemberList &list, const Common::String &pattern) {
	return Common::FSDirectory(ConfMan.get("path")).listMatchingMembers(list, pattern);
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
				"c:/documents and settings/radimk/plocha/projekt/", // Basis Octavus refers to several files named "c:\documents and settings\radimk\plocha\projekt\sprites\clock*.bmp"
				"c:/program files/wme devkit beta/projects/amu/data/", // Five Magical Amulets refers to "c:\program files\wme devkit beta\projects\amu\data\scenes\prvnimenu\scr\scene_init.script"
				"c:/users/mathieu/desktop/wintermute engine development kit/jeu verve/vervegame/data/", // Machu Mayu refers to "c:\users\mathieu\desktop\wintermute engine development kit\jeu verve\vervegame\data\interface\system\cr<0xE9>dits.script"
				"c:/windows/fonts/", // East Side Story refers to "c:\windows\fonts\framd.ttf"
				"c:/carol6/svn/data/", // Carol Reed 6: Black Circle refers to "c:\carol6\svn\data\sprites\system\help.png"
				"d:/engine/\322\303" "2/tg_ie_080128_1005/data/", // Tanya Grotter and the Disappearing Floor refers to "d:\engine\<0xD2><0xC3>2\tg_ie_080128_1005\data\interface\pixel\pixel.png"
				"e:/users/jonathan/onedrive/knossos/data/", // K'NOSSOS refers to "e:\users\jonathan\onedrive\knossos\data\entities\helprobot\helprobot.script"
				"f:/dokument/spel 5/demo/data/", // Carol Reed 5 (non-demo) refers to "f:\dokument\spel 5\demo\data\scenes\credits\op_cred_00\op_cred_00.jpg"
				"f:/quest!!!/engine/quest/data/" // Book of Gron Part One refers to several files named "f:\quest!!!\engine\quest\data\entities\dver\*"
		};

	// There are also some EDITOR_BG_FILE paths that refer to absolute paths
	// However, EDITOR_BG_FILE is out of ScummVM scope, so there is currently no need to check for those prefixes:
	// "c:\documents and settings\kumilanka\desktop\white.png" is used as EDITOR_BG_FILE at Alpha Polaris
	// "c:\documents and settings\nir\desktop\untitled111.bmp" is used as EDITOR_BG_FILE at Pizza Morgana: Episode 1 - Monsters and Manipulations in the Magical Forest
	// "c:\documents and settings\user\desktop\untitled111.bmp" is used as EDITOR_BG_FILE at Pizza Morgana: Episode 1 - Monsters and Manipulations in the Magical Forest
	// "c:\dokumente und einstellungen\frank\desktop\position_qualm_auf_kaputter_jungfrau_2.png" is used as EDITOR_BG_FILE at 1 1/2 Ritter: Auf der Suche nach der hinreissenden Herzelinde
	// "c:\tib_forest_d_2007120_111637000.jpg" is used as EDITOR_BG_FILE at Fairy Tales About Toshechka and Boshechka
	// "c:\<0xC1><0xE5><0xE7><0xFB><0xEC><0xFF><0xED><0xFB><0xE9>.bmp" is used as EDITOR_BG_FILE at Fairy Tales About Toshechka and Boshechka
	// "d:\projects\dirty split\tempfolder\background\sprite_help.png" is used as EDITOR_BG_FILE at Dirty Split
	// "d:\<0xCE><0xE1><0xEC><0xE5><0xED>\*" are used as EDITOR_BG_FILE at Fairy Tales About Toshechka and Boshechka
	// "e:\pictures\fraps screenshot\*" are used as EDITOR_BG_FILE at Fairy Tales About Toshechka and Boshechka
	// "e:\work\!<0xC4><0xE5><0xEB><0xE0>\1 computergames\6 gamelet\work\*" are used as EDITOR_BG_FILE at Hamlet or the last game without MMORPG features, shaders and product placement
	// "e:\<0xC4><0xE5><0xE5><0xE2>\graphics\*" are used as EDITOR_BG_FILE at Fairy Tales About Toshechka and Boshechka
	// "f:\quest!!!\*" are used as EDITOR_BG_FILE at Book of Gron Part One
	// "f:\<0xD2><0xE5><0xEA><0xF1><0xF2><0xF3><0xF0><0xFB>\<0xE1><0xEE><0xF2><0xE0><0xED><0xE8><0xEA><0xE0>\index\barks.jpg" is used as EDITOR_BG_FILE at Tanya Grotter and the Disappearing Floor
	// "g:\<0xC4><0xEE><0xEA><0xF3><0xEC><0xE5><0xED><0xF2><0xFB>\<0xCF><0xF0><0xEE><0xE5><0xEA><0xF2><0xFB>\<0xD2><0xE8><0xC1>\*" are used as EDITOR_BG_FILE at Fairy Tales About Toshechka and Boshechka
	// "untitled-1.jpg" & "untitled-1.png" with very various paths (including "c:\untitled-1.jpg" and "d:\untitled-1.jpg") are also used as EDITOR_BG_FILE at Fairy Tales About Toshechka and Boshechka

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
