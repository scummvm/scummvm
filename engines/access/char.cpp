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

#include "common/memstream.h"
#include "access/access.h"
#include "access/char.h"
#include "access/amazon/amazon_resources.h"

namespace Access {

CharEntry::CharEntry(const byte *data) {
	Common::MemoryReadStream s(data, 999);

	_charFlag = s.readByte();
	_estabFlag = s.readSint16LE();
	_screenFile._fileNum = s.readSint16LE();
	_screenFile._subfile = s.readSint16LE();

	_paletteFile._fileNum = s.readSint16LE();
	_paletteFile._subfile = s.readUint16LE();
	_startColor = s.readUint16LE();
	_numColors = s.readUint16LE();

	// Load cells
	for (byte cell = s.readByte(); cell != 0xff; cell = s.readByte()) {
		CellIdent ci;
		ci._cell = cell;
		ci._fileNum = s.readSint16LE();
		ci._subfile = s.readUint16LE();

		_cells.push_back(ci);
	}

	_animFile._fileNum = s.readSint16LE();
	_animFile._subfile = s.readUint16LE();
	_scriptFile._fileNum = s.readSint16LE();
	_scriptFile._subfile = s.readUint16LE();

	for (int16 v = s.readSint16LE(); v != -1; v = s.readSint16LE()) {
		ExtraCell ec;
		ec._vidTable = v;
		ec._vidTable1 = s.readSint16LE();
		ec._vidSTable = s.readSint16LE();
		ec._vidSTable1 = s.readSint16LE();

		_extraCells.push_back(ec);
	}
}

CharEntry::CharEntry() {
	_charFlag = 0;
	_estabFlag = 0;
	_startColor = _numColors = 0;
}

/*------------------------------------------------------------------------*/

CharManager::CharManager(AccessEngine *vm) : Manager(vm) {
	switch (vm->getGameID()) {
	case GType_Amazon:
		// Setup character list
		for (int i = 0; i < 37; ++i)
			_charTable.push_back(CharEntry(Amazon::CHARTBL[i]));
		break;
	default:
		error("Unknown game");
	}
}

} // End of namespace Access
