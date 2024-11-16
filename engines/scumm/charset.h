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

#ifndef SCUMM_CHARSET_H
#define SCUMM_CHARSET_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "graphics/sjis.h"
#include "scumm/charset_v7.h"
#include "scumm/scumm.h"
#include "scumm/gfx.h"

namespace Graphics {
class Font;
}

namespace Scumm {

class ScummEngine;
class NutRenderer;
struct VirtScreen;

static inline bool checkKSCode(byte hi, byte lo) {
	//hi : xx
	//lo : yy
	if ((0xA1 > lo) || (0xFE < lo)) {
		return false;
	}
	if ((hi >= 0xB0) && (hi <= 0xC8)) {
		return true;
	}
	return false;
}

static inline bool checkSJISCode(byte c) {
	if ((c >= 0x80 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfd))
		return true;
	return false;
}

static inline bool is2ByteCharacter(Common::Language lang, byte c) {
	if (lang == Common::JA_JPN)
		return (c >= 0x80 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFD);
	else if (lang == Common::KO_KOR)
		return (c >= 0xB0 && c <= 0xD0);
	else if (lang == Common::ZH_TWN || lang == Common::ZH_CHN)
		return (c >= 0x80);
	return false;
}

class CharsetRenderer {
public:

	Common::Rect _str;

	int _top;
	int _left, _startLeft;
	int _right;

protected:
	byte _color;

public:
	bool _center;

	bool _hasMask;	// True if "removable" text is visible somewhere (should be called _hasText or so)
	VirtScreenNumber _textScreenID;	// ID of the virtual screen on which the text is visible.

	bool _blitAlso;
	bool _firstChar;
	bool _disableOffsX;

protected:
	ScummEngine *_vm;
	int32 _curId;

public:
	CharsetRenderer(ScummEngine *vm);
	virtual ~CharsetRenderer();

	virtual void printChar(int chr, bool ignoreCharsetMask) = 0;
	virtual void drawChar(int chr, Graphics::Surface &s, int x, int y) {}

	virtual int getStringWidth(int arg, const byte *text);
	void addLinebreaks(int a, byte *str, int pos, int maxwidth);
	void translateColor();

	virtual void setCurID(int32 id) = 0;
	int getCurID() { return _curId; }

	virtual int getFontHeight() const = 0;
	virtual int getCharHeight(uint16 chr) const { return getFontHeight(); }
	virtual int getCharWidth(uint16 chr) const = 0;

	virtual void setColor(byte color, bool shadowModeSpecialFlag = false) { _color = color; translateColor(); }
	virtual byte getColor() { return _color; }

	void saveLoadWithSerializer(Common::Serializer &ser);
};

class CharsetRendererCommon : public CharsetRenderer {
public:
	enum ShadowType {
		kNoShadowType,
		kNormalShadowType,
		kHorizontalShadowType,
		kOutlineShadowType
	};

	CharsetRendererCommon(ScummEngine *vm);

	void setCurID(int32 id) override;

	int getFontHeight() const override;

protected:
	const byte *_fontPtr;
	int _bitsPerPixel;
	int _fontHeight;
	int _numChars;

	byte _shadowColor;
	ShadowType _shadowType;
};

class CharsetRendererPC : public CharsetRendererCommon {
public:
	CharsetRendererPC(ScummEngine *vm) : CharsetRendererCommon(vm) { }

protected:
	virtual void setShadowMode(ShadowType mode);
	virtual void drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height);
	void drawBits1Kor(Graphics::Surface &dest, int x1, int y1, const byte *src, int drawTop, int width, int height);
};

class CharsetRendererClassic : public CharsetRendererPC {
protected:
	virtual void drawBitsN(const Graphics::Surface &s, byte *dst, const byte *src, byte bpp, int drawTop, int width, int height);
	void printCharIntern(bool is2byte, const byte *charPtr, int origWidth, int origHeight, int width, int height, VirtScreen *vs, bool ignoreCharsetMask);
	virtual bool prepareDraw(uint16 chr);

	int _width, _height, _origWidth, _origHeight;
	int _cjkSpacing;
	int _offsX, _offsY;
	const byte *_charPtr;

	// On which virtual screen will be drawn right now
	VirtScreenNumber _drawScreen;

public:
	CharsetRendererClassic(ScummEngine *vm, int cjkSpacing) : CharsetRendererPC(vm), _width(0), _height(0), _origWidth(0), _origHeight(0),
		_cjkSpacing(cjkSpacing), _offsX(0), _offsY(0), _charPtr(nullptr), _drawScreen(kMainVirtScreen) {}
	CharsetRendererClassic(ScummEngine *vm) : CharsetRendererClassic(vm, vm->_game.id == GID_INDY4 &&
									 (vm->_game.platform == Common::kPlatformMacintosh || vm->_game.platform == Common::kPlatformDOS) &&
									 vm->_language == Common::JA_JPN ? -3 : 0) {}

	void printChar(int chr, bool ignoreCharsetMask) override;
	void drawChar(int chr, Graphics::Surface &s, int x, int y) override;

	int getCharWidth(uint16 chr) const override;
};

#ifdef USE_RGB_COLOR
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
class CharsetRendererTownsClassic : public CharsetRendererClassic {
public:
	CharsetRendererTownsClassic(ScummEngine *vm);

	int getCharWidth(uint16 chr) const override;
	int getFontHeight() const override;

private:
	void drawBitsN(const Graphics::Surface &s, byte *dst, const byte *src, byte bpp, int drawTop, int width, int height) override;
	bool prepareDraw(uint16 chr) override;
	void setupShadowMode();
	bool useFontRomCharacter(uint16 chr) const;
	void processCharsetColors();

	uint16 _sjisCurChar;
};
#endif
#endif

class CharsetRendererNES : public CharsetRendererCommon {
protected:
	byte *_trTable = nullptr;

	void drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height);

public:
	CharsetRendererNES(ScummEngine *vm) : CharsetRendererCommon(vm) {}

	void setCurID(int32 id) override {}
	void printChar(int chr, bool ignoreCharsetMask) override;
	void drawChar(int chr, Graphics::Surface &s, int x, int y) override;

	int getFontHeight() const override { return 8; }
	int getCharWidth(uint16 chr) const override { return 8; }
};

class CharsetRendererV3 : public CharsetRendererPC {
protected:
	virtual int getDrawWidthIntern(uint16 chr);
	virtual int getDrawHeightIntern(uint16 chr);
	virtual void setDrawCharIntern(uint16 chr) {}

	const byte *_widthTable = nullptr;

public:
	CharsetRendererV3(ScummEngine *vm) : CharsetRendererPC(vm) {}

	void printChar(int chr, bool ignoreCharsetMask) override;
	void drawChar(int chr, Graphics::Surface &s, int x, int y) override;
	void setCurID(int32 id) override;
	void setColor(byte color, bool shadowModeSpecialFlag) override;
	int getCharWidth(uint16 chr) const override;
};

class CharsetRendererTownsV3 : public CharsetRendererV3 {
public:
	CharsetRendererTownsV3(ScummEngine *vm);

	int getCharWidth(uint16 chr) const override;
	int getFontHeight() const override;

private:
	void setShadowMode(ShadowType mode) override;
	void drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height) override;
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	int getDrawWidthIntern(uint16 chr) override;
	int getDrawHeightIntern(uint16 chr) override;
	void setDrawCharIntern(uint16 chr) override;
#endif
	uint16 _sjisCurChar;
};

#ifdef USE_RGB_COLOR
class CharsetRendererPCE : public CharsetRendererV3 {
private:
	void drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height) override;

	int getDrawWidthIntern(uint16 chr) override;
	int getDrawHeightIntern(uint16 chr) override;
	void setDrawCharIntern(uint16 chr) override;

	uint16 _sjisCurChar;

public:
	CharsetRendererPCE(ScummEngine *vm) : CharsetRendererV3(vm), _sjisCurChar(0) {}

	void setColor(byte color, bool) override;
};
#endif

class CharsetRendererV2 : public CharsetRendererV3 {
protected:
	bool _deleteFontPtr;

public:
	CharsetRendererV2(ScummEngine *vm, Common::Language language);
	~CharsetRendererV2() override;

	void setCurID(int32 id) override {}
	int getCharWidth(uint16 chr) const override { return 8; }
};

class CharsetRendererMac : public CharsetRendererCommon {
protected:
	const Graphics::Font *_font = nullptr;
	bool _useCorrectFontSpacing;
	bool _pad;
	int _lastTop;

	int getDrawWidthIntern(uint16 chr) const;
	void printCharInternal(int chr, int color, bool shadow, int x, int y);

	byte getTextColor();
	byte getTextShadowColor();

	Graphics::Surface *_glyphSurface;

public:
	CharsetRendererMac(ScummEngine *vm, const Common::Path &fontFile);
	~CharsetRendererMac() override;

	void setCurID(int32 id) override;

	int getStringWidth(int arg, const byte *text) override;
	int getFontHeight() const override;
	int getCharWidth(uint16 chr) const override;
	void printChar(int chr, bool ignoreCharsetMask) override;
	void setColor(byte color, bool) override;
};

#ifdef ENABLE_SCUMM_7_8
class CharsetRendererV7 : public CharsetRendererClassic, public GlyphRenderer_v7 {
public:
	CharsetRendererV7(ScummEngine *vm);
	~CharsetRendererV7() override {};

	void printChar(int, bool) override { error("CharsetRendererV7::printChar(): Unexpected call to deprecated function"); }

	int draw2byte(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, uint16 chr) override;
	int drawCharV7(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags, byte chr) override;
	int getCharWidth(uint16 chr) const override;
	int getCharHeight(uint16 chr) const override { return ((chr & 0x80) && _vm->_useCJKMode) ? _vm->_2byteHeight : _fontHeight; }
	int getFontHeight() const override { return _fontHeight; }
	int setFont(int) override { return 0; }
	bool newStyleWrapping() const override { return _newStyle; }
private:
	const bool _newStyle;
	const int _direction;
};

class CharsetRendererNut : public CharsetRenderer, public GlyphRenderer_v7 {
public:
	CharsetRendererNut(ScummEngine *vm);
	~CharsetRendererNut() override;

	void printChar(int, bool) override { error("CharsetRendererNut::printChar(): Unexpected call to deprecated function"); }

	void setCurID(int32 id) override;
	int setFont(int id) override;
	bool newStyleWrapping() const override { return true; }

	int draw2byte(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, uint16 chr) override;
	int drawCharV7(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags, byte chr) override;

	int getFontHeight() const override;
	int getCharWidth(uint16 chr) const override;
	int getCharHeight(uint16 chr) const override;

private:
	NutRenderer *_fr[5];
	NutRenderer *_current;
};
#endif

} // End of namespace Scumm


#endif
