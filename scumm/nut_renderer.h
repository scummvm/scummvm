/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef NUT_RENDERER_H
#define NUT_RENDERER_H

#include "common/file.h"

namespace Scumm {

class ScummEngine;

class NutRenderer {
protected:
	ScummEngine *_vm;
	bool _initialized;
	bool _loaded;
	int _nbChars;
	struct {
		int xoffs;
		int yoffs;
		int width;
		int height;
		byte *src;
	} _chars[256];

	int32 decodeCodec44(byte *dst, const byte *src, uint32 length);

	void drawChar(byte *dst, byte *mask, byte c, int x, int y, byte color);
	void draw2byte(byte *dst, byte *mask, int c, int x, int y, byte color);

public:
	NutRenderer(ScummEngine *vm);
	virtual ~NutRenderer();
	int getNbChars() { return _nbChars; }

	bool loadFont(const char *filename, const char *dir);

	void drawFrame(byte *dst, int c, int x, int y);
	void drawShadowChar(int c, int x, int y, byte color, bool useMask);

	int getCharWidth(byte c);
	int getCharHeight(byte c);
};

} // End of namespace Scumm

#endif
