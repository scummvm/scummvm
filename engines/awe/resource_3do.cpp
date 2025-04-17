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

#include "awe/resource_3do.h"
#include "awe/util.h"

namespace Awe {

static const int ISO_BLOCK_SIZE = 2048;

struct OperaIsoEntry {
	char name[16];
	uint32_t offset;
	uint32_t size;
};

static int compareOperaIsoEntry(const void *a, const void *b) {
	const OperaIsoEntry *entry1 = (const OperaIsoEntry *)a;
	const OperaIsoEntry *entry2 = (const OperaIsoEntry *)b;
	return strcmp(entry1->name, entry2->name);
}

static int compareOperaIsoEntryName(const void *name, const void *p) {
	const OperaIsoEntry *entry = (const OperaIsoEntry *)p;
	return strcmp((const char *)name, entry->name);
}

struct OperaIso {
	File _f;
	OperaIsoEntry *_entries;
	int _entriesCount;

	OperaIso(const char *filePath)
		: _entries(0), _entriesCount(0) {
		_f.open(filePath);
	}
	~OperaIso() {
		free(_entries);
	}
	void readToc() {
		uint8_t buf[128];
		const int count = _f.read(buf, sizeof(buf));
		if (count != sizeof(buf)) {
			warning("Failed to read %d bytes", count);
			return;
		}
		if (buf[0] != 1 || memcmp(buf + 40, "CD-ROM", 6) != 0) {
			warning("Unexpected Opera ISO signature");
			return;
		}
		const int block = READ_BE_UINT32(buf + 100);
		readTocEntry(block);
		qsort(_entries, _entriesCount, sizeof(OperaIsoEntry), compareOperaIsoEntry);
	}
	void readTocEntry(int block) {
		uint32_t attr = 0;
		do {
			_f.seek(block * ISO_BLOCK_SIZE + 20, SEEK_SET);
			do {
				uint8_t buf[72];
				_f.read(buf, sizeof(buf));
				attr = READ_BE_UINT32(buf);
				const char *name = (const char *)buf + 32;
				const uint32_t count = READ_BE_UINT32(buf + 64);
				const uint32_t offset = READ_BE_UINT32(buf + 68);
				_f.seek(count * 4, SEEK_CUR);
				switch (attr & 255) {
				case 2:
					_entries = (OperaIsoEntry *)realloc(_entries, (_entriesCount + 1) * sizeof(OperaIsoEntry));
					if (_entries) {
						OperaIsoEntry *e = &_entries[_entriesCount];
						strncpy(e->name, name, sizeof(e->name) - 1);
						e->name[sizeof(e->name) - 1] = 0;
						e->offset = offset * ISO_BLOCK_SIZE;
						e->size = READ_BE_UINT32(buf + 16);
						++_entriesCount;
					}
					break;
				case 7:
					if (strcmp(name, "GameData") == 0) {
						readTocEntry(offset);
					}
					break;
				}
			} while (attr != 0 && attr < 256);
			++block;
		} while ((attr >> 24) == 0x40);
	}
	const OperaIsoEntry *find(const char *name) const {
		return (const OperaIsoEntry *)bsearch(name, _entries, _entriesCount, sizeof(OperaIsoEntry), compareOperaIsoEntryName);
	}
};

static int decodeLzss(const uint8_t *src, uint32_t len, uint8_t *dst) {
	uint32_t rd = 0, wr = 0;
	while (rd < len) {
		const uint8_t code = src[rd++];
		for (int j = 0; j < 8 && rd < len; ++j) {
			if (code & (1 << j)) {
				dst[wr++] = src[rd++];
			} else {
				const uint16_t offset = 0xF000 | src[rd] | ((src[rd + 1] & 0xF) << 8);
				const int llen = (src[rd + 1] >> 4) + 3;
				rd += 2;
				for (int i = 0; i < llen; ++i) {
					dst[wr] = dst[wr + (int16_t)offset];
					++wr;
				}
			}
		}
	}
	return wr;
}

static void decodeCcb16(int ccbWidth, int ccbHeight, File *f, uint32_t dataSize, uint16_t *dst) {
	for (int y = 0; y < ccbHeight; ++y) {
		const int scanlineSize = 4 * (f->readUint16BE() + 2);
		int scanlineLen = 2;
		int w = ccbWidth;
		while (w > 0) {
			uint8_t code = f->readByte();
			++scanlineLen;
			const int count = (code & 63) + 1;
			code >>= 6;
			if (code == 0) {
				break;
			}
			switch (code) {
			case 1:
				for (int i = 0; i < count; ++i) {
					*dst++ = f->readUint16BE();
				}
				scanlineLen += count * 2;
				break;
			case 2:
				memset(dst, 0, count * sizeof(uint16_t));
				dst += count;
				break;
			case 3:
			{
				const uint16_t color = f->readUint16BE();
				for (int i = 0; i < count; ++i) {
					*dst++ = color;
				}
				scanlineLen += 2;
			}
			break;
			}
			w -= count;
		}
		assert(w >= 0);
		if (w > 0) {
			dst += w;
		}
		const int align = scanlineSize - scanlineLen;
		if (align != 0) {
			f->seek(align, SEEK_CUR);
		}
	}
}

static const uint8_t _ccb_bppTable[8] = {
		0, 1, 2, 4, 6, 8, 16, 0
};

static uint16_t *decodeShapeCcb(File *f, int dataSize, int *w, int *h) {
	const uint32_t flags = f->readUint32BE();
	f->seek(4, SEEK_CUR);
	const uint32_t celData = f->readUint32BE();
	f->seek(40, SEEK_CUR);
	const uint32_t pre0 = f->readUint32BE();
	const uint32_t pre1 = f->readUint32BE();
	assert(celData == 0x30);
	assert(flags & (1 << 9));
	const int bpp = _ccb_bppTable[pre0 & 7];
	assert(bpp == 16);
	const uint32_t width = (pre1 & 0x3FF) + 1;
	const uint32_t height = ((pre0 >> 6) & 0x3FF) + 1;
	uint16_t *buffer = (uint16_t *)malloc(width * height * sizeof(uint16_t));
	if (buffer) {
		decodeCcb16(width, height, f, dataSize - 60, buffer);
		*w = width;
		*h = height;
	}
	return buffer;
}

Resource3do::Resource3do(const char *dataPath)
	: _dataPath(dataPath) {

	Common::FSNode f(dataPath);

	if (f.exists()) {
		_iso = new OperaIso(dataPath);
	} else {
		_iso = 0;
	}
}

Resource3do::~Resource3do() {
	delete _iso;
}

bool Resource3do::readEntries() {
	if (_iso) {
		_iso->readToc();
		return _iso->_entriesCount != 0;
	}
	return true;
}

uint8_t *Resource3do::loadFile(int num, uint8_t *dst, uint32_t *size) {
	uint8_t *in = dst;
	if (_iso) {
		char name[16];
		snprintf(name, sizeof(name), "File%d", num);
		const OperaIsoEntry *e = _iso->find(name);
		if (e) {
			if (!dst) {
				dst = (uint8_t *)malloc(e->size);
				if (!dst) {
					warning("Unable to allocate %d bytes", e->size);
					return 0;
				}
			}
			*size = e->size;
			_iso->_f.seek(e->offset);
			_iso->_f.read(dst, e->size);
		} else {
			warning("Failed to load '%s'", name);
			return 0;
		}
	} else {
		char path[MAXPATHLEN];
		snprintf(path, sizeof(path), "%s/GameData/File%d", _dataPath, num);
		File f;
		if (f.open(path)) {
			const int sz = f.size();
			if (!dst) {
				dst = (uint8_t *)malloc(sz);
				if (!dst) {
					warning("Unable to allocate %d bytes", sz);
					return 0;
				}
			}
			*size = sz;
			f.read(dst, sz);
		} else {
			warning("Failed to load '%s'", path);
			return 0;
		}
	}
	if (dst && memcmp(dst, "\x00\xf4\x01\x00", 4) == 0) {
		static const int SZ = 64000 * 2;
		uint8_t *tmp = (uint8_t *)calloc(1, SZ);
		if (!tmp) {
			warning("Unable to allocate %d bytes", SZ);
			if (in != dst) free(dst);
			return 0;
		}
		const int decodedSize = decodeLzss(dst + 4, *size - 4, tmp);
		if (in != dst) free(dst);
		if (decodedSize != SZ) {
			warning("Unexpected LZSS decoded size %d", decodedSize);
			return 0;
		}
		*size = decodedSize;
		return tmp;
	}
	return dst;
}

uint16_t *Resource3do::loadShape555(const char *name, int *w, int *h) {
	if (_iso) {
		const OperaIsoEntry *e = _iso->find(name);
		if (e) {
			_iso->_f.seek(e->offset);
			return decodeShapeCcb(&_iso->_f, e->size, w, h);
		}
	} else {
		char path[MAXPATHLEN];
		snprintf(path, sizeof(path), "%s/GameData/%s", _dataPath, name);
		File f;
		if (f.open(path)) {
			const uint32_t dataSize = f.size();
			return decodeShapeCcb(&f, dataSize, w, h);
		}
	}
	return 0;
}

const char *Resource3do::getMusicName(int num, uint32_t *offset) {
	*offset = 0;
	if (_iso) {
		char name[16];
		snprintf(name, sizeof(name), "song%d", num);
		const OperaIsoEntry *e = _iso->find(name);
		if (e) {
			*offset = e->offset;
		}
		return 0;
	}
	snprintf(_musicPath, sizeof(_musicPath), "GameData/song%d", num);
	return _musicPath;
}

const char *Resource3do::getCpak(const char *name, uint32_t *offset) {
	*offset = 0;
	if (_iso) {
		const OperaIsoEntry *e = _iso->find(name);
		if (e) {
			*offset = e->offset;
		}
		return 0;
	}
	snprintf(_cpakPath, sizeof(_cpakPath), "GameData/%s", name);
	return _cpakPath;
}

} // namespace Awe
