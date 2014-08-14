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

#include "common/scummsys.h"
#include "common/memstream.h"
#include "access/access.h"
#include "access/resources.h"
#include "access/room.h"

namespace Access {

Room::Room(AccessEngine *vm) : Manager(vm) {
	_function = 0;
	_roomFlag = 0;
	_playField = nullptr;
	_playFieldWidth = _playFieldHeight = 0;
	_matrixSize = 0;
	_tile = nullptr;
}

Room::~Room() {
	delete[] _playField;
	delete[] _tile;
}

void Room::freePlayField() {
	delete[] _playField;
	_playField = nullptr;
}

void Room::freeTileData() {
	delete[] _tile;
	_tile = nullptr;
}

void Room::doRoom() {
	bool reloadFlag = false;

	while (!_vm->shouldQuit()) {
		if (!reloadFlag) {
			_vm->_images.clear();
			_vm->_newRects.clear();
			_vm->_oldRects.clear();
			_vm->_numAnimTimers = 0;

			reloadRoom();
		}

		reloadFlag = false;
		_vm->_startup = 0;
		_function = 0;

		while (!_vm->shouldQuit()) {
			_vm->_images.clear();
			if (_vm->_startup != -1 && --_vm->_startup != 0) {
				_vm->_events->showCursor();
				_vm->_screen->fadeIn();
			}

			// TODO: Game loop doesn't seem to have any delay. For now,
			// introduce a slight delay here
			_vm->_events->delay(50);

			// Handle any events
			_vm->_events->pollEvents();

			_vm->_player->walk();
			_vm->_sound->midiRepeat();
			_vm->_screen->checkScroll();
			doCommands();

			// DOROOMFLASHBACK jump point
			if (_function == 1) {
				clearRoom();
				break;
			} else if (_function == 2) {
				clearRoom();
				return;
			} else if (_function == 3) {
				reloadRoom1();
				reloadFlag = true;
				break;
			} else if (_function == 4) {
				break;
			}

			if (_vm->_screen->_scrollFlag) {
				_vm->copyBF1BF2();
				_vm->_newRects.clear();
				_function = 0;
				roomLoop();

				if (_function == 1) {
					clearRoom();
					break;
				} else {
					_vm->plotList();
					_vm->copyRects();
					_vm->copyBF2Vid();
				}
			} else {
				_vm->copyBF1BF2();
				_vm->_newRects.clear();
				_function = 0;
				roomLoop();

				if (_function == 1) {
					clearRoom();
					break;
				} else {
					_vm->plotList();
					_vm->copyBlocks();
				}
			}
		}
	}
}

void Room::clearRoom() {
	if (_vm->_sound->_music) {
		_vm->_sound->stopSong();
		delete[] _vm->_sound->_music;
		_vm->_sound->_music = nullptr;
	}

	_vm->_sound->freeSounds();
	_vm->_numAnimTimers = 0;

	_vm->_animation->freeAnimationData();
	_vm->_scripts->freeScriptData();
	_vm->freeCells();
	freePlayField();
	_vm->freeInactiveData();
	_vm->freeManData();
}

void Room::loadRoomData(const byte *roomData) {
	RoomInfo roomInfo(roomData);

	_roomFlag = roomInfo._roomFlag;

	_vm->_establishFlag = false;
	if (roomInfo._estIndex != -1) {
		_vm->_establishFlag = true;
		if (_vm->_establishTable[roomInfo._estIndex] != 1) {
			_vm->_establishTable[roomInfo._estIndex] = 1;
			_vm->establish(0);
		}
	}

	_vm->_sound->freeMusic();
	if (roomInfo._musicFile._fileNum != -1) {
		_vm->_sound->_music = _vm->_files->loadFile(roomInfo._musicFile._fileNum,
			roomInfo._musicFile._subfile);
		_vm->_sound->_midiSize = _vm->_files->_filesize;
		_vm->_sound->midiPlay();
		_vm->_sound->_musicRepeat = true;
	}

	_vm->_scaleH1 = roomInfo._scaleH1;
	_vm->_scaleH2 = roomInfo._scaleH2;
	_vm->_scaleN1 = roomInfo._scaleN1;
	_vm->_scaleT1 = ((_vm->_scaleH2 - _vm->_scaleH1) << 8) / _vm->_scaleN1;

	if (roomInfo._playFieldFile._fileNum != -1) {
		loadPlayField(roomInfo._playFieldFile._fileNum,
			roomInfo._playFieldFile._subfile);
		setupRoom();

		_vm->_scaleMaxY = _vm->_playFieldHeight << 4;
	}

	// Load cells
	_vm->loadCells(roomInfo._cells);

	// Load script data
	_vm->_scripts->freeScriptData();
	if (roomInfo._scriptFile._fileNum != -1) {
		const byte *data = _vm->_files->loadFile(roomInfo._scriptFile._fileNum,
			roomInfo._scriptFile._subfile);
		_vm->_scripts->setScript(data, _vm->_files->_filesize);
	}

	// Load animation data
	_vm->_animation->freeAnimationData();
	if (roomInfo._animFile._fileNum != -1) {
		byte *data = _vm->_files->loadFile(roomInfo._animFile._fileNum,
			roomInfo._animFile._subfile);
		_vm->_animation->loadAnimations(data, _vm->_files->_filesize);
	}

	_vm->_scaleI = roomInfo._scaleI;
	_vm->_screen->_scrollThreshold = roomInfo._scrollThreshold;

	// Handle loading scene palette data
	if (roomInfo._paletteFile._fileNum != -1) {
		_vm->_screen->_startColor = roomInfo._startColor;
		_vm->_screen->_numColors = roomInfo._numColors;
		_vm->_screen->loadPalette(roomInfo._paletteFile._fileNum,
			roomInfo._paletteFile._subfile);
	}

	// Load extra cells
	_vm->_extraCells.clear();
	for (uint i = 0; i < roomInfo._vidTable.size(); ++i) {
		ExtraCell ec;
		ec._vidTable = roomInfo._vidTable[i] & 0xffff;
		ec._vidTable1 = roomInfo._vidTable[i] >> 16;

		_vm->_extraCells.push_back(ec);
	}

	// Load sounds for the scene
	_vm->_sound->loadSounds(roomInfo._sounds);
}

void Room::roomLoop() {
	_vm->_scripts->_sequence = 2000;
	_vm->_scripts->searchForSequence();
	_vm->_scripts->executeScript();
}

void Room::setupRoom() {
	_vm->_screen->setScaleTable(_vm->_scale);
	_vm->_screen->setBufferScan();

	if (_roomFlag != 2)
		setIconPalette();

	if (_vm->_screen->_vWindowWidth == _playFieldWidth) {
		_vm->_screen->_scrollX = 0;
		_vm->_screen->_scrollCol = 0;
	} else {
		_vm->_screen->_scrollX = _vm->_player->_rawPlayer.x -
			(_vm->_player->_rawPlayer.x >> 4);
		int xp = MAX((_vm->_player->_rawPlayer.x >> 4) -
			(_vm->_screen->_vWindowWidth / 2), 0);
		_vm->_screen->_scrollCol = xp;

		xp = xp + _vm->_screen->_vWindowWidth - _playFieldWidth;
		if (xp >= 0) {
			_vm->_screen->_scrollCol = xp + 1;
		}
	}
	
	if (_vm->_screen->_vWindowHeight == _playFieldHeight) {
		_vm->_screen->_scrollY = 0;
		_vm->_screen->_scrollRow = 0;
	} else {
		_vm->_screen->_scrollY = _vm->_player->_rawPlayer.y -
			(_vm->_player->_rawPlayer.y >> 4);
		int yp = MAX((_vm->_player->_rawPlayer.y >> 4) -
			(_vm->_screen->_vWindowHeight / 2), 0);
		_vm->_screen->_scrollRow = yp;

		yp = yp + _vm->_screen->_vWindowHeight - _playFieldHeight;
		if (yp >= 0) {
			_vm->_screen->_scrollRow = yp + 1;
		}
	}
}

void Room::setWallCodes() {
	_jetFrame.clear();
	_jetFrame.resize(_plotter._walls.size());

	_vm->_player->_rawXTemp = _vm->_player->_rawPlayer.x;
	_vm->_player->_rawYTemp = _vm->_player->_rawPlayer.y;
}

void Room::buildScreen() {
	int scrollCol = _vm->_screen->_scrollCol;
	int cnt = _vm->_screen->_vWindowWidth + 1;
	int offset = 0;

	for (int idx = 0; idx < cnt; offset += TILE_WIDTH, ++idx) {
		buildColumn(_vm->_screen->_scrollCol, offset);
		++_vm->_screen->_scrollCol;
	}

	_vm->_screen->_scrollCol = scrollCol;
	_vm->copyBF1BF2();
}

void Room::buildColumn(int playX, int screenX) {
	const byte *pSrc = _playField + _vm->_screen->_scrollRow * 
		_playFieldWidth + playX;

	for (int y = 0; y <= _vm->_screen->_vWindowHeight; ++y) {
		byte *pTile = _tile + (*pSrc << 8);
		byte *pDest = (byte *)_vm->_buffer1.getBasePtr(screenX, y * TILE_HEIGHT);

		for (int tileY = 0; tileY < TILE_HEIGHT; ++tileY) {
			Common::copy(pTile, pTile + TILE_WIDTH, pDest);
			pTile += TILE_WIDTH;
			pDest += _vm->_buffer1.pitch;
		}

		pSrc += _playFieldWidth;
	}
}

void Room::init4Quads() {
	error("TODO: init4Quads");
}

void Room::loadPlayField(int fileNum, int subfile) {
	byte *playData = _vm->_files->loadFile(fileNum, subfile);
	Common::MemoryReadStream stream(playData + 0x10, _vm->_files->_filesize - 0x10);
	Screen &screen = *_vm->_screen;

	// Copy the new palette
	screen.loadRawPalette(&stream);

	// Copy off the tile data
	_tileSize = playData[2] << 8;
	_tile = new byte[_tileSize];
	stream.read(_tile, _tileSize);

	// Copy off the playfield data
	_matrixSize = playData[0] * playData[1];
	_playField = new byte[_matrixSize];
	stream.read(_playField, _matrixSize);

	// Load the plotter data
	int numWalls = READ_LE_UINT16(playData + 6);
	int numBlocks = playData[8];
	_plotter.load(&stream, numWalls, numBlocks);

	_playFieldWidth = playData[0];
	_playFieldHeight = playData[1];
	screen._vWindowWidth = playData[3];
	screen._vWindowBytesWide = screen._vWindowWidth << 4;
	screen._bufferBytesWide = screen._vWindowBytesWide + 16;
	screen._vWindowHeight = playData[4];
	screen._vWindowLinesTall = screen._vWindowHeight << 4;

	_vm->_screen->setBufferScan();
	delete[] playData;
}

/*------------------------------------------------------------------------*/

Plotter::Plotter() {
	_delta = _blockIn = 0;
}

void Plotter::load(Common::SeekableReadStream *stream, int wallCount, int blockCount) {
	// Load the wall count
	_walls.resize(wallCount);
	
	for (int i = 0; i < wallCount; ++i)
		_walls[i].left = stream->readSint16LE();
	for (int i = 0; i < wallCount; ++i)
		_walls[i].top = stream->readSint16LE();
	for (int i = 0; i < wallCount; ++i)
		_walls[i].right = stream->readSint16LE();
	for (int i = 0; i < wallCount; ++i)
		_walls[i].bottom = stream->readSint16LE();

	// Load the block list
	_blocks.resize(blockCount);

	for (int i = 0; i < blockCount; ++i)
		_blocks[i].left = stream->readSint16LE();
	for (int i = 0; i < blockCount; ++i)
		_blocks[i].top = stream->readSint16LE();
	for (int i = 0; i < blockCount; ++i)
		_blocks[i].right = stream->readSint16LE();
	for (int i = 0; i < blockCount; ++i)
		_blocks[i].bottom = stream->readSint16LE();
}

/*------------------------------------------------------------------------*/

RoomInfo::RoomInfo(const byte *data) {
	Common::MemoryReadStream stream(data, 999);

	_roomFlag = stream.readByte();
	_estIndex = (int16)stream.readUint16LE();
	_musicFile._fileNum = (int16)stream.readUint16LE();
	_musicFile._subfile = stream.readUint16LE();
	_scaleH1 = stream.readByte();
	_scaleH2 = stream.readByte();
	_scaleN1 = stream.readByte();
	_playFieldFile._fileNum = (int16)stream.readUint16LE();
	_playFieldFile._subfile = stream.readUint16LE();

	for (byte cell = stream.readByte(); cell != 0xff; cell = stream.readByte()) {
		CellIdent ci;
		ci._cell = cell;
		ci._fileNum = (int16)stream.readUint16LE();
		ci._subfile = stream.readUint16LE();

		_cells.push_back(ci);
	}

	_scriptFile._fileNum = (int16)stream.readUint16LE();
	_scriptFile._subfile = stream.readUint16LE();
	_animFile._fileNum = (int16)stream.readUint16LE();
	_animFile._subfile = stream.readUint16LE();
	_scaleI = stream.readByte();
	_scrollThreshold = stream.readByte();
	_paletteFile._fileNum = (int16)stream.readUint16LE();
	_paletteFile._subfile = stream.readUint16LE();
	if (_paletteFile._fileNum == -1) {
		_startColor = _numColors = 0;
	} else {
		_startColor = stream.readUint16LE();
		_numColors = stream.readUint16LE();
	}

	for (int16 v = (int16)stream.readUint16LE(); v != -1;
			v = (int16)stream.readUint16LE()) {
		uint16 v2 = stream.readUint16LE();

		_vidTable.push_back(v | ((uint32)v2 << 16));
	}

	for (int16 fileNum = (int16)stream.readUint16LE(); fileNum != -1;
			fileNum = (int16)stream.readUint16LE()) {
		SoundIdent fi;
		fi._fileNum = fileNum;
		fi._subfile = stream.readUint16LE();
		fi._priority = stream.readUint16LE();

		_sounds.push_back(fi);
	}
};

} // End of namespace Access
