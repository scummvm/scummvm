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

#include "igor/igor.h"
#include "common/debug-channels.h"

namespace Igor {

void IgorEngine::copyArea(uint8 *dst, int dstOffset, int dstPitch, const uint8 *src, int srcPitch, int w, int h, bool transparent) {
	uint8 *p = dst + dstOffset;
	for (int y = 0; y < h; ++y) {
		if (transparent) {
			for (int x = 0; x < w; ++x) {
				if (src[x] != 0) {
					p[x] = src[x];
				}
			}
		} else {
			memcpy(p, src, w);
		}
		p += dstPitch;
		src += srcPitch;
	}
}

void IgorEngine::decodeRoomAreas(const uint8 *p, int count) {
	for (int i = 0; i < count; ++i) {
		RoomObjectArea *roa = &_roomObjectAreasTable[i];
		roa->area = *p++;
		assert((roa->area & 0x80) == 0);
		roa->object = *p++;
		roa->y1Lum = *p++;
		roa->y2Lum = *p++;
		roa->deltaLum = *p++;
	}
}

void IgorEngine::decodeRoomMask(const uint8 *p) {
	uint8 *dst = _screenLayer2;
	int sz = 320 * 144;
	while (sz != 0) {
		uint8 b = *p++;
		int len = READ_LE_UINT16(p);
		p += 2;
		if (len > sz) {
			len = sz;
		}
		memset(dst, b, len);
		dst += len;
		sz -= len;
	}
}

static void decodeRoomString(const uint8 *src, char *dst, int sz) {
	for (int i = 0; i < sz; ++i) {
		uint8 code = *src++;
		if ((code >= 0xAE && code <= 0xC7) || (code >= 0xCE && code <= 0xE7)) {
			code -= 0x6D;
		} else if (code > 0xE7) {
			switch (code) {
			case 0xE8:
				code = 0xA0;
				break;
			case 0xE9:
				code = 0x82;
				break;
			case 0xEA:
				code = 0xA1;
				break;
			case 0xEB:
				code = 0xA2;
				break;
			case 0xEC:
				code = 0xA3;
				break;
			case 0xED:
				code = 0xA4;
				break;
			case 0xEE:
				code = 0xA5;
				break;
			}
		}
		*dst++ = (char)code;
	}
}

void IgorEngine::decodeRoomStrings(const uint8 *p, bool skipObjectNames) {
	if (!skipObjectNames) {
		for (int i = 0; i < 20; ++i) {
			_roomObjectNames[i][0] = '\0';
		}
		uint8 code = *p++;
		int index = -1;
		while (code != 0xF6) {
			if (code == 0xF4) {
				++index;
			}
			int len = *p++;
			if (len != 0) {
				assert(index >= 0);
				decodeRoomString(p, _roomObjectNames[index], len);
				p += len;
				_roomObjectNames[index][len] = '\0';
				debugC(9, kDebugResource, "decodeRoomStrings() _roomObjectNames[%d] '%s'", index, _roomObjectNames[index]);
			}
			code = *p++;
		}
	}
	for (int i = 200; i < 250; ++i) {
		_globalDialogueTexts[i][0] = '\0';
	}
	uint8 code = *p++;
	int index = 200;
	while (code != 0xF6) {
		if (code == 0xF4) {
			++index;
		}
		int len = *p++;
		if (len != 0) {
			decodeRoomString(p, _globalDialogueTexts[index], len);
			p += len;
			_globalDialogueTexts[index][len] = '\0';
			debugC(9, kDebugResource, "decodeRoomStrings() _globalDialogueTexts[%d] '%s'", index, _globalDialogueTexts[index]);
		}
		code = *p++;
	}
}

void IgorEngine::decodeRoomText(const uint8 *p) {
	debugC(9, kDebugResource, "decodeRoomText()");
	memcpy(_walkXScaleRoom, p, 320);
	p += 320;
	memcpy(_walkYScaleRoom, p, 432);
	p += 432;
	decodeRoomStrings(p);
}

void IgorEngine::loadRoomData(int pal, int img, int box, int msk, int txt) {
	if (pal != 0) {
		loadData(pal, _paletteBuffer);
	}
	if (img != 0) {
		loadData(img, _screenLayer1);
	}
	if (box != 0) {
		int sz;
		uint8 *p = loadData(box, 0, &sz);
		memset(_roomObjectAreasTable, 0, sizeof(_roomObjectAreasTable));
		assert((sz % 5) == 0);
		decodeRoomAreas(p, sz / 5);
		free(p);
	}
	if (msk != 0) {
		uint8 *p = loadData(msk);
		decodeRoomMask(p);
		free(p);
	}
	if (txt != 0) {
		uint8 *p = loadData(txt);
		decodeRoomText(p);
		free(p);
	}
}

} // End of namespace Igor
