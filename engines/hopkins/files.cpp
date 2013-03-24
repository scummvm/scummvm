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

#include "hopkins/files.h"

#include "hopkins/hopkins.h"
#include "hopkins/globals.h"

#include "common/system.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/str.h"
#include "common/savefile.h"

namespace Hopkins {

FileManager::FileManager(HopkinsEngine *vm) {
	_vm = vm;
}

/**
 * Load a file
 */
byte *FileManager::loadFile(const Common::String &file) {
	Common::File f;
	if (!f.open(file))
		error("Error opening %s", file.c_str());

	// Allocate space for the file contents
	size_t filesize = f.size();
	byte *data = _vm->_globals->allocMemory(filesize);
	if (!data)
		error("Error allocating space for file being loaded - %s", file.c_str());

	readStream(f, data, filesize);
	f.close();

	return data;
}

/**
 * Read a given number of bytes from a Stream into a pre-allocated buffer
 */
int FileManager::readStream(Common::ReadStream &stream, void *buf, size_t nbytes) {
	return stream.read(buf, nbytes);
}

/**
 * Initialize censorship based on blood.dat file
 */
void FileManager::initCensorship() {
	_vm->_globals->_censorshipFl = false;

	// If file doesn't exist, fallback to uncensored
	if (fileExists("BLOOD.DAT")) {
		char *data = (char *)loadFile("BLOOD.DAT");

		if ((data[6] == 'u' && data[7] == 'k') || (data[6] == 'U' && data[7] == 'K'))
			_vm->_globals->_censorshipFl = true;

		_vm->_globals->freeMemory((byte *)data);
	}
}

/**
 * Check if a file is present
 */
bool FileManager::fileExists(const Common::String &file) {
	Common::File f;

	return f.exists(file);
}

/**
 * Search file in Cat file
 */
byte *FileManager::searchCat(const Common::String &file, CatMode mode) {
	byte *ptr = NULL;
	Common::File f;

	Common::String filename = file;
	Common::String secondaryFilename = "";
	filename.toUppercase();

	switch (mode) {
	case RES_INI:
		if (!f.exists("RES_INI.CAT"))
			return g_PTRNUL;

		ptr = loadFile("RES_INI.CAT");
		secondaryFilename = "RES_INI.RES";
		break;

	case RES_REP:
		if (!f.exists("RES_REP.CAT"))
			return g_PTRNUL;

		ptr = loadFile("RES_REP.CAT");
		secondaryFilename = "RES_REP.RES";
		break;

	case RES_LIN:
		if (!f.exists("RES_LIN.CAT"))
			return g_PTRNUL;

		ptr = loadFile("RES_LIN.CAT");
		secondaryFilename = "RES_LIN.RES";
		break;

	case RES_PER:
		if (!f.exists("RES_PER.CAT"))
			return g_PTRNUL;

		ptr = loadFile("RES_PER.CAT");
		secondaryFilename = "RES_PER.RES";
		break;

	case RES_PIC:
		if (!f.exists("PIC.CAT"))
			return g_PTRNUL;

		ptr = loadFile("PIC.CAT");
		break;

	case RES_SAN:
		if (!f.exists("RES_SAN.CAT"))
			return g_PTRNUL;

		ptr = loadFile("RES_SAN.CAT");
		break;

	case RES_SLI:
		if (!f.exists("RES_SLI.CAT"))
			return g_PTRNUL;

		ptr = loadFile("RES_SLI.CAT");
		break;

	case RES_VOI: {
		Common::String tmpFilename;
		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			tmpFilename = "ENG_VOI.CAT";
		// Win95 and Linux versions uses another set of names
		else {
			switch (_vm->_globals->_language) {
			case LANG_EN:
				tmpFilename = "RES_VAN.CAT";
				break;
			case LANG_FR:
				tmpFilename = "RES_VFR.CAT";
				break;
			case LANG_SP:
				tmpFilename = "RES_VES.CAT";
				break;
			}
		}

		if (!f.exists(tmpFilename))
			return g_PTRNUL;

		ptr = loadFile(tmpFilename);
		break;
		}
		// Deliberate fall-through to
	default:
		break;
	}

	// Scan for an entry in the catalogue
	byte *result;
	bool matchFlag = false;
	int offsetVal = 0;

	while (!matchFlag) {
		Common::String name = (const char *)ptr + offsetVal;

		if (name == filename) {
			// Found entry for file, so get it's details from the catalogue entry
			const byte *pData = ptr + offsetVal;
			_vm->_globals->_catalogPos = READ_LE_UINT32(pData + 15);
			_vm->_globals->_catalogSize = READ_LE_UINT32(pData + 19);
			matchFlag = true;
		}

		if (name == "FINIS") {
			_vm->_globals->freeMemory(ptr);
			return g_PTRNUL;
		}

		offsetVal += 23;
	}

	_vm->_globals->freeMemory(ptr);

	if (secondaryFilename != "") {
		if (!f.open(secondaryFilename))
			error("CHARGE_FICHIER");

		f.seek(_vm->_globals->_catalogPos);

		byte *catData = _vm->_globals->allocMemory(_vm->_globals->_catalogSize);
		if (catData == g_PTRNUL)
			error("CHARGE_FICHIER");

		readStream(f, catData, _vm->_globals->_catalogSize);
		f.close();
		result = catData;
	} else {
		result = NULL;
	}

	return result;
}

/**
 * Returns the size of a file. Throws an error if the file can't be found
 */
uint32 FileManager::fileSize(const Common::String &filename) {
	Common::File f;
	uint32 size;

	if (!f.open(filename))
		error("Could not find file %s", filename.c_str());

	size = f.size();
	f.close();

	return size;
}

} // End of namespace Hopkins
