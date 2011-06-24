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

#ifndef NEVERHOOD_GRAPHICS_H
#define NEVERHOOD_GRAPHICS_H

#include "common/array.h"
#include "common/file.h"
#include "neverhood/neverhood.h"

namespace Neverhood {

struct NDimensions {
	int16 width, height;
};

struct NUnknown {
	int16 unk1, unk2;
};

void parseBitmapResource(byte *sprite, bool *rle, NDimensions *dimensions, NUnknown *unknown, byte **palette, byte **pixels);
void unpackSpriteRle(byte *source, int width, int height, byte *dest, int destPitch, bool flipX, bool flipY);

} // End of namespace Neverhood

#endif /* NEVERHOOD_GRAPHICS_H */
