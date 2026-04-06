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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "file.h"

namespace Scooby {

File::File(ScoobyEngine *vm) : ReadStreamEndian(true), _vm(vm) {
	if (!_fd.open("scooby.md")) {
		error("Failed to open file scooby.md");
	}
}

File::~File() {
	_fd.close();
}

bool CARRY4(uint32 a, uint32 b) {
	return ((uint64)a + (uint64)b) > 0xffffffffL;
}

void printBuf(byte *buf, int size) {
	debugN("\n 0000: ");
	for (int i = 0; i < size; i++) {
		if ((i % 16) == 0) {
			debugN("\n %04x: ", i);
		}
		debugN("%02x ", buf[i]);
	}
}

uint32 File::decompressBytes(uint32 offset, byte *destBufArg, uint32 dataSize) {
	uint32 unkTbl[17] = {
			0,    0xFFFE0000,    0xFFFC0000,    0xFFF80000,
			0xFFF00000,    0xFFE00000,    0xFFC00000,    0xFF800000,
			0xFF000000,    0xFE000000,    0xFC000000,    0xF8000000,
			0xF0000000,    0xE0000000,    0xC0000000,    0x80000000,
			0
	};

	byte param_2[0x10000]; //TODO fix size;

	byte bVar1;
	uint32 uVar2;
	uint32 uVar3;
	short counter;
	short sVar4;
	uint16 uVar5;
	uint16 uVar6;
	uint32 *puVar7;
	uint32 *puVar8;
//	uint32 *srcBufPtr;
	byte *pbVar9;
	byte *destBufPtr;

	offsetFromStart(offset);

	counter = 0x20;
	uVar6 = 1;
	//srcBufPtr = srcBuf + 1;
	uVar3 = readUint32(); //READ_BE_UINT32(srcBuf);
	destBufPtr = destBufArg;
	while( true ) {
		while( true ) {
			counter = counter + -1;
//			puVar7 = srcBufPtr;
			if (counter < 0) {
				counter = 0x1f;
//				puVar7 = srcBufPtr + 1;
				uVar3 = readUint32(); //READ_BE_UINT32(srcBufPtr);
			}
			uVar2 = uVar3 + uVar3;
			if (!CARRY4(uVar3,uVar3)) break;
			sVar4 = counter + -0x10;
//			srcBufPtr = puVar7;
			if (counter < 0x10) {
				counter = counter + 0x10;
//				srcBufPtr = (uint32 *)((uint16 *)puVar7 + 1);
				debug("sVar4 = %d, unkTbl[abs(sVar4)] = 0x%X\n", sVar4, unkTbl[abs(sVar4)]);
				uVar2 = uVar2 & unkTbl[abs(sVar4)] /* *(uint32 *)((int)UINT_ARRAY_000097f4 + (int)(short)(sVar4 * -4))*/ | (uint32)readUint16()/*READ_BE_UINT16(puVar7)*/ << ((uint32)(uint16)-sVar4 & 0x3fu);
			}
			counter = counter + -8;
			uVar3 = uVar2 << 8 | uVar2 >> 0x18;
			bVar1 = (byte)(uVar2 >> 0x18);
			*destBufPtr = bVar1;
			param_2[(short)uVar6] = bVar1;
			uVar6 = uVar6 + 1 & 0xfff;
			destBufPtr = destBufPtr + 1;
		}
		sVar4 = counter + -0x10;
		// puVar8 = puVar7;
		if (counter < 0x10) {
			counter = counter + 0x10;
			// puVar8 = (uint32 *)((uint16 *)puVar7 + 1);
			uVar2 = uVar2 & unkTbl[abs(sVar4)] /* *(uint32 *)((int)UINT_ARRAY_000097f4 + (int)(short)(sVar4 * -4))*/ | (uint32)readUint16()/*READ_BE_UINT16(puVar7)*/ << ((uint32)(uint16)-sVar4 & 0x3fu);
		}
		sVar4 = counter + -0xc;
		uVar3 = uVar2 << 0xc | uVar2 >> 0x14;
		uVar5 = (uint16)uVar3 & 0xfffu;
		if (uVar2 >> 0x14u == 0) break;
//		srcBufPtr = puVar8;
		if (sVar4 < 0x10) {
			sVar4 = counter + 4;
//			srcBufPtr = (uint32 *)((uint16 *)puVar8 + 1);
			debug("counter - 0x1c = %d, unkTbl[abs(counter - 0x1c)] = %X\n", counter - 0x1c, unkTbl[abs(counter - 0x1c)]);
			uVar3 = uVar3 & unkTbl[abs(counter - 0x1c)] /* *(uint32 *)((int)decomp_unktbl + (int)(short)((counter + -0x1c) * -4))*/ | (uint32)readUint16()/*READ_BE_UINT16(puVar8)*/ << ((uint32)(uint16)-(counter + -0x1c) & 0x3f);
		}
		counter = sVar4 + -4;
		uVar3 = uVar3 << 4 | uVar3 >> 0x1c;
		sVar4 = ((uint16)uVar3 & 0xf) + 1 + uVar5;
		pbVar9 = destBufPtr;
		debug("write run length: %d value: %02X out src offset: %04x dest offset: %04X\n",
				sVar4 - (short)uVar5, param_2[(short)(uVar5 & 0xfff)], pos()/*srcBufPtr - srcBuf*/, destBufPtr - destBufArg);
		do {
			destBufPtr = pbVar9 + 1;
			*pbVar9 = param_2[(short)(uVar5 & 0xfff)];
			/* decompiler bug. this should be param_2[uVar7] = param_2[uVar6]; */
			param_2[(short)uVar6] = param_2[(short)uVar5];
			uVar6 = uVar6 + 1 & 0xfff;
			uVar5 = uVar5 + 1;
			pbVar9 = destBufPtr;
		} while ((short)uVar5 <= sVar4);
	}
	*param_2 = destBufPtr - destBufArg;
	debug("decomp size: %u", destBufPtr - destBufArg);
	debug("src size: %u", pos() - offset); //(byte *)srcBufPtr - (byte *)startSrcBuf);

//	printBuf(destBufArg, destBufPtr - destBufArg);
	return destBufPtr - destBufArg; //decomp size
}

bool File::eos() const {
	return _fd.eos();
}

uint32 File::read(void *dataPtr, uint32 dataSize) {
	return _fd.read(dataPtr, dataSize);
}

int64 File::pos() const {
	return _fd.pos();
}

int64 File::size() const {
	return _fd.size();
}

bool File::seek(int64 offset, int whence) {
	return _fd.seek(offset, whence);
}

bool File::offsetFromStart(int32 offset) {
	return seek(offset, SEEK_SET);
}

} // End of namespace Scooby
