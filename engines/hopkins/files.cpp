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

void FileManager::initSaves() {
	Common::String dataFilename = "HISCORE.DAT";
	byte data[100];
	Common::fill(&data[0], &data[100], 0);
	
	SAUVE_FICHIER(dataFilename, data, 100);
}

bool FileManager::SAUVE_FICHIER(const Common::String &file, const void *buf, size_t n) {
	return bsave(file, buf, n);
}

bool FileManager::bsave(const Common::String &file, const void *buf, size_t n) {
	Common::OutSaveFile *f = g_system->getSavefileManager()->openForSaving(file);

	if (f) {
		size_t bytesWritten = f->write(buf, n);
		f->finalize();
		delete f;

		return bytesWritten == n;
	} else 
		return false;
}

void FileManager::Chage_Inifile(Common::StringMap &iniParams) {
	char *iniData = (char *)CHARGE_FICHIER("config.ini");
	char *ptr = iniData;

	bool endOfFile = false;
	while (!endOfFile) {
		if (*ptr == '%') {
			if (*(ptr + 1) == '%') {
				endOfFile = true;
			} else {
				++ptr;

				// Get the parameter name
				Common::String paramName;
				while (*ptr == ' ') 
					++ptr;
				while (*ptr != '=') {
					paramName += toupper(*ptr++);
				}
				while (paramName.lastChar() == ' ')
					paramName.deleteLastChar();

				// Get the parameter value
				Common::String paramValue;
				while (*++ptr == ' ') 
					;
				while (*ptr != ';') {
					paramValue += toupper(*ptr++);
				}
				while (paramValue.lastChar() == ' ')
					paramValue.deleteLastChar();

				iniParams[paramName] = paramValue;
			}
		}
    
		++ptr;
	}

	free(iniData);
}

byte *FileManager::CHARGE_FICHIER(const Common::String &file) {
	DMESS1();

	Common::File f;
	if (!f.open(file))
		error("Error opening %s", file.c_str());

	// Allocate space for the file contents
	size_t filesize = f.size();
	byte *data = GLOBALS.dos_malloc2(filesize);
	if (!data)
		error("Error allocating space for file being loaded - %s", file.c_str());

	bload_it(f, data, filesize);
	f.close();
  
	return data;
}

void FileManager::DMESS() {
	// No implementation in original
}

void FileManager::DMESS1() {
	// No implementation in original
}

void FileManager::bload(const Common::String &file, byte *buf) {
	Common::File f;
	if (!f.open(file))
		error("Error openinig file - %s", file.c_str());
	int32 filesize = f.size();
	FileManager::bload_it(f, buf, filesize);
	f.close();
}

int FileManager::bload_it(Common::ReadStream &stream, void *buf, size_t nbytes) {
	return stream.read(buf, nbytes);
}

void FileManager::F_Censure() {
	GLOBALS.CENSURE = false;

	CONSTRUIT_SYSTEM("BLOOD.DAT");
	char *data = (char *)CHARGE_FICHIER(GLOBALS.NFICHIER);
  
	if (*(data + 6) == 'f' && *(data + 7) == 'r')
		GLOBALS.CENSURE = false;
	if (*(data + 6) == 'F' && *(data + 7) == 'R')
		GLOBALS.CENSURE = false;
	if (*(data + 6) == 'u' && *(data + 7) == 'k')
		GLOBALS.CENSURE = true;
	if (*(data + 6) == 'U' && *(data + 7) == 'K')
		GLOBALS.CENSURE = true;

	free(data);
}

int FileManager::CONSTRUIT_SYSTEM(const Common::String &file) {
	GLOBALS.NFICHIER = Common::String::format("system/%s", file.c_str());
	return GLOBALS.NFICHIER.size();
}

void FileManager::CONSTRUIT_FICHIER(const Common::String &hop, const Common::String &file) {
	// At this point, the original program did a big switch statement to determine
	// whether to preprend the CD or installed directory path into REPJEU

	if (hop[0] == 'A' && hop[1] == 'N' && hop[2] == 'N') {
		error("TODO: CONSTRUIT_FICHIER");
	}

	GLOBALS.NFICHIER = Common::String::format("%s/%s", hop.c_str(), file.c_str());
}

byte *FileManager::LIBERE_FICHIER(byte *ptr) {
	free(ptr);
	return PTRNUL;
}

byte *FileManager::RECHERCHE_CAT(const Common::String &file, int a2) {
	byte *ptr = NULL;
	Common::File f;

	switch (a2) {
	case 1:
		CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "RES_INI.CAT");
		if (!f.exists(GLOBALS.NFICHIER))
			return PTRNUL;
		
		ptr = CHARGE_FICHIER(GLOBALS.NFICHIER);
		CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "RES_INI.RES");
		break;

	case 2:
		CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "RES_REP.CAT");
		if (!f.exists(GLOBALS.NFICHIER))
			return PTRNUL;

		ptr = CHARGE_FICHIER(GLOBALS.NFICHIER);
		CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "RES_REP.RES");
		break;

	case 3:
		CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "RES_LIN.CAT");
		if (!f.exists(GLOBALS.NFICHIER))
			return PTRNUL;

		ptr = CHARGE_FICHIER(GLOBALS.NFICHIER);
		CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "RES_LIN.RES");
		break;

	case 4:
		CONSTRUIT_FICHIER(GLOBALS.HOPANIM, "RES_ANI.CAT");
		if (!f.exists(GLOBALS.NFICHIER))
			return PTRNUL;

		ptr = CHARGE_FICHIER(GLOBALS.NFICHIER);
		CONSTRUIT_FICHIER(GLOBALS.HOPANIM, "RES_ANI.RES");
		break;

	case 5:
		CONSTRUIT_FICHIER(GLOBALS.HOPANIM, "RES_PER.CAT");
		if (!f.exists(GLOBALS.NFICHIER))
			return PTRNUL;

		ptr = CHARGE_FICHIER(GLOBALS.NFICHIER);
		CONSTRUIT_FICHIER(GLOBALS.HOPANIM, "RES_PER.RES");
		break;

	case 6:
		CONSTRUIT_FICHIER(GLOBALS.HOPIMAGE, "PIC.CAT");
		if (!f.exists(GLOBALS.NFICHIER))
			return PTRNUL;

		ptr = CHARGE_FICHIER(GLOBALS.NFICHIER);
		break;

	case 7:
		CONSTRUIT_FICHIER(GLOBALS.HOPANIM, "RES_SAN.CAT");
		if (!f.exists(GLOBALS.NFICHIER))
			return PTRNUL;

		ptr = CHARGE_FICHIER(GLOBALS.NFICHIER);
		break;

	case 8:
		CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "RES_SLI.CAT");
		if (!f.exists(GLOBALS.NFICHIER))
			return PTRNUL;

		ptr = CHARGE_FICHIER(GLOBALS.NFICHIER);
		break;

	case 9:
		switch (GLOBALS.FR) {
		case 0:
			CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "RES_VAN.CAT");
			break;
		case 1:
			CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "RES_VFR.CAT");
			break;
		case 2:
			CONSTRUIT_FICHIER(GLOBALS.HOPLINK, "RES_VES.CAT");
			break;
		}

		if (!f.exists(GLOBALS.NFICHIER))
			return PTRNUL;

		ptr = CHARGE_FICHIER(GLOBALS.NFICHIER);
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
    
		if (name == file) {
			// Found entry for file, so get it's details from the catalogue entry
			const byte *pData = ptr + offsetVal;
			GLOBALS.CAT_POSI = READ_LE_UINT32(pData + 15);
			GLOBALS.CAT_TAILLE = READ_LE_UINT32(pData + 19);
			matchFlag = true;
		}

		if (name == "FINIS") {
			GLOBALS.dos_free2(ptr);
			return false;
		}
    
		offsetVal += 23;
	} while (!matchFlag);

	GLOBALS.dos_free2(ptr);

	// TODO: Double check whether this really should be an unsigned int comparison
	if ((uint16)(a2 - 6) > 1 && (uint16)(a2 - 8) > 1) {
		if (!f.open(GLOBALS.NFICHIER))
			error("CHARGE_FICHIER");

		f.seek(GLOBALS.CAT_POSI);

		byte *catData = GLOBALS.dos_malloc2(GLOBALS.CAT_TAILLE);
		if (catData == PTRNUL)
			error("CHARGE_FICHIER");

		bload_it(f, catData, GLOBALS.CAT_TAILLE);
		f.close();
		result = catData;
	} else {
		result = NULL;
	}

	return result;
}

uint32 FileManager::FLONG(const Common::String &filename) {
	Common::File f;
	uint32 size;

	if (!f.open(filename))
		error("Could not find file %s", filename.c_str());

	size = f.size();
	f.close();
	
	return size;
}

void FileManager::CONSTRUIT_LINUX(const Common::String &file) {
	warning("TODO: CONSTRUIT_LINUX");
}

} // End of namespace Hopkins
