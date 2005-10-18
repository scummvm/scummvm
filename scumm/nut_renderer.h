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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 */

#if !defined(NUT_RENDERER_H) && !defined(DISABLE_SCUMM_7_8)
#define NUT_RENDERER_H

#include "common/file.h"
#include "graphics/surface.h"

namespace Scumm {

class ScummEngine;

class NutRenderer {
protected:
	ScummEngine *_vm;
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
	int getNumChars() const { return _numChars; }

	bool loadFont(const char *filename);

	void drawFrame(byte *dst, int c, int x, int y);
	void drawShadowChar(const Graphics::Surface &s, int c, int x, int y, byte color, bool showShadow);

	int getCharWidth(byte c) const;
	int getCharHeight(byte c) const;
};

} // End of namespace Scumm

#endif
