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

#ifndef SCI_GRAPHICS_GFXDRIVERS_H
#define SCI_GRAPHICS_GFXDRIVERS_H

#include "common/platform.h"
#include "common/rect.h"
#include "graphics/pixelformat.h"

namespace Graphics {
	class Cursor;
}

namespace Sci {

struct PaletteMod;

class GfxDriver {
public:
	enum DrawFlags : uint32 {
		kHiResMode		=	1 << 0,
		kMovieMode		=	1 << 1
	};

	GfxDriver(uint16 screenWidth, uint16 screenHeight, int numColors) : _screenW(screenWidth), _screenH(screenHeight), _numColors(numColors), _ready(false), _pixelSize(1) {}
	virtual ~GfxDriver() {}
	virtual void initScreen(const Graphics::PixelFormat *srcRGBFormat = nullptr) = 0; // srcRGBFormat: expect incoming data to have the specified rgb pixel format (used for Mac hicolor videos)
	virtual void setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) = 0;
	virtual void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) = 0;
	virtual void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) = 0;
	virtual void replaceMacCursor(const Graphics::Cursor *cursor) = 0;
	virtual Common::Point getMousePos() const;
	virtual void setMousePos(const Common::Point &pos) const;
	virtual void setShakePos(int shakeXOffset, int shakeYOffset) const;
	virtual void clearRect(const Common::Rect &r) const;
	virtual void copyCurrentBitmap(byte *dest, uint32 size) const = 0;
	virtual void copyCurrentPalette(byte *dest, int start, int num) const;
	virtual void drawTextFontGlyph(const byte *src, int pitch, int hiresDestX, int hiresDestY, int hiresW, int hiresH, int transpColor, const PaletteMod *palMods, const byte *palModMapping) = 0;
	virtual byte remapTextColor(byte color) const { return color; }
	virtual void setColorMap(const byte *colorMap) {}
	virtual Common::Point getRealCoords(Common::Point &pos) const { return pos; }
	virtual void setFlags(uint32 flags) {}
	virtual void clearFlags(uint32 flags) {}
	virtual bool supportsPalIntensity() const = 0;
	virtual bool supportsHiResGraphics() const = 0;
	virtual bool driverBasedTextRendering() const = 0;
	uint16 numColors() const { return _numColors; }
	byte pixelSize() const { return _pixelSize; }

protected:
	bool _ready;
	static bool checkDriver(const char *const *driverNames, int listSize);
	const uint16 _screenW;
	const uint16 _screenH;
	uint16 _numColors;
	byte _pixelSize;
};

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

class SCI0_CGADriver final : public SCI0_DOSPreVGADriver {
public:
	SCI0_CGADriver(bool emulateCGAModeOnEGACard, bool rgbRendering);
	~SCI0_CGADriver() override;
	void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod*, const byte*) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformDOS) && checkDriver(&_driverFile, 1); }
private:
	void setupRenderProc() override;
	uint16 *_cgaPatterns;
	byte _palette[12];
	const bool _disableMode5;
	typedef void (*LineProc)(byte*&, const byte*, int, int, int, const uint16*, const byte*);
	LineProc _renderLine;
	static const char *_driverFile;
};

class SCI0_CGABWDriver final : public SCI0_DOSPreVGADriver {
public:
	SCI0_CGABWDriver(uint32 monochromeColor, bool rgbRendering);
	~SCI0_CGABWDriver() override;
	void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod*, const byte*) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	Common::Point getMousePos() const override;
	void setMousePos(const Common::Point &pos) const override;
	void setShakePos(int shakeXOffset, int shakeYOffset) const override;
	void clearRect(const Common::Rect &r) const override;
	Common::Point getRealCoords(Common::Point &pos) const override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformDOS) && checkDriver(_driverFiles, 2); }
private:
	void setupRenderProc() override;
	byte _monochromePalette[6];
	const byte *_monochromePatterns;
	bool _earlyVersion;
	typedef void (*LineProc)(byte*&, const byte*, int, int, int, const byte*, const byte*);
	LineProc _renderLine;
	static const char *_driverFiles[2];
};

class SCI0_HerculesDriver final : public SCI0_DOSPreVGADriver {
public:
	SCI0_HerculesDriver(uint32 monochromeColor, bool rgbRendering, bool cropImage);
	~SCI0_HerculesDriver() override;
	void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod*, const byte*) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	Common::Point getMousePos() const override;
	void setMousePos(const Common::Point &pos) const override;
	void setShakePos(int shakeXOffset, int shakeYOffset) const override;
	void clearRect(const Common::Rect &r) const override;
	Common::Point getRealCoords(Common::Point &pos) const override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformDOS) && checkDriver(&_driverFile, 1); }
private:
	void setupRenderProc() override;
	const uint16 _centerX;
	const uint16 _centerY;
	byte _monochromePalette[6];
	const byte *_monochromePatterns;
	typedef void (*LineProc)(byte*&, const byte*, int, int, int, const byte*, const byte*);
	LineProc _renderLine;
	static const char *_driverFile;
};

class SCI1_VGAGreyScaleDriver final : public GfxDefaultDriver {
public:
	SCI1_VGAGreyScaleDriver(bool rgbRendering);
	~SCI1_VGAGreyScaleDriver() override;
	void setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformDOS || p == Common::kPlatformWindows) && checkDriver(&_driverFile, 1); }
private:
	byte *_greyScalePalette;
	static const char *_driverFile;
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
	byte *_compositeBuffer;
	byte *_currentPalette;
private:
	virtual void loadData();
	virtual void renderBitmap(byte *dst, const byte *src, int pitch, int y, int w, int h, const byte *patterns, const byte *palette, uint16 &realWidth, uint16 &realHeight);
	byte *_currentBitmap;
	uint8 _colAdjust;
	const byte *_internalPalette;
	const bool _requestRGBMode;
	static const char *_driverFile;
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
	void drawTextFontGlyph(const byte *src, int pitch, int hiresDestX, int hiresDestY, int hiresW, int hiresH, int transpColor, const PaletteMod *palMods, const byte *palModMapping) override; // For HiRes fonts. PC-98 versions bypass the video driver for this and render directly on top of the vram.
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

class KQ6WinGfxDriver final : public UpscaledGfxDriver {
public:
	KQ6WinGfxDriver(bool dosStyleCursors, bool smallWindow, bool rgbRendering);
	~KQ6WinGfxDriver() override {}
	void initScreen(const Graphics::PixelFormat *format) override;
	void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	Common::Point getRealCoords(Common::Point &pos) const override;
	void setColorMap(const byte *colorMap) override { _colorMap = colorMap; }
	void setFlags(uint32 flags) override;
	void clearFlags(uint32 flags) override;
	bool supportsHiResGraphics() const override { return !_smallWindow; }
protected:
	typedef void (*LineProc)(byte*&, const byte*, int, int, int);
	LineProc _renderLine;
private:
	typedef void (*LineProcSpec)(byte*&, const byte*, int, int, const byte*);
	LineProcSpec _renderLine2;
	void renderBitmap(const byte *src, int pitch, int dx, int dy, int w, int h, int &realWidth, int &realHeight) override;
	uint32 _flags;
	const byte *_colorMap;
	const bool _smallWindow;
	const bool _dosStyleCursors;
	uint16 _vScaleMult2;
};

class KQ6WinGfx16ColorsDriver final : public SCI1_EGADriver {
public:
	// The original does not take into account the extra lines required for the 200->440 vertical scaling. There is a noticeable dithering glitch every 11th line, as the
	// two pixels of the checkerbox pattern appear in the wrong order. I have implemented a fix for this which can be activated with the fixDithering parameter.
	KQ6WinGfx16ColorsDriver(bool altCursor, bool fixDithering, bool rgbRendering);
	~KQ6WinGfx16ColorsDriver() override;
	void initScreen(const Graphics::PixelFormat *format) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	Common::Point getRealCoords(Common::Point &pos) const override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformDOS || p == Common::kPlatformWindows); }
private:
	void loadData() override;
	void renderBitmap(byte *dst, const byte *src, int pitch, int y, int w, int h, const byte *patterns, const byte *palette, uint16 &realWidth, uint16 &realHeight) override;
	LineProc _renderLine2;
	const bool _enhancedDithering;
	const bool _altCursor;
	static const byte _win16ColorsDitherPatterns[512];
};

class PC98Gfx16ColorsDriver final : public UpscaledGfxDriver {
public:
	enum SjisFontStyle {
		kFontStyleNone,
		kFontStyleTextMode,
		kFontStyleSpecialSCI1
	};

	PC98Gfx16ColorsDriver(int textAlignX, bool cursorScaleWidth, bool cursorScaleHeight, SjisFontStyle sjisFontStyle, bool rgbRendering, bool needsUnditheringPalette);
	~PC98Gfx16ColorsDriver() override;
	void initScreen(const Graphics::PixelFormat *format) override;
	void setPalette(const byte*, uint, uint, bool, const PaletteMod*, const byte*) override {}
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	byte remapTextColor(byte color) const override;
private:
	const byte *_convPalette;
	const byte *_textModePalette;
	const bool _cursorScaleHeightOnly;
	SjisFontStyle _fontStyle;
};

class SCI0_PC98Gfx8ColorsDriver final : public UpscaledGfxDriver {
public:
	SCI0_PC98Gfx8ColorsDriver(bool cursorScaleHeight, bool useTextModeForSJISChars, bool rgbRendering);
	~SCI0_PC98Gfx8ColorsDriver() override;
	void initScreen(const Graphics::PixelFormat *format) override;
	void setPalette(const byte*, uint, uint, bool, const PaletteMod*, const byte*) override {}
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	byte remapTextColor(byte color) const override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformPC98) && checkDriver(_driverFiles, 2); }
private:
	const byte *_convPalette;
	const bool _cursorScaleHeightOnly;
	const bool _useTextMode;
	static const char *_driverFiles[2];
};

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

} // End of namespace Sci

#endif // SCI_GRAPHICS_GFXDRIVERS_H
