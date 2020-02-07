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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "hopkins/saveload.h"

#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"


#include "common/system.h"
#include "common/savefile.h"
#include "graphics/surface.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"

namespace Hopkins {

const char *SAVEGAME_STR = "HOPKINS";
#define SAVEGAME_STR_SIZE 13

SaveLoadManager::SaveLoadManager(HopkinsEngine *vm) {
	_vm = vm;
}

bool SaveLoadManager::save(const Common::String &file, const void *buf, size_t n) {
	Common::OutSaveFile *savefile = g_system->getSavefileManager()->openForSaving(file);

	if (savefile) {
		size_t bytesWritten = savefile->write(buf, n);
		savefile->finalize();
		delete savefile;

		return bytesWritten == n;
	} else
		return false;
}

bool SaveLoadManager::saveExists(const Common::String &file) {
	Common::InSaveFile *savefile = g_system->getSavefileManager()->openForLoading(file);
	bool result = savefile != NULL;
	delete savefile;
	return result;
}

// Save File
bool SaveLoadManager::saveFile(const Common::String &file, const void *buf, size_t n) {
	return save(file, buf, n);
}

void SaveLoadManager::load(const Common::String &file, byte *buf) {
	Common::InSaveFile *savefile = g_system->getSavefileManager()->openForLoading(file);
	if (savefile == NULL)
		error("Error opening file - %s", file.c_str());

	int32 filesize = savefile->size();
	savefile->read(buf, filesize);
	delete savefile;
}

WARN_UNUSED_RESULT bool SaveLoadManager::readSavegameHeader(Common::InSaveFile *in, hopkinsSavegameHeader &header, bool skipThumbnail) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version > HOPKINS_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0') header._saveName += ch;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header._thumbnail, skipThumbnail)) {
		return false;
	}

	// Read in save date/time
	header._year = in->readSint16LE();
	header._month = in->readSint16LE();
	header._day = in->readSint16LE();
	header._hour = in->readSint16LE();
	header._minute = in->readSint16LE();
	header._totalFrames = in->readUint32LE();

	return true;
}

void SaveLoadManager::writeSavegameHeader(Common::OutSaveFile *out, hopkinsSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(HOPKINS_SAVEGAME_VERSION);

	// Write savegame name
	out->write(header._saveName.c_str(), header._saveName.size() + 1);

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
	out->writeUint32LE(_vm->_events->_gameCounter);
}

Common::Error SaveLoadManager::saveGame(int slot, const Common::String &saveName) {
	/* Pack any necessary data into the savegame data structure */
	// Set the selected slot number
	_vm->_globals->_saveData->_data[svLastSavegameSlot] = slot;

	// Set up the inventory
	for (int i = 0; i < 35; ++i)
		_vm->_globals->_saveData->_inventory[i] = _vm->_globals->_inventory[i];

	_vm->_globals->_saveData->_mapCarPosX = _vm->_objectsMan->_mapCarPosX;
	_vm->_globals->_saveData->_mapCarPosY = _vm->_objectsMan->_mapCarPosY;

	/* Create the savegame */
	Common::OutSaveFile *savefile = g_system->getSavefileManager()->openForSaving(
		_vm->getSaveStateName(slot));
	if (!savefile)
		return Common::kCreatingFileFailed;

	// Set up the serializer
	Common::Serializer serializer(NULL, savefile);

	// Write out the savegame header
	hopkinsSavegameHeader header;
	header._saveName = saveName;
	header._version = HOPKINS_SAVEGAME_VERSION;
	writeSavegameHeader(savefile, header);

	// Write out the savegame data
	syncSavegameData(serializer, header._version);

	// Save file complete
	savefile->finalize();
	delete savefile;

	return Common::kNoError;
}

Common::Error SaveLoadManager::loadGame(int slot) {
	// Try and open the save file for reading
	Common::InSaveFile *savefile = g_system->getSavefileManager()->openForLoading(
		_vm->getSaveStateName(slot));
	if (!savefile)
		return Common::kReadingFailed;

	// Set up the serializer
	Common::Serializer serializer(savefile, NULL);

	// Read in the savegame header
	hopkinsSavegameHeader header;
	if (!readSavegameHeader(savefile, header)) {
		delete savefile;
		return Common::kReadingFailed;
	}

	// Read in the savegame data
	syncSavegameData(serializer, header._version);

	// Loading save file complete
	delete savefile;

	// Unpack the inventory
	for (int i = 0; i < 35; ++i)
		_vm->_globals->_inventory[i] = _vm->_globals->_saveData->_inventory[i];

	// Set variables from loaded data as necessary
	_vm->_globals->_saveData->_data[svLastSavegameSlot] = slot;
	_vm->_globals->_exitId = _vm->_globals->_saveData->_data[svLastScreenId];
	_vm->_globals->_saveData->_data[svLastPrevScreenId] = 0;
	_vm->_globals->_screenId = 0;
	_vm->_objectsMan->_mapCarPosX = _vm->_globals->_saveData->_mapCarPosX;
	_vm->_objectsMan->_mapCarPosY = _vm->_globals->_saveData->_mapCarPosY;

	return Common::kNoError;
}

WARN_UNUSED_RESULT bool SaveLoadManager::readSavegameHeader(int slot, hopkinsSavegameHeader &header, bool skipThumbnail) {
	// Try and open the save file for reading
	Common::InSaveFile *savefile = g_system->getSavefileManager()->openForLoading(
		_vm->getSaveStateName(slot));
	if (!savefile)
		return false;

	bool result = readSavegameHeader(savefile, header, skipThumbnail);
	delete savefile;
	return result;
}

#define REDUCE_AMOUNT 80

void SaveLoadManager::createThumbnail(Graphics::Surface *s) {
	int w = _vm->_graphicsMan->zoomOut(SCREEN_WIDTH, REDUCE_AMOUNT);
	int h = _vm->_graphicsMan->zoomOut(SCREEN_HEIGHT - 40, REDUCE_AMOUNT);

	Graphics::Surface thumb8;
	thumb8.create(w, h, Graphics::PixelFormat::createFormatCLUT8());

	_vm->_graphicsMan->reduceScreenPart(_vm->_graphicsMan->_frontBuffer, (byte *)thumb8.getPixels(),
		_vm->_events->_startPos.x, 20, SCREEN_WIDTH, SCREEN_HEIGHT - 40, 80);

	// Convert the 8-bit pixel to 16 bit surface
	s->create(w, h, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	const byte *srcP = (const byte *)thumb8.getPixels();
	uint16 *destP = (uint16 *)s->getPixels();

	for (int yp = 0; yp < h; ++yp) {
		// Copy over the line, using the source pixels as lookups into the pixels palette
		const byte *lineSrcP = srcP;
		uint16 *lineDestP = destP;

		for (int xp = 0; xp < w; ++xp)
			*lineDestP++ = *(uint16 *)&_vm->_graphicsMan->_palettePixels[*lineSrcP++ * 2];

		// Move to the start of the next line
		srcP += w;
		destP += w;
	}
	thumb8.free();
}

void SaveLoadManager::syncSavegameData(Common::Serializer &s, int version) {
	// The brief version 3 had the highscores embedded. They're in a separate file now, so skip
	if (version == 3 && s.isLoading())
		s.skip(100);

	s.syncBytes(&_vm->_globals->_saveData->_data[0], 2050);
	syncCharacterLocation(s, _vm->_globals->_saveData->_cloneHopkins);
	syncCharacterLocation(s, _vm->_globals->_saveData->_realHopkins);
	syncCharacterLocation(s, _vm->_globals->_saveData->_samantha);

	for (int i = 0; i < 35; ++i)
		s.syncAsSint16LE(_vm->_globals->_saveData->_inventory[i]);

	if (version > 1) {
		s.syncAsSint16LE(_vm->_globals->_saveData->_mapCarPosX);
		s.syncAsSint16LE(_vm->_globals->_saveData->_mapCarPosY);
	} else {
		_vm->_globals->_saveData->_mapCarPosX = _vm->_globals->_saveData->_mapCarPosY = 0;
	}

}

void SaveLoadManager::syncCharacterLocation(Common::Serializer &s, CharacterLocation &item) {
	s.syncAsSint16LE(item._pos.x);
	s.syncAsSint16LE(item._pos.y);
	s.syncAsSint16LE(item._startSpriteIndex);
	s.syncAsSint16LE(item._location);
	s.syncAsSint16LE(item._zoomFactor);
}

void SaveLoadManager::convertThumb16To8(Graphics::Surface *thumb16, Graphics::Surface *thumb8) {
	thumb8->create(thumb16->w, thumb16->h, Graphics::PixelFormat::createFormatCLUT8());
	Graphics::PixelFormat pixelFormat16(2, 5, 6, 5, 0, 11, 5, 0, 0);

	byte paletteR[PALETTE_SIZE];
	byte paletteG[PALETTE_SIZE];
	byte paletteB[PALETTE_SIZE];
	for (int palIndex = 0; palIndex < PALETTE_SIZE; ++palIndex) {
		uint16 p = READ_UINT16(&_vm->_graphicsMan->_palettePixels[palIndex * 2]);
		pixelFormat16.colorToRGB(p, paletteR[palIndex], paletteG[palIndex], paletteB[palIndex]);
	}

	const uint16 *srcP = (const uint16 *)thumb16->getPixels();
	byte *destP = (byte *)thumb8->getPixels();

	for (int yp = 0; yp < thumb16->h; ++yp) {
		const uint16 *lineSrcP = srcP;
		byte *lineDestP = destP;

		for (int xp = 0; xp < thumb16->w; ++xp) {
			byte r, g, b;
			pixelFormat16.colorToRGB(*lineSrcP++, r, g, b);

			// Do like in the original and show thumbnail as a grayscale picture
			int lum = (r * 21 + g * 72 + b * 7) / 100;
			r = g = b = lum;

			// Scan the palette for the closest match
			int difference = 99999, foundIndex = 0;
			for (int palIndex = 0; palIndex < PALETTE_SIZE; ++palIndex) {
				byte rCurrent = paletteR[palIndex];
				byte gCurrent = paletteG[palIndex];
				byte bCurrent = paletteB[palIndex];

				int diff = ABS((int)r - (int)rCurrent) + ABS((int)g - (int)gCurrent) + ABS((int)b - (int)bCurrent);
				if (diff < difference) {
					difference = diff;
					foundIndex = palIndex;
				}
			}

			*lineDestP++ = foundIndex;
		}

		// Move to the start of the next line
		srcP += thumb16->w;
		destP += thumb16->w;
	}
}

} // End of namespace Hopkins
