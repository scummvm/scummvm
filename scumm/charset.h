/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef CHARSET_H
#define CHARSET_H

#include "common/scummsys.h"

class Scumm;
class NutRenderer;
struct VirtScreen;

class CharsetRenderer {
public:
	int _strLeft, _strRight, _strTop, _strBottom;
	int _nextLeft, _nextTop;

	int _top;
	int _left, _startLeft;
	int _right;

	byte _color;

	bool _center;
	bool _hasMask;
	bool _ignoreCharsetMask;
	bool _blitAlso;
	bool _firstChar;
	bool _disableOffsX;

protected:
	Scumm *_vm;
	byte _curId;

	virtual int getCharWidth(byte chr) = 0;

public:
	CharsetRenderer(Scumm *vm) : _vm(vm) {}
	virtual ~CharsetRenderer() {}

	virtual void printChar(int chr) = 0;

	int getStringWidth(int a, byte *str);
	void addLinebreaks(int a, byte *str, int pos, int maxwidth);
	
	virtual void setCurID(byte id) = 0;
	int getCurID() { return _curId; }
	
	virtual int getFontHeight() = 0;
};


class CharsetRendererCommon : public CharsetRenderer {
protected:
	byte *_fontPtr;

public:
	CharsetRendererCommon(Scumm *vm) : CharsetRenderer(vm) {}

	void setCurID(byte id);
	
	int getFontHeight() { return _fontPtr[1]; }
};

class CharsetRendererClassic : public CharsetRendererCommon {
protected:
	byte _bpp;
	byte *_charPtr;

	int getCharWidth(byte chr);
	void drawBits(VirtScreen *vs, byte *dst, byte *mask, int drawTop, int width, int height);

public:
	CharsetRendererClassic(Scumm *vm) : CharsetRendererCommon(vm) {}
	
	void printChar(int chr);
};


class CharsetRendererOld256 : public CharsetRendererCommon {
protected:
	int getCharWidth(byte chr);

public:
	CharsetRendererOld256(Scumm *vm) : CharsetRendererCommon(vm) {}
	
	void printChar(int chr);
};

class CharsetRendererNut : public CharsetRenderer {
protected:
	int getCharWidth(byte chr);

	NutRenderer *_fr[4];
	NutRenderer *_current;

public:
	CharsetRendererNut(Scumm *vm);
	~CharsetRendererNut();
	
	void printChar(int chr);

	void setCurID(byte id);
	
	int getFontHeight();
};

#endif
