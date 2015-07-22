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

#ifndef ACCESS_CHAR_H
#define ACCESS_CHAR_H

#include "common/scummsys.h"
#include "common/array.h"
#include "access/data.h"

namespace Access {

class CharEntry {
public:
	int _charFlag;
	int _estabIndex;
	FileIdent _screenFile;
	FileIdent _paletteFile;
	int _startColor, _numColors;
	Common::Array<CellIdent> _cells;
	FileIdent _animFile;
	FileIdent _scriptFile;
	Common::Array<ExtraCell> _extraCells;
public:
	CharEntry(const byte *data, AccessEngine *vm);

	CharEntry();
};

class CharManager : public Manager {
private:
	void charMenu();
public:
	Common::Array<CharEntry> _charTable;
	int _charFlag;

public:
	CharManager(AccessEngine *vm);

	void loadChar(int charId);
};

} // End of namespace Access

#endif /* ACCESS_CHAR_H */
