/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
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
	bool _bitmapFont;
	int _numChars;
	byte *_decodedData;
	struct {
		uint16 width;
		uint16 height;
		byte *src;
	} _chars[256];

	void codec1(byte *dst, const byte *src, int width, int height, int pitch);
	void codec21(byte *dst, const byte *src, int width, int height, int pitch);

	void drawChar(const Graphics::Surface &s, byte c, int x, int y, byte color);
	void draw2byte(const Graphics::Surface &s, int c, int x, int y, byte color);

	void loadFont(const char *filename);

public:
	NutRenderer(ScummEngine *vm, const char *filename, bool bitmap);
	virtual ~NutRenderer();
	int getNumChars() const { return _numChars; }

	void drawFrame(byte *dst, int c, int x, int y);
	void drawShadowChar(const Graphics::Surface &s, int c, int x, int y, byte color, bool showShadow);

	int getCharWidth(byte c) const;
	int getCharHeight(byte c) const;
};

} // End of namespace Scumm

#endif
