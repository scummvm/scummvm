/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ACCESS_RESOURCES_H
#define ACCESS_RESOURCES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/language.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/str-array.h"
#include "common/stream.h"

namespace Access {

extern const byte INITIAL_PALETTE[18 * 3];

extern const char *const GENERAL_MESSAGES[];
extern const char *const ESP_GENERAL_MESSAGES[];

extern const int INVCOORDS[][4];

class AccessEngine;

class Resources {
	struct DATEntry {
		byte _gameId;
		byte _discType;
		byte _demoType;
		Common::Language _language;
		uint _fileOffset;
	};
	struct DeathEntry {
		byte _screenId;
		Common::String _msg;
	};
	struct InventoryEntry {
		Common::String _desc;
		int _combo[4];
	};
protected:
	struct RoomEntry {
		Common::String _desc;
		Common::Point _travelPos;
		Common::Array<byte> _data;
	};

	AccessEngine *_vm;
	Common::Array<DATEntry> _datIndex;

	/**
	 * Locate a specified entry in the index and return it's file offset
	 */
	uint findEntry(byte gameId, byte discType, byte demoType, Common::Language language);

	/**
	 * Load data from the access.dat file
	 */
	virtual void load(Common::SeekableReadStream &s);
public:
	Common::Array<Common::Path> FILENAMES;
	Common::Array< Common::Array<byte> > CHARTBL;
	Common::Array<RoomEntry> ROOMTBL;
	Common::Array<DeathEntry> DEATHS;
	Common::Array<InventoryEntry> INVENTORY;
	Common::String CANT_GET_THERE;
public:
	Resources(AccessEngine *vm) : _vm(vm) {}
	virtual ~Resources() {}
	static Resources *init(AccessEngine *vm);

	/**
	 * Load the access.dat file
	 */
	bool load(Common::U32String &errorMessage);

	/**
	 * Get the raw data for the given cursor number
	 */
	virtual const byte *getCursor(int num) const = 0;

	/**
	 * Get the name of the lead character
	 */
	virtual const char *getEgoName() const = 0;

	/**
	 * Get the room mouse values
	 */
	virtual int getRMouse(int i, int j) const = 0;

	/**
	 * Find if the mouse X is inside the range for a button,
	 * or -1 if no button range matches.
	 */
	virtual int inButtonXRange(int x) const = 0;
};

} // End of namespace Access

#endif /* ACCESS_RESOURCES_H */
