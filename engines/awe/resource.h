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
	uint8 valid;         // 0x0
	uint8 type;          // 0x1, Resource::ResType
	uint8 *bufPtr;       // 0x2
	uint16 unk4;         // 0x4, unused ?
	uint8 rankNum;       // 0x6
	uint8 bankNum;       // 0x7
	uint32 bankPos;      // 0x8 0xA
	uint16 unkC;         // 0xC, unused ?
	uint16 packedSize;   // 0xE
	uint16 unk10;        // 0x10, unused ?
	uint16 unpackedSize; // 0x12
};

struct Serializer;
struct Video;

struct Resource {
	enum ResType {
		RT_SOUND  = 0,
		RT_MUSIC  = 1,
		RT_VIDBUF = 2, // full screen video buffer, size=0x7D00
		RT_PAL    = 3, // palette (1024=vga + 1024=ega), size=2048
		RT_SCRIPT = 4,
		RT_VBMP   = 5
	};
	
	enum {
		MEM_BLOCK_SIZE = 600 * 1024
	};
	
	static const uint16 _memListAudio[];
	static const uint16 _memListParts[][4];
	
	Video *_vid;
	const char *_dataDir;
	MemEntry _memList[150];
	uint16 _numMemList;
	uint16 _curPtrsId, _newPtrsId;
	uint8 *_memPtrStart, *_scriptBakPtr, *_scriptCurPtr, *_vidBakPtr, *_vidCurPtr;
	bool _useSegVideo2;
	uint8 *_segVideoPal;
	uint8 *_segCode;
	uint8 *_segVideo1;
	uint8 *_segVideo2;

	Resource(Video *vid, const char *dataDir);
	
	void readBank(const MemEntry *me, uint8 *dstBuf);
	void readEntries();
	void load();
	void invalidateAll();
	void invalidateRes();	
	void update(uint16 num);
	void setupPtrs(uint16 ptrId);
	void allocMemBlock();
	void freeMemBlock();
	
	void saveOrLoad(Serializer &ser);
};

} // namespace Awe

#endif
