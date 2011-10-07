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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/sci/graphics/text16.h $
 * $Id: text16.h 55178 2011-01-08 23:16:44Z thebluegr $
 *
 */

#ifndef SCI_GRAPHICS_TEXT32_H
#define SCI_GRAPHICS_TEXT32_H

#include "common/hashmap.h"

namespace Sci {

struct TextEntry {
	reg_t object;
	uint16 x;
	uint16 y;
	uint16 width;
	uint16 height;
	byte *surface;
	Common::String text;
};

// TODO: Move to Cache, perhaps?
#define MAX_CACHED_TEXTS 20
typedef Common::HashMap<uint32, TextEntry *> TextCache;

/**
 * Text32 class, handles text calculation and displaying of text for SCI2, SCI21 and SCI3 games
 */
class GfxText32 {
public:
	GfxText32(SegManager *segMan, GfxCache *fonts, GfxScreen *screen);
	~GfxText32();
	void createTextBitmap(reg_t textObject);
	void drawTextBitmap(reg_t textObject, uint16 textX, uint16 textY, uint16 w);
	int16 GetLongest(const char *text, int16 maxWidth, GfxFont *font);
	TextEntry *getTextEntry(reg_t textObject);

private:
	TextEntry *createTextEntry(reg_t textObject);
	void purgeCache();

	SegManager *_segMan;
	GfxCache *_cache;
	TextCache _textCache;
	GfxScreen *_screen;
};

} // End of namespace Sci

#endif
