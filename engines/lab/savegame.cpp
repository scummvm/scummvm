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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/savefile.h"

#include "graphics/surface.h"
#include "graphics/thumbnail.h"

#include "lab/lab.h"
#include "lab/labfun.h"

namespace Lab {

// Labyrinth of Time ScummVM
#define SAVEGAME_ID       MKTAG('L', 'O', 'T', 'S')
#define SAVEGAME_VERSION  1

/*----- The machine independent section of saveGame.c -----*/

void writeSaveGameHeader(Common::OutSaveFile *out, const Common::String &saveName) {
	out->writeUint32BE(SAVEGAME_ID);

	// Write version
	out->writeByte(SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(saveName);
	out->writeByte(0);

	// Save the game thumbnail
	Graphics::saveThumbnail(*out);

	// Creation date/time
	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;

	out->writeUint32BE(saveDate);
	out->writeUint16BE(saveTime);
	out->writeUint32BE(playTime);
}

bool readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader &header) {
	uint32 id = in->readUint32BE();

	// Check if it's a valid ScummVM savegame
	if (id != SAVEGAME_ID)
		return false;

	// Read in the version
	header._version = in->readByte();

	// Check that the save version isn't newer than this binary
	if (header._version > SAVEGAME_VERSION)
		return false;

	// Read in the save name
	Common::String saveName;
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		saveName += ch;
	header._descr.setDescription(saveName);

	// Get the thumbnail
	header._descr.setThumbnail(Graphics::loadThumbnail(*in));

	uint32 saveDate = in->readUint32BE();
	uint16 saveTime = in->readUint16BE();
	uint32 playTime = in->readUint32BE();

	int day = (saveDate >> 24) & 0xFF;
	int month = (saveDate >> 16) & 0xFF;
	int year = saveDate & 0xFFFF;
	header._descr.setSaveDate(year, month, day);

	int hour = (saveTime >> 8) & 0xFF;
	int minutes = saveTime & 0xFF;
	header._descr.setSaveTime(hour, minutes);

	header._descr.setPlayTime(playTime * 1000);
	g_engine->setTotalPlayTime(playTime * 1000);

	return true;
}

/*****************************************************************************/
/* Writes the game out to disk.                                              */
/*****************************************************************************/
bool saveGame(uint16 Direction, uint16 Quarters, int slot, Common::String desc) {
	uint16 i, j;
	Common::String fileName = g_lab->generateSaveFileName(slot);
	Common::SaveFileManager *saveFileManager = g_system->getSavefileManager();
	Common::OutSaveFile *file = saveFileManager->openForSaving(fileName);

	if (!file)
		return false;

	// Load scene pic
	CloseDataPtr cPtr = nullptr;
	g_lab->_graphics->readPict(g_lab->getPictName(&cPtr), true);

	writeSaveGameHeader(file, desc);
	file->writeUint16LE(g_lab->_roomNum);
	file->writeUint16LE(Direction);
	file->writeUint16LE(Quarters);

	// Conditions
	for (i = 0; i < g_lab->_conditions->_lastElement / (8 * 2); i++)
		file->writeUint16LE(g_lab->_conditions->_array[i]);

	// Rooms found
	for (i = 0; i < g_lab->_roomsFound->_lastElement / (8 * 2); i++)
		file->writeUint16LE(g_lab->_roomsFound->_array[i]);

	// Combination lock and tile stuff
	for (i = 0; i < 6; i++)
		file->writeByte(g_lab->_combination[i]);

	// Tiles
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			file->writeUint16LE(g_lab->_curTile[i][j]);

	// Breadcrumbs
	for (i = 0; i < sizeof(g_lab->_breadCrumbs); i++) {
		file->writeUint16LE(g_lab->_breadCrumbs[i]._roomNum);
		file->writeUint16LE(g_lab->_breadCrumbs[i]._direction);
	}

	file->flush();
	file->finalize();
	delete file;

	return true;
}



/*****************************************************************************/
/* Reads the game from disk.                                                 */
/*****************************************************************************/
bool loadGame(uint16 *Direction, uint16 *Quarters, int slot) {
	uint16 i, j;
	Common::String fileName = g_lab->generateSaveFileName(slot);
	Common::SaveFileManager *saveFileManager = g_system->getSavefileManager();
	Common::InSaveFile *file = saveFileManager->openForLoading(fileName);

	if (!file)
		return false;

	SaveGameHeader header;
	readSaveGameHeader(file, header);
	g_lab->_roomNum = file->readUint16LE();
	*Direction = file->readUint16LE();
	*Quarters = file->readUint16LE();

	// Conditions
	for (i = 0; i < g_lab->_conditions->_lastElement / (8 * 2); i++)
		g_lab->_conditions->_array[i] = file->readUint16LE();

	// Rooms found
	for (i = 0; i < g_lab->_roomsFound->_lastElement / (8 * 2); i++)
		g_lab->_roomsFound->_array[i] = file->readUint16LE();

	// Combination lock and tile stuff
	for (i = 0; i < 6; i++)
		g_lab->_combination[i] = file->readByte();

	// Tiles
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			g_lab->_curTile[i][j] = file->readUint16LE();

	// Breadcrumbs
	for (i = 0; i < 128; i++) {
		g_lab->_breadCrumbs[i]._roomNum = file->readUint16LE();
		g_lab->_breadCrumbs[i]._direction = file->readUint16LE();
	}

	g_lab->_droppingCrumbs = (g_lab->_breadCrumbs[0]._roomNum != 0);
	g_lab->_followingCrumbs = false;

	for (i = 0; i < 128; i++) {
		if (g_lab->_breadCrumbs[i]._roomNum == 0)
			break;
		g_lab->_numCrumbs = i;
	}

	delete file;

	return true;
}


} // End of namespace Lab
