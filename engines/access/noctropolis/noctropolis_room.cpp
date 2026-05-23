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

#include "common/scummsys.h"

#include "access/noctropolis/noctropolis_game.h"
#include "access/noctropolis/noctropolis_room.h"
#include "access/noctropolis/noctropolis_player.h"
#include "access/noctropolis/noctropolis_resources.h"
#include "access/access.h"

namespace Access {

namespace Noctropolis {

NoctropolisRoom::NoctropolisRoom(AccessEngine *vm): Room(vm) {
}

void NoctropolisRoom::reloadRoom() {
	loadRoom(_vm->_player->_roomNumber);

	loadPlayer1();

	reloadRoom1();
}

void NoctropolisRoom::loadPlayer1() {
	int subFileBase = 1;
	int numSubFiles = -1;
	int objBase = -1;
	int fileNum = -1;
	int palOffset = 6;
	if (!(_roomFlag & kRoomFlagTopView)) {
		palOffset = 6;
		if ((int8)_roomFlag > -1) {
			if ((_vm->_flags[1] & 1) == 0) {
				// Peter
				numSubFiles = 5;
				objBase = 100;
				fileNum = 0xfe;
			} else {
				// Dark
				numSubFiles = 5;
				objBase = 105;
				fileNum = 0xff;
			}
		} else {
			// kRoomFlagNoPlayer.
		}
	} else {
		// Top
		numSubFiles = 1;
		objBase = 115;
		fileNum = 0xfc;
		palOffset = 2;
	}

	if (fileNum > 0) {
		_vm->_player->loadNoctPalette(fileNum, _palIntensity + palOffset);
		((NoctropolisPlayer *)_vm->_player)->loadAnimation(fileNum, 0);

		for (int i = subFileBase; i <= numSubFiles; i++) {
			Resource *data = _vm->_files->loadFile(fileNum, i);
			_vm->_objectsTable[objBase + i - subFileBase] = new SpriteResource(_vm, data);
		}
	}
}

void NoctropolisRoom::reloadRoom1() {
	// Is this also in the FN_RELOAD part of afterDoCommandsTick
	NoctropolisEngine *vm = (NoctropolisEngine *)_vm;
	vm->_events->_interfaceOff = false;
	vm->_player->_playerMove = false;
	vm->_player->_playerDirection = Direction::NONE;
	vm->_stil->_playerDirection = Direction::NONE;
	vm->_flags[0] = 0;
	_selectCommand = -1;
	vm->_boxSelect = false; //-1
	vm->_player->_playerOff = false;
	vm->_stil->_playerOff = false;

	if (!vm->_loadFlag)
		vm->setStilettoPos();

	vm->_loadFlag = false;
	_vm->_screen->forceFadeOut();
	_vm->_screen->clearScreen();
	_vm->_buffer1.clear();
	_vm->_buffer2.clear();
	roomInit();
	_vm->_player->load();

	_vm->_screen->setBufferScan();
	setupRoom();
	setWallCodes();
	buildScreen();
	_vm->copyBF2Vid();

	if (_roomFlag >= 0 && !(kRoomFlagNoPlayer & _roomFlag)) {
		if (_roomFlag & kRoomFlagStiletto)
			_vm->_screen->setStilPalette();
		_vm->_screen->setManPalette();
		_vm->_screen->setPalette();
	}
	_vm->_player->_frame = 0;
	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_events->clearEvents();
}

void NoctropolisRoom::clearRoom() {
	Room::clearRoom();

	_vm->_manScaleOff = 0;
	_vm->_stilScaleOff = 0;
}

void NoctropolisRoom::roomInit() {
	Room::roomInit();

	for (int i = 0; i < 8; i++)
		_vm->_flags[178 + i] = 0;
}

void NoctropolisRoom::buildScreenXScroll() {
	int drawCol = _vm->_scrollCol;
	int offset = -_vm->_scrollX;

	// Clear current background buffer
	_vm->_buffer1.clearBuffer();

	int w = MIN(_vm->_screen->_vWindowWidth + 1, _playFieldWidth);

	// Loop through drawing each column of tiles forming the background
	for (int idx = 0; idx < w; offset += TILE_WIDTH, ++idx) {
		buildColumnXScroll(drawCol, offset);
		++drawCol;
	}
}

void NoctropolisRoom::buildColumnXScroll(int playX, int screenX) {
	if (playX < 0 || playX >= _playFieldWidth)
		return;

	const uint16 *pSrc = _playField + _vm->_scrollRow *
		_playFieldWidth + playX;

	// WORKAROUND: Original's use of '+ 1' would frequently cause memory overruns
	int h = MIN(_vm->_screen->_vWindowHeight + 1, _playFieldHeight - _vm->_scrollRow);

	int colWidth = MIN(_vm->_screen->w - screenX, TILE_WIDTH);

	if (colWidth == 0)
		return;

	int xo = 0;
	if (screenX < 0) {
		xo = -screenX;
		screenX = 0;
	}

	if (colWidth <= xo)
		return;

	for (int y = 0; y < h; ++y) {
		uint16 tileNum = *pSrc;
		const byte *pTile = _tile + tileNum * TILE_WIDTH * TILE_HEIGHT;
		byte *pDest = (byte *)_vm->_buffer1.getBasePtr(screenX, y * TILE_HEIGHT);

		for (int tileY = 0; tileY < TILE_HEIGHT; ++tileY) {
			Common::copy(pTile + xo, pTile + colWidth, pDest);
			pTile += TILE_WIDTH;
			pDest += _vm->_buffer1.pitch;
		}

		pSrc += _playFieldWidth;
	}
}

void NoctropolisRoom::doCommands() {
	// aka NoctDoCommandLoop::ticker

	if (_vm->_events->_interfaceOff) {
		_vm->_events->setNormalCursor(CURSOR_DARK_ANKH);
	} else {
		if (_vm->_events->_rightButton) {
			_vm->_events->debounceRight();
			roomMenu();
		}

		// TODO: Check keyboard commands
		Common::Point pt = _vm->_events->calcRawMouse();
		int hotspotIndex = -1;

		_vm->_exitBox = false;

		if (_selectCommand != kNoctCmdOpen && _selectCommand != kNoctCmdMove &&
			_selectCommand != kNoctCmdGetTake && _selectCommand != kNoctCmdUse &&
			_selectCommand != kNoctCmdGoto) {
			if (!checkPlayerBox(pt)) {
				hotspotIndex = _plotter._blockIn;
				// Mouse is at the player or Stiletto, check what the script says
				if (hotspotIndex >= 0 && (validateBox(hotspotIndex) & 0x80))
					hotspotIndex = -1;
			}
		}

		if (_vm->_player->_roomNumber == 59 && _selectCommand == kNoctCmdGetTake && hotspotIndex == 3) {
			_vm->_scripts->_continuenceFlag = true;
			_vm->_scripts->_continuenceType = 3;
			warning("TODO: Implement room 59 hack from original?");
		}

		if (hotspotIndex < 0) {
			hotspotIndex = checkBoxes1(pt);
			while (hotspotIndex >= 0 && (validateBox(hotspotIndex) & 0x80))
				hotspotIndex = checkBoxes2(pt, hotspotIndex + 1, _plotter._blocks.size() - hotspotIndex);
		}

		if (hotspotIndex >= 0) {
			if (_vm->_exitBox)
				_vm->_events->setNormalCursor(CURSOR_NOCT_EXIT);
			else if (_selectCommand <= 6)
				_vm->_events->setNormalCursor((CursorType)(_selectCommand + 2));
			else
				_vm->_events->setNormalCursor(CURSOR_ARROW);

			if (_vm->_events->_leftButton) {
				_vm->_events->debounceLeft();
				// Double-clicking on something selects the goto command
				uint32 endTicks = g_system->getMillis() + _vm->_events->getDoubleClickTime();
				while (g_system->getMillis() < endTicks) {
					_vm->_events->pollEventsAndWait();
					if (_vm->_events->_leftButton) {
						_selectCommand = kNoctCmdGoto;
						break;
					}
				}
				// Also the script can force a goto command
				if (_vm->_exitBox)
					_selectCommand = kNoctCmdGoto;
				_vm->_events->debounceLeft();
				if (_selectCommand != -1) {
					_vm->_scripts->executeScript();
				}
			}
		} else {
			_vm->_exitBox = false;
			_vm->_events->setNormalCursor(CURSOR_ARROW);
		}

	}
}


int NoctropolisRoom::checkPlayerBox(const Common::Point &pt) {
	if (!_vm->_player->_playerOff) {
		_vm->_player->calcManScale();
		if (_vm->_player->_rawPlayer.x <= pt.x) {
			int scaleY = 200;
			if (_roomFlag & kRoomFlagTopView)
				scaleY = 6;

			int scaleX = 60;
			if (_roomFlag & kRoomFlagTopView)
				scaleX = 6;

			if (((pt.y <= _vm->_player->_rawPlayer.y) && ((_vm->_player->_rawPlayer.y - _vm->_screen->_scaleTable1[scaleY]) <= pt.y)) &&
				(pt.x <= (_vm->_player->_rawPlayer.x + _vm->_screen->_scaleTable1[scaleX]))) {
				_plotter._blockIn = 99;
				return 0;
			}
		}
	}

	// These flags are vm->_stilFlag1 and vm->_stilFlag2
	if (_vm->_flags[205] == 1 || (_roomFlag & kRoomFlagStiletto) == 0 || _vm->_flags[234] != 2)
		return 1;

	Player *stil = ((NoctropolisEngine *)_vm)->_stil;

	if (stil->_playerOff)
		return 1;

	_vm->_scale = _vm->_stilScaleOff;
	if (_vm->_stilScaleOff == 0) {
		_vm->_scale = (_vm->_scaleI *
						((((_vm->_scaleH2 << 8) +
						  _vm->_scaleT1 *
						  (_vm->_scaleN1 + (stil->_rawPlayer.y - _vm->_scaleMaxY))) & 0xff00)
						 / (uint)_vm->_scaleH1) >> 8);
	}
	_vm->_screen->setScaleTable(_vm->_scale);
	stil->_playerOffset.y = _vm->_screen->_scaleTable1[180];
	stil->_playerOffset.x = _vm->_screen->_scaleTable1[60];
	if (pt.x < stil->_rawPlayer.x || stil->_rawPlayer.y < pt.y)
		return 1;

	if ((stil->_rawPlayer.y - stil->_playerOffset.y) <= pt.y && (pt.x <= (stil->_playerOffset.x + stil->_rawPlayer.x))) {
		_plotter._blockIn = 98;
		return 0;
	}
	return 1;
}


void NoctropolisRoom::mainAreaLClick() {
	const Common::Point &mousePos = _vm->_events->_mousePos;
	const Common::Point pt = _vm->_events->calcRawMouse();
	const Screen &screen = *_vm->_screen;

	//if (_selectCommand == -1) {
	//	player._moveTo = pt;
	//	player._playerMove = true;
	/*} else {*/
	if (mousePos.x >= screen._windowXAdd &&
		mousePos.x <= (screen._windowXAdd + screen._vWindowBytesWide) &&
		mousePos.y >= screen._windowYAdd &&
		mousePos.y <= (screen._windowYAdd + screen._vWindowLinesTall)) {
			if (checkBoxes1(pt) >= 0) {
				checkBoxes3();
			}
	}
}

int NoctropolisRoom::validateBox(int boxId) {
	_vm->_scripts->_continuenceFlag = false;
	_vm->_scripts->_continuenceType = 0;

	int result = Room::validateBox(boxId);

	//debug("NoctRoom::validateBox(%d) -> %d", boxId, result);

	if (_vm->_player->_roomNumber == 54) {
		_vm->_scripts->_continuenceType = 1;
	}
	return result;
}


void NoctropolisRoom::roomMenu() {
	// Move the mouse to the centre of the menu
	// see NoctRoomMenu::setPositionFromMouse.
	Common::Point mousePt = _vm->_events->getMousePos();
	int baseX = mousePt.x - 127;
	int baseY = mousePt.y - 75;

	if (baseX < 0) {
		baseX = 0;
		mousePt.x = 127;
	} else if (baseX >= 640 - 255) {
		baseX = 640 - 255;
		mousePt.x = 640 - 127;
	}

	if (baseY < 0) {
		baseY = 0;
		mousePt.y = 75;
	} else if (baseY >= 400 - 151) {
		baseY = 400 - 151;
		mousePt.y = 400 - 75;
	}

	g_system->warpMouse(mousePt.x, mousePt.y);

	const SpriteResource *icons = _vm->getIcons();

	int16 width = icons->getFrame(8)->w + icons->getFrame(9)->w;
	int16 height = MAX(icons->getFrame(8)->h, icons->getFrame(9)->h);

	_vm->_screen->saveScreen();
	_vm->_screen->setDisplayScan();

	_vm->_screen->saveBlock(Common::Rect(Common::Point(baseX, baseY), width, height));

	_vm->_destIn = _vm->_screen;	// TODO: Redundant?
	_vm->_screen->plotImage(icons, 8, Common::Point(baseX, baseY));
	_vm->_screen->plotImage(icons, 9, Common::Point(baseX + 127, baseY));

	int cmdIndex = -1;
	while (cmdIndex == -1 && !_vm->shouldQuitOrRestart()) {
		if (_vm->_events->_rightButton) {
			cmdIndex = -2;
			_vm->_events->debounceRight();
		} else if (_vm->_events->_leftButton) {
			mousePt = _vm->_events->getMousePos();
			cmdIndex = ((NoctropolisResources *)_vm->_res)->menuAt(mousePt.x - baseX, mousePt.y - baseY);
			_vm->_events->debounceLeft();
		}
		_vm->_events->pollEventsAndWait();
	}

	// Restore screen background
	_vm->_screen->restoreBlock();
	_vm->_screen->restoreScreen();
	debug("menu cmdIndex = %d", cmdIndex);

	if (cmdIndex >= 0)
		handleCommand(cmdIndex);
}

} // end namespace Noctropolis

} // end namespace Access
