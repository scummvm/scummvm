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


#ifndef SCI_GRAPHICS_DRIVERS_GFXDRIVER_INTERN_H
#define SCI_GRAPHICS_DRIVERS_GFXDRIVER_INTERN_H

#include "common/platform.h"
#include "sci/graphics/drivers/gfxdriver.h"

namespace Sci {

class GfxDefaultDriver : public GfxDriver {
public:
	GfxDefaultDriver(uint16 screenWidth, uint16 screenHeight, bool isSCI0, bool rgbRendering);
	~GfxDefaultDriver() override;
	void initScreen(const Graphics::PixelFormat *srcRGBFormat) override; // srcRGBFormat: expect incoming data to have the specified rgb pixel format (used for Mac hicolor videos)
	void setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) override;
	void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	void replaceMacCursor(const Graphics::Cursor*) override;
	void copyCurrentBitmap(byte *dest, uint32 size) const override;
	void copyCurrentPalette(byte *dest, int start, int num) const override;
	void drawTextFontGlyph(const byte*, int, int, int, int, int, int, const PaletteMod*, const byte*) override; // Only for HiRes fonts. Not implemented here.
	bool supportsPalIntensity() const override { return true; }
	bool supportsHiResGraphics() const override { return false; }
	bool driverBasedTextRendering() const override { return false; }
protected:
	void updatePalette(const byte *colors, uint start, uint num);
	byte *_compositeBuffer;
	byte *_currentBitmap;
	byte *_currentPalette;
	byte *_internalPalette;
	uint16 _virtualW;
	uint16 _virtualH;
	Graphics::PixelFormat _format;
	byte _srcPixelSize;
	bool _cursorUsesScreenPalette;
	const bool _alwaysCreateBmpBuffer;
	const bool _requestRGBMode;
	typedef void (*ColorConvProc)(byte*, const byte*, int, int, int, const byte*);
	ColorConvProc _colorConv;
	typedef void (*ColorConvModProc)(byte*, const byte*, int, int, int, const byte*, const byte*, Graphics::PixelFormat&, const PaletteMod*, const byte*);
	ColorConvModProc _colorConvMod;
private:
	void generateOutput(byte *dst, const byte *src, int pitch, int w, int h, const PaletteMod *palMods, const byte *palModMapping);
};

class SCI0_DOSPreVGADriver : public GfxDriver {
public:
	SCI0_DOSPreVGADriver(int numColors, int screenW, int screenH, bool rgbRendering);
	~SCI0_DOSPreVGADriver() override;
	void initScreen(const Graphics::PixelFormat*) override;
	void setPalette(const byte*, uint, uint, bool, const PaletteMod*, const byte*) override {}
	void replaceMacCursor(const Graphics::Cursor*) override;
	void copyCurrentBitmap(byte*, uint32) const override;
	void drawTextFontGlyph(const byte*, int, int, int, int, int, int, const PaletteMod*, const byte*) override; // Only for HiRes fonts. Not implemented here.
	void copyCurrentPalette(byte *dest, int start, int num) const override;
	bool supportsPalIntensity() const override { return false; }
	bool supportsHiResGraphics() const override { return false; }
	bool driverBasedTextRendering() const override { return false; }
protected:
	void assignPalette(const byte *colors);
	byte *_compositeBuffer;
	const byte *_internalPalette;
private:
	virtual void setupRenderProc() = 0;
	const bool _requestRGBMode;
	const byte *_colors;
};

class UpscaledGfxDriver : public GfxDefaultDriver {
public:
	UpscaledGfxDriver(int16 textAlignX, bool scaleCursor, bool rgbRendering);
	~UpscaledGfxDriver() override;
	void initScreen(const Graphics::PixelFormat *format) override;
	void setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) override;
	void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	Common::Point getMousePos() const override;
	void setMousePos(const Common::Point &pos) const override;
	void setShakePos(int shakeXOffset, int shakeYOffset) const override;
	void clearRect(const Common::Rect &r) const override;
	Common::Point getRealCoords(Common::Point &pos) const override;
	void drawTextFontGlyph(const byte *src, int pitch, int hiresDestX, int hiresDestY, int hiresW, int hiresH, int transpColor, const PaletteMod *palMods, const byte *palModMapping) override; // For HiRes fonts.
	bool driverBasedTextRendering() const override { return true; }
protected:
	UpscaledGfxDriver(uint16 scaledW, uint16 scaledH, int16 textAlignX, bool scaleCursor, bool rgbRendering);
	void updateScreen(int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping);
	void adjustCursorBuffer(uint16 newWidth, uint16 newHeight);
	typedef void (*GlyphRenderProc)(byte*, int, const byte*, int, int, int, int);
	GlyphRenderProc _renderGlyph;
	typedef void (*ScaledRenderProc)(byte*, const byte*, int, int, int);
	ScaledRenderProc _renderScaled;
	uint16 _textAlignX;
	uint16 _hScaleMult;
	uint16 _vScaleMult;
	uint16 _vScaleDiv;
	byte *_scaledBitmap;
private:
	virtual void renderBitmap(const byte *src, int pitch, int dx, int dy, int w, int h, int &realWidth, int &realHeight);
	const bool _scaleCursor;
	uint16 _cursorWidth;
	uint16 _cursorHeight;
	bool _needCursorBuffer;
};

class SCI1_EGADriver : public GfxDriver {
public:
	SCI1_EGADriver(bool rgbRendering);
	~SCI1_EGADriver() override;
	void initScreen(const Graphics::PixelFormat*) override;
	void setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod*, const byte*) override;
	void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod*, const byte*) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	void replaceMacCursor(const Graphics::Cursor *cursor) override {}
	void copyCurrentBitmap(byte *dest, uint32 size) const override;
	void copyCurrentPalette(byte *dest, int start, int num) const override;
	void drawTextFontGlyph(const byte*, int, int, int, int, int, int, const PaletteMod*, const byte*) override; // Only for HiRes fonts. Not implemented here.
	Common::Point getMousePos() const override;
	void setMousePos(const Common::Point &pos) const override;
	void setShakePos(int shakeXOffset, int shakeYOffset) const override;
	void clearRect(const Common::Rect &r) const override;
	Common::Point getRealCoords(Common::Point &pos) const override;
	bool supportsPalIntensity() const override { return false; }
	bool supportsHiResGraphics() const override { return false; }
	bool driverBasedTextRendering() const override { return false; }
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformDOS || p == Common::kPlatformWindows) && checkDriver(&_driverFile, 1); }
protected:
	typedef void (*LineProc)(byte*&, const byte*, int, const byte*, const byte*, bool);
	LineProc _renderLine;
	const byte *_convPalette;
	uint16 _vScaleMult, _vScaleDiv;
	const byte *_egaMatchTable;
	byte *_egaColorPatterns;
	uint8 _colAdjust;
	byte *_compositeBuffer;
	byte *_currentPalette;
private:
	virtual void loadData();
	virtual void renderBitmap(byte *dst, const byte *src, int pitch, int y, int w, int h, const byte *patterns, const byte *palette, uint16 &realWidth, uint16 &realHeight);
	byte *_currentBitmap;
	const byte *_internalPalette;
	const bool _requestRGBMode;
	static const char *_driverFile;
};

#define GFXDRV_ASSERT_READY \
	if (!_ready) \
		error("%s(): initScreen() must be called before using this method", __FUNCTION__)

#define GFXDRV_ERR_OPEN(x) \
	error("%s(): Failed to open '%s'", __FUNCTION__, x)

#define GFXDRV_ERR_VERSION(x) \
	error("%s(): Driver file '%s' unknown version", __FUNCTION__, x)

#define SCI_GFXDRV_VALIDATE_IMPL(name) \
	bool name##Driver_validateMode(Common::Platform platform) { \
		return name##Driver::validateMode(platform); \
	}

} // End of namespace Sci

namespace SciGfxDrvInternal {
	template <typename T> void updateRGBPalette(byte *dest, const byte *src, uint start, uint num, Graphics::PixelFormat &f);
	template <typename T> void scale2x(byte *dst, const byte *src, int pitch, int w, int h);
	void updateBitmapBuffer(byte *dst, int dstPitch, const byte *src, int srcPitch, int x, int y, int w, int h);
	byte findColorInPalette(uint32 rgbTriplet, const byte *palette, int numColors);
	void renderWinMonochromeCursor(byte *dst, const void *src, const byte *palette, uint &w, uint &h, int &hotX, int &hotY, byte blackColor, byte whiteColor, uint32 &keycolor, bool noScale);
	void renderPC98GlyphFat(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h, int transpCol);
	const byte *monochrInit(const char *drvFile, bool &earlyVersion);

} // End of namespace SciGfxDrvInternal

#endif // SCI_GRAPHICS_DRIVERS_GFXDRIVER_INTERN_H
