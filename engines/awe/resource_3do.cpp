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

#include "common/file.h"

#include "awe/resource_3do.h"

namespace Awe {

static int decodeLzss(const uint8 *src, uint32 len, uint8 *dst) {
	uint32 rd = 0, wr = 0;
	while (rd < len) {
		const uint8 code = src[rd++];
		for (int j = 0; j < 8 && rd < len; ++j) {
			if (code & (1 << j)) {
				dst[wr++] = src[rd++];
			} else {
				const uint16 offset = 0xF000 | src[rd] | ((src[rd + 1] & 0xF) << 8);
				const int llen = (src[rd + 1] >> 4) + 3;
				rd += 2;
				for (int i = 0; i < llen; ++i) {
					dst[wr] = dst[wr + (int16)offset];
					++wr;
				}
			}
		}
	}
	return wr;
}

static void decodeCcb16(int ccbWidth, int ccbHeight, Common::File *f, uint32 dataSize, uint16 *dst) {
	for (int y = 0; y < ccbHeight; ++y) {
		const int scanlineSize = 4 * (f->readUint16BE() + 2);
		int scanlineLen = 2;
		int w = ccbWidth;
		while (w > 0) {
			uint8 code = f->readByte();
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
				memset(dst, 0, count * sizeof(uint16));
				dst += count;
				break;
			case 3:
				{
				const uint16 color = f->readUint16BE();
				for (int i = 0; i < count; ++i) {
					*dst++ = color;
				}
				scanlineLen += 2;
				}
				break;

			default:
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

static const uint8 _ccb_bppTable[8] = {
	0, 1, 2, 4, 6, 8, 16, 0
};

static uint16 *decodeShapeCcb(Common::File *f, int dataSize, int *w, int *h) {
	const uint32 flags = f->readUint32BE();
	f->seek(4, SEEK_CUR);
	const uint32 celData = f->readUint32BE();
	f->seek(40, SEEK_CUR);
	const uint32 pre0 = f->readUint32BE();
	const uint32 pre1 = f->readUint32BE();
	assert(celData == 0x30);
	assert(flags & (1 << 9));
	const int bpp = _ccb_bppTable[pre0 & 7];
	assert(bpp == 16);
	const uint32 width = (pre1 & 0x3FF) + 1;
	const uint32 height = ((pre0 >> 6) & 0x3FF) + 1;
	uint16 *buffer = (uint16 *)malloc(width * height * sizeof(uint16));
	if (buffer) {
		decodeCcb16(width, height, f, dataSize - 60, buffer);
		*w = width;
		*h = height;
	}
	return buffer;
}

bool Resource3do::readEntries() {
	return true;
}

uint8 *Resource3do::loadFile(int num, uint8 *dst, uint32 *size) {
	uint8 *in = dst;

	char path[MAXPATHLEN];
	snprintf(path, sizeof(path), "GameData/File%d", num);
	Common::File f;
	if (f.open(path)) {
		const int sz = f.size();
		if (!dst) {
			dst = (uint8 *)malloc(sz);
			if (!dst) {
				error("loadFile - Unable to allocate %d bytes", sz);
			}
		}
		*size = sz;
		f.read(dst, sz);
	} else {
		warning("Failed to load '%s'", path);
		return nullptr;
	}

	if (memcmp(dst, "\x00\xf4\x01\x00", 4) == 0) {
		static const int SZ = 64000 * 2;
		uint8 *tmp = (uint8 *)calloc(1, SZ);
		if (!tmp) {
			error("loadFile - Unable to allocate %d bytes", SZ);
		}
		const int decodedSize = decodeLzss(dst + 4, *size - 4, tmp);
		if (in != dst)
			free(dst);

		if (decodedSize != SZ) {
			warning("Unexpected LZSS decoded size %d", decodedSize);
			free(tmp);
			*size = 0;
			return nullptr;
		}
		*size = decodedSize;
		return tmp;
	}
	return dst;
}

uint16 *Resource3do::loadShape555(const char *name, int *w, int *h) {
	char path[MAXPATHLEN];
	snprintf(path, sizeof(path), "GameData/%s", name);
	Common::File f;
	if (f.open(path)) {
		const uint32 dataSize = f.size();
		return decodeShapeCcb(&f, dataSize, w, h);
	}

	return nullptr;
}

const char *Resource3do::getMusicName(int num, uint32 *offset) {
	*offset = 0;

	snprintf(_musicPath, sizeof(_musicPath), "GameData/song%d", num);
	return _musicPath;
}

const char *Resource3do::getCpak(const char *name, uint32 *offset) {
	*offset = 0;

	snprintf(_cpakPath, sizeof(_cpakPath), "GameData/%s", name);
	return _cpakPath;
}

} // namespace Awe
