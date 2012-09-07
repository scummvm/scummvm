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

void *FileManager::CHARGE_FICHIER(const Common::String &file) {
	DMESS1();

	Common::File f;
	if (!f.open(file))
		error("Error opening %s", file.c_str());

	// Allocate space for the file contents
	size_t filesize = f.size();
	void *data = malloc(filesize);
	if (!data)
		error("Error allocating space for file being loaded - %s", file.c_str());

	bload_it(f, data, filesize);
	f.close();
  
	return data;
}

void FileManager::DMESS1() {
}

int FileManager::bload_it(Common::ReadStream &stream, void *buf, size_t nbytes) {
	return stream.read(buf, nbytes);
}

} // End of namespace Hopkins
