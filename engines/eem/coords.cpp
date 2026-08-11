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

#include "eem/coords.h"
#include "eem/eem.h"

namespace EEM {

uint16 readScriptU16(const byte *p, bool bigEndian) {
	return bigEndian ? READ_BE_UINT16(p) : READ_LE_UINT16(p);
}

int16 readScriptS16(const byte *p, bool bigEndian) {
	return (int16)readScriptU16(p, bigEndian);
}

Common::Rect readRectXYXY(const byte *p, bool bigEndian) {
	const int16 x1 = readScriptS16(p, bigEndian);
	const int16 y1 = readScriptS16(p + 2, bigEndian);
	const int16 x2 = readScriptS16(p + 4, bigEndian);
	const int16 y2 = readScriptS16(p + 6, bigEndian);

	return Common::Rect(x1, y1, x2, y2);
}

Common::Rect readDosRectLE(const byte *p) {
	return readRectXYXY(p, false);
}

Common::Rect readMacQuickDrawRectBE(const byte *p) {
	const int16 top = readScriptS16(p, true);
	const int16 left = readScriptS16(p + 2, true);
	const int16 bottom = readScriptS16(p + 4, true);
	const int16 right = readScriptS16(p + 6, true);

	return Common::Rect(left, top, right, bottom);
}

Common::Rect readMacQuickDrawRectLE(const byte *p) {
	const int16 top = readScriptS16(p, false);
	const int16 left = readScriptS16(p + 2, false);
	const int16 bottom = readScriptS16(p + 4, false);
	const int16 right = readScriptS16(p + 6, false);

	return Common::Rect(left, top, right, bottom);
}

Common::Rect scaleDosRectIfMac(const EEMEngine &vm, const Common::Rect &rect) {
	return vm.scaleRect(rect);
}

} // End of namespace EEM
