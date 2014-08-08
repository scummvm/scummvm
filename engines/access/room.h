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

#ifndef ACCESS_ROOM_H
#define ACCESS_ROOM_H

#include "common/scummsys.h"

namespace Access {

class AccessEngine;

class Room {
private:
	void roomLoop();
protected:
	AccessEngine *_vm;

	void loadRoomData(const byte *roomData);
	void setupRoom();
	void setWallCodes();
	void buildScreen();

	virtual void loadRoom(int roomNumber) = 0;

	virtual void reloadRoom() = 0;

	virtual void reloadRoom1() = 0;

	virtual void setIconPalette() {}
public:
	int _function;
	int _roomFlag;
public:
	Room(AccessEngine *vm);

	void doRoom();

	void doCommands();

	/**
	 * Clear all the data used by the room
	 */
	void clearRoom();
};


class RoomInfo {
public:
	struct FileIdent {
		int _fileNum;
		int _subfile;
	};

	struct CellIdent : FileIdent {
		byte _cell;
	};
	
	struct SoundIdent : FileIdent {
		int _priority;
	};
public:
	int _roomFlag;
	int _estIndex;
	FileIdent _musicFile;
	int _scaleH1;
	int _scaleH2;
	int _scaleN1;
	FileIdent _playFieldFile;
	Common::Array<CellIdent> _cells;
	FileIdent _scriptFile;
	FileIdent _animFile;
	int _scaleI;
	int _scrollThreshold;
	FileIdent _paletteFile;
	int _startColor;
	int _numColors;
	Common::Array<uint32> _vidTable;
	Common::Array<SoundIdent> _sounds;
public:
	RoomInfo(const byte *data);
};

} // End of namespace Access

#endif /* ACCESS_ROOM_H */
