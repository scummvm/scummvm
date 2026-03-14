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

static const int kInventoryX = 64;
static const int kInventoryY = 48;
static const int kLogoX = 160;
static const int kLogoY = 0;
static const int kWaitX = 250;
static const int kWaitY = 160;
static const int kQuickTipsOverlayX = 177;
static const int kQuickTipsOverlayY = 85;
static const int kCursorX = 320;
static const int kCursorY = 200;

static const char *const kTextboxPaths[] = {
	"1:/GRAPHIC/OTHER/TEXTBOX1.BM",
	"1:/GRAPHIC/OTHER/TEXTBOX2.BM",
	"1:/GRAPHIC/OTHER/TEXTBOX3.BM",
	"1:/GRAPHIC/OTHER/TEXTBOX4.BM"
};

static const char *const kAmmoIconPaths[] = {
	"1:/GRAPHIC/ROOMOBJ/BULLET.BM",
	"1:/GRAPHIC/OTHER/SHOTSHEL.BM",
	"1:/GRAPHIC/ROOMOBJ/NAIL.BM",
	"1:/GRAPHIC/ROOMOBJ/GASCAN.BM"
};

} // End of anonymous namespace

bool StartupArt::load(ResourceManager &resources) {
	_waitFrames.clear();
	_pointerFrames.clear();
	_textboxes.clear();
	_ammoIcons.clear();
	_inventoryBitmap = IndexedBitmap();
	_logoBitmap = IndexedBitmap();
	_pcRoomBitmap = IndexedBitmap();
	_pcRoomMaskA = IndexedBitmap();
	_pcRoomMaskB = IndexedBitmap();
	_tipsBitmap = IndexedBitmap();

	return loadPalette(resources, "1:/GRAPHIC/PAL/WAIT.PAL", _waitPalette) &&
	       loadAnimation(resources, "1:/GRAPHIC/OTHER/WAIT.ABM", _waitFrames) &&
	       loadAnimation(resources, "1:/GRAPHIC/POINTERS/POINTERS.ABM", _pointerFrames) &&
	       loadBitmap(resources, "1:/GRAPHIC/OTHER/INVENTRY.BM", _inventoryBitmap) &&
	       loadBitmap(resources, "1:/GRAPHIC/OTHER/HARVLOGO.BM", _logoBitmap);
}

bool StartupArt::loadQuickTipsResources(ResourceManager &resources) {
	_textboxes.resize(ARRAYSIZE(kTextboxPaths));
	for (uint i = 0; i < _textboxes.size(); ++i) {
		if (!loadBitmap(resources, kTextboxPaths[i], _textboxes[i]))
			return false;
	}

	_ammoIcons.resize(ARRAYSIZE(kAmmoIconPaths));
	for (uint i = 0; i < _ammoIcons.size(); ++i) {
		if (!loadBitmap(resources, kAmmoIconPaths[i], _ammoIcons[i]))
			return false;
	}

	return loadAnimation(resources, "1:/GRAPHIC/MONSTERS/PC/PC0.ABM", _pcActorFrames) &&
	       loadPalette(resources, "1:/GRAPHIC/PAL/PCROOM.PAL", _pcRoomPalette) &&
	       loadBitmap(resources, "1:/GRAPHIC/ROOMS/PCROOM.BM", _pcRoomBitmap) &&
	       loadBitmap(resources, "1:/GRAPHIC/MASKS/PCRPRTA.BM", _pcRoomMaskA) &&
	       loadBitmap(resources, "1:/GRAPHIC/MASKS/PCRPRTB.BM", _pcRoomMaskB) &&
	       loadAnimation(resources, "1:/GRAPHIC/ROOMANIM/PCRMCLOK.ABM", _pcRoomClockFrames) &&
	       loadBitmap(resources, "1:/GRAPHIC/OTHER/TIPS.BM", _tipsBitmap);
}

void StartupArt::drawWaitFrame() const {
	if (_waitFrames.empty() || !_waitFrames[0].isValid())
		return;

	g_system->getPaletteManager()->setPalette(_waitPalette, 0, 256);
	g_system->fillScreen(0);
	blitBitmap(_inventoryBitmap, kInventoryX, kInventoryY);
	blitBitmap(_logoBitmap, kLogoX, kLogoY);
	blitAnimationFrame(_waitFrames, 0, kWaitX, kWaitY);
	g_system->updateScreen();
}

void StartupArt::drawQuickTipsScreen() const {
	if (!_pcRoomBitmap.isValid())
		return;

	g_system->getPaletteManager()->setPalette(_pcRoomPalette, 0, 256);
	g_system->fillScreen(0);
	blitBitmap(_pcRoomBitmap, 0, 0);
	blitBitmap(_tipsBitmap, kQuickTipsOverlayX, kQuickTipsOverlayY);
	blitAnimationFrame(_pointerFrames, 0, kCursorX, kCursorY);
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

	debug(1, "Harvester: loaded %u frames from '%s'", (uint)frames.size(), path.c_str());
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

void StartupArt::blitBitmap(const IndexedBitmap &bitmap, int x, int y) const {
	if (!bitmap.isValid())
		return;

	g_system->copyRectToScreen(bitmap.pixels.data(), bitmap.width, x, y, bitmap.width, bitmap.height);
}

void StartupArt::blitAnimationFrame(const Common::Array<AbmFrame> &frames, uint frameIndex, int x, int y) const {
	if (frameIndex >= frames.size() || !frames[frameIndex].isValid())
		return;

	const AbmFrame &frame = frames[frameIndex];
	blitBitmap(frame, x + frame.xOffset, y + frame.yOffset);
}

} // End of namespace Harvester
