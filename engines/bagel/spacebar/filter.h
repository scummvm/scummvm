
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

#include "bagel/spacebar/boflib/gfx/bitmap.h"
#include "bagel/boflib/rect.h"

namespace Bagel {
namespace SpaceBar {

const uint16 kTriFilter = (uint16)1;
const uint16 kHalucinateFilter = (uint16)2;
const uint16 kVildroidFilter = (uint16)4;
const uint16 kZzazzlFilter = (uint16)8;
const uint16 kLightningFilter = (uint16)16;

typedef bool(*FilterFxn)(uint16, CBofBitmap *, CBofRect *);

void vilInitFilters(CBofBitmap *pBmp);
void triInitFilters(CBofBitmap *pBmp);
void lightningInitFilters();
void destroyFilters();

bool doFilters(uint16 nFilterId, CBofBitmap *pBmp, CBofRect *pRect);

} // namespace SpaceBar
} // namespace Bagel

#endif
