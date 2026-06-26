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

#ifndef EEM_COORDS_H
#define EEM_COORDS_H

#include "common/rect.h"
#include "common/scummsys.h"

namespace EEM {

class EEMEngine;

uint16 readScriptU16(const byte *p, bool bigEndian);
int16 readScriptS16(const byte *p, bool bigEndian);

/// x1, y1, x2, y2, with the requested word endian.
Common::Rect readRectXYXY(const byte *p, bool bigEndian);

/// DOS script/site rectangle: x1, y1, x2, y2, little-endian.
Common::Rect readDosRectLE(const byte *p);

/// Mac QuickDraw rectangle: top, left, bottom, right.
Common::Rect readMacQuickDrawRectBE(const byte *p);
Common::Rect readMacQuickDrawRectLE(const byte *p);

Common::Rect scaleDosRectIfMac(const EEMEngine &vm, const Common::Rect &rect);

} // End of namespace EEM

#endif
