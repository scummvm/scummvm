/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef KYRA_SCREEN_EOB_H
#define KYRA_SCREEN_EOB_H

#ifdef ENABLE_EOB

#include "kyra/graphics/screen.h"

namespace Kyra {

class EoBCoreEngine;
class SegaRenderer;
class SegaAnimator;
class Screen_EoB : public Screen {
friend class SegaRenderer;
public:
	// The purpose of this enum is to keep better track of which page is used
	// when and for which purpose. We use the pages for more backup operations
	// than the original and also have to deal with the different ports which
	// all do their own things. This is supposed to help avoid using pages that
	// are already in use for something else. It also allows for quick fixes
	// if necessary.
	enum {
		kSegaInitShapesPage		=	7,
		kSegaRenderPage			=	8,
		kDefeatMsgBackupPage	=	10,
		kCheckPwBackupPage		=	10,
		kSpellbookBackupPage	=	10,
		kEoB2ScriptHelperPage	=	12,
		kCampMenuBackupPage		=	12
	};

public:
	Screen_EoB(EoBCoreEngine *vm, OSystem *system);
	~Screen_EoB() override;

	bool init() override;

	void setClearScreenDim(int dim);
	void clearCurDim();
	void clearCurDimOvl(int pageNum);

	void setMouseCursor(int x, int y, const byte *shape) override;
	void setMouseCursor(int x, int y, const byte *shape, const uint8 *ovl);

	void loadFileDataToPage(Common::SeekableReadStream *s, int pageNum, uint32 size);

	void printShadedText(const char *string, int x, int y, int col1, int col2, int shadowCol, int pitch = -1);

	void loadBitmap(const char *filename, int tempPage, int dstPage, Palette *pal, bool skip = false) override;
	void loadEoBBitmap(const char *file, const uint8 *cgaMapping, int tempPage, int destPage, int convertToPage);
	void loadShapeSetBitmap(const char *file, int tempPage, int destPage);

	void convertPage(int srcPage, int dstPage, const uint8 *cgaMapping);

	void setScreenPalette(const Palette &pal) override;
	void getRealPalette(int num, uint8 *dst) override;

	uint8 *encodeShape(uint16 x, uint16 y, uint16 w, uint16 h, bool encode8bit = false, const uint8 *cgaMapping = 0);
	void drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd = -1, int flags = 0, ...) override;
	const uint8 *scaleShape(const uint8 *shapeData, int blockDistance);
	const uint8 *scaleShapeStep(const uint8 *shp);
	const uint8 *generateShapeOverlay(const uint8 *shp, const uint8 *fadingTable);

	void setShapeFrame(int x1, int y1, int x2, int y2);
	void enableShapeBackgroundFading(bool enable);
	void setShapeFadingLevel(int val);

	void setGfxParameters(int x, int y, int col);
	void drawExplosion(int scale, int radius, int numElements, int stepSize, int aspectRatio, const uint8 *colorTable, int colorTableSize);
	void drawVortex(int numElements, int radius, int stepSize, int, int disorder, const uint8 *colorTable, int colorTableSize);

	void fadeTextColor(Palette *pal, int color, int rate);
	bool delayedFadePalStep(Palette *fadePal, Palette *destPal, int rate);

	void setTextColorMap(const uint8 *cmap) override {}
	int getRectSize(int w, int h) override;

	void setFadeTable(const uint8 *table);
	void createFadeTable(const uint8 *palData, uint8 *dst, uint8 rootColor, uint8 weight);
	void createFadeTable16bit(const uint16 *palData, uint16 *dst, uint16 rootColor, uint8 weight);

	const uint16 *getCGADitheringTable(int index);
	const uint8 *getEGADitheringTable();

	bool loadFont(FontId fontId, const char *filename) override;

	// FM-Towns specific
	void decodeSHP(const uint8 *data, int dstPage);
	void convertToHiColor(int page);
	void shadeRect(int x1, int y1, int x2, int y2, int shadingLevel);

	// PC-98 specific
	void selectPC98Palette(int palID, Palette &dest, int brightness = 0, bool set = false);
	void decodeBIN(const uint8 *src, uint8 *dst, uint16 inSize);
	void decodePC98PlanarBitmap(uint8 *srcDstBuffer, uint8 *tmpBuffer, uint16 size = 64000);

	struct PalCycleData {
		const int8 *data;
		uint8 delay;
	};

	void initPC98PaletteCycle(int palID, PalCycleData *data);
	void updatePC98PaletteCycle(int brightness);

	PalCycleData *_activePalCycle;
	uint8 *_cyclePalette;

	// Amiga specific
	void loadSpecialAmigaCPS(const char *fileName, int destPage, bool isGraphics);
	// This is a simple way of emulating the Amiga copper list palette magic for more than 32 colors.
	// I use colors 32 to 63 for these extra colors (which the Amiga copper sends to the color
	// registers on the fly at vertical beam position 120).
	void setDualPalettes(Palette &top, Palette &bottom);

	// SegaCD specific
	void sega_initGraphics();
	void sega_selectPalette(int srcPalID, int dstPalID, bool set = false);
	void sega_loadCustomPaletteData(Common::ReadStream *in);
	void sega_updatePaletteFaders(int palID);
	void sega_fadePalette(int delay, int16 brEnd, int dstPalID = -1, bool waitForCompletion = true, bool noUpdate = false);
	void sega_fadeToBlack(int delay) { sega_fadePalette(delay, -7); }
	void sega_fadeToWhite(int delay) { sega_fadePalette(delay, 7); }
	void sega_fadeToNeutral(int delay) { sega_fadePalette(delay, 0); }
	void sega_paletteOps(int16 opPal, int16 par1, int16 par2);
	void sega_setTextBuffer(uint8 *buffer, uint32 bufferSize);
	void sega_clearTextBuffer(uint8 col);
	void sega_loadTextBackground(const uint8 *src, uint16 size);
	void sega_drawTextBox(int pW, int pH, int x, int y, int w, int h, uint8 color1, uint8 color2);
	void sega_loadTextBufferToVRAM(uint16 srcOffset, uint16 addr, int size);
	void sega_gfxScale(uint8 *out, uint16 w, uint16 h, uint16 pitch, const uint8 *in, const uint16 *stampMap, const uint16 *traceVectors);
	void sega_drawClippedLine(int pW, int pH, int x, int y, int w, int h, uint8 color);
	uint8 *sega_convertShape(const uint8 *src, int w, int h, int pal, int hOffs = 0);
	void sega_encodeShapesFromSprites(const uint8 **dst, const uint8 *src, int numShapes, int w, int h, int pal, bool removeSprites = true);

	SegaRenderer *sega_getRenderer() const { return _segaRenderer; }
	SegaAnimator *sega_getAnimator() const { return _segaAnimator; }

private:
	void updateDirtyRects() override;
	void ditherRect(const uint8 *src, uint8 *dst, int dstPitch, int srcW, int srcH, int colorKey = -1);

	void drawShapeSetPixel(uint8 *dst, uint8 col);
	void scaleShapeProcessLine2Bit(uint8 *&shpDst, const uint8 *&shpSrc, uint32 transOffsetDst, uint32 transOffsetSrc);
	void scaleShapeProcessLine4Bit(uint8 *&dst, const uint8 *&src);
	bool posWithinRect(int posX, int posY, int x1, int y1, int x2, int y2);

	void setPagePixel16bit(int pageNum, int x, int y, uint16 color);

	void generateEGADitheringTable(const Palette &pal);
	void generateCGADitheringTables(const uint8 *mappingData);

	int _dsDiv, _dsRem, _dsScaleTrans;
	uint8 *_cgaScaleTable;
	int16 _gfxX, _gfxY;
	uint16 _gfxCol;
	const uint8 *_gfxMaxY;

	int16 _dsX1, _dsX2, _dsY1, _dsY2;

	bool _dsBackgroundFading;
	int16 _dsBackgroundFadingXOffs;
	uint8 _shapeOverlay[16];

	uint8 *_dsTempPage;
	uint8 *_shpBuffer;
	uint8 *_convertHiColorBuffer;

	uint16 *_cgaDitheringTables[2];
	const uint8 *_cgaMappingDefault;

	uint8 *_egaDitheringTable;
	uint8 *_egaDitheringTempPage;

	Common::String _cpsFilePattern;

	const uint16 _cursorColorKey16Bit;

	static const uint8 _egaMatchTable[];
	static const ScreenDim _screenDimTable[];
	static const int _screenDimTableCount;

	// SegaCD specific
	struct PaletteFader {
		PaletteFader() : _brCur(0), _brDest(0), _fadeIncr(0), _fadeDelay(0), _fadeTimer(0), _needRefresh(false) {}
		int16 _brCur;
		int16 _brDest;
		int16 _fadeIncr;
		int16 _fadeDelay;
		int16 _fadeTimer;
		bool _needRefresh;
	};

	PaletteFader *_palFaders;
	bool _specialColorReplace;
	SegaRenderer *_segaRenderer;
	SegaAnimator *_segaAnimator;
	uint16 _segaCurPalette[64];
	uint16 *_segaCustomPalettes;
	uint8 *_defaultRenderBuffer;
	int _defaultRenderBufferSize;
};

/**
* Implementation of the Font interface for old DOS fonts used
* in EOB and EOB II.
*
*/
class OldDOSFont : public Font {
public:
	OldDOSFont(Common::RenderMode mode, uint8 shadowColor);
	~OldDOSFont() override;

	bool load(Common::SeekableReadStream &file) override;
	Type getType() const override { return kASCII; }
	int getHeight() const override { return _height; }
	int getWidth() const override { return _width; }
	int getCharWidth(uint16 c) const override;
	void setColorMap(const uint8 *src) override;
	void set16bitColorMap(const uint16 *src) override { _colorMap16bit = src; }
	void setStyles(int styles) override { _style = styles; }
	void drawChar(uint16 c, byte *dst, int pitch, int bpp) const override;

protected:
	void unload();

	int _style;
	const uint8 *_colorMap8bit;
	uint8 *_data;
	uint16 *_bitmapOffsets;
	int _width, _height;
	int _numGlyphs;
	uint8 _shadowColor;

private:
	void drawCharIntern(uint16 c, byte *dst, int pitch, int bpp, int col1, int col2) const;
	virtual uint16 convert(uint16 c) const;
	Common::RenderMode _renderMode;
	const uint16 *_colorMap16bit;

	static uint16 *_cgaDitheringTable;
	static int _numRef;
};

/**
 * Implementation of the Font interface for native AmigaDOS system fonts (normally to be loaded via diskfont.library)
 */
class Resource;
class AmigaDOSFont : public Font {
public:
	AmigaDOSFont(Resource *res, bool needsLocalizedFont = false);
	~AmigaDOSFont() override { unload(); }

	bool load(Common::SeekableReadStream &file) override;
	Type getType() const override { return kASCII; }
	int getHeight() const override { return _height; }
	int getWidth() const override { return _width; }
	int getCharWidth(uint16 c) const override;
	void setColorMap(const uint8 *src) override { _colorMap = src; }
	void drawChar(uint16 c, byte *dst, int pitch, int) const override;

	static void errorDialog(int index);

private:
	void unload();

	struct TextFont {
		TextFont() : data(0), bitmap(0), location(0), spacing(0), kerning(0), height(0), width(0), baseLine(0), firstChar(0), lastChar(0), modulo(0) {}
		~TextFont() {
			delete[] data;
		}

		uint16 height;
		uint16 width;
		uint16 baseLine;
		uint8 firstChar;
		uint8 lastChar;
		uint16 modulo;
		const uint8 *data;
		const uint8 *bitmap;
		const uint16 *location;
		const int16 *spacing;
		const int16 *kerning;
	};

	TextFont *loadContentFile(const Common::String fileName);
	void selectMode(int mode);

	struct FontContent {
		FontContent() : height(0), style(0), flags(0) {}
		~FontContent() {
			data.reset();
		}

		Common::String contentFile;
		Common::SharedPtr<TextFont> data;
		uint16 height;
		uint8 style;
		uint8 flags;
	};

	int _width, _height;
	uint8 _first, _last;
	FontContent *_content;
	uint16 _numElements;
	uint16 _selectedElement;

	const uint8 *_colorMap;
	const uint16 _maxPathLen;
	const bool _needsLocalizedFont;

	static uint8 _errorDialogDisplayed;

	Resource *_res;
};

/**
* SJIS Font variant used in EOB I PC-98. It converts 1-byte characters into 2-byte characters and has a specific shadowing to the left.
*/
class SJISFontEoB1PC98 : public SJISFont {
public:
	SJISFontEoB1PC98(Common::SharedPtr<Graphics::FontSJIS> &font, /*uint8 shadowColor,*/ const uint16 *convTable1, const uint16 *convTable2);
	~SJISFontEoB1PC98() override {}
	int getCharWidth(uint16 c) const override;
	void drawChar(uint16 c, byte *dst, int pitch, int) const override;

private:
	uint16 convert(uint16 c) const;
	const uint16 *_convTable1, *_convTable2;
	bool _defaultConv;
	/*uint8 _shadowColor;*/
};

/**
* OldDOSFont variant used in EOB I PC-98. It uses the same drawing routine, but has a different loader. It contains
* ASCII and Katakana characters and requires several conversion tables to display these. It gets drawn on the SJIS overlay.
*/
class Font12x12PC98 : public OldDOSFont{
public:
	Font12x12PC98(uint8 shadowColor, const uint16 *convTable1, const uint16 *convTable2, const uint8 *lookupTable);
	~Font12x12PC98() override;
	bool usesOverlay() const override { return true; }
	Type getType() const override { return kSJIS; }
	int getHeight() const override { return _height >> 1; }
	int getWidth() const override { return _width >> 1; }
	int getCharWidth(uint16 c) const override { return _width >> 1; };
	bool load(Common::SeekableReadStream &file) override;

private:
	uint16 convert(uint16 c) const override;
	const uint16 *_convTable1, *_convTable2;
	uint16 *_bmpOffs;
};

/**
* SJIS Font variant used in the intro and outro of EOB II FM-Towns. It appears twice as large, since it is not rendered on the hires overlay pages.
*/
class SJISFontLarge : public SJISFont {
public:
	SJISFontLarge(Common::SharedPtr<Graphics::FontSJIS> &font);
	~SJISFontLarge() override {}

	bool usesOverlay() const override { return false; }
	void drawChar(uint16 c, byte *dst, int pitch, int) const override;
};

/**
* 12 x 12 SJIS font for EOB II FM-Towns. The data for this font comes from a file, not from the font rom.
*/
class SJISFont12x12 : public Font {
public:
	SJISFont12x12(const uint16 *searchTable);
	~SJISFont12x12() override { unload(); }

	bool load(Common::SeekableReadStream &file) override;
	Type getType() const override { return kSJIS; }
	bool usesOverlay() const override { return true; }
	int getHeight() const override { return _height; }
	int getWidth() const override { return _width; }
	int getCharWidth(uint16 c) const override { return _width; }
	void setColorMap(const uint8 *src) override { _colorMap = src; }
	void drawChar(uint16 c, byte *dst, int pitch, int) const override;

private:
	void unload();

	uint8 *_data;
	Common::HashMap<uint16, uint8> _searchTable;

	const uint8 *_colorMap;
	const int _height, _width;
};

class SegaCDFont : public Font {
public:
	SegaCDFont(Common::Language lang, const uint16 *convTable1, const uint16 *convTable2, const uint8 *widthTable1, const uint8 *widthTable2, const uint8 *widthTable3);
	~SegaCDFont() override;

	bool load(Common::SeekableReadStream &file) override;
	Type getType() const override { return _forceOneByte ? kASCII : kSJIS; }
	int getHeight() const override { return _height; }
	int getWidth() const override { return _width; }
	int getCharWidth(uint16 c) const override;
	int getCharHeight(uint16 c) const override;
	void setStyles(int styles) override;
	void setColorMap(const uint8 *src) override { _colorMap = src; }
	void drawChar(uint16 c, byte *dst, int pitch, int bpp) const override { drawChar(c, dst, pitch, 0, 0); }
	void drawChar(uint16 c, byte *dst, int pitch, int xOffs, int yOffs) const override;

private:
	const uint8 *getGlyphData(uint16 c, uint8 &charWidth, uint8 &charHeight, uint8 &pitch) const;

	const uint8 *_data;
	const uint8 *_buffer;
	bool _forceTwoByte;
	bool _forceOneByte;
	Common::Language _lang;
	uint8 _style;

	const uint8 *_colorMap;
	const int _height, _width;
	const uint16 *_convTable1, *_convTable2;
	const uint8 *_widthTable1, *_widthTable2, *_widthTable3;
};

} // End of namespace Kyra

#endif // ENABLE_EOB

#endif
