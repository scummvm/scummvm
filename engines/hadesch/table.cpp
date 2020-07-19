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
#include "common/file.h"
#include "hadesch/table.h"

namespace Hadesch {

TableLine::TableLine(Common::SharedPtr<Common::SeekableReadStream> stream, int numcols) {
	Common::String line = stream->readLine();
	Common::Array<size_t> quotes;
	size_t cur = 0;
	_valid = false;
	if (line.find("//") < line.find('"')) {
		return;
	}
	for (int i = 0; i < 2 * numcols; i++) {
		size_t next = line.find('"', cur);
		if (next == Common::String::npos) {
			return;
		}
		cur = next + 1;
		quotes.push_back(next);
	}
	if (line.substr(quotes[0] + 1, quotes[1] - quotes[0] - 1) == "sentinel") {
		return;
	}
	_valid = true;
	for (int i = 0; i < numcols; i++) {
		_cells.push_back(line.substr(quotes[2 * i] + 1, quotes[2 * i + 1] - quotes[2 * i] - 1));
	}
}

TableLine::TableLine() {
	_valid = false;
}

bool TableLine::isValid() const {
	return _valid;
}

Common::String TableLine::operator[](int idx) const {
	return _cells[idx];
}

TextTable::TextTable() {
}

TextTable::TextTable(
	Common::SharedPtr<Common::SeekableReadStream> stream, int numcols) {
	while (!stream->eos() && !stream->err()) {
		TableLine tl(stream, numcols);
		if (!tl.isValid())
			continue;
		if (!_header.isValid()) {
			_header = tl;
			for (int i = 0; i < numcols; i++) {
				_colMap[tl[i]] = i;
			}
			continue;
		}
		_lines.push_back(tl);
		_rowMap[tl[0]].push_back(_lines.size() - 1);
	}
}

Common::String TextTable::get(const Common::String &row, int col, int rowidx) const {
	if (!_rowMap.contains(row))
		return "";
	return _lines[_rowMap[row][rowidx]][col];
}

int TextTable::rowCount(const Common::String &row) const {
	if (!_rowMap.contains(row))
		return 0;
	return _rowMap[row].size();
}

Common::String TextTable::get(const Common::String &row, const Common::String &col, int rowidx) const {
	if (!_colMap.contains(col))
		return "";
	return _lines[_rowMap[row][rowidx]][_colMap[col]];
}

Common::String TextTable::get(int row, const Common::String &col) const {
	if (!_colMap.contains(col))
		return "";
	return _lines[row][_colMap[col]];
}

int TextTable::size() const {
	return _lines.size();
}

}

