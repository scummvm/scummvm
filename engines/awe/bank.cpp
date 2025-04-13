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
#include "awe/awe.h"
#include "awe/bank.h"
#include "awe/resource.h"

namespace Awe {

Bank::Bank() {
}

bool Bank::read(const MemEntry *me, uint8 *buf) {
	bool ret = false;
	char bankName[10];
	Common::sprintf_s(bankName, "bank%02x", me->bankNum);
	Common::File f;

	if (f.open(bankName)) {
		f.seek(me->bankPos);
		if (me->packedSize == me->unpackedSize) {
			f.read(buf, me->packedSize);
			ret = true;
		} else {
			f.read(buf, me->packedSize);
			_startBuf = buf;
			_iBuf = buf + me->packedSize - 4;
			ret = unpack();
		}
	} else {
		error("Bank::read() unable to open '%s'", bankName);
	}

	return ret;
}

void Bank::decUnk1(uint8 numChunks, uint8 addCount) {
	uint16 count = getCode(numChunks) + addCount + 1;
	debugC(kDebugBank, "Bank::decUnk1(%d, %d) count=%d", numChunks, addCount, count);
	_unpCtx.datasize -= count;
	while (count--) {
		assert(_oBuf >= _iBuf && _oBuf >= _startBuf);
		*_oBuf = (uint8)getCode(8);
		--_oBuf;
	}
}

void Bank::decUnk2(uint8 numChunks) {
	uint16 i = getCode(numChunks);
	uint16 count = _unpCtx.size + 1;
	debugC(kDebugBank, "Bank::decUnk2(%d) i=%d count=%d", numChunks, i, count);
	_unpCtx.datasize -= count;
	while (count--) {
		assert(_oBuf >= _iBuf && _oBuf >= _startBuf);
		*_oBuf = *(_oBuf + i);
		--_oBuf;
	}
}

bool Bank::unpack() {
	_unpCtx.size = 0;
	_unpCtx.datasize = READ_BE_UINT32(_iBuf); _iBuf -= 4;
	_oBuf = _startBuf + _unpCtx.datasize - 1;
	_unpCtx.crc = READ_BE_UINT32(_iBuf); _iBuf -= 4;
	_unpCtx.chk = READ_BE_UINT32(_iBuf); _iBuf -= 4;
	_unpCtx.crc ^= _unpCtx.chk;
	do {
		if (!nextChunk()) {
			_unpCtx.size = 1;
			if (!nextChunk()) {
				decUnk1(3, 0);
			} else {
				decUnk2(8);
			}
		} else {
			uint16 c = getCode(2);
			if (c == 3) {
				decUnk1(8, 8);
			} else {
				if (c < 2) {
					_unpCtx.size = c + 2;
					decUnk2(c + 9);
				} else {
					_unpCtx.size = getCode(8);
					decUnk2(12);
				}
			}
		}
	} while (_unpCtx.datasize > 0);

	return (_unpCtx.crc == 0);
}

uint16 Bank::getCode(uint8 numChunks) {
	uint16 c = 0;
	while (numChunks--) {
		c <<= 1;
		if (nextChunk()) {
			c |= 1;
		}			
	}

	return c;
}

bool Bank::nextChunk() {
	bool CF = rcr(false);
	if (_unpCtx.chk == 0) {
		assert(_iBuf >= _startBuf);
		_unpCtx.chk = READ_BE_UINT32(_iBuf); _iBuf -= 4;
		_unpCtx.crc ^= _unpCtx.chk;
		CF = rcr(true);
	}

	return CF;
}

bool Bank::rcr(bool CF) {
	bool rCF = (_unpCtx.chk & 1);
	_unpCtx.chk >>= 1;
	if (CF) _unpCtx.chk |= 0x80000000;

	return rCF;
}

} // namespace Awe
