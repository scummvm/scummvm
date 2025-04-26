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

#include "awe/detection.h"
#include "awe/intern.h"

namespace Awe {

struct MemEntry {
	uint8 status = 0;        // 0x0
	uint8 type = 0;          // 0x1, Resource::ResType
	uint8 *bufPtr = nullptr; // 0x2
	uint8 rankNum = 0;       // 0x6
	uint8 bankNum = 0;       // 0x7
	uint32 bankPos = 0;      // 0x8
	uint32 packedSize = 0;   // 0xC
	uint32 unpackedSize = 0; // 0x12

	void load(Common::SeekableReadStream *src);
};

struct AmigaMemEntry {
	uint8 type;
	uint8 bank;
	uint32 offset;
	uint32 packedSize;
	uint32 unpackedSize;
};

struct DemoJoy {
	uint8 keymask = 0;
	uint8 counter = 0;

	uint8 *bufPtr = nullptr;
	int bufPos = 0, bufSize = 0;

	bool start() {
		if (bufSize > 0) {
			keymask = bufPtr[0];
			counter = bufPtr[1];
			bufPos = 2;
			return true;
		}
		return false;
	}

	uint8 update() {
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

typedef void (*PreloadSoundProc)(void *userdata, int num, const uint8 *data);

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

	static const AmigaMemEntry MEMLIST_AMIGA_FR[ENTRIES_COUNT];
	static const AmigaMemEntry MEMLIST_AMIGA_EN[ENTRIES_COUNT];
	static const AmigaMemEntry MEMLIST_ATARI_EN[ENTRIES_COUNT];
	static const uint8 MEMLIST_PARTS[][4];
	static const AmigaMemEntry *detectAmigaAtari();

	Video *_vid;
	DataType _dataType;
	MemEntry _memList[ENTRIES_COUNT_20TH];
	uint16 _numMemList = 0;
	uint16 _currentPart = 0, _nextPart = 0;
	uint8 *_memPtrStart = nullptr,
		*_scriptBakPtr = nullptr,
		*_scriptCurPtr = nullptr,
		*_vidCurPtr = nullptr;
	bool _useSegVideo2 = false;
	uint8 *_segVideoPal = nullptr;
	uint8 *_segCode = nullptr;
	uint8 *_segVideo1 = nullptr;
	uint8 *_segVideo2 = nullptr;
	const char *_bankPrefix = "bank";
	bool _hasPasswordScreen = true;
	ResourceNth *_nth = nullptr;
	ResourceWin31 *_win31 = nullptr;
	Resource3do *_3do = nullptr;
	Language _lang = Language::EN_ANY;
	const AmigaMemEntry *_amigaMemList;
	DemoJoy _demo3Joy;
	bool _copyProtection = false;

	Resource(Video *vid, DataType dataType);
	~Resource();

	DataType getDataType() const {
		return _dataType;
	}
	const char *getGameTitle(Language lang) const;
	bool readBank(const MemEntry *me, uint8 *dstBuf);
	void readEntries();
	void readEntriesAmiga(const AmigaMemEntry *entries, int count);
	void dumpEntries();
	void load();
	void invalidateAll();
	void invalidateRes();
	void update(uint16 num, PreloadSoundProc, void *);
	void loadBmp(int num);
	uint8 *loadDat(int num);
	void loadFont();
	void loadHeads();
	uint8 *loadWav(int num, uint32 *size = nullptr);
	const char *getString(int num);
	const char *getMusicPath(int num, char *buf, int bufSize, uint32 *offset = nullptr);
	void setupPart(int part);
	void allocMemBlock();
	void freeMemBlock();
	void readDemo3Joy();
};

} // namespace Awe

#endif
