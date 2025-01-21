/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "agi/agi.h"
#include "agi/loader.h"
#include "agi/lzw.h"
#include "agi/words.h"

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/textconsole.h"

namespace Agi {

void AgiLoader_v3::init() {
	// Find the game's name by locating a file that ends in "dir".
	// Amiga games don't use the game's name as a prefix.
	_name.clear();
	Common::FSList fslist;
	Common::FSNode dir(ConfMan.getPath("path"));

	if (!dir.getChildren(fslist, Common::FSNode::kListFilesOnly)) {
		warning("AgiLoader_v3: invalid game path '%s'", dir.getPath().toString(Common::Path::kNativeSeparator).c_str());
		return;
	}

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		Common::String fileName = file->getName();
		if (fileName.size() > 3 && fileName.hasSuffixIgnoreCase("dir")) {
			_name = fileName.substr(0, fileName.size() - 3);
			_name.toLowercase();
			debugC(3, kDebugLevelResources, "game name: %s", _name.c_str());
			break;
		}
	}
}

int AgiLoader_v3::loadDir(AgiDir *agid, Common::File *fp, uint32 offs, uint32 len) {
	fp->seek(offs, SEEK_SET);
	uint8 *mem = (uint8 *)malloc(len);
	if (mem == nullptr) {
		return errNotEnoughMemory;
	}

	fp->read(mem, len);

	// read directory entries
	for (uint32 i = 0; i + 2 < len; i += 3) {
		agid[i / 3].volume = *(mem + i) >> 4;
		agid[i / 3].offset = READ_BE_UINT24(mem + i) & (uint32) _EMPTY;
		debugC(3, kDebugLevelResources, "%d: volume %d, offset 0x%05x", i / 3, agid[i / 3].volume, agid[i / 3].offset);
	}	

	free(mem);
	return errOK;
}

int AgiLoader_v3::loadDirs() {
	int ec = errOK;
	uint8 fileHeader[8];
	Common::File fp;
	Common::Path path;

	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		// Amiga directory file is always named "dirs"
		path = Common::Path("dirs");
	} else {
		if (_name.empty()) {
			warning("AgiLoader_v3: directory file not found");
			return errBadResource;
		}
		path = Common::Path(_name + DIR_);
	}

	if (!fp.open(path)) {
		warning("Failed to open '%s'", path.toString().c_str());
		return errBadFileOpen;
	}
	// build offset table for v3 directory format
	fp.read(&fileHeader, 8);
	fp.seek(0, SEEK_END);

	uint16 dirOffsets[4];
	for (int i = 0; i < 4; i++)
		dirOffsets[i] = READ_LE_UINT16(&fileHeader[i * 2]);

	uint32 dirLengths[4];
	dirLengths[0] = dirOffsets[1] - dirOffsets[0];
	dirLengths[1] = dirOffsets[2] - dirOffsets[1];
	dirLengths[2] = dirOffsets[3] - dirOffsets[2];
	dirLengths[3] = fp.pos() - dirOffsets[3];

	if (dirLengths[3] > 256 * 3)
		dirLengths[3] = 256 * 3;

	fp.seek(0, SEEK_SET);

	// read in directory files
	ec = loadDir(_vm->_game.dirLogic, &fp, dirOffsets[0], dirLengths[0]);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirPic, &fp, dirOffsets[1], dirLengths[1]);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirView, &fp, dirOffsets[2], dirLengths[2]);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirSound, &fp, dirOffsets[3], dirLengths[3]);

	return ec;
}

/**
 * This function loads a raw resource into memory.
 * If further decoding is required, it must be done by another
 * routine.
 *
 * NULL is returned if unsuccessful.
 */
uint8 *AgiLoader_v3::loadVolumeResource(AgiDir *agid) {
	uint8 volumeHeader[7];
	uint8 *data = nullptr;
	Common::File fp;
	Common::Path path;

	debugC(3, kDebugLevelResources, "(%p)", (void *)agid);
	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		path = Common::String::format("vol.%i", agid->volume);
	} else {
		path = Common::String::format("%svol.%i", _name.c_str(), agid->volume);
	}

	if (agid->offset != _EMPTY && fp.open(path)) {
		fp.seek(agid->offset, SEEK_SET);
		fp.read(&volumeHeader, 7);

		uint16 signature = READ_BE_UINT16(volumeHeader);
		if (signature != 0x1234) {
			warning("AgiLoader_v3::loadVolRes: bad signature %04x", signature);
			return nullptr;
		}

		agid->len = READ_LE_UINT16(volumeHeader + 3);    // uncompressed size
		agid->clen = READ_LE_UINT16(volumeHeader + 5);   // compressed len

		uint8 *compBuffer = (uint8 *)calloc(1, agid->clen + 32); // why the extra 32 bytes?
		fp.read(compBuffer, agid->clen);

		if (volumeHeader[2] & 0x80) { // compressed pic
			// effectively uncompressed, but having only 4-bit parameters for F0 / F2 commands
			// Manhunter 2 uses such pictures
			data = compBuffer;
			agid->flags |= RES_PICTURE_V3_NIBBLE_PARM;
		} else if (agid->len == agid->clen) {
			// do not decompress
			data = compBuffer;
		} else {
			// it is compressed
			data = (uint8 *)calloc(1, agid->len + 32); // why the extra 32 bytes?
			lzwExpand(compBuffer, data, agid->len);
			free(compBuffer);
			agid->flags |= RES_COMPRESSED;
		}
	} else {
		// we have a bad volume resource
		// set that resource to NA
		agid->offset = _EMPTY;
	}

	return data;
}

int AgiLoader_v3::loadObjects() {
	return _vm->loadObjects(OBJECTS);
}

int AgiLoader_v3::loadWords() {
	return _vm->_words->loadDictionary(WORDS);
}

} // End of namespace Agi
