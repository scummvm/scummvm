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


#include "common/file.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "sci/graphics/drivers/gfxdriver_intern.h"

namespace Sci {

class SCI1_PC98Gfx8ColorsDriver final : public UpscaledGfxDriver {
public:
	SCI1_PC98Gfx8ColorsDriver(bool rgbRendering);
	~SCI1_PC98Gfx8ColorsDriver() override;
	void initScreen(const Graphics::PixelFormat *format) override;
	void setPalette(const byte*, uint, uint, bool, const PaletteMod*, const byte*) override {}
	void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	byte remapTextColor(byte) const override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformPC98) && checkDriver(&_driverFile, 1); }
private:
	const byte *_ditheringTable;
	const byte *_convPalette;
	static const char *_driverFile;
};

SCI1_PC98Gfx8ColorsDriver::SCI1_PC98Gfx8ColorsDriver(bool rgbRendering) : UpscaledGfxDriver(1, true, rgbRendering), _ditheringTable(nullptr), _convPalette(nullptr) {
	Common::File drv;
	if (!drv.open(_driverFile))
		GFXDRV_ERR_OPEN(_driverFile);

	uint16 eprcOffs = 0;

	uint32 cmd = drv.readUint32LE();
	if ((cmd & 0xFF) == 0xE9)
		eprcOffs = ((cmd >> 8) & 0xFFFF) + 3;

	if (!eprcOffs || drv.readUint32LE() != 0x87654321 || !drv.skip(1) || !drv.seek(drv.readByte(), SEEK_CUR) || !drv.seek(drv.readByte(), SEEK_CUR))
		GFXDRV_ERR_VERSION(_driverFile);

	uint32 pos = (drv.pos() + 1) & ~1;

	drv.seek(pos + 2);
	drv.seek(drv.readUint16LE());
	byte *buff = new byte[190];
	drv.read(buff, 190);

	uint16 tableOffs = 0;
	int step = 0;
	for (int i = 0; i < 182 && !tableOffs; ++i) {
		uint32 c = READ_BE_UINT32(buff + i);
		if (step == 0 && ((c & 0xFFF0FFF0) != 0xD1E0D1E0 || (READ_BE_UINT32(buff + i + 4) ^ c)))
			continue;

		if (step == 0) {
			step = 1;
			i += 7;
			continue;
		}

		if (c >> 20 != 0x81C || ((READ_BE_UINT32(buff + i + 4) >> 20) ^ (c >> 20)))
			continue;

		if ((c & 0xFFFF) == READ_BE_UINT16(buff + i + 6))
			tableOffs = FROM_BE_16(c);
	}
	delete[] buff;

	if (!tableOffs)
		error("SCI1_PC98Gfx8ColorsDriver: Failed to load dithering data from '%s'", _driverFile);

	drv.seek(tableOffs);
	byte *dmx = new byte[96]();
	drv.read(dmx, 96);

	if (drv.readUint16LE() != 0xA800 || drv.readUint16LE() != 0xB000)
		GFXDRV_ERR_VERSION(_driverFile);

	drv.close();

	byte *dt = new byte[1536]();
	_ditheringTable = dt;

	for (uint16 i = 0; i < 256; ++i) {
		for (int ii = 0; ii < 6; ++ii)
			*dt++ = (dmx[(i >> 4) * 6 + ii] & 0xCC) | (dmx[(i & 0x0f) * 6 + ii] & 0x33);
	}

	delete[] dmx;

	_textAlignX = 1;

	byte *col = new byte[8 * 3]();
	_convPalette = col;

	for (uint8 i = 0; i < 8; ++i) {
		*col++ = (i & 2) ? 0xff : 0;
		*col++ = (i & 1) ? 0xff : 0;
		*col++ = (i & 4) ? 0xff : 0;
	}
}

SCI1_PC98Gfx8ColorsDriver::~SCI1_PC98Gfx8ColorsDriver() {
	delete[] _ditheringTable;
	delete[] _convPalette;
}

void renderPlanarMatrix(byte *dst, const byte *src, int pitch, int w, int h, const byte *tbl) {
	int dstPitch = pitch << 1;
	byte *d1 = dst;
	byte *d2 = d1 + dstPitch;
	pitch -= w;
	dstPitch += (pitch << 1);

	while (h--) {
		byte sh = 0;
		for (int i = 0; i < (w >> 1); ++i) {
			const byte *c = &tbl[(src[0] << 4 | src[1]) * 6];
			for (int ii = sh; ii < sh + 4; ++ii) {
				*d1++ = (((c[0] >> (7 - ii)) & 1) << 2) | (((c[1] >> (7 - ii)) & 1) << 1) | ((c[2] >> (7 - ii)) & 1);
				*d2++ = (((c[3] >> (7 - ii)) & 1) << 2) | (((c[4] >> (7 - ii)) & 1) << 1) | ((c[5] >> (7 - ii)) & 1);
			}
			src += 2;
			sh ^= 4;
		}

		src += pitch;
		d1 += dstPitch;
		d2 += dstPitch;
	}
}

void SCI1_PC98Gfx8ColorsDriver::initScreen(const Graphics::PixelFormat *format) {
	UpscaledGfxDriver::initScreen(format);

	_renderGlyph = &SciGfxDrvInternal::renderPC98GlyphFat;

	assert(_convPalette);
	GfxDefaultDriver::setPalette(_convPalette, 0, 8, true, nullptr, nullptr);
}

void SCI1_PC98Gfx8ColorsDriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	assert (h >= 0 && w >= 0);

	byte diff = srcX & 7;
	srcX &= ~7;
	destX &= ~7;
	w = (w + diff + 7) & ~7;

	src += (srcY * pitch + srcX * _srcPixelSize);
	if (src != _currentBitmap)
		SciGfxDrvInternal::updateBitmapBuffer(_currentBitmap, _virtualW * _srcPixelSize, src, pitch, destX * _srcPixelSize, destY, w * _srcPixelSize, h);

	byte *scb = _scaledBitmap + (destY << 1) * _screenW * _srcPixelSize + (destX << 1) * _srcPixelSize;
	renderPlanarMatrix(scb, src, pitch, w, h, _ditheringTable);

	updateScreen(destX << 1, destY << 1,  w << 1, h << 1, palMods, palModMapping);
}

void SCI1_PC98Gfx8ColorsDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	adjustCursorBuffer(w << 1, h << 1);
	const byte *s = reinterpret_cast<const byte*>(cursor);
	byte *d1 = _compositeBuffer;
	uint32 newKeyColor = 0xFF;

	int dstPitch = (w << 1);
	byte *d2 = _compositeBuffer + dstPitch;

	for (uint i = 0; i < h; ++i) {
		for (uint ii = 0; ii < w; ++ii) {
			byte col = *s++;
			if (col == keycolor) {
				*d1++ = *d2++ = newKeyColor;
				*d1++ = *d2++ = newKeyColor;
			} else {
				*d1++ = *d2++ = (col & 7);
				*d1++ = *d2++ = (col & 8) ? (col & 7) : 0;
			}
		}
		d1 += dstPitch;
		d2 += dstPitch;
	}

	CursorMan.replaceCursor(_compositeBuffer, w << 1, h << 1, hotspotX << 1, hotspotY << 1, newKeyColor);
}

byte SCI1_PC98Gfx8ColorsDriver::remapTextColor(byte) const {
	// Always black. The driver opcode actually has a pen color argument, but it isn't put to any use. Not sure if it is intended.
	return 0;
}

const char *SCI1_PC98Gfx8ColorsDriver::_driverFile = "9801V8.DRV";

SCI_GFXDRV_VALIDATE_IMPL(SCI1_PC98Gfx8Colors)

GfxDriver *SCI1_PC98Gfx8ColorsDriver_create(int rgbRendering, ...) {
	return new SCI1_PC98Gfx8ColorsDriver(rgbRendering != 0);
}

} // End of namespace Sci
