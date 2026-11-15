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

#include "tesoro/core/speechtable.h"
#include "tesoro/core/archive.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/stdiotext.h"

namespace Tesoro {

static bool keyLess(uint64 a, uint64 b) {
	if (uint32(a) != uint32(b))
		return uint32(a) < uint32(b);
	return uint32(a >> 32) < uint32(b >> 32);
}

void SpeechTable::load(const byte *text, uint32 size, const Archive &archive) {
	_lines.clear();

	// The first pass only counts the lines; it stops once it has read into the last eight bytes.
	StdioText in(text, size);
	Common::Array<char> idBuf(size + 1), lineBuf(size + 1);
	char *id = idBuf.begin();
	char *line = lineBuf.begin();
	id[0] = line[0] = '\0';
	uint32 count = 0;
	while (size >= 8 && in.consumed() <= size - 8) {
		in.scan(" %[^,]s", id);
		in.scan(", \"%[^\"]s", line);
		++count;
		in.get();
		in.get();
	}

	in.seek(0);
	for (uint32 i = 0; i < count; ++i) {
		Line l;
		in.scan(" %[^,]s", id);
		line[0] = '\0';
		in.scan(", \"%[^\"]s", line);
		l.key = nameKey64(id);
		l.text.resize(strlen(line) + 1);
		memcpy(l.text.begin(), line, l.text.size());
		char wav[96];
		snprintf(wav, sizeof(wav), "%s.WAV", id);
		// The speech table already gives the full file name.
		Common::String file(wav);
		file.toUppercase();
		l.wavOffset = archive.findEntry(file.c_str());
		_lines.push_back(l);
		in.get();
		in.get();
	}

	// Selection sort, largest key first;
	for (uint32 i = 0; i + 1 < _lines.size(); ++i) {
		uint32 best = i;
		for (uint32 j = i + 1; j < _lines.size(); ++j) {
			if (keyLess(_lines[best].key, _lines[j].key))
				best = j;
		}
		SWAP(_lines[i], _lines[best]);
	}
}

int SpeechTable::find(uint64 key) const {
	int lo = 0, hi = (int)_lines.size() - 1;
	while (lo <= hi) {
		const int mid = (lo + hi) >> 1;
		if (_lines[mid].key == key)
			return mid;
		if (keyLess(key, _lines[mid].key))
			lo = mid + 1;
		else
			hi = mid - 1;
	}
	return -1;
}

uint32 SpeechTables::find(const char *name) const {
	const uint64 key = nameKey64(name);
	for (uint t = 0; t < kCount; ++t) {
		const int i = _tables[t].find(key);
		if (i >= 0)
			return ((t << 24) | i) + 1;
	}
	return 0;
}

} // End of namespace Tesoro
