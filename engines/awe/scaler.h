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

#ifndef AWE_SCALER_H
#define AWE_SCALER_H

#include "awe/intern.h"

namespace Awe {

typedef void (*ScaleProc)(int factor, int byteDepth, uint8_t *dst, int dstPitch, const uint8_t *src, int srcPitch, int w, int h);

#define SCALER_TAG 1

struct Scaler {
	uint32_t tag;
	const char *name;
	int factorMin, factorMax;
	int bpp;
	ScaleProc scale;
};

const Scaler *findScaler(const char *name);

} // namespace Awe

#endif
