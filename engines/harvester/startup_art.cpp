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

#include "harvester/startup_art.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "harvester/resources.h"

namespace Harvester {

namespace {

static byte expand6BitColor(byte value) {
	return (value * 255 + 31) / 63;
}

} // End of anonymous namespace

bool StartupArt::load(ResourceManager &resources) {
	_waitFrames.clear();
	_pointerFrames.clear();
	_inventoryBitmap = IndexedBitmap();
	_logoBitmap = IndexedBitmap();

	return loadPalette(resources, "1:/GRAPHIC/PAL/WAIT.PAL", _waitPalette) &&
	       loadAnimation(resources, "1:/GRAPHIC/OTHER/WAIT.ABM", _waitFrames) &&
	       loadAnimation(resources, "1:/GRAPHIC/POINTERS/POINTERS.ABM", _pointerFrames) &&
	       loadBitmap(resources, "1:/GRAPHIC/OTHER/INVENTRY.BM", _inventoryBitmap) &&
	       loadBitmap(resources, "1:/GRAPHIC/OTHER/HARVLOGO.BM", _logoBitmap);
}

void StartupArt::drawWaitFrame() const {
	if (_waitFrames.empty() || !_waitFrames[0].isValid())
		return;

	g_system->getPaletteManager()->setPalette(_waitPalette, 0, 256);

	const int screenWidth = g_system->getWidth();
	const int screenHeight = g_system->getHeight();
	g_system->fillScreen(0);

	if (_inventoryBitmap.isValid()) {
		const int inventoryX = MAX<int>(0, (screenWidth - (int)_inventoryBitmap.width) / 2);
		const int inventoryY = MAX<int>(0, (screenHeight - (int)_inventoryBitmap.height) / 2);
		g_system->copyRectToScreen(_inventoryBitmap.pixels.data(), _inventoryBitmap.width, inventoryX, inventoryY,
			_inventoryBitmap.width, _inventoryBitmap.height);
	}

	if (_logoBitmap.isValid()) {
		const int logoX = MAX<int>(0, (screenWidth - (int)_logoBitmap.width) / 2);
		g_system->copyRectToScreen(_logoBitmap.pixels.data(), _logoBitmap.width, logoX, 0,
			_logoBitmap.width, _logoBitmap.height);
	}

	const AbmFrame &frame = _waitFrames[0];
	const int waitX = MAX<int>(0, (screenWidth - (int)frame.width) / 2);
	const int waitY = MAX<int>(0, (screenHeight - (int)frame.height) / 2);
	g_system->copyRectToScreen(frame.pixels.data(), frame.width, waitX, waitY, frame.width, frame.height);
	g_system->updateScreen();
}

bool StartupArt::loadPalette(ResourceManager &resources, const Common::String &path, byte *dest) const {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3) {
		warning("Harvester: unable to load palette '%s'", path.c_str());
		return false;
	}

	for (uint i = 0; i < 256 * 3; ++i)
		dest[i] = expand6BitColor(data[i]);

	return true;
}

bool StartupArt::loadBitmap(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) const {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12) {
		warning("Harvester: unable to load bitmap '%s'", path.c_str());
		return false;
	}

	bitmap.width = READ_LE_UINT32(data.data());
	bitmap.height = READ_LE_UINT32(data.data() + 4);
	const uint32 pixelCount = bitmap.width * bitmap.height;
	if (bitmap.width == 0 || bitmap.height == 0 || data.size() < 12 + pixelCount) {
		warning("Harvester: invalid bitmap '%s'", path.c_str());
		return false;
	}

	bitmap.pixels.resize(pixelCount);
	memcpy(bitmap.pixels.data(), data.data() + 12, pixelCount);
	return true;
}

bool StartupArt::loadAnimation(ResourceManager &resources, const Common::String &path, Common::Array<AbmFrame> &frames) const {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 8) {
		warning("Harvester: unable to load animation '%s'", path.c_str());
		return false;
	}

	const uint32 frameCount = READ_LE_UINT32(data.data());
	uint32 offset = 8;
	frames.resize(frameCount);

	for (uint32 i = 0; i < frameCount; ++i) {
		if (data.size() < offset + 25) {
			warning("Harvester: short ABM header in '%s'", path.c_str());
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
			warning("Harvester: invalid ABM frame %u in '%s'", i, path.c_str());
			return false;
		}

		frame.pixels.resize(pixelCount);
		if (!decodeAnimationFrame(data.data() + payloadOffset, sourceSize, compressed, frame.pixels))
			return false;

		offset = payloadOffset + sourceSize;
	}

	debug(1, "Harvester: loaded %u WAIT.ABM frames", (uint)frames.size());
	return true;
}

bool StartupArt::decodeAnimationFrame(const byte *source, uint32 sourceSize, bool compressed, Common::Array<byte> &dest) const {
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
			const uint32 literalCount = MIN<uint32>(control, MIN<uint32>(sourceSize - srcOffset, dest.size() - dstOffset));
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

} // End of namespace Harvester
