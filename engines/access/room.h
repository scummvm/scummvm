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

#ifndef ACCESS_ROOM_H
#define ACCESS_ROOM_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "access/data.h"

#define TILE_WIDTH 16
#define TILE_HEIGHT 16

namespace Access {

class Plotter {
public:
	Common::Array<Common::Rect> _walls;
	Common::Array<Common::Rect> _blocks;
	int _blockIn;
	int _delta;
public:
	Plotter();

	void load(Common::SeekableReadStream *stream, int wallCount, int blockCount);
};

class JetFrame {
public:
	int _wallCode;
	int _wallCodeOld;
	int _wallCode1;
	int _wallCode1Old;

	JetFrame() {
		_wallCode = _wallCodeOld = 0;
		_wallCode1 = _wallCode1Old = 0;
	}
};

enum Function { FN_NONE = 0, FN_CLEAR1 = 1, FN_CLEAR2 = 2, FN_RELOAD = 3, FN_BREAK = 4 };

class Room : public Manager {
private:
	void roomLoop();

	void loadPlayField(int fileNum, int subfile);

	void commandOff();

	void swapOrg();
	int calcLR(int yp);
	int calcUD(int xp);

	void takePicture();

	/**
	 * Cycles forwards or backwards through the list of commands
	 */
	void cycleCommand(int incr);

	bool checkCode(int v1, int v2);
protected:
	void loadRoomData(const byte *roomData);

	/**
	* Free the playfield data
	*/
	void freePlayField();

	/**
	* Free tile data
	*/
	void freeTileData();

	int checkBoxes();
	int checkBoxes1(const Common::Point &pt);
	int checkBoxes2(const Common::Point &pt, int start, int count);
	void checkBoxes3();

	int validateBox(int boxId);

	/**
	 * Inner handler for switching to a given command mode
	 */
	void executeCommand(int commandId);

	void clearCamera();

	virtual void reloadRoom() = 0;

	virtual void reloadRoom1() = 0;

	virtual void setupRoom();

	virtual void doCommands();

	virtual void mainAreaClick() = 0;

	virtual void walkCursor();
public:
	Plotter _plotter;
	Common::Array<JetFrame> _jetFrame;
	Function _function;
	int _roomFlag;
	byte *_playField;
	int _matrixSize;
	int _playFieldWidth;
	int _playFieldHeight;
	byte *_tile;
	int _selectCommand;
	bool _conFlag;
	int _rMouse[10][2];
public:
	Room(AccessEngine *vm);

	virtual ~Room();

	void doRoom();

	virtual void loadRoom(int roomNumber) = 0;

	virtual void roomMenu() = 0;

	/**
	 * Clear all the data used by the room
	 */
	virtual void clearRoom();

	/**
	 * Builds up a game screen
	 */
	void buildScreen();

	/**
	 * Draw a column of a game scene
	 */
	void buildColumn(int playX, int screenX);

	/**
	 * Draw a row of a game scene
	 */
	void buildRow(int playY, int screenY);

	virtual void init4Quads() = 0;

	void setWallCodes();

	bool codeWalls();

	/**
	* Switch to a given command mode
	*/
	void handleCommand(int commandId);
};

class RoomInfo {
public:
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
	Common::Array<ExtraCell> _extraCells;
	Common::Array<SoundIdent> _sounds;
public:
	RoomInfo(const byte *data, int gameType, bool isCD, bool isDemo);
};

} // End of namespace Access

#endif /* ACCESS_ROOM_H */
