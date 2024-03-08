
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

#ifndef BAGEL_SPACEBAR_FILTER_H
#define BAGEL_SPACEBAR_FILTER_H

#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/rect.h"

namespace Bagel {
namespace SpaceBar {

const USHORT kTriFilter = (USHORT)1;
const USHORT kHalucinateFilter = (USHORT)2;
const USHORT kVildroidFilter = (USHORT)4;
const USHORT kZzazzlFilter = (USHORT)8;
const USHORT kLightningFilter = (USHORT)16;

typedef BOOL(*FilterFxn)(const USHORT, CBofBitmap *, CBofRect *);

VOID VilInitFilters(CBofBitmap *pBmp);
VOID TriInitFilters(CBofBitmap *pBmp);
VOID LightningInitFilters(VOID);
VOID DestroyFilters();

BOOL DoFilters(const USHORT nFilterId, CBofBitmap *pBmp, CBofRect *viewRect);

static DWORD waitCount;

} // namespace SpaceBar
} // namespace Bagel

#endif
