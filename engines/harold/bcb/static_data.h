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

#ifndef BCB_STATIC_DATA_H
#define BCB_STATIC_DATA_H

#include "common/rect.h"
#include "common/scummsys.h"
#include "harold/bcb/ui.h"

namespace BCB {

struct CompositeBitmapVariantSet {
	uint pathCount;
	const char *paths[4];
};

extern const ButtonControlRecord kTitleButtonTable[5];
extern const ButtonControlRecord kSystemSubmodeButtonTable[5];
extern const ButtonControlRecord kSystemButtonTable[9];
extern const ButtonControlRecord kSceneChoiceButtonTable[10];
extern const ButtonControlRecord kMapButtonTable[19];
extern const int kMapButtonSecondaryStateIndexTable[19];
extern const ButtonControlRecord kComputerPrimaryButtonTable[2];
extern const ButtonControlRecord kComputerDetailButtonTable[3];
extern const ButtonControlRecord kComputerChoiceLayoutSlots[21];
extern const ButtonControlRecord kComputerChoiceButtonTable[30];
extern const StaticHitRect kComputerChoiceGroupHitRects[3];
extern const byte kComputerChoiceDependencyStateIndex[30];
extern const CompositeBitmapVariantSet kComputerDetailBitmapVariantsBySelectedValue[31];
extern const byte kSceneChoiceValueByButtonIndex[10];
extern const int kStillImageDurationTicks[8];
extern const char *const kStillImageHighColorPaths[8];

} // End of namespace BCB

#endif // BCB_STATIC_DATA_H
