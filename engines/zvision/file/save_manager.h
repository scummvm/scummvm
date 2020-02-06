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
#include "common/memstream.h"

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
	int16 saveYear, saveMonth, saveDay;
	int16 saveHour, saveMinutes;
	uint32 playTime;
};

class SaveManager {
public:
	SaveManager(ZVision *engine) : _engine(engine), _tempSave(NULL), _tempThumbnail(NULL), _lastSaveTime(0) {}
	~SaveManager() {
		flushSaveBuffer();
	}

	uint32 getLastSaveTime() const {
		return _lastSaveTime;
	}

private:
	ZVision *_engine;
	uint32 _lastSaveTime;
	static const uint32 SAVEGAME_ID;

	enum {
		SAVE_ORIGINAL = 0,
		SAVE_VERSION  = 2
	};

	Common::MemoryWriteStreamDynamic *_tempThumbnail;
	Common::MemoryWriteStreamDynamic *_tempSave;

public:
	/**
	 * Copies the data from the last auto-save into a new save file. We
	 * can't use the current state data because the save menu *IS* a room.
	 * The file is named using ZVision::generateSaveFileName(slot)
	 *
	 * @param slot        The save slot this save pertains to. Must be [1, 20]
	 * @param saveName    The internal name for this save. This is NOT the name of the actual save file.
	 */
	void saveGame(uint slot, const Common::String &saveName, bool useSaveBuffer);
	/**
	 * Loads the state data from the save file that slot references. Uses
	 * ZVision::generateSaveFileName(slot) to get the save file name.
	 *
	 * @param slot    The save slot to load. Must be [1, 20]
	 */
	Common::Error loadGame(int slot);

	Common::SeekableReadStream *getSlotFile(uint slot);
	bool readSaveGameHeader(Common::SeekableReadStream *in, SaveGameHeader &header, bool skipThumbnail = true);

	void prepareSaveBuffer();
	void flushSaveBuffer();
	bool scummVMSaveLoadDialog(bool isSave);
private:
	void writeSaveGameHeader(Common::OutSaveFile *file, const Common::String &saveName, bool useSaveBuffer);
};

} // End of namespace ZVision

#endif
