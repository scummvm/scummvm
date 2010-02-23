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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_GRAPHICS_GUI32_H
#define SCI_GRAPHICS_GUI32_H

#include "sci/graphics/helpers.h"

namespace Sci {

class GfxCursor;
class GfxScreen;
class GfxPalette;
class GfxCache;
class GfxCoordAdjuster32;
class GfxCompare;
class GfxFrameout;
class GfxPaint32;

class SciGui32 {
public:
	SciGui32(SegManager *segMan, SciEvent *event, GfxScreen *screen, GfxPalette *palette, GfxCache *cache, GfxCursor *cursor);
	~SciGui32();

	void init();

	void textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);

	void drawRobot(GuiResourceId robotId);

protected:
	GfxCursor *_cursor;
	GfxScreen *_screen;
	GfxPalette *_palette;
	GfxCache *_cache;
	GfxCoordAdjuster32 *_coordAdjuster;
	GfxCompare *_compare;
	GfxFrameout *_frameout;
	GfxPaint32 *_paint32;

private:
};

} // End of namespace Sci

#endif
