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

#include "common/endian.h"
#include "scumm/he/localizer.h"
#include "common/file.h"
#include "common/debug.h"

namespace Scumm {

Localizer::Localizer() {
	Common::File _file;

	if (!_file.open("lokalizator.big"))
		return;

	uint _fileSize = _file.size();
	if (_fileSize < 0x18)
		return;
	byte *lkBig = new byte[_fileSize];
	_file.read(lkBig, _fileSize);
	// Obfuscation round 1
	for (uint i = 0; i < _fileSize; i++)
		lkBig[i] ^= (214013 * i + 2531011) >> 16;

	uint32 numFiles = READ_LE_UINT32(lkBig + 0x14);
	uint32 localeMsgOffset = 0, localeMsgSize = 0;
	uint32 talkieDatOffset = 0, talkieDatSize = 0;

	for (uint i = 0; i < numFiles; i++) {
		byte *fileHdr = lkBig + 0x18 + 0x4c * i;
		if (strcmp((char *) fileHdr, "locale.msg") == 0) {
			localeMsgOffset = READ_LE_UINT32(fileHdr + 0x48);
			localeMsgSize = READ_LE_UINT32(fileHdr + 0x44);
			continue;
		}
		if (strcmp((char *) fileHdr, "talkie.dat") == 0) {
			talkieDatOffset = READ_LE_UINT32(fileHdr + 0x48);
			talkieDatSize = READ_LE_UINT32(fileHdr + 0x44);
			continue;
		}
	}

	_isValid = true;

	if (localeMsgSize > 4) {
		uint32 msgCount = READ_LE_UINT32(lkBig + localeMsgOffset);
		// Obfuscation round 2
		uint32 st = 0x12345678;
		for (uint i = 0; i < localeMsgSize - 4; i++) {
			byte x = 0;
			switch (i & 3) {
			case 0:
				x = st;
				break;
			case 1:
				x = st + 35;
				break;
			case 2:
				x = st + 70;
				break;
			case 3:
				x = st + 105;
				st += 45707404;
				break;
			}
			lkBig[i + localeMsgOffset + 4] ^= x;
		}

		uint32 cur = localeMsgOffset + 4;

		for (uint i = 0; i < msgCount && cur < localeMsgOffset + localeMsgSize; i++) {
			cur += 4; // Domain id or something like this, always zero
			uint32 lenOrig = READ_LE_UINT32(lkBig + cur); cur += 4;
			Common::String orig((char *) lkBig + cur, (char *) lkBig + cur + lenOrig);
			cur += lenOrig;
			uint32 lenTrans = READ_LE_UINT32(lkBig + cur); cur += 4;
			Common::String trans((char *) lkBig + cur, (char *) lkBig + cur + lenTrans);
			cur += lenTrans;
			_translationMap[orig] = trans;
		}
	}

	for (uint32 cur = talkieDatOffset; cur < talkieDatOffset + talkieDatSize; cur += 16) {
	    _talkMap[READ_LE_UINT32(lkBig+cur+4)] = READ_LE_UINT32(lkBig+cur+12);
	}
}

Common::String Localizer::translate(const Common::String &orig) {
	if (_translationMap.contains(orig)) {
		return _translationMap[orig];
	}
	return orig;
}

uint32 Localizer::mapTalk(uint32 orig) {
	if (_talkMap.contains(orig)) {
		return _talkMap[orig];
	}
	return orig;
}

}
