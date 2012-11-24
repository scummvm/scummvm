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
	createThumbnail(thumb);
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

Common::Error SaveLoadManager::save(int slot, const Common::String &saveName) {
	/* Pack any necessary data into the savegame data structure */
	// Set the selected slot number
	_vm->_globals.SAUVEGARDE->data[svField10] = slot;

	// Set up the inventory
	for (int i = 0; i < 35; ++i)
		_vm->_globals.SAUVEGARDE->inventory[i] = _vm->_globals.INVENTAIRE[i];

	/* Create the savegame */
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(
		_vm->generateSaveName(slot));
	if (!saveFile)
		return Common::kCreatingFileFailed;

	// Set up the serializer
	Common::Serializer serializer(NULL, saveFile);

	// Write out the savegame header
	hopkinsSavegameHeader header;
	header.saveName = saveName;
	header.version = HOPKINS_SAVEGAME_VERSION;
	writeSavegameHeader(saveFile, header);

	// Write out the savegame data
	syncSavegameData(serializer);

	// Save file complete
	saveFile->finalize();
	delete saveFile;

	return Common::kNoError;
}

Common::Error SaveLoadManager::restore(int slot) {
	// Try and open the save file for reading
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
		_vm->generateSaveName(slot));
	if (!saveFile)
		return Common::kReadingFailed;

	// Set up the serializer
	Common::Serializer serializer(saveFile, NULL);

	// Read in the savegame header
	hopkinsSavegameHeader header;
	readSavegameHeader(saveFile, header);
	if (header.thumbnail)
		header.thumbnail->free();
	delete header.thumbnail;

	// Read in the savegame data
	syncSavegameData(serializer);

	// Loading save file complete
	delete saveFile;

	// Unpack the inventory
	for (int i = 0; i < 35; ++i) 
		_vm->_globals.INVENTAIRE[i] = _vm->_globals.SAUVEGARDE->inventory[i];

	// Set variables from loaded data as necessary
	_vm->_globals.SAUVEGARDE->data[svField10] = slot;
	_vm->_globals.SORTIE = _vm->_globals.SAUVEGARDE->data[svField5];
	_vm->_globals.SAUVEGARDE->data[svField6] = 0;
	_vm->_globals.ECRAN = 0;

	return Common::kNoError;
}

bool SaveLoadManager::readSavegameHeader(int slot, hopkinsSavegameHeader &header) {
	// Try and open the save file for reading
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
		g_vm->generateSaveName(slot));
	if (!saveFile)
		return false;

	bool result = readSavegameHeader(saveFile, header);
	delete saveFile;
	return result;
}

#define REDUCE_AMOUNT 80

void SaveLoadManager::createThumbnail(Graphics::Surface *s) {
	int w = _vm->_graphicsManager.Reel_Reduc(SCREEN_WIDTH, REDUCE_AMOUNT);
	int h = _vm->_graphicsManager.Reel_Reduc(SCREEN_HEIGHT - 40, REDUCE_AMOUNT); 

	s->create(w, h, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));	

	_vm->_graphicsManager.Reduc_Ecran(_vm->_graphicsManager.VESA_BUFFER, (byte *)s->pixels, 
		_vm->_eventsManager.start_x, 20, SCREEN_WIDTH, SCREEN_HEIGHT - 40, 80);
	_vm->_graphicsManager.INIT_TABLE(45, 80, _vm->_graphicsManager.Palette);
//	_vm->_graphicsManager.Trans_bloc2((byte *)s->pixels, _vm->_graphicsManager.TABLE_COUL, 11136);
}

void SaveLoadManager::syncSavegameData(Common::Serializer &s) {
	s.syncBytes(&_vm->_globals.SAUVEGARDE->data[0], 0x802);
	syncCharacterLocation(s, _vm->_globals.SAUVEGARDE->field360);
	syncCharacterLocation(s, _vm->_globals.SAUVEGARDE->field370);
	syncCharacterLocation(s, _vm->_globals.SAUVEGARDE->field380);

	for (int i = 0; i < 35; ++i)
		s.syncAsSint16LE(_vm->_globals.SAUVEGARDE->inventory[i]);
}

void SaveLoadManager::syncCharacterLocation(Common::Serializer &s, CharacterLocation &item) {
	s.syncAsSint16LE(item.xp);
	s.syncAsSint16LE(item.yp);
	s.syncAsSint16LE(item.field2);
	s.syncAsSint16LE(item.location);
	s.syncAsSint16LE(item.field4);
}

} // End of namespace Hopkins
