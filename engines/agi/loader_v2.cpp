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

#include "common/textconsole.h"

#include "agi/agi.h"
#include "agi/loader.h"
#include "agi/lzw.h"
#include "agi/words.h"

namespace Agi {

int AgiLoader_v2::loadDir(AgiDir *agid, const char *fname) {
	debug(0, "Loading directory: %s", fname);

	Common::File fp;
	if (!fp.open(fname)) {
		return errBadFileOpen;
	}

	fp.seek(0, SEEK_END);
	uint32 flen = fp.pos();
	fp.seek(0, SEEK_SET);

	uint8 *mem = (uint8 *)malloc(flen);
	if (mem == nullptr) {
		return errNotEnoughMemory;
	}

	fp.read(mem, flen);

	// read directory entries
	for (uint32 i = 0; i + 2 < flen; i += 3) {
		agid[i / 3].volume = *(mem + i) >> 4;
		agid[i / 3].offset = READ_BE_UINT24(mem + i) & (uint32) _EMPTY;
		debugC(3, kDebugLevelResources, "%d: volume %d, offset 0x%05x", i / 3, agid[i / 3].volume, agid[i / 3].offset);
	}

	free(mem);
	return errOK;
}

/**
 * Detects if the volume format is really V3.
 *
 * The CoCo3 version of Leisure Suit Larry uses a V3 volume, even
 * though it is a V2 game with V2 directory files. Sierra's other
 * CoCo3 release, King's Quest III, uses regular V2 volumes.
 * Fan ports of DOS games to CoCo3 use V3 volumes; presumably they
 * used the Leisure Suit Larry interpreter.
 * 
 * Returns true if Logic 0's volume matches the V3 format.
 */
bool AgiLoader_v2::detectV3VolumeFormat() {
	uint8 volume = _vm->_game.dirLogic[0].volume;
	Common::Path path(Common::String::format("vol.%i", volume));
	Common::File volumeFile;
	if (!volumeFile.open(path)) {
		return false;
	}

	// read the first few entries and see if they match the 7 byte header
	uint8 volumeHeader[7];
	for (int i = 0; i < 10; i++) {
		if (volumeFile.read(&volumeHeader, 7) != 7) {
			return false;
		}

		// signature
		if (READ_BE_UINT16(volumeHeader) != 0x1234) {
			return false;
		}

		// volume number (high bit == pic compression)
		if ((volumeHeader[2] & 0x7f) != volume) {
			return false;
		}

		// uncompressed and compressed resource length.
		// we can't validate these values against each other.
		// uncompressed should always be greater than or equal
		// to compressed, but fan tools compressed small resources
		// even when the result was larger than uncompressed.
		// (Coco3 fan ports of KQ4, MH1, MH2)
		uint16 compressedResourceLength = READ_LE_UINT16(volumeHeader + 5);

		if (!volumeFile.seek(compressedResourceLength, SEEK_CUR)) {
			return false;
		}
	}
	return true;
}

int AgiLoader_v2::loadDirs() {
	int ec = errOK;

	// load directory files
	ec = loadDir(_vm->_game.dirLogic, LOGDIR);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirPic, PICDIR);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirView, VIEWDIR);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirSound, SNDDIR);
	if (ec == errOK)
		_hasV3VolumeFormat = detectV3VolumeFormat();

	return ec;
}

/**
 * This function loads a raw resource into memory,
 * if further decoding is required, it must be done by another
 * routine. NULL is returned if unsuccessful.
 */
uint8 *AgiLoader_v2::loadVolumeResource(AgiDir *agid) {
	uint8 *data = nullptr;
	uint8 volumeHeader[7];
	Common::File fp;
	Common::Path path(Common::String::format("vol.%i", agid->volume));

	debugC(3, kDebugLevelResources, "Vol res: path = %s", path.toString().c_str());

	if (agid->offset != _EMPTY && fp.open(path)) {
		debugC(3, kDebugLevelResources, "loading resource at offset %d", agid->offset);
		fp.seek(agid->offset, SEEK_SET);
		fp.read(&volumeHeader, _hasV3VolumeFormat ? 7 : 5);
		uint16 signature = READ_BE_UINT16(volumeHeader);
		if (signature != 0x1234) {
			warning("AgiLoader_v2::loadVolRes: bad signature %04x", signature);
			return nullptr;
		}

		agid->len = READ_LE_UINT16(volumeHeader + 3);
		if (_hasV3VolumeFormat) {
			agid->clen = READ_LE_UINT16(volumeHeader + 5);
		} else {
			agid->clen = agid->len;
		}

		uint8 *compBuffer = (uint8 *)calloc(1, agid->clen + 32); // why the extra 32 bytes?
		fp.read(compBuffer, agid->clen);

		if ((volumeHeader[2] & 0x80) && _hasV3VolumeFormat) { // compressed pic
			// effectively uncompressed, but having only 4-bit parameters for F0 / F2 commands
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

int AgiLoader_v2::loadObjects() {
	return _vm->loadObjects(OBJECTS);
}

int AgiLoader_v2::loadWords() {
	if (_vm->getFeatures() & GF_EXTCHAR) {
		return _vm->_words->loadExtendedDictionary(WORDS);
	} else {
		return _vm->_words->loadDictionary(WORDS);
	}
}

} // End of namespace Agi
