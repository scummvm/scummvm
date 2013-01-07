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

FileManager::FileManager() {
}

void FileManager::setParent(HopkinsEngine *vm) {
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
	byte *data = _vm->_globals.allocMemory(filesize);
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
	_vm->_globals._censorshipFl = false;

	// If file doesn't exist, fallback to uncensored
	if (!fileExists(_vm->_globals.HOPSYSTEM, "BLOOD.DAT")) {
		constructFilename(_vm->_globals.HOPSYSTEM, "BLOOD.DAT");
		char *data = (char *)loadFile(_vm->_globals._curFilename);

		if ((data[6] == 'u' && data[7] == 'k') || (data[6] == 'U' && data[7] == 'K'))
			_vm->_globals._censorshipFl = true;

		free(data);
	}
}

/**
 * Construct a filename based on a suggested folder and filename.
 * @param folder		Folder to use. May be overriden for animations.
 * @param file			Filename
 */
void FileManager::constructFilename(const Common::String &folder, const Common::String &file) {
	Common::String folderToUse = folder;

	// A lot of the code in the original engine based on COPIE_SEQ was used to determine
	// whether a file resided on the CD or hard disk. Since the ScummVM implementation
	// requires all the files in the same location, we only need to do a somewhat simpler
	// check for animations that don't exist in the ANM folder, but rather in special
	// sub-folders depending on the physical screen resolution being used.

	if (folder == _vm->_globals.HOPANM) {
		switch (_vm->_globals.SVGA) {
		case 1:
			if (fileExists(folderToUse, file))
				folderToUse = _vm->_globals.HOPTSVGA;
			break;
		case 2:
			if (fileExists(folderToUse, file))
				folderToUse = _vm->_globals.HOPSVGA;
			break;
		case 3:
			if (fileExists(folderToUse, file))
				folderToUse = _vm->_globals.HOPVGA;
			break;
		default:
			break;
		}
	}

	_vm->_globals._curFilename = Common::String::format("%s/%s", folderToUse.c_str(), file.c_str());
}

/**
 * Construct Linux filename
 */
Common::String FileManager::constructLinuxFilename(const Common::String &file) {
	_vm->_globals._curFilename = file;
	return file;
}

/**
 * Check if a file is present in a given (optional) folder
 */
bool FileManager::fileExists(const Common::String &folder, const Common::String &file) {
	Common::String filename = folder.empty() ? file :
		Common::String::format("%s/%s", folder.c_str(), file.c_str());

	Common::File f;
	return !f.exists(filename);
}

/**
 * Search file in Cat file
 */
byte *FileManager::searchCat(const Common::String &file, int a2) {
	byte *ptr = NULL;
	Common::File f;

	Common::String filename = file;
	filename.toUppercase();

	switch (a2) {
	case 1:
		constructFilename(_vm->_globals.HOPLINK, "RES_INI.CAT");
		if (!f.exists(_vm->_globals._curFilename))
			return g_PTRNUL;

		ptr = loadFile(_vm->_globals._curFilename);
		constructFilename(_vm->_globals.HOPLINK, "RES_INI.RES");
		break;

	case 2:
		constructFilename(_vm->_globals.HOPLINK, "RES_REP.CAT");
		if (!f.exists(_vm->_globals._curFilename))
			return g_PTRNUL;

		ptr = loadFile(_vm->_globals._curFilename);
		constructFilename(_vm->_globals.HOPLINK, "RES_REP.RES");
		break;

	case 3:
		constructFilename(_vm->_globals.HOPLINK, "RES_LIN.CAT");
		if (!f.exists(_vm->_globals._curFilename))
			return g_PTRNUL;

		ptr = loadFile(_vm->_globals._curFilename);
		constructFilename(_vm->_globals.HOPLINK, "RES_LIN.RES");
		break;

	case 4:
		constructFilename(_vm->_globals.HOPANIM, "RES_ANI.CAT");
		if (!f.exists(_vm->_globals._curFilename))
			return g_PTRNUL;

		ptr = loadFile(_vm->_globals._curFilename);
		constructFilename(_vm->_globals.HOPANIM, "RES_ANI.RES");
		break;

	case 5:
		constructFilename(_vm->_globals.HOPANIM, "RES_PER.CAT");
		if (!f.exists(_vm->_globals._curFilename))
			return g_PTRNUL;

		ptr = loadFile(_vm->_globals._curFilename);
		constructFilename(_vm->_globals.HOPANIM, "RES_PER.RES");
		break;

	case 6:
		constructFilename(_vm->_globals.HOPIMAGE, "PIC.CAT");
		if (!f.exists(_vm->_globals._curFilename))
			return g_PTRNUL;

		ptr = loadFile(_vm->_globals._curFilename);
		break;

	case 7:
		constructFilename(_vm->_globals.HOPANIM, "RES_SAN.CAT");
		if (!f.exists(_vm->_globals._curFilename))
			return g_PTRNUL;

		ptr = loadFile(_vm->_globals._curFilename);
		break;

	case 8:
		constructFilename(_vm->_globals.HOPLINK, "RES_SLI.CAT");
		if (!f.exists(_vm->_globals._curFilename))
			return g_PTRNUL;

		ptr = loadFile(_vm->_globals._curFilename);
		break;

	case 9:
		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			_vm->_fileManager.constructFilename(_vm->_globals.HOPVOICE, "ENG_VOI.RES");
		// Win95 and Linux versions uses another set of names
		else {
			switch (_vm->_globals._language) {
			case LANG_EN:
				constructFilename(_vm->_globals.HOPLINK, "RES_VAN.CAT");
				break;
			case LANG_FR:
				constructFilename(_vm->_globals.HOPLINK, "RES_VFR.CAT");
				break;
			case LANG_SP:
				constructFilename(_vm->_globals.HOPLINK, "RES_VES.CAT");
				break;
			}
		}

		if (!f.exists(_vm->_globals._curFilename))
			return g_PTRNUL;

		ptr = loadFile(_vm->_globals._curFilename);
		break;
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
			_vm->_globals._catalogPos = READ_LE_UINT32(pData + 15);
			_vm->_globals._catalogSize = READ_LE_UINT32(pData + 19);
			matchFlag = true;
		}

		if (name == "FINIS") {
			_vm->_globals.freeMemory(ptr);
			return g_PTRNUL;
		}

		offsetVal += 23;
	}

	_vm->_globals.freeMemory(ptr);

	// TODO: Double check whether this really should be an unsigned int comparison
	if ((uint16)(a2 - 6) > 1 && (uint16)(a2 - 8) > 1) {
		if (!f.open(_vm->_globals._curFilename))
			error("CHARGE_FICHIER");

		f.seek(_vm->_globals._catalogPos);

		byte *catData = _vm->_globals.allocMemory(_vm->_globals._catalogSize);
		if (catData == g_PTRNUL)
			error("CHARGE_FICHIER");

		readStream(f, catData, _vm->_globals._catalogSize);
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
