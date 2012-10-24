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

#include "common/system.h"
#include "common/savefile.h"
#include "hopkins/saveload.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"

namespace Hopkins {

void SaveLoadManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

bool SaveLoadManager::bsave(const Common::String &file, const void *buf, size_t n) {
	Common::OutSaveFile *f = g_system->getSavefileManager()->openForSaving(file);

	if (f) {
		size_t bytesWritten = f->write(buf, n);
		f->finalize();
		delete f;

		return bytesWritten == n;
	} else 
		return false;
}

// Save File
bool SaveLoadManager::SAUVE_FICHIER(const Common::String &file, const void *buf, size_t n) {
	return bsave(file, buf, n);
}

void SaveLoadManager::initSaves() {
	Common::String dataFilename = "HISCORE.DAT";
	byte data[100];
	Common::fill(&data[0], &data[100], 0);
	
	SAUVE_FICHIER(dataFilename, data, 100);
}

void SaveLoadManager::bload(const Common::String &file, byte *buf) {
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(file);
	if (f == NULL)
		error("Error openinig file - %s", file.c_str());

	int32 filesize = f->size();
	f->read(buf, filesize);
	delete f;
}

} // End of namespace Hopkins
