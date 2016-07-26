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

#include "common/memstream.h"
#include "access/access.h"
#include "access/char.h"
#include "access/amazon/amazon_resources.h"

namespace Access {

CharEntry::CharEntry(const byte *data, AccessEngine *vm) {
	Common::MemoryReadStream s(data, 999);

	_charFlag = s.readByte();
	if (vm->getGameID() != GType_Amazon || !vm->isCD()) {
		_screenFile.load(s);
		_estabIndex = s.readSint16LE();
	} else {
		_estabIndex = s.readSint16LE();
		_screenFile.load(s);
	}

	_paletteFile.load(s);
	_startColor = s.readUint16LE();
	if (vm->getGameID() == GType_MartianMemorandum) {
		int lastColor = s.readUint16LE();
		_numColors = lastColor - _startColor;
	} else
		_numColors = s.readUint16LE();

	// Load cells
	for (byte cell = s.readByte(); cell != 0xff; cell = s.readByte()) {
		CellIdent ci;
		ci._cell = cell;
		ci.load(s);

		_cells.push_back(ci);
	}

	_animFile.load(s);
	_scriptFile.load(s);

	for (int16 v = s.readSint16LE(); v != -1; v = s.readSint16LE()) {
		ExtraCell ec;
		ec._vid._fileNum = v;
		ec._vid._subfile = s.readSint16LE();
		ec._vidSound.load(s);

		_extraCells.push_back(ec);
	}
}

CharEntry::CharEntry() {
	_charFlag = 0;
	_estabIndex = 0;
	_startColor = _numColors = 0;
}

/*------------------------------------------------------------------------*/

CharManager::CharManager(AccessEngine *vm) : Manager(vm) {
	// Setup character list
	for (uint idx = 0; idx < _vm->_res->CHARTBL.size(); ++idx) {
		if (_vm->_res->CHARTBL[idx].size() == 0)
			_charTable.push_back(CharEntry());
		else
			_charTable.push_back(CharEntry(&_vm->_res->CHARTBL[idx][0], _vm));
	}

	_charFlag = 0;
}

void CharManager::loadChar(int charId) {
	CharEntry &ce = _charTable[charId];
	_charFlag = ce._charFlag;

	_vm->_establishFlag = false;
	if (ce._estabIndex != -1) {
		_vm->_establishFlag = true;
		if (!_vm->_establishTable[ce._estabIndex]) {
			_vm->_establishTable[ce._estabIndex] = true;
			_vm->establish(0, ce._estabIndex);
		}
	}

	if (_charFlag != 0 && _charFlag != 3) {
		if (!_vm->_establishFlag)
			_vm->_screen->fadeOut();

		_vm->_files->loadScreen(ce._screenFile._fileNum, ce._screenFile._subfile);
		_vm->_screen->setIconPalette();
		_vm->_screen->fadeIn();
	}

	_vm->_buffer1.blitFrom(*_vm->_screen);
	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_screen->setDisplayScan();

	if (_charFlag != 2 && _charFlag != 3) {
		charMenu();
	}

	_vm->_screen->_startColor = ce._startColor;
	_vm->_screen->_numColors = ce._numColors;
	if (ce._paletteFile._fileNum != -1) {
		_vm->_screen->loadPalette(ce._paletteFile._fileNum, ce._paletteFile._subfile);
	}
	_vm->_screen->setIconPalette();
	_vm->_screen->setPalette();

	_vm->loadCells(ce._cells);
	if (ce._animFile._fileNum != -1) {
		Resource *data = _vm->_files->loadFile(ce._animFile);
		_vm->_animation->loadAnimations(data);
		delete data;
	}

	// Load script data
	_vm->_scripts->freeScriptData();
	if (ce._scriptFile._fileNum != -1) {
		Resource *data = _vm->_files->loadFile(ce._scriptFile);
		_vm->_scripts->setScript(data);
	}

	// Load extra cells
	_vm->_extraCells.clear();
	for (uint i = 0; i < ce._extraCells.size(); ++i)
		_vm->_extraCells.push_back(ce._extraCells[i]);
}

void CharManager::charMenu() {
	Resource *iconData = _vm->_files->loadFile("ICONS.LZ");
	SpriteResource *spr = new SpriteResource(_vm, iconData);
	delete iconData;

	Screen &screen = *_vm->_screen;
	screen.saveScreen();
	screen.setDisplayScan();

	if (_vm->getGameID() == GType_MartianMemorandum) {
		screen.plotImage(spr, 17, Common::Point(0, 184));
		screen.plotImage(spr, 18, Common::Point(193, 184));
	} else if (_vm->getGameID() == GType_Amazon) {
		screen.plotImage(spr, 17, Common::Point(0, 176));
		screen.plotImage(spr, 18, Common::Point(155, 176));
	} else
		error("Game not supported");

	// Make a backup copy of the screen including the character buttons,
	// for restoring when erasing conversation boxes
	screen.copyTo(&_vm->_buffer1);

	screen.restoreScreen();
	delete spr;
}

} // End of namespace Access
