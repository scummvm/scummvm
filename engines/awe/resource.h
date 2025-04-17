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

#ifndef AWE_RESOURCE_H
#define AWE_RESOURCE_H

#include "awe/intern.h"

namespace Awe {

struct MemEntry {
	uint8_t status;        // 0x0
	uint8_t type;          // 0x1, Resource::ResType
	uint8_t *bufPtr;       // 0x2
	uint8_t rankNum;       // 0x6
	uint8_t bankNum;       // 0x7
	uint32_t bankPos;      // 0x8
	uint32_t packedSize;   // 0xC
	uint32_t unpackedSize; // 0x12
};

struct AmigaMemEntry {
	uint8_t type;
	uint8_t bank;
	uint32_t offset;
	uint32_t packedSize;
	uint32_t unpackedSize;
};

struct DemoJoy {
	uint8_t keymask;
	uint8_t counter;

	uint8_t *bufPtr;
	int bufPos, bufSize;

	bool start() {
		if (bufSize > 0) {
			keymask = bufPtr[0];
			counter = bufPtr[1];
			bufPos = 2;
			return true;
		}
		return false;
	}

	uint8_t update() {
		if (bufPos >= 0 && bufPos < bufSize) {
			if (counter == 0) {
				keymask = bufPtr[bufPos++];
				counter = bufPtr[bufPos++];
			} else {
				--counter;
			}
			return keymask;
		}
		return 0;
	}
};

struct ResourceNth;
struct ResourceWin31;
struct Resource3do;
struct Video;

typedef void (*PreloadSoundProc)(void *userdata, int num, const uint8_t *data);

struct Resource {

	enum ResType {
		RT_SOUND = 0,
		RT_MUSIC = 1,
		RT_BITMAP = 2, // full screen 4bpp video buffer, size=200*320/2
		RT_PALETTE = 3, // palette (1024=vga + 1024=ega), size=2048
		RT_BYTECODE = 4,
		RT_SHAPE = 5,
		RT_BANK = 6, // common part shapes (bank2.mat)
	};

	enum DataType {
		DT_DOS,
		DT_AMIGA,
		DT_ATARI,
		DT_15TH_EDITION,
		DT_20TH_EDITION,
		DT_WIN31,
		DT_3DO,
		DT_ATARI_DEMO, // ST Action Issue44 Disk28
	};

	enum {
		MEM_BLOCK_SIZE = 1 * 1024 * 1024,
		ENTRIES_COUNT = 146,
		ENTRIES_COUNT_20TH = 178,
	};

	enum {
		STATUS_NULL,
		STATUS_LOADED,
		STATUS_TOLOAD,
	};

	static const AmigaMemEntry _memListAmigaFR[ENTRIES_COUNT];
	static const AmigaMemEntry _memListAmigaEN[ENTRIES_COUNT];
	static const AmigaMemEntry _memListAtariEN[ENTRIES_COUNT];
	static const uint8_t _memListParts[][4];

	Video *_vid;
	const char *_dataDir;
	MemEntry _memList[ENTRIES_COUNT_20TH];
	uint16_t _numMemList;
	uint16_t _currentPart, _nextPart;
	uint8_t *_memPtrStart, *_scriptBakPtr, *_scriptCurPtr, *_vidCurPtr;
	bool _useSegVideo2;
	uint8_t *_segVideoPal;
	uint8_t *_segCode;
	uint8_t *_segVideo1;
	uint8_t *_segVideo2;
	const char *_bankPrefix;
	bool _hasPasswordScreen;
	DataType _dataType;
	ResourceNth *_nth;
	ResourceWin31 *_win31;
	Resource3do *_3do;
	Language _lang;
	const AmigaMemEntry *_amigaMemList;
	DemoJoy _demo3Joy;

	Resource(Video *vid, const char *dataDir);
	~Resource();

	DataType getDataType() const {
		return _dataType;
	}
	void detectVersion();
	const char *getGameTitle(Language lang) const;
	bool readBank(const MemEntry *me, uint8_t *dstBuf);
	void readEntries();
	void readEntriesAmiga(const AmigaMemEntry *entries, int count);
	void dumpEntries();
	void load();
	void invalidateAll();
	void invalidateRes();
	void update(uint16_t num, PreloadSoundProc, void *);
	void loadBmp(int num);
	uint8_t *loadDat(int num);
	void loadFont();
	void loadHeads();
	uint8_t *loadWav(int num);
	const char *getString(int num);
	const char *getMusicPath(int num, char *buf, int bufSize, uint32_t *offset = 0);
	void setupPart(int part);
	void allocMemBlock();
	void freeMemBlock();
	void readDemo3Joy();
};

} // namespace Awe

#endif
