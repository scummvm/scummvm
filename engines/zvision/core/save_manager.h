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

#ifndef ZVISION_SAVE_MANAGER_H
#define ZVISION_SAVE_MANAGER_H

#include "common/savefile.h"

namespace Common {
class String;
}

namespace Graphics {
struct Surface;
}

namespace ZVision {

class ZVision;

struct SaveGameHeader {
	byte version;
	Common::String saveName;
	Graphics::Surface *thumbnail;
	int saveYear, saveMonth, saveDay;
	int saveHour, saveMinutes;
};

class SaveManager {
public:
	SaveManager(ZVision *engine) : _engine(engine) {}

private:
	ZVision *_engine;
	static const uint32 SAVEGAME_ID;

	enum {
		SAVE_VERSION = 1
	};

public:
	/**
	 * Called every room change. Saves the state of the room just before
	 * we switched rooms. Uses ZVision::generateAutoSaveFileName() to
	 * create the save file name.
	 */
	void autoSave();
	/**
	 * Copies the data from the last auto-save into a new save file. We
	 * can't use the current state data because the save menu *IS* a room.
	 * The file is named using ZVision::generateSaveFileName(slot)
	 *
	 * @param slot        The save slot this save pertains to. Must be [1, 20]
	 * @param saveName    The internal name for this save. This is NOT the name of the actual save file.
	 */
	void saveGame(uint slot, const Common::String &saveName);
	/**
	 * Loads the state data from the save file that slot references. Uses
	 * ZVision::generateSaveFileName(slot) to get the save file name.
	 *
	 * @param slot    The save slot to load. Must be [1, 20]
	 */
	Common::Error loadGame(uint slot);

private:
	void writeSaveGameData(Common::OutSaveFile *file);
	bool readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader &header);
};

} // End of namespace ZVision

#endif
