/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
#include "graphics/surface.h"

namespace Scumm {

class ScummEngine;

class NutRenderer {
protected:
	ScummEngine *_vm;
	bool _initialized;
	bool _loaded;
	int _numChars;
	struct {
		int xoffs;
		int yoffs;
		int width;
		int height;
		byte *src;
	} _chars[256];

	void drawChar(const Graphics::Surface &s, byte c, int x, int y, byte color);
	void draw2byte(const Graphics::Surface &s, int c, int x, int y, byte color);

public:
	NutRenderer(ScummEngine *vm);
	virtual ~NutRenderer();
	int getNumChars() { return _numChars; }

	bool loadFont(const char *filename);

	void drawFrame(byte *dst, int c, int x, int y);
	void drawShadowChar(const Graphics::Surface &s, int c, int x, int y, byte color, bool showShadow);

	int getCharWidth(byte c) const;
	int getCharHeight(byte c) const;
};

} // End of namespace Scumm

#endif
