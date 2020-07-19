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
 * Copyright 2020 Google
 *
 */
#ifndef HADESCH_TABLE_H
#define HADESCH_TABLE_H

#include "common/array.h"
#include "common/stream.h"
#include "common/str.h"
#include "common/hash-str.h"

namespace Hadesch {
class TableLine {
public:
	TableLine(Common::SharedPtr<Common::SeekableReadStream> stream, int numcols);
	TableLine();
	bool isValid() const;
	Common::String operator[](int idx) const;
private:
	bool _valid;
	Common::Array <Common::String> _cells;
};

class TextTable {
public:
	TextTable(Common::SharedPtr<Common::SeekableReadStream> stream, int numcols);
	TextTable();
	Common::String get(int row, int col) const;
	Common::String get(const Common::String &row, int col, int rowidx = 0) const;
	Common::String get(const Common::String &row, const Common::String &col, int rowidx = 0) const;
	Common::String get(int row, const Common::String &col) const;
	int size() const;
	int rowCount(const Common::String &row) const;
private:
	TableLine _header;
	Common::HashMap<Common::String, int> _colMap;
	Common::HashMap<Common::String, Common::Array<int> > _rowMap;
	Common::Array<TableLine> _lines;
};
}
#endif
