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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
// Heavily based on code by X

#include "private/decompiler.h"

namespace Private {

const char *kHeader = "Precompiled Game Matrix";
const uint kHeaderSize = 23;

Decompiler::Decompiler(char *buf, uint32 fileSize, bool mac) {

	Common::Array<byte> array;
	uint32 i = 0;
	while (i < fileSize) {
		array.push_back(buf[i]);
		i++;
	}

	Common::String firstBytes((const char *)array.begin(), (const char *)array.begin() + kHeaderSize);

	if (firstBytes != kHeader) {
		debug("Not a precompiled game matrix");
		_result = Common::String(buf);
		return;
	}

	decompile(array, mac);
}

void Decompiler::decompile(Common::Array<byte> &buffer, bool mac) {
	Common::Array<byte>::iterator it = buffer.begin();

	Common::String ss;
	bool inDefineRects = false;
	for (it += kHeaderSize; it != buffer.end();) {
		byte b = *it++;
		if (b == kCodeString) {
			byte len = *it++;
			Common::String s((const char *)it, (const char *)it + len);
			it += len;
			ss += Common::String::format("\"%s\"", s.c_str());
		} else if (b == kCodeShortLiteral || b == kCodeShortId) {
			byte b1 = *it++;
			byte b2 = *it++;
			uint number = mac ? b2 + (b1 << 8) : b1 + (b2 << 8);
			if (b == kCodeShortId)
				ss += "k";
			ss += Common::String::format("%d", number);
		} else if (b == kCodeRect && inDefineRects) {
			ss += "RECT"; // override CRect
		} else if (b <= kCodeShortId && strlen(kCodeTable[b]) > 0) {
			ss += kCodeTable[b];
		} else {
			error("decompile(): Unknown byte code (%d %c)", b, b);
		}

		if (b == kCodeRects) {
			inDefineRects = true;
		} else if (b == kCodeBraceClose && inDefineRects) {
			inDefineRects = false;
		}
	}
	_result = ss;
}

Common::String Decompiler::getResult() const {
	return _result;
}

} // namespace Private
