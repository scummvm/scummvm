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

#include "common/scummsys.h"
#include "common/memstream.h"
#include "access/access.h"
#include "access/resources.h"
#include "access/room.h"
#include "access/amazon/amazon_resources.h"
#include "access/martian/martian_resources.h"

namespace Access {

Room::Room(AccessEngine *vm) : Manager(vm) {
	_function = FN_NONE;
	_roomFlag = 0;
	_playField = nullptr;
	_playFieldWidth = _playFieldHeight = 0;
	_matrixSize = 0;
	_tile = nullptr;
	_selectCommand = 0;
	_conFlag = false;
	_selectCommand = -1;

	switch (vm->getGameID()) {
	case GType_Amazon:
		for (int i = 0; i < 10; i++) {
			_rMouse[i][0] = Amazon::RMOUSE[i][0];
			_rMouse[i][1] = Amazon::RMOUSE[i][1];
		}
		break;
	case GType_MartianMemorandum:
		for (int i = 0; i < 10; i++) {
			_rMouse[i][0] = Martian::RMOUSE[i][0];
			_rMouse[i][1] = Martian::RMOUSE[i][1];
		}
		break;
	default:
		error("Game not supported");
	}
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

void Room::clearCamera() {
	_vm->_player->_scrollFlag = true;
	_vm->_events->hideCursor();

	_vm->_screen->_orgX1 = 48;
	_vm->_screen->_orgY1 = 24;
	_vm->_screen->_orgX2 = 274;
	_vm->_screen->_orgY2 = 152;
	_vm->_screen->_lColor = 0;
	_vm->_screen->drawRect();

	_vm->_events->showCursor();

	_vm->_events->_vbCount = 4;
	while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0)
		_vm->_events->pollEventsAndWait();
}

void Room::takePicture() {
	_vm->_events->pollEvents();
	if (!_vm->_events->_leftButton)
		return;

	Common::Array<Common::Rect> pictureCoords;
	for (int i = 0; Martian::PICTURERANGE[i][0] != -1; i += 2) {
		pictureCoords.push_back(Common::Rect(Martian::PICTURERANGE[i][0], Martian::PICTURERANGE[i + 1][0],
			                                 Martian::PICTURERANGE[i][1], Martian::PICTURERANGE[i + 1][1]));
	}

	int result = _vm->_events->checkMouseBox1(pictureCoords);

	if (result == 4) {
		_vm->_events->debounceLeft();
		if (_vm->_inventory->_inv[44]._value != ITEM_IN_INVENTORY) {
			Common::String msg = "YOU HAVE NO MORE FILM.";
			_vm->_scripts->doCmdPrint_v1(msg);
			return;
		}

		if ((_vm->_scrollCol < 35) || (_vm->_scrollRow >= 20)){
			Common::String msg = "THAT ISN'T INTERESTING ENOUGH TO WASTE FILM ON.";
			_vm->_scripts->doCmdPrint_v1(msg);
			return;
		}

		if (_vm->_inventory->_inv[26]._value != ITEM_USED) {
			Common::String msg = "ALTHOUGH IT WOULD MAKE A NICE PICTURE, YOU MAY FIND SOMETHING MORE INTERESTING TO USE YOUR FILM ON.";
			_vm->_scripts->doCmdPrint_v1(msg);
			return;
		}

		Common::String msg = "THAT PHOTO MAY COME IN HANDY SOME DAY.";
		_vm->_scripts->doCmdPrint_v1(msg);
		_vm->_inventory->_inv[8]._value = ITEM_IN_INVENTORY;
		_vm->_pictureTaken++;
		if (_vm->_pictureTaken == 16)
			_vm->_inventory->_inv[44]._value = ITEM_USED;

		_vm->_events->debounceLeft();
		_vm->_sound->playSound(0);
		clearCamera();
		return;
	} else if (result == 5) {
		if (_vm->_flags[26] != 2) {
			_vm->_video->closeVideo();
			_vm->_video->_videoEnd = true;
		}
		_vm->_player->_roomNumber = 7;
		_vm->_room->_function = FN_CLEAR1;
		return;
	} else if (result >= 0)
		_vm->_player->_move = (Direction)(result + 1);

	_vm->_player->_scrollFlag = false;
	if (_vm->_player->_move == UP)
		_vm->_player->scrollDown(2);
	else if (_vm->_player->_move == DOWN)
		_vm->_player->scrollUp(2);
	else if (_vm->_player->_move == LEFT)
		_vm->_player->scrollRight(2);
	else if (_vm->_player->_move == RIGHT)
		_vm->_player->scrollLeft(2);
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
		_vm->_startup = 8;
		_function = FN_NONE;
		_vm->_screen->_fadeIn = false;

		while (!_vm->shouldQuit()) {
			_vm->_images.clear();
			if (_vm->_screen->_fadeIn) {
				_vm->_events->showCursor();
				_vm->_screen->fadeIn();
				_vm->_screen->_fadeIn = false;
			}

			// Poll for events
			_vm->_canSaveLoad = true;
			_vm->_events->pollEventsAndWait();
			_vm->_canSaveLoad = false;

			if ((_vm->getGameID() == GType_MartianMemorandum) && (_vm->_player->_roomNumber == 47)) {
				takePicture();
			} else {
				_vm->_player->walk();
				_vm->_midi->midiRepeat();
				_vm->_player->checkScroll();
			}

			doCommands();
			if (_vm->shouldQuitOrRestart())
				return;

			// DOROOMFLASHBACK jump point
			if (_function == FN_CLEAR1) {
				clearRoom();
				break;
			} else if (_function == FN_CLEAR2) {
				clearRoom();
				return;
			} else if (_function == FN_RELOAD) {
				reloadRoom1();
				reloadFlag = true;
				break;
			} else if (_function == FN_BREAK) {
				break;
			}

			if (_vm->_player->_scrollFlag) {
				_vm->copyBF1BF2();
				_vm->_newRects.clear();
				_function = FN_NONE;
				roomLoop();

				if (_function == FN_CLEAR1) {
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
				_function = FN_NONE;

				roomLoop();
				if (_vm->shouldQuitOrRestart())
					return;

				if (_function == FN_CLEAR1) {
					clearRoom();
					break;
				} else {
					_vm->plotList();

					if (((_vm->getGameID() == GType_MartianMemorandum) && (_vm->_events->_mousePos.y < 184)) ||
						((_vm->getGameID() == GType_Amazon) && (_vm->_events->_mousePos.y < 177)))
						_vm->_events->setCursor(_vm->_events->_normalMouse);
					else
						_vm->_events->setCursor(CURSOR_ARROW);

					_vm->copyBlocks();
				}
			}
		}
	}
}

void Room::clearRoom() {
	if (_vm->_midi->_music) {
		_vm->_midi->stopSong();
		_vm->_midi->freeMusic();
	}

	_vm->_sound->freeSounds();
	_vm->_numAnimTimers = 0;

	_vm->_animation->freeAnimationData();
	_vm->_scripts->freeScriptData();
	_vm->freeCells();
	freePlayField();
	freeTileData();
	_vm->_player->freeSprites();
}

void Room::loadRoomData(const byte *roomData) {
	RoomInfo roomInfo(roomData, _vm->getGameID(), _vm->isCD(), _vm->isDemo());

	_roomFlag = roomInfo._roomFlag;

	_vm->_establishFlag = false;
	if (roomInfo._estIndex != -1) {
		_vm->_establishFlag = true;
		if (!_vm->_establishTable[roomInfo._estIndex]) {
			_vm->_establishTable[roomInfo._estIndex] = true;
			_vm->establish(0, roomInfo._estIndex);
		}
	}

	_vm->_midi->freeMusic();
	if (roomInfo._musicFile._fileNum != -1) {
		_vm->_midi->loadMusic(roomInfo._musicFile);
		_vm->_midi->midiPlay();
		_vm->_midi->setLoop(true);
	}

	_vm->_scaleH1 = roomInfo._scaleH1;
	_vm->_scaleH2 = roomInfo._scaleH2;
	_vm->_scaleN1 = roomInfo._scaleN1;
	_vm->_scaleT1 = ((_vm->_scaleH1 - _vm->_scaleH2) << 8) / _vm->_scaleN1;

	if (roomInfo._playFieldFile._fileNum != -1) {
		loadPlayField(roomInfo._playFieldFile._fileNum,
			roomInfo._playFieldFile._subfile);
		setupRoom();

		_vm->_scaleMaxY = _playFieldHeight << 4;
	}

	// Load cells
	_vm->loadCells(roomInfo._cells);

	// Load script data
	_vm->_scripts->freeScriptData();
	if (roomInfo._scriptFile._fileNum != -1) {
		Resource *newScript = _vm->_files->loadFile(roomInfo._scriptFile);
		_vm->_scripts->setScript(newScript);
	}

	// Load animation data
	_vm->_animation->freeAnimationData();
	if (roomInfo._animFile._fileNum != -1) {
		Resource *anim = _vm->_files->loadFile(roomInfo._animFile);
		_vm->_animation->loadAnimations(anim);
		delete anim;
	}

	_vm->_scale = _vm->_scaleI = roomInfo._scaleI;
	_vm->_screen->setScaleTable(_vm->_scale);
	_vm->_player->_scrollThreshold = roomInfo._scrollThreshold;

	// Handle loading scene palette data
	if (roomInfo._paletteFile._fileNum != -1) {
		_vm->_screen->_startColor = roomInfo._startColor;
		_vm->_screen->_numColors = roomInfo._numColors;
		_vm->_screen->loadPalette(roomInfo._paletteFile._fileNum,
			roomInfo._paletteFile._subfile);
	}

	// Load extra cells
	_vm->_extraCells.clear();
	for (uint i = 0; i < roomInfo._extraCells.size(); ++i)
		_vm->_extraCells.push_back(roomInfo._extraCells[i]);

	// Load sounds for the scene
	_vm->_sound->loadSounds(roomInfo._sounds);
}

void Room::roomLoop() {
	_vm->_scripts->_sequence = ROOM_SCRIPT;
	_vm->_scripts->searchForSequence();
	_vm->_scripts->executeScript();
}

void Room::setupRoom() {
	Screen &screen = *_vm->_screen;
	screen.setScaleTable(_vm->_scale);
	screen.setBufferScan();

	if (_roomFlag != 2)
		screen.setIconPalette();

	if (screen._vWindowWidth == _playFieldWidth) {
		_vm->_scrollX = 0;
		_vm->_scrollCol = 0;
	} else {
		int xv = _vm->_player->_rawPlayer.x / TILE_WIDTH;
		_vm->_scrollX = _vm->_player->_rawPlayer.x % TILE_WIDTH;
		_vm->_scrollCol = MAX(xv - (screen._vWindowWidth / 2), 0);

		int sx = _vm->_scrollCol + screen._vWindowWidth - _playFieldWidth;
		if (sx >= 0) {
			_vm->_scrollCol -= sx + 1;
		}
	}

	if (screen._vWindowHeight == _playFieldHeight) {
		_vm->_scrollY = 0;
		_vm->_scrollRow = 0;
	} else {
		_vm->_scrollY = _vm->_player->_rawPlayer.y -
			(_vm->_player->_rawPlayer.y / 16) * 16;
		int yc = MAX((_vm->_player->_rawPlayer.y >> 4) -
			(screen._vWindowHeight / 2), 0);
		_vm->_scrollRow = yc;

		yc = yc + screen._vWindowHeight - _playFieldHeight;
		if (yc >= 0) {
			_vm->_scrollRow = _playFieldHeight - screen._vWindowHeight;
			_vm->_scrollY = 0;
		}
	}
}

void Room::setWallCodes() {
	_jetFrame.clear();
	_jetFrame.resize(_plotter._walls.size());

	_vm->_player->_rawXTemp = _vm->_player->_rawPlayer.x;
	_vm->_player->_rawYTemp = _vm->_player->_rawPlayer.y;
	codeWalls();
}

void Room::buildScreen() {
	int scrollCol = _vm->_scrollCol;
	int offset = 0;

	// Clear current background buffer
	_vm->_buffer1.clearBuffer();

	// WORKAROUND: Original's use of '+ 1' would frequently cause memory overruns
	int w = MIN(_vm->_screen->_vWindowWidth + 1, _playFieldWidth);

	// Loop through drawing each column of tiles forming the background
	for (int idx = 0; idx < w; offset += TILE_WIDTH, ++idx) {
		buildColumn(_vm->_scrollCol, offset);
		++_vm->_scrollCol;
	}

	_vm->_scrollCol = scrollCol;
	_vm->copyBF1BF2();
}

void Room::buildColumn(int playX, int screenX) {
	if (playX < 0 || playX >= _playFieldWidth)
		return;

	const byte *pSrc = _playField + _vm->_scrollRow *
		_playFieldWidth + playX;

	// WORKAROUND: Original's use of '+ 1' would frequently cause memory overruns
	int h = MIN(_vm->_screen->_vWindowHeight + 1, _playFieldHeight);

	for (int y = 0; y < h; ++y) {
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

void Room::buildRow(int playY, int screenY) {
	if (playY < 0 || playY >= _playFieldHeight)
		return;
	assert(screenY <= (_vm->_screen->h - TILE_HEIGHT));

	const byte *pSrc = _playField + playY *_playFieldWidth + _vm->_scrollCol;

	// WORKAROUND: Original's use of '+ 1' would frequently cause memory overruns
	int w = MIN(_vm->_screen->_vWindowWidth + 1, _playFieldWidth);

	for (int x = 0; x < w; ++x) {
		byte *pTile = _tile + (*pSrc << 8);
		byte *pDest = (byte *)_vm->_buffer1.getBasePtr(x * TILE_WIDTH, screenY);

		for (int tileY = 0; tileY < TILE_HEIGHT; ++tileY) {
			Common::copy(pTile, pTile + TILE_WIDTH, pDest);
			pTile += TILE_WIDTH;
			pDest += _vm->_buffer1.pitch;
		}

		++pSrc;
	}
}

void Room::loadPlayField(int fileNum, int subfile) {
	Resource *playData = _vm->_files->loadFile(fileNum, subfile);
	byte header[16];
	playData->_stream->read(&header[0], 16);
	Screen &screen = *_vm->_screen;

	// Copy the new palette
	screen.loadRawPalette(playData->_stream);

	// Copy off the tile data
	int tileSize = (int)header[2] << 8;
	_tile = new byte[tileSize];
	playData->_stream->read(_tile, tileSize);

	// Copy off the playfield data
	_matrixSize = header[0] * header[1];
	_playField = new byte[_matrixSize];
	playData->_stream->read(_playField, _matrixSize);

	// Load the plotter data
	int numWalls = READ_LE_UINT16(&header[6]);
	int numBlocks = header[8];
	_plotter.load(playData->_stream, numWalls, numBlocks);

	_playFieldWidth = header[0];
	_playFieldHeight = header[1];
	screen._vWindowWidth = header[3];
	screen._vWindowBytesWide = screen._vWindowWidth << 4;
	screen._bufferBytesWide = screen._vWindowBytesWide + 16;
	screen._vWindowHeight = header[4];
	screen._vWindowLinesTall = screen._vWindowHeight << 4;

	_vm->_screen->setBufferScan();
	delete playData;
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

void Room::doCommands() {
	int commandId = 0;
	Common::KeyState keyState;

	if (_vm->_startup != -1)
		return;

	if (_vm->_inventory->_invChangeFlag)
		_vm->_inventory->refreshInventory();

	if (_vm->_screen->_screenChangeFlag) {
		_vm->_screen->_screenChangeFlag = false;
		_vm->_events->_cursorExitFlag = true;
		executeCommand(7);
	}
	else if (_vm->_events->_wheelUp || _vm->_events->_wheelDown) {
		// Handle scrolling mouse wheel
		cycleCommand(_vm->_events->_wheelUp ? 1 : -1);

	} else if (_vm->_events->_middleButton) {
		// Switch back to walking
		handleCommand(7);

	} else if (_vm->_events->_leftButton) {
		if (_vm->_events->_mouseRow >= 22) {
			// Mouse in user interface area
			for (commandId = 0; commandId < 10; ++commandId) {
				if (_vm->_events->_mousePos.x >= _rMouse[commandId][0] &&
					_vm->_events->_mousePos.x < _rMouse[commandId][1])
					break;
			}
			if (commandId < 10)
				handleCommand(commandId);

		} else {
			// Mouse click in main game area
			mainAreaClick();
		}
	} else if (_vm->_events->getKey(keyState)) {
		if (keyState.keycode == Common::KEYCODE_F1)
			handleCommand(keyState.keycode - Common::KEYCODE_F1 + 1);
		else if (keyState.keycode >= Common::KEYCODE_F2 && keyState.keycode <= Common::KEYCODE_F10)
			handleCommand(keyState.keycode - Common::KEYCODE_F1);
	}
}

void Room::cycleCommand(int incr) {
	int command = _selectCommand + incr;
	if (command < -1)
		command = 6;
	else if (command == -1)
		command = 7;
	else  if (command == 1)
		command = (incr == 1) ? 2 : 0;
	else if (command == 4)
		command = (incr == 1) ? 5 : 3;

	handleCommand(command);
}

void Room::handleCommand(int commandId) {
	if (commandId == 9) {
		_vm->_events->debounceLeft();
		_vm->_canSaveLoad = true;
		_vm->openMainMenuDialog();
		_vm->_canSaveLoad = false;
	}  else if (commandId == _selectCommand) {
		_vm->_events->debounceLeft();
		commandOff();
	} else {
		_vm->_events->debounceLeft();
		executeCommand(commandId);
	}
}

void Room::executeCommand(int commandId) {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	_selectCommand = commandId;

	if (_vm->getGameID() == GType_MartianMemorandum) {
		switch (commandId) {
		case 4:
			events.setCursor(CURSOR_ARROW);
			if (_vm->_inventory->displayInv() == 2) {
				commandOff();
				return;
			}
			if (_vm->_useItem == 39) {
				if (_vm->_player->_roomNumber == 23)
					_vm->_currentMan = 1;
				commandOff();
				return;
			} else if (_vm->_useItem == 6) {
				_vm->_flags[3] = 2;
				_vm->_scripts->converse1(24);

				_conFlag = true;
				while (_conFlag && !_vm->shouldQuitOrRestart()) {
					_conFlag = false;
					_vm->_scripts->executeScript();
				}

				_vm->_boxSelect = true;
				return;
			}
			break;
		case 7:
			walkCursor();
			return;
		case 8: {
			events.forceSetCursor(CURSOR_CROSSHAIRS);
			_vm->_scripts->_sequence = 10000;
			_vm->_scripts->searchForSequence();

			_conFlag = true;
			while (_conFlag && !_vm->shouldQuitOrRestart()) {
				_conFlag = false;
				_vm->_scripts->executeScript();
			}

			_vm->_boxSelect = true;
			return;
			}
		default:
			// No set cursor in MM. Forcing to CROSSHAIRS
			events.setCursor(CURSOR_CROSSHAIRS);
			break;
		}
	} else {
		switch (commandId) {
		case 0:
		case 1:
			events.forceSetCursor(CURSOR_LOOK);
			break;
		case 2:
			events.forceSetCursor(CURSOR_USE);
			break;
		case 3:
			events.forceSetCursor(CURSOR_TAKE);
			break;
		case 4:
			events.setCursor(CURSOR_ARROW);
			if (_vm->_inventory->newDisplayInv() == 2) {
				commandOff();
				return;
			}
			break;
		case 5:
			events.forceSetCursor(CURSOR_CLIMB);
			break;
		case 6:
			events.forceSetCursor(CURSOR_TALK);
			break;
		case 7:
			walkCursor();
			return;
		case 8:
			events.forceSetCursor(CURSOR_HELP);
			break;
		default:
			break;
		}
	}
	screen.saveScreen();
	screen.setDisplayScan();

	// Get the toolbar icons resource
	Resource *iconData = _vm->_files->loadFile("ICONS.LZ");
	SpriteResource *spr = new SpriteResource(_vm, iconData);
	delete iconData;

	// Draw the button as selected
	screen.plotImage(spr, 0, Common::Point(0, 177));
	screen.plotImage(spr, 1, Common::Point(143, 177));
	screen.plotImage(spr, _selectCommand + 2,
		Common::Point(_rMouse[_selectCommand][0], (_vm->getGameID() == GType_MartianMemorandum) ? 184 : 176));

	delete spr;

	_vm->_screen->restoreScreen();
	_vm->_boxSelect = true;
}

void Room::walkCursor() {
	EventsManager &events = *_vm->_events;

	events.forceSetCursor(CURSOR_CROSSHAIRS);
	_vm->_scripts->_sequence = 5000;
	_vm->_scripts->searchForSequence();
	roomMenu();
	_selectCommand = -1;

	_conFlag = true;
	while (_conFlag && !_vm->shouldQuitOrRestart()) {
		_conFlag = false;
		_vm->_scripts->executeScript();
	}

	_vm->_boxSelect = true;
}

void Room::commandOff() {
	_selectCommand = -1;
	_vm->_events->forceSetCursor(CURSOR_CROSSHAIRS);
	roomMenu();
}

int Room::checkBoxes() {
	return checkBoxes1(_vm->_player->_rawPlayer);
}

int Room::checkBoxes1(const Common::Point &pt) {
	return checkBoxes2(pt, 0, _plotter._blocks.size());
}

int Room::checkBoxes2(const Common::Point &pt, int start, int count) {
	for (; count > 0; --count, ++start) {
		if (_plotter._blocks[start].contains(pt)) {
			_plotter._blockIn = start;
			return start;
		}
	}

	return -1;
}

void Room::checkBoxes3() {
	Common::Point pt = _vm->_events->calcRawMouse();

	for (uint start = 0; start < _plotter._blocks.size(); ++start) {
		if (_plotter._blocks[start].contains(pt)) {
			_plotter._blockIn = start;
			if (!(validateBox(start) & 0x80)) {
				_vm->_events->debounceLeft();
				_vm->_boxSelect = start;

				_conFlag = true;
				while (_conFlag && !_vm->shouldQuitOrRestart()) {
					_conFlag = false;
					_vm->_scripts->executeScript();
				}

				_vm->_boxSelect = true;
				return;
			}
		}
	}
}

int Room::validateBox(int boxId) {
	_vm->_scripts->_sequence = boxId;
	_vm->_scripts->searchForSequence();
	return _vm->_scripts->executeScript();
}

void Room::swapOrg() {
	SWAP<int>(_vm->_screen->_orgX1, _vm->_screen->_orgX2);
	SWAP<int>(_vm->_screen->_orgY1, _vm->_screen->_orgY2);
}

int Room::calcLR(int yp) {
	const Screen &screen = *_vm->_screen;

	int yv = (yp - screen._orgY1) * (screen._orgX2 - screen._orgX1);
	int yd = screen._orgY2 - screen._orgY1;

	int rem = (yv % yd) << 1;
	yv /= yd;
	if (rem >= yd || rem < 0)
		++yv;

	return yv + screen._orgX1;
}

int Room::calcUD(int xp) {
	const Screen &screen = *_vm->_screen;

	int xv = (xp - screen._orgX1) * (screen._orgY2 - screen._orgY1);
	int xd = screen._orgX2 - screen._orgX1;

	int rem = (xv % xd) << 1;
	xv /= xd;
	if (rem >= xd || rem < 0)
		++xv;

	return xv + screen._orgY1;
}

bool Room::codeWalls() {
	Screen &screen = *_vm->_screen;
	Player &player = *_vm->_player;

	if (_plotter._walls.size() == 0)
		return false;

	for (uint i = 0; i < _plotter._walls.size(); ++i) {
		Common::Rect &r = _plotter._walls[i];
		JetFrame &jf = _jetFrame[i];

		jf._wallCode = 0;
		jf._wallCode1 = 0;
		screen._orgX1 = r.left;
		screen._orgY1 = r.top;
		screen._orgX2 = r.right;
		screen._orgY2 = r.bottom;

		if (screen._orgY2 != screen._orgY1) {
			if (screen._orgY2 < screen._orgY1)
				swapOrg();

			if ((player._rawYTemp >= screen._orgY1) &&
					(player._rawYTemp <= screen._orgY2)) {
				jf._wallCode |= (calcLR(player._rawYTemp) - player._rawXTemp) < 0 ? 2 : 1;
				jf._wallCode1 |= (calcLR(player._rawYTemp) -
					(player._rawXTemp + player._playerOffset.x)) < 0 ? 2 : 1;
			}
		}

		if (screen._orgX2 != screen._orgX1) {
			if (screen._orgX2 < screen._orgX1)
				swapOrg();

			if ((player._rawXTemp >= screen._orgX1) &&
					(player._rawXTemp <= screen._orgX2)) {
				int y = screen._orgY2;
				if (y != screen._orgY1)
					y = calcUD(player._rawXTemp);

				jf._wallCode |= (player._rawYTemp - y) < 0 ? 4 : 8;
			}

			int x = player._rawXTemp + player._playerOffset.x;
			if ((x >= screen._orgX1) && (x <= screen._orgX2)) {
				int y = screen._orgY2;
				if (screen._orgY2 != screen._orgY1)
					y = calcUD(player._rawXTemp + player._playerOffset.x);

				jf._wallCode1 |= (player._rawYTemp - y) < 0 ? 4 : 8;
			}
		}
	}

	for (uint i = 0; i < _jetFrame.size(); ++i) {
		JetFrame &jf = _jetFrame[i];
		if (checkCode(jf._wallCode, jf._wallCodeOld) ||
			checkCode(jf._wallCode1, jf._wallCode1Old))
			return true;
	}

	// Copy the current wall calculations to the old properties
	for (uint i = 0; i < _jetFrame.size(); ++i) {
		JetFrame &jf = _jetFrame[i];
		jf._wallCodeOld = jf._wallCode;
		jf._wallCode1Old = jf._wallCode1;
	}

	return false;
}

bool Room::checkCode(int v1, int v2) {
	Player &p = *_vm->_player;

	if (!v1 || !v2 || (v1 == v2))
		return false;

	if (v1 & 1) {
		if (v2 & 2) {
			p._collideFlag = true;
			return true;
		}
	} else if (v1 & 2) {
		if (v2 & 1) {
			p._collideFlag = true;
			return true;
		}
	} else if (v1 & 4) {
		if (v2 & 8) {
			p._collideFlag = true;
			return true;
		}
	} else if (v1 & 8) {
		if (v2 & 4) {
			p._collideFlag = true;
			return true;
		}
	}

	return false;
}

/*------------------------------------------------------------------------*/

RoomInfo::RoomInfo(const byte *data, int gameType, bool isCD, bool isDemo) {
	Common::MemoryReadStream stream(data, 999);

	_roomFlag = stream.readByte();

	_estIndex = -1;
	if (gameType == GType_Amazon && isCD)
		_estIndex = stream.readSint16LE();

	_musicFile.load(stream);
	_scaleH1 = stream.readByte();
	_scaleH2 = stream.readByte();
	_scaleN1 = stream.readByte();
	_playFieldFile.load(stream);

	for (byte cell = stream.readByte(); cell != 0xff; cell = stream.readByte()) {
		CellIdent ci;
		ci._cell = cell;
		ci.load(stream);

		_cells.push_back(ci);
	}

	_scriptFile.load(stream);
	_animFile.load(stream);
	_scaleI = stream.readByte();
	_scrollThreshold = stream.readByte();
	_paletteFile.load(stream);
	if (_paletteFile._fileNum == -1) {
		_startColor = _numColors = 0;
	} else {
		_startColor = stream.readUint16LE();
		_numColors = stream.readUint16LE();
	}

	for (int16 v = stream.readSint16LE(); v != -1; v = stream.readSint16LE()) {
		ExtraCell ec;
		ec._vid._fileNum = v;
		ec._vid._subfile = stream.readSint16LE();

		_extraCells.push_back(ec);
	}

	for (int16 fileNum = stream.readSint16LE(); fileNum != -1; fileNum = stream.readSint16LE()) {
		SoundIdent fi;
		fi._fileNum = fileNum;
		fi._subfile = stream.readUint16LE();
		fi._priority = stream.readUint16LE();

		_sounds.push_back(fi);
	}
}

} // End of namespace Access
