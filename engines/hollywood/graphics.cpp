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

#include "hollywood/graphics.h"

#include "common/rect.h"
#include "common/system.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

uint16 readUint16LE(const Common::Array<byte> &source, uint offset) {
	if (offset + 2 > source.size())
		return 0;

	return source[offset] | (source[offset + 1] << 8);
}

int16 readSint16LE(const Common::Array<byte> &source, uint offset) {
	return (int16)readUint16LE(source, offset);
}

uint32 readUint32LE(const Common::Array<byte> &source, uint offset) {
	if (offset + 4 > source.size())
		return 0;

	return source[offset] |
		(source[offset + 1] << 8) |
		(source[offset + 2] << 16) |
		(source[offset + 3] << 24);
}

Palette6Bit::Palette6Bit() :
		_palette(256),
		_dirtyStart(0),
		_dirtyEnd(256) {
}

void Palette6Bit::setFrom6Bit(const Common::Array<byte> &palette) {
	for (uint i = 0; i < 256; ++i) {
		const uint sourceOffset = i * 3;
		const byte red = sourceOffset < palette.size() ? palette[sourceOffset] : 0;
		const byte green = sourceOffset + 1 < palette.size() ? palette[sourceOffset + 1] : 0;
		const byte blue = sourceOffset + 2 < palette.size() ? palette[sourceOffset + 2] : 0;
		const byte red8 = MIN<byte>(255, red * 4);
		const byte green8 = MIN<byte>(255, green * 4);
		const byte blue8 = MIN<byte>(255, blue * 4);
		byte previousRed = 0;
		byte previousGreen = 0;
		byte previousBlue = 0;
		_palette.get(i, previousRed, previousGreen, previousBlue);
		if (red8 != previousRed || green8 != previousGreen || blue8 != previousBlue) {
			_palette.set(i, red8, green8, blue8);
			_dirtyStart = MIN<uint>(_dirtyStart, i);
			_dirtyEnd = MAX<uint>(_dirtyEnd, i + 1);
		}
	}
}

void Palette6Bit::markAllDirty() {
	_dirtyStart = 0;
	_dirtyEnd = 256;
}

void Palette6Bit::upload() {
	if (_dirtyStart >= _dirtyEnd)
		return;

	g_system->getPaletteManager()->setPalette(_palette.data() + _dirtyStart * 3, _dirtyStart, _dirtyEnd - _dirtyStart);
	_dirtyStart = 256;
	_dirtyEnd = 0;
}

void Palette6Bit::uploadFrom6Bit(const Common::Array<byte> &palette) {
	setFrom6Bit(palette);
	upload();
}

IndexedSurfaceBuffer::IndexedSurfaceBuffer() :
		_surface(),
		_byteCount(0) {
}

void IndexedSurfaceBuffer::resize(uint byteCount) {
	_byteCount = byteCount;
	if (byteCount == 0) {
		_surface.free();
		return;
	}

	const uint width = HollywoodEngine::kSceneBufferWidth;
	const uint height = (byteCount + width - 1) / width;
	_surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

void IndexedSurfaceBuffer::clear(byte value) {
	if (_byteCount == 0 || _surface.empty())
		return;

	memset(data(), value, _byteCount);
}

byte *IndexedSurfaceBuffer::data() {
	return (byte *)_surface.getPixels();
}

const byte *IndexedSurfaceBuffer::data() const {
	return (const byte *)_surface.getPixels();
}

byte &IndexedSurfaceBuffer::operator[](uint offset) {
	return data()[offset];
}

const byte &IndexedSurfaceBuffer::operator[](uint offset) const {
	return data()[offset];
}

void presentIndexedFrame(const Graphics::Surface &framebuffer, const Common::Array<byte> &palette,
		Graphics::ManagedSurface &screen, Palette6Bit &convertedPalette, uint rowOffset, uint xOffset) {
	convertedPalette.uploadFrom6Bit(palette);

	if (screen.empty())
		screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());

	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		byte *destination = (byte *)screen.getBasePtr(0, y);
		const uint sourceY = y + rowOffset;
		if (sourceY < (uint)framebuffer.h && xOffset + HollywoodEngine::kScreenWidth <= (uint)framebuffer.w) {
			memcpy(destination, framebuffer.getBasePtr(xOffset, sourceY), HollywoodEngine::kScreenWidth);
		} else {
			memset(destination, 0, HollywoodEngine::kScreenWidth);
		}
	}

	g_system->copyRectToScreen(screen.getPixels(), screen.pitch, 0, 0, screen.w, screen.h);
	g_system->updateScreen();
}

void copySurfaceRun(const Graphics::Surface &source, Graphics::Surface &destination, int y, int x, int width) {
	if (width <= 0 || y < 0 || x < 0 || y >= source.h || y >= destination.h ||
			x >= source.w || x >= destination.w)
		return;

	const int copyWidth = MIN<int>(width, MIN<int>(source.w - x, destination.w - x));
	if (copyWidth <= 0)
		return;

	destination.copyRectToSurface(source, x, y, Common::Rect(x, y, x + copyWidth, y + 1));
}

void clearSurfaceRun(Graphics::Surface &destination, int y, int x, int width) {
	if (width <= 0 || y < 0 || x < 0 || y >= destination.h || x >= destination.w)
		return;

	const int clearWidth = MIN<int>(width, destination.w - x);
	if (clearWidth <= 0)
		return;

	memset(destination.getBasePtr(x, y), 0, clearWidth * destination.format.bytesPerPixel);
}

void restoreSpriteBackground(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset,
		uint16 descriptorCount, uint16 descriptorIndex, const Graphics::Surface &background,
		Graphics::Surface &destination, int yOffset) {
	if (descriptorIndex >= descriptorCount)
		return;

	const uint entryOffset = baseOffset + descriptorTableOffset + (kFrameDescriptorSize * descriptorIndex);
	if (entryOffset + kFrameDescriptorSize > resource.size())
		return;

	const uint32 packedWidth = readUint32LE(resource, entryOffset + 4);
	const uint32 packedRows = readUint32LE(resource, entryOffset + 8);
	const int copyWidth = (packedWidth >> 16) & 0xffff;
	const int x = packedWidth & 0xffff;
	const int firstRow = (int)(packedRows & 0xffff) + yOffset;
	const int lastRow = (int)((packedRows >> 16) & 0xffff) + yOffset;
	if (copyWidth == 0 || firstRow > lastRow)
		return;

	for (int row = firstRow; row <= lastRow; ++row)
		copySurfaceRun(background, destination, row, x, copyWidth);
}

void restoreSpriteBackground(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset,
		uint16 descriptorCount, uint16 descriptorIndex, const Graphics::ManagedSurface &background,
		Graphics::ManagedSurface &destination, int yOffset) {
	restoreSpriteBackground(resource, baseOffset, descriptorTableOffset, descriptorCount, descriptorIndex,
		background.rawSurface(), *destination.surfacePtr(), yOffset);
}

void drawStripSpriteFrame(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset,
		uint16 descriptorCount, uint16 descriptorIndex, Graphics::Surface &destination, int yOffset) {
	if (descriptorIndex >= descriptorCount)
		return;

	const uint entryOffset = baseOffset + descriptorTableOffset + (kFrameDescriptorSize * descriptorIndex);
	if (entryOffset + kFrameDescriptorSize > resource.size())
		return;

	const uint16 spanCount = readUint16LE(resource, entryOffset + 12);
	uint cursor = baseOffset + descriptorTableOffset + (kFrameDescriptorSize * descriptorCount) + readUint32LE(resource, entryOffset);
	if (cursor > resource.size())
		return;

	for (uint spanIndex = 0; spanIndex < spanCount; ++spanIndex) {
		if (cursor + 5 > resource.size())
			return;

		const uint32 packedDestination = readUint32LE(resource, cursor);
		const int dataLength = resource[cursor + 4];
		cursor += 5;

		if (cursor + dataLength > resource.size())
			return;

		const int x = packedDestination & 0xffff;
		const int y = (int)((packedDestination >> 16) & 0xffff) + yOffset;
		if (y >= 0 && y < destination.h && x >= 0 && x < destination.w) {
			const int drawWidth = MIN<int>(dataLength, destination.w - x);
			if (drawWidth > 0)
				destination.copyRectToSurface(resource.data() + cursor, dataLength, x, y, drawWidth, 1);
		}

		cursor += dataLength;
	}
}

void drawStripSpriteFrame(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset,
		uint16 descriptorCount, uint16 descriptorIndex, Graphics::ManagedSurface &destination, int yOffset) {
	drawStripSpriteFrame(resource, baseOffset, descriptorTableOffset, descriptorCount, descriptorIndex,
		*destination.surfacePtr(), yOffset);
}

void drawResourceBlockList(const Common::Array<byte> &resource, uint32 baseOffset,
		Graphics::Surface &destination, int yOffset) {
	if (baseOffset + 2 > resource.size())
		return;

	const uint16 blockCount = readUint16LE(resource, baseOffset);
	uint cursor = baseOffset + 2;
	for (uint blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
		if (cursor + 6 > resource.size())
			return;

		const uint32 packedDestination = readUint32LE(resource, cursor);
		const int size = readUint16LE(resource, cursor + 4);
		cursor += 6;

		if (cursor + size > resource.size())
			return;

		const int x = packedDestination & 0xffff;
		const int y = (int)((packedDestination >> 16) & 0xffff) + yOffset;
		if (y >= 0 && y < destination.h && x >= 0 && x < destination.w) {
			const int drawWidth = MIN<int>(size, destination.w - x);
			if (drawWidth > 0)
				destination.copyRectToSurface(resource.data() + cursor, size, x, y, drawWidth, 1);
		}
		cursor += size;
	}
}

void drawResourceBlockList(const Common::Array<byte> &resource, uint32 baseOffset,
		Graphics::ManagedSurface &destination, int yOffset) {
	drawResourceBlockList(resource, baseOffset, *destination.surfacePtr(), yOffset);
}

} // End of namespace Hollywood
