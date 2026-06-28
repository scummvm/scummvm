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

#ifndef HOLLYWOOD_GRAPHICS_H
#define HOLLYWOOD_GRAPHICS_H

#include "common/array.h"
#include "common/types.h"

namespace Hollywood {

enum {
	kPaletteSize = 0x300,
	kFrameDescriptorSize = 14
};

uint16 readUint16LE(const Common::Array<byte> &source, uint offset);
int16 readSint16LE(const Common::Array<byte> &source, uint offset);
uint32 readUint32LE(const Common::Array<byte> &source, uint offset);

void uploadPalette6Bit(const Common::Array<byte> &palette);
void presentIndexedFrame(const Common::Array<byte> &framebuffer, const Common::Array<byte> &palette,
	Common::Array<byte> &screen, uint rowOffset = 0, uint xOffset = 0);

void copyFramebufferRun(const Common::Array<byte> &source, Common::Array<byte> &destination, int y, int x, int width);
void clearFramebufferRun(Common::Array<byte> &destination, int y, int x, int width);

void restoreSpriteBackground(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset,
	uint16 descriptorCount, uint16 descriptorIndex, const Common::Array<byte> &background,
	Common::Array<byte> &destination, int yOffset = 0);
void drawStripSpriteFrame(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset,
	uint16 descriptorCount, uint16 descriptorIndex, Common::Array<byte> &destination, int yOffset = 0);
void drawResourceBlockList(const Common::Array<byte> &resource, uint32 baseOffset, Common::Array<byte> &destination, int yOffset = 0);

} // End of namespace Hollywood

#endif // HOLLYWOOD_GRAPHICS_H
