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

#ifndef ZVISION_SAVE_MANAGER_H
#define ZVISION_SAVE_MANAGER_H

#include "common/types.h"

#include "common/savefile.h"

namespace Common {
class String;
}

namespace ZVision {

class ZVision;

struct SaveGameHeader {
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

public:
	void autoSave();
	void saveGame(uint slot, const Common::String &saveName);
	Common::Error loadGame(uint slot);

private:
	void writeSaveGameData(Common::OutSaveFile *file);
	bool readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader &header);
};

} // End of namespace ZVision

#endif
