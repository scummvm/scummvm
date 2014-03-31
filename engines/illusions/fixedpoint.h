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

#ifndef ILLUSIONS_FIXEDPOINT_H
#define ILLUSIONS_FIXEDPOINT_H

#include "common/rect.h"

namespace Illusions {

typedef int32 FP16;

FP16 floatToFixed(float value);
float fixedToFloat(FP16 value);
FP16 fixedMul(FP16 a, FP16 b);
FP16 fixedDiv(FP16 a, FP16 b);
int16 fixedTrunc(FP16 value);
FP16 fixedDistance(FP16 x1, FP16 y1, FP16 x2, FP16 y2);
FP16 fixedAtan(FP16 value);
FP16 fixedCos(FP16 value);
FP16 fixedSin(FP16 value);

} // End of namespace Illusions

#endif // ILLUSIONS_FIXEDPOINT_H
