/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 */

#ifndef CHARSET_H
#define CHARSET_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "scumm/gfx.h"

namespace Scumm {

class ScummEngine;
class NutRenderer;
struct VirtScreen;

static inline bool checkSJISCode(byte c) {
	if ((c > 0x84 && c < 0x88) || (c > 0x9f && c < 0xe0) || (c > 0xea /* && c <= 0xff */))
		return false;
	return true;
}


class CharsetRenderer {
public:

	Common::Rect _str;
	int _nextLeft, _nextTop;

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

	/**
	 * All text is normally rendered into this overlay surface. Then later
	 * drawStripToScreen() composits it over the game graphics.
	 */
	Graphics::Surface _textSurface;

protected:
	ScummEngine *_vm;
	byte _curId;

public:
	CharsetRenderer(ScummEngine *vm);
	virtual ~CharsetRenderer();

	void restoreCharsetBg();
	void clearCharsetMask();
	void clearTextSurface();

	virtual void printChar(int chr, bool ignoreCharsetMask) = 0;
	virtual void drawChar(int chr, const Graphics::Surface &s, int x, int y) {}

	int getStringWidth(int a, const byte *str);
	void addLinebreaks(int a, byte *str, int pos, int maxwidth);
	void translateColor();

	virtual void setCurID(byte id) = 0;
	int getCurID() { return _curId; }

	virtual int getFontHeight() = 0;
	virtual int getCharHeight(byte chr) { return getFontHeight(); }
	virtual int getCharWidth(byte chr) = 0;

	virtual void setColor(byte color) { _color = color; translateColor(); }
};

class CharsetRendererCommon : public CharsetRenderer {
protected:
	byte *_fontPtr;
	int _bitDepth;
	int _fontHeight;
	int _numChars;

	enum ShadowMode {
		kNoShadowMode,
		kFMTOWNSShadowMode,
		kNormalShadowMode
	};
	byte _shadowColor;
	ShadowMode _shadowMode;

	void enableShadow(bool enable);
	void drawBits1(const Graphics::Surface &s, byte *dst, const byte *src, int drawTop, int width, int height);

public:
	CharsetRendererCommon(ScummEngine *vm);

	void setCurID(byte id);

	int getFontHeight();
};

class CharsetRendererClassic : public CharsetRendererCommon {
protected:
	void drawBitsN(const Graphics::Surface &s, byte *dst, const byte *src, byte bpp, int drawTop, int width, int height);

public:
	CharsetRendererClassic(ScummEngine *vm) : CharsetRendererCommon(vm) {}

	void printChar(int chr, bool ignoreCharsetMask);
	void drawChar(int chr, const Graphics::Surface &s, int x, int y);

	int getCharWidth(byte chr);
};

class CharsetRendererNES : public CharsetRendererCommon {
protected:
	byte *_trTable;

	void drawBits1(const Graphics::Surface &s, byte *dst, const byte *src, int drawTop, int width, int height);

public:
	CharsetRendererNES(ScummEngine *vm) : CharsetRendererCommon(vm) {}

	void setCurID(byte id) {}
	void printChar(int chr, bool ignoreCharsetMask);
	void drawChar(int chr, const Graphics::Surface &s, int x, int y);

	int getFontHeight() { return 8; }
	int getCharWidth(byte chr) { return 8; }
};

class CharsetRendererV3 : public CharsetRendererCommon {
protected:
	byte *_widthTable;

public:
	CharsetRendererV3(ScummEngine *vm) : CharsetRendererCommon(vm) {}

	void printChar(int chr, bool ignoreCharsetMask);
	void drawChar(int chr, const Graphics::Surface &s, int x, int y);
	void setCurID(byte id);
	void setColor(byte color);
	int getFontHeight();
	int getCharWidth(byte chr);
};

class CharsetRendererV2 : public CharsetRendererV3 {
public:
	CharsetRendererV2(ScummEngine *vm, Common::Language language);

	void setCurID(byte id) {}
	int getCharWidth(byte chr) { return 8; }
};

#ifndef DISABLE_SCUMM_7_8
class CharsetRendererNut : public CharsetRenderer {
protected:
	NutRenderer *_fr[5];
	NutRenderer *_current;

public:
	CharsetRendererNut(ScummEngine *vm);
	~CharsetRendererNut();

	void printChar(int chr, bool ignoreCharsetMask);

	void setCurID(byte id);

	int getFontHeight();
	int getCharHeight(byte chr);
	int getCharWidth(byte chr);
};
#endif

} // End of namespace Scumm


#endif
