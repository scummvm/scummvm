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

#include "private/decompiler.h"

namespace Private {

Decompiler::Decompiler(char *buf, uint32 fileSize, bool mac) {

    Common::Array<unsigned char> array;
	uint32 i = 0;
	while (i < fileSize) {
		array.push_back(buf[i]);
		i++;
	}

	Common::String firstBytes((const char *) array.begin(), (const char *) array.begin() + kHeader.size());
 
	if (firstBytes != kHeader) {
		debug("Not a precompiled game matrix");
		_result = Common::String(buf);
		return;
	}

	decompile(array, mac);
}

void Decompiler::decompile(Common::Array<unsigned char> &buffer, bool mac) {
	Common::Array<unsigned char>::iterator it = buffer.begin();
    
	Common::String ss;
	bool inDefineRects = false;
	for (it += kHeader.size() ; it != buffer.end() ; ) {
		unsigned char byte = *it++;
		if (byte == kCodeString) {
			unsigned char len = *it++;
			Common::String s((const char *)it,(const char *)it+len);
			it += len;
			ss += Common::String::format("\"%s\"",  s.c_str());
		} else if (byte == kCodeShortLiteral || byte == kCodeShortId) {
			unsigned char b1 = *it++;
			unsigned char b2 = *it++;
			unsigned int number = mac ? b2 + (b1 << 8) : b1 + (b2 << 8);
			if (byte == kCodeShortId) ss += "k";
			ss += Common::String::format("%d",  number);
		} else if (byte == kCodeRect && inDefineRects) {
			ss += "RECT"; // override CRect
		} else if (byte <= kCodeShortId && strlen(kCodeTable[byte]) > 0) {
			ss += kCodeTable[byte];
		} else {
			error("Unknown byte code");
		}

		if (byte == kCodeRects) {
			inDefineRects = true;
		} else if (byte == kCodeBraceClose && inDefineRects) {
			inDefineRects = false;
		}
	}
	_result = ss;
}

Common::String Decompiler::getResult() const {
	return _result;
}

}