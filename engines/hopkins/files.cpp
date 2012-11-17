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

// Load INI File
void FileManager::Chage_Inifile(Common::StringMap &iniParams) {
	// TODO: Review whether we can do something cleaner with ScummVM initialisation than 
	// just initialising the INI array as if it had read in the INI file

	iniParams["FULLSCREEN"] = "NO";
	iniParams["SETMODE"] = "1";		// 640x480
	iniParams["ZOOM"] = "100";		// No zooming
	iniParams["VIDEOMEM"] = "YES";
	iniParams["FORCE8BITS"] = "NO";
	iniParams["FORCE16BITS"] = "YES";
	iniParams["SOUND"] = "YES";
}

// Load File
byte *FileManager::CHARGE_FICHIER(const Common::String &file) {
	DMESS1();

	Common::File f;
	if (!f.open(file))
		error("Error opening %s", file.c_str());

	// Allocate space for the file contents
	size_t filesize = f.size();
	byte *data = _vm->_globals.dos_malloc2(filesize);
	if (!data)
		error("Error allocating space for file being loaded - %s", file.c_str());

	bload_it(f, data, filesize);
	f.close();
  
	return data;
}

// Load File 2
void FileManager::CHARGE_FICHIER2(const Common::String &file, byte *buf) {
	Common::File f;
	size_t filesize;

	DMESS1();
	if (!f.open(file))
		error("Error opening file - %s", file.c_str());

	filesize = f.size();
	_vm->_fileManager.bload_it(f, buf, filesize);
	f.close();
}

// Guess: Debug Message
void FileManager::DMESS() {
	// No implementation in original
}

// Guess: Debug Message 1
void FileManager::DMESS1() {
	// No implementation in original
}

void FileManager::bload(const Common::String &file, byte *buf) {
	Common::File f;
	if (!f.open(file))
		error("Error opening file - %s", file.c_str());
	int32 filesize = f.size();
	_vm->_fileManager.bload_it(f, buf, filesize);
	f.close();
}

int FileManager::bload_it(Common::ReadStream &stream, void *buf, size_t nbytes) {
	return stream.read(buf, nbytes);
}

// Censorship
void FileManager::F_Censure() {
	_vm->_globals.CENSURE = false;

	CONSTRUIT_SYSTEM("BLOOD.DAT");
	char *data = (char *)CHARGE_FICHIER(_vm->_globals.NFICHIER);
  
	if (*(data + 6) == 'f' && *(data + 7) == 'r')
		_vm->_globals.CENSURE = false;
	if (*(data + 6) == 'F' && *(data + 7) == 'R')
		_vm->_globals.CENSURE = false;
	if (*(data + 6) == 'u' && *(data + 7) == 'k')
		_vm->_globals.CENSURE = true;
	if (*(data + 6) == 'U' && *(data + 7) == 'K')
		_vm->_globals.CENSURE = true;

	free(data);
}

// Build System
int FileManager::CONSTRUIT_SYSTEM(const Common::String &file) {
	_vm->_globals.NFICHIER = Common::String::format("system/%s", file.c_str());
	return _vm->_globals.NFICHIER.size();
}

void FileManager::CONSTRUIT_FICHIER(const Common::String &folder, const Common::String &file) {
	Common::String folderToUse = folder;

	// A lot of the code in the original engine based on COPIE_SEQ was used to determine
	// whether a file resided on the CD or hard disk. Since the ScummVM implementatoin
	// requires all the files in the same location, we only need to do a somewhat simpler
	// check for animations that don't exist in the ANM folder, but rather in special
	// sub-folders depending on the physical screen resolution being used.

	if (folder == _vm->_globals.HOPANM) {
		switch (_vm->_globals.SVGA) {
		case 1:
			if (TEST_REP(folderToUse, file))
				folderToUse = _vm->_globals.HOPTSVGA;
			break;
		case 2:
			if (TEST_REP(folderToUse, file))
				folderToUse = _vm->_globals.HOPSVGA;
			break;
		case 3:
			if (TEST_REP(folderToUse, file))
				folderToUse = _vm->_globals.HOPVGA;
			break;
		default:
			break;
		}
	}

	_vm->_globals.NFICHIER = Common::String::format("%s/%s", folderToUse.c_str(), file.c_str());
}

bool FileManager::TEST_REP(const Common::String &folder, const Common::String &file) {
	Common::String filename = folder.empty() ? file : 
		Common::String::format("%s/%s", folder.c_str(), file.c_str());

	Common::File f;
	return !f.exists(filename);
}

// Free File
byte *FileManager::LIBERE_FICHIER(byte *ptr) {
	free(ptr);
	return g_PTRNUL;
}

// Search Cat
byte *FileManager::RECHERCHE_CAT(const Common::String &file, int a2) {
	byte *ptr = NULL;
	Common::File f;
	
	Common::String filename = file;
	filename.toUppercase();

	switch (a2) {
	case 1:
		CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_INI.CAT");
		if (!f.exists(_vm->_globals.NFICHIER))
			return g_PTRNUL;
		
		ptr = CHARGE_FICHIER(_vm->_globals.NFICHIER);
		CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_INI.RES");
		break;

	case 2:
		CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_REP.CAT");
		if (!f.exists(_vm->_globals.NFICHIER))
			return g_PTRNUL;

		ptr = CHARGE_FICHIER(_vm->_globals.NFICHIER);
		CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_REP.RES");
		break;

	case 3:
		CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_LIN.CAT");
		if (!f.exists(_vm->_globals.NFICHIER))
			return g_PTRNUL;

		ptr = CHARGE_FICHIER(_vm->_globals.NFICHIER);
		CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_LIN.RES");
		break;

	case 4:
		CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, "RES_ANI.CAT");
		if (!f.exists(_vm->_globals.NFICHIER))
			return g_PTRNUL;

		ptr = CHARGE_FICHIER(_vm->_globals.NFICHIER);
		CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, "RES_ANI.RES");
		break;

	case 5:
		CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, "RES_PER.CAT");
		if (!f.exists(_vm->_globals.NFICHIER))
			return g_PTRNUL;

		ptr = CHARGE_FICHIER(_vm->_globals.NFICHIER);
		CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, "RES_PER.RES");
		break;

	case 6:
		CONSTRUIT_FICHIER(_vm->_globals.HOPIMAGE, "PIC.CAT");
		if (!f.exists(_vm->_globals.NFICHIER))
			return g_PTRNUL;

		ptr = CHARGE_FICHIER(_vm->_globals.NFICHIER);
		break;

	case 7:
		CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, "RES_SAN.CAT");
		if (!f.exists(_vm->_globals.NFICHIER))
			return g_PTRNUL;

		ptr = CHARGE_FICHIER(_vm->_globals.NFICHIER);
		break;

	case 8:
		CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_SLI.CAT");
		if (!f.exists(_vm->_globals.NFICHIER))
			return g_PTRNUL;

		ptr = CHARGE_FICHIER(_vm->_globals.NFICHIER);
		break;

	case 9:
		switch (_vm->_globals.FR) {
		case 0:
			CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_VAN.CAT");
			break;
		case 1:
			CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_VFR.CAT");
			break;
		case 2:
			CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_VES.CAT");
			break;
		}

		if (!f.exists(_vm->_globals.NFICHIER))
			return g_PTRNUL;

		ptr = CHARGE_FICHIER(_vm->_globals.NFICHIER);
		break;
		// Deliberate fall-through to
	default:
		break;
	}

	// Scan for an entry in the catalogue
	byte *result;
	bool matchFlag = false;
	int offsetVal = 0;
	
	do {
		Common::String name = (const char *)ptr + offsetVal;
    
		if (name == filename) {
			// Found entry for file, so get it's details from the catalogue entry
			const byte *pData = ptr + offsetVal;
			_vm->_globals.CAT_POSI = READ_LE_UINT32(pData + 15);
			_vm->_globals.CAT_TAILLE = READ_LE_UINT32(pData + 19);
			matchFlag = true;
		}

		if (name == "FINIS") {
			_vm->_globals.dos_free2(ptr);
			return g_PTRNUL;
		}
    
		offsetVal += 23;
	} while (!matchFlag);

	_vm->_globals.dos_free2(ptr);

	// TODO: Double check whether this really should be an unsigned int comparison
	if ((uint16)(a2 - 6) > 1 && (uint16)(a2 - 8) > 1) {
		if (!f.open(_vm->_globals.NFICHIER))
			error("CHARGE_FICHIER");

		f.seek(_vm->_globals.CAT_POSI);

		byte *catData = _vm->_globals.dos_malloc2(_vm->_globals.CAT_TAILLE);
		if (catData == g_PTRNUL)
			error("CHARGE_FICHIER");

		bload_it(f, catData, _vm->_globals.CAT_TAILLE);
		f.close();
		result = catData;
	} else {
		result = NULL;
	}

	return result;
}

// File Size
uint32 FileManager::FLONG(const Common::String &filename) {
	Common::File f;
	uint32 size;

	if (!f.open(filename))
		error("Could not find file %s", filename.c_str());

	size = f.size();
	f.close();
	
	return size;
}

// Build Linux
Common::String FileManager::CONSTRUIT_LINUX(const Common::String &file) {
	_vm->_globals.NFICHIER = file;
	return file;
}

} // End of namespace Hopkins
