/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_MMSHELL_MMCONSTANTS_H
#define PEGASUS_MMSHELL_MMCONSTANTS_H

#include "pegasus/MMShell/MMTypes.h"

namespace Pegasus {

const tDisplayElementID kNoDisplayElement = -1;
const tDisplayElementID kHighestReservedElementID = -2;

const tDisplayElementID kCursorID = kHighestReservedElementID;
const tDisplayElementID kLoadScreenID = kCursorID - 1;

const tDisplayOrder kMinAvailableOrder = 0;
const tDisplayOrder kMaxAvailableOrder = 999998;
const tDisplayOrder kLoadScreenOrder = 900000;
const tDisplayOrder kCursorOrder = 1000000;

const tHotSpotID kNoHotSpotID = -1;
const tHotSpotFlags kNoHotSpotFlags = 0;
const tHotSpotFlags kAllHotSpotFlags = ~kNoHotSpotFlags;

const tNotificationFlags kNoNotificationFlags = 0;

const tDisplayElementID kCurrentDragSpriteID = 1000;

// TODO
//const Fixed kFixed1 = 1 << 16;
//const Fixed kFixedMinus1 = -1 << 16;

const TimeScale kDefaultTimeScale = 600;

// TODO
//const RGBColor kWhiteRGB = {0xFFFF, 0xFFFF, 0xFFFF};

} // End of namespace Pegasus

#endif
