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
#include "graphics/managed_surface.h"
#include "graphics/palette.h"

namespace Graphics {
class ManagedSurface;
struct Surface;
}

namespace Hollywood {

enum {
	kPaletteSize = 0x300,
	kFrameDescriptorSize = 14
};

uint16 readUint16LE(const Common::Array<byte> &source, uint offset);
int16 readSint16LE(const Common::Array<byte> &source, uint offset);
uint32 readUint32LE(const Common::Array<byte> &source, uint offset);

/**
 * Converts the game's 256-entry RGB palette to backend 8-bit components.
 *
 * setFrom6Bit() scales each component by four and extends one dirty range.
 * upload() sends only that range; markAllDirty() forces a complete upload.
 */
class Palette6Bit {
public:
	Palette6Bit();

	void setFrom6Bit(const Common::Array<byte> &palette);
	void markAllDirty();
	void upload();
	void uploadFrom6Bit(const Common::Array<byte> &palette);

	const Graphics::Palette &palette() const { return _palette; }

private:
	Graphics::Palette _palette;
	uint _dirtyStart;
	uint _dirtyEnd;
};

// Owns a CLUT8 scene-sized surface while preserving byte-offset access for
// original resource decoders that address 1024-wide framebuffers directly.
class IndexedSurfaceBuffer {
public:
	IndexedSurfaceBuffer();

	void resize(uint byteCount);
	void clear(byte value = 0);

	bool empty() const { return _surface.empty(); }
	uint size() const { return _byteCount; }
	byte *data();
	const byte *data() const;
	byte &operator[](uint offset);
	const byte &operator[](uint offset) const;
	Graphics::ManagedSurface &managedSurface() { return _surface; }
	const Graphics::ManagedSurface &managedSurface() const { return _surface; }
	Graphics::Surface &surface() { return *_surface.surfacePtr(); }
	const Graphics::Surface &surface() const { return _surface.rawSurface(); }

private:
	Graphics::ManagedSurface _surface;
	uint _byteCount;
};

void presentIndexedFrame(const Graphics::Surface &framebuffer, const Common::Array<byte> &palette,
	Graphics::ManagedSurface &screen, Palette6Bit &convertedPalette, uint rowOffset = 0, uint xOffset = 0);

void copySurfaceRun(const Graphics::Surface &source, Graphics::Surface &destination, int y, int x, int width);
void clearSurfaceRun(Graphics::Surface &destination, int y, int x, int width);

void restoreSpriteBackground(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset,
	uint16 descriptorCount, uint16 descriptorIndex, const Graphics::Surface &background,
	Graphics::Surface &destination, int yOffset = 0);
void restoreSpriteBackground(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset,
	uint16 descriptorCount, uint16 descriptorIndex, const Graphics::ManagedSurface &background,
	Graphics::ManagedSurface &destination, int yOffset = 0);
void drawStripSpriteFrame(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset,
	uint16 descriptorCount, uint16 descriptorIndex, Graphics::Surface &destination, int yOffset = 0);
void drawStripSpriteFrame(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset,
	uint16 descriptorCount, uint16 descriptorIndex, Graphics::ManagedSurface &destination, int yOffset = 0);
void drawResourceBlockList(const Common::Array<byte> &resource, uint32 baseOffset, Graphics::Surface &destination, int yOffset = 0);
void drawResourceBlockList(const Common::Array<byte> &resource, uint32 baseOffset, Graphics::ManagedSurface &destination, int yOffset = 0);

} // End of namespace Hollywood

#endif // HOLLYWOOD_GRAPHICS_H
