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

#include "harvester/bitmap.h"

#include "common/debug.h"
#include "common/endian.h"
#include "graphics/managed_surface.h"
#include "harvester/detection.h"

namespace Harvester {

void blitTransparentBitmap(Graphics::ManagedSurface &surface, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid() || x >= surface.w || y >= surface.h ||
			x <= -(int)bitmap.width || y <= -(int)bitmap.height)
		return;

	Graphics::ManagedSurface source;
	// Borrow the bitmap pixels without taking ownership.
	source.surfacePtr()->init(bitmap.width, bitmap.height, bitmap.width,
		const_cast<byte *>(bitmap.pixels.data()), Graphics::PixelFormat::createFormatCLUT8());
	source.setTransparentColor(0);
	surface.simpleBlitFrom(source, Common::Point(x, y));
}

namespace {

static bool decodeAnimationFrame(const byte *source, uint32 sourceSize, bool compressed, Common::Array<byte> &dest) {
	if (!compressed) {
		if (sourceSize < dest.size())
			return false;

		memcpy(dest.data(), source, dest.size());
		return true;
	}

	uint32 srcOffset = 0;
	uint32 dstOffset = 0;
	while (srcOffset < sourceSize && dstOffset < dest.size()) {
		const byte control = source[srcOffset++];
		if ((control & 0x80) == 0) {
			const uint32 literalCount = MIN<uint32>(control,
				MIN<uint32>(sourceSize - srcOffset, dest.size() - dstOffset));
			memcpy(dest.data() + dstOffset, source + srcOffset, literalCount);
			srcOffset += literalCount;
			dstOffset += literalCount;
		} else {
			if (srcOffset >= sourceSize)
				return false;

			const uint32 repeatCount = MIN<uint32>(control & 0x7f, dest.size() - dstOffset);
			memset(dest.data() + dstOffset, source[srcOffset++], repeatCount);
			dstOffset += repeatCount;
		}
	}

	return dstOffset == dest.size();
}

} // End of anonymous namespace

bool decodeBitmap(const Common::Array<byte> &data, IndexedBitmap &bitmap) {
	if (data.size() < 12) {
		debugC(1, kDebugResources, "Harvester: short BM header bytes=%u", data.size());
		return false;
	}

	bitmap.width = READ_LE_UINT32(data.data());
	bitmap.height = READ_LE_UINT32(data.data() + 4);
	const uint32 pixelCount = bitmap.width * bitmap.height;
	if (bitmap.width == 0 || bitmap.height == 0 || data.size() < 12 + pixelCount) {
		debugC(1, kDebugResources, "Harvester: invalid BM size=%ux%u bytes=%u",
			bitmap.width, bitmap.height, data.size());
		return false;
	}

	bitmap.pixels.resize(pixelCount);
	memcpy(bitmap.pixels.data(), data.data() + 12, pixelCount);
	debugC(3, kDebugResources, "Harvester: decoded BM size=%ux%u pixels=%u",
		bitmap.width, bitmap.height, pixelCount);
	return true;
}

bool decodeAbm(const Common::Array<byte> &data, Common::Array<AbmFrame> &frames) {
	if (data.size() < 8) {
		debugC(1, kDebugResources, "Harvester: short ABM header bytes=%u", data.size());
		return false;
	}

	const uint32 frameCount = READ_LE_UINT32(data.data());
	uint32 offset = 8;
	frames.resize(frameCount);

	for (uint32 i = 0; i < frameCount; ++i) {
		if (data.size() < offset + 25) {
			debugC(1, kDebugResources, "Harvester: short ABM frame header frame=%u offset=%u bytes=%u",
				i, offset, data.size());
			return false;
		}

		AbmFrame &frame = frames[i];
		frame.xOffset = (int32)READ_LE_UINT32(data.data() + offset);
		frame.yOffset = (int32)READ_LE_UINT32(data.data() + offset + 4);
		frame.width = READ_LE_UINT32(data.data() + offset + 8);
		frame.height = READ_LE_UINT32(data.data() + offset + 12);
		const bool compressed = data[offset + 16] != 0;
		const uint32 sourceSize = READ_LE_UINT32(data.data() + offset + 17);
		const uint32 pixelCount = frame.width * frame.height;
		const uint32 payloadOffset = offset + 25;

		if (frame.width == 0 || frame.height == 0 || data.size() < payloadOffset + sourceSize) {
			debugC(1, kDebugResources, "Harvester: invalid ABM frame=%u size=%ux%u offset=%u source_bytes=%u bytes=%u",
				i, frame.width, frame.height, payloadOffset, sourceSize, data.size());
			return false;
		}

		frame.pixels.resize(pixelCount);
		if (!decodeAnimationFrame(data.data() + payloadOffset, sourceSize, compressed, frame.pixels)) {
			debugC(1, kDebugResources, "Harvester: incomplete ABM pixels frame=%u source_bytes=%u pixels=%u",
				i, sourceSize, pixelCount);
			return false;
		}

		debugC(3, kDebugResources, "Harvester: decoded ABM frame=%u size=%ux%u position=(%d,%d) offset=%u source_bytes=%u compressed=%d",
			i, frame.width, frame.height, frame.xOffset, frame.yOffset, payloadOffset, sourceSize, compressed);
		offset = payloadOffset + sourceSize;
	}

	debugC(2, kDebugResources, "Harvester: decoded ABM frames=%u bytes=%u", frameCount, data.size());
	return true;
}

} // End of namespace Harvester
