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
#include "graphics/surface.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "hopkins/saveload.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

const char *SAVEGAME_STR = "HOPKINS";
#define SAVEGAME_STR_SIZE 13

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

bool SaveLoadManager::readSavegameHeader(Common::InSaveFile *in, hopkinsSavegameHeader &header) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];
	header.thumbnail = NULL;

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header.version = in->readByte();
	if (header.version > HOPKINS_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header.saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0') header.saveName += ch;

	// Get the thumbnail
	header.thumbnail = Graphics::loadThumbnail(*in);
	if (!header.thumbnail)
		return false;

	// Read in save date/time
	header.saveYear = in->readSint16LE();
	header.saveMonth = in->readSint16LE();
	header.saveDay = in->readSint16LE();
	header.saveHour = in->readSint16LE();
	header.saveMinutes = in->readSint16LE();
	header.totalFrames = in->readUint32LE();

	return true;
}

void SaveLoadManager::writeSavegameHeader(Common::OutSaveFile *out, hopkinsSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(HOPKINS_SAVEGAME_VERSION);

	// Write savegame name
	out->write(header.saveName.c_str(), header.saveName.size() + 1);

	// Create a thumbnail and save it
	Graphics::Surface *thumb = new Graphics::Surface();
//	::createThumbnail(thumb, _vm->_graphicsManager.VESA_SCREEN, SCREEN_WIDTH, SCREEN_HEIGHT, NULL);
	Graphics::saveThumbnail(*out, *thumb);
	thumb->free();
	delete thumb;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
	out->writeUint32LE(_vm->_eventsManager._gameCounter);
}

} // End of namespace Hopkins
