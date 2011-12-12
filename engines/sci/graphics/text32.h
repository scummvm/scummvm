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

namespace Sci {

/**
 * Text32 class, handles text calculation and displaying of text for SCI2, SCI21 and SCI3 games
 */
class GfxText32 {
public:
	GfxText32(SegManager *segMan, GfxCache *fonts, GfxScreen *screen);
	~GfxText32();
	reg_t createTextBitmap(reg_t textObject, uint16 maxWidth = 0, uint16 maxHeight = 0, reg_t prevHunk = NULL_REG);
	void disposeTextBitmap(reg_t hunkId);
	void drawTextBitmap(uint16 x, uint16 y, Common::Rect planeRect, reg_t textObject);
	int16 GetLongest(const char *text, int16 maxWidth, GfxFont *font);

	void kernelTextSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);

private:
	int16 Size(Common::Rect &rect, const char *text, GuiResourceId fontId, int16 maxWidth);
	void Width(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight, bool restoreFont);
	void StringWidth(const char *str, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight);

	SegManager *_segMan;
	GfxCache *_cache;
	GfxScreen *_screen;
};

} // End of namespace Sci

#endif
