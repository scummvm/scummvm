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

#include "harvester/art.h"

#include "common/debug.h"
#include "common/endian.h"
#include "graphics/blit.h"
#include "graphics/screen.h"
#include "harvester/detection.h"
#include "harvester/palette_utils.h"
#include "harvester/resources.h"

namespace Harvester {

namespace {

static void getPaletteByteRange(const byte *palette, byte &minValue, byte &maxValue) {
	minValue = 255;
	maxValue = 0;

	for (uint i = 0; i < 256 * 3; ++i) {
		minValue = MIN(minValue, palette[i]);
		maxValue = MAX(maxValue, palette[i]);
	}
}

static uint32 hashPalette(const byte *palette) {
	uint32 hash = 2166136261U;
	for (uint i = 0; i < 256 * 3; ++i) {
		hash ^= palette[i];
		hash *= 16777619U;
	}

	return hash;
}

static void logPaletteSummary(const char *label, const Common::String &path, const byte *palette) {
	if (!palette)
		return;

	byte minValue = 0;
	byte maxValue = 0;
	getPaletteByteRange(palette, minValue, maxValue);

	debugC(1, kDebugGeneral,
		"Harvester: %s '%s' bytes=%u range=[%u,%u] hash=%08x idx0=(%u,%u,%u) idx1=(%u,%u,%u) idx127=(%u,%u,%u) idx255=(%u,%u,%u)",
		label, path.c_str(), 256U * 3U, minValue, maxValue, hashPalette(palette),
		palette[0], palette[1], palette[2],
		palette[3], palette[4], palette[5],
		palette[127 * 3], palette[127 * 3 + 1], palette[127 * 3 + 2],
		palette[255 * 3], palette[255 * 3 + 1], palette[255 * 3 + 2]);
}

static const int kWaitX = 250;
static const int kWaitY = 160;
static const byte kTransparentPaletteIndex = 0;

static const char *const kTextboxPaths[] = {
	"1:/GRAPHIC/OTHER/TEXTBOX1.BM",
	"1:/GRAPHIC/OTHER/TEXTBOX2.BM",
	"1:/GRAPHIC/OTHER/TEXTBOX3.BM",
	"1:/GRAPHIC/OTHER/TEXTBOX4.BM",
	"1:/GRAPHIC/OTHER/TEXTBOX5.BM",
	"1:/GRAPHIC/OTHER/TEXTBOX6.BM",
	"1:/GRAPHIC/OTHER/TEXTBOX7.BM",
	"1:/GRAPHIC/OTHER/TEXTBOX8.BM"
};

static const char *const kAmmoIconPaths[] = {
	"1:/GRAPHIC/ROOMOBJ/BULLET.BM",
	"1:/GRAPHIC/OTHER/SHOTSHEL.BM",
	"1:/GRAPHIC/ROOMOBJ/NAIL.BM",
	"1:/GRAPHIC/ROOMOBJ/GASCAN.BM"
};

} // End of anonymous namespace

bool Art::load(ResourceManager &resources) {
	_waitFrames.clear();
	_textboxes.clear();
	_ammoIcons.clear();
	_inventoryBitmap = IndexedBitmap();
	_logoBitmap = IndexedBitmap();
	_tipsBitmap = IndexedBitmap();

	return loadPalette(resources, "1:/GRAPHIC/PAL/WAIT.PAL", _waitPalette) &&
	       loadAnimation(resources, "1:/GRAPHIC/OTHER/WAIT.ABM", _waitFrames) &&
	       loadBitmap(resources, "1:/GRAPHIC/OTHER/INVENTRY.BM", _inventoryBitmap) &&
	       loadBitmap(resources, "1:/GRAPHIC/OTHER/HARVLOGO.BM", _logoBitmap);
}

bool Art::loadQuickTipsResources(ResourceManager &resources) {
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

	return loadBitmap(resources, "1:/GRAPHIC/OTHER/TIPS.BM", _tipsBitmap);
}

void Art::drawWaitFrame(Graphics::Screen &screen) const {
	if (_waitFrames.empty() || !_waitFrames[0].isValid())
		return;

	// Native room_setup redraws the wait screen through flush_dirty_rects_to_screen(),
	// which fills uncovered areas with palette index 0 before WAIT.PAL is uploaded.
	// Clear first so the transparent WAIT art sits on black instead of old room pixels.
	screen.fillRect(screen.getBounds(), kTransparentPaletteIndex);
	logPaletteSummary("applying wait palette", "1:/GRAPHIC/PAL/WAIT.PAL", _waitPalette);
	byte displayPalette[256 * 3];
	buildHarvesterDisplayPalette(_waitPalette, 1.0f, displayPalette);
	screen.setPalette(displayPalette);
	blitTransparentAnimationFrame(screen, _waitFrames, 0, kWaitX, kWaitY);
	screen.makeAllDirty();
	screen.update();
}

bool Art::loadPalette(ResourceManager &resources, const Common::String &path, byte *dest) const {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3) {
		warning("Harvester: unable to load palette '%s'", path.c_str());
		return false;
	}

	memcpy(dest, data.data(), 256 * 3);

	logPaletteSummary("loaded palette", path, dest);
	return true;
}

bool Art::loadBitmap(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) const {
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

bool Art::loadAnimation(ResourceManager &resources, const Common::String &path, Common::Array<AbmFrame> &frames) const {
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

bool Art::decodeAnimationFrame(const byte *source, uint32 sourceSize, bool compressed, Common::Array<byte> &dest) const {
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

void Art::blitTransparentBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) const {
	if (!bitmap.isValid())
		return;

	int srcX = 0;
	int srcY = 0;
	int destX = x;
	int destY = y;
	int width = (int)bitmap.width;
	int height = (int)bitmap.height;

	if (destX < 0) {
		srcX = -destX;
		width += destX;
		destX = 0;
	}
	if (destY < 0) {
		srcY = -destY;
		height += destY;
		destY = 0;
	}
	if (destX >= screen.w || destY >= screen.h || width <= 0 || height <= 0)
		return;

	width = MIN<int>(width, screen.w - destX);
	height = MIN<int>(height, screen.h - destY);
	if (width <= 0 || height <= 0)
		return;

	const byte *src = bitmap.pixels.data() + srcY * bitmap.width + srcX;
	byte *dst = (byte *)screen.getBasePtr(destX, destY);
	Graphics::keyBlit(dst, src, screen.pitch, bitmap.width, width, height,
		screen.format.bytesPerPixel, kTransparentPaletteIndex);
}

void Art::blitTransparentAnimationFrame(Graphics::Screen &screen, const Common::Array<AbmFrame> &frames,
		uint frameIndex, int x, int y) const {
	if (frameIndex >= frames.size() || !frames[frameIndex].isValid())
		return;

	const AbmFrame &frame = frames[frameIndex];
	blitTransparentBitmap(screen, frame, x + frame.xOffset, y + frame.yOffset);
}

} // End of namespace Harvester
