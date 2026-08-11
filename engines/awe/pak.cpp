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

#include "awe/pak.h"
#include "awe/detection.h"

namespace Awe {

// static const uint32 XOR_KEY1 = 0x31111612;
// static const uint32 CHECKSUM = 0x20202020;

static const uint32 XOR_KEY2 = 0x22683297;

static uint8 *decode_toodc(uint8 *p, int count) {
	uint32 key = XOR_KEY2;
	uint32 acc = 0;
	for (int i = 0; i < count; ++i) {
		uint8 *q = p + i * 4;
		const uint32 data = READ_LE_UINT32(q) ^ key;
		uint32 r = (q[2] + q[1] + q[0]) ^ q[3];
		r += acc;
		key += r;
		acc += 0x4D;
		WRITE_LE_UINT32(q, data);
	}
	return p + 4;
}

const char *Pak::FILENAME = "Pak01.pak";

Pak::Pak()
	: _entries(nullptr), _entriesCount(0) {
}

Pak::~Pak() {
	close();
}

void Pak::open(const char *dataPath) {
	_f.open(Common::Path(FILENAME));
}

void Pak::close() {
	free(_entries);
	_entries = nullptr;
	_entriesCount = 0;
}

static int comparePakEntry(const void *a, const void *b) {
	return scumm_stricmp(((const PakEntry *)a)->name, ((const PakEntry *)b)->name);
}

void Pak::readEntries() {
	uint8 header[12];

	memset(header, 0, sizeof(header));
	_f.read(header, sizeof(header));
	if (_f.err() || memcmp(header, "PACK", 4) != 0) {
		return;
	}
	const uint32 entriesOffset = READ_LE_UINT32(header + 4);
	_f.seek(entriesOffset);
	const uint32 entriesSize = READ_LE_UINT32(header + 8);
	_entriesCount = entriesSize / 0x40;
	debugC(kDebugPak, "Pak::readEntries() entries count %d", _entriesCount);
	_entries = (PakEntry *)calloc(_entriesCount, sizeof(PakEntry));
	if (!_entries) {
		_entriesCount = 0;
		return;
	}
	for (int i = 0; i < _entriesCount; ++i) {
		uint8 buf[0x40];
		_f.read(buf, sizeof(buf));
		if (_f.err()) {
			break;
		}
		const char *name = (const char *)buf;
		if (strncmp(name, "dlx/", 4) != 0) {
			continue;
		}
		PakEntry *e = &_entries[i];
		Common::strcpy_s(e->name, name + 4);
		e->offset = READ_LE_UINT32(buf + 0x38);
		e->size = READ_LE_UINT32(buf + 0x3C);
		debugC(kDebugPak, "Pak::readEntries() buf '%s' size %d", e->name, e->size);
	}
	qsort(_entries, _entriesCount, sizeof(PakEntry), comparePakEntry);

#if 0
	// the original executable descrambles the (ke)y.txt file and check the last 4 bytes.
	// this has been disabled in later re-releases and a key is bundled in the data files
	uint8 buf[128];
	const PakEntry *e = find("check.txt");
	if (e && e->size <= sizeof(buf)) {
		uint32 size = 0;
		loadData(e, buf, &size);
		assert(size >= 4);
		const uint32 num = READ_LE_UINT32(buf + size - 4);
		assert(num == CHECKSUM);
	}
#endif
}

const PakEntry *Pak::find(const char *name) {
	debugC(kDebugPak, "Pak::find() '%s'", name);
	PakEntry tmp;
	Common::strcpy_s(tmp.name, name);
	return (const PakEntry *)bsearch(&tmp, _entries, _entriesCount, sizeof(PakEntry), comparePakEntry);
}

void Pak::loadData(const PakEntry *e, uint8 *buf, uint32 *size) {
	debugC(kDebugPak, "Pak::loadData() %d bytes from 0x%x", e->size, e->offset);
	_f.seek(e->offset);
	if (_f.err()) {
		*size = 0;
		return;
	}
	_f.read(buf, e->size);
	if (e->size > 5 && memcmp(buf, "TooDC", 5) == 0) {
		const int dataSize = e->size - 6;
		debugC(kDebugPak, "Pak::loadData() encoded TooDC data, size %d", dataSize);
		if ((dataSize & 3) != 0) {
			// descrambler operates on uint32
			warning("Unexpected size %d for encoded TooDC data '%s'", dataSize, e->name);
		}
		*size = dataSize - 4;
		decode_toodc(buf + 6, (dataSize + 3) / 4);
		memmove(buf, buf + 10, dataSize - 4);
	} else {
		*size = e->size;
	}
}

} // namespace Awe
