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

#ifndef TRECISION_LLINC_H
#define TRECISION_LLINC_H

#include "common/rect.h"

#include "trecision/defines.h"



namespace Common {
class SeekableReadStream;
}

namespace Trecision {

// DTEXT
extern char DTextLines[MAXDTEXTLINES][MAXDTEXTCHARS];
// MOUSE
extern uint8  TextStatus;

int Compare(const void *p1, const void *p2);
void RedrawRoom();
void ReadLoc();
void TendIn();
void ReadObj(Common::SeekableReadStream *stream);
void ReadExtraObj2C();
void ReadExtraObj41D();

} // End of namespace Trecision

#endif
