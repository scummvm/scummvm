/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGSCREDITZ_DRAWING_H
#define AGS_PLUGINS_AGSCREDITZ_DRAWING_H

#include "common/scummsys.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

class Drawing {
public:
void drawPixel(uint8 *bitmap, int32 x, int32 y,
                      uint col, int32 pitch, int32 coldepth) {
	switch (coldepth) {
	case 8:
		bitmap[x + y * pitch] = col;
		break;
	case 16:
		*((uint16 *)(bitmap + y * pitch + x * 2)) = col;
		break;
	case 32:
		*((uint32 *)(bitmap + y * pitch + x * 4)) = col;
		break;
	}
}

uint getPixelColor(uint8 *bitmap, int32 x, int32 y,
                          int32 pitch, int32 coldepth) {
	switch (coldepth) {
	case 8:
		return bitmap[x + y * pitch];
	case 16:
		return *((uint16 *)(bitmap + y * pitch + x * 2));
	case 32:
		return *((uint32 *)(bitmap + y * pitch + x * 4));
	}
	return 0;
}
};

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3

#endif
