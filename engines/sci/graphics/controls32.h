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
 */

#ifndef SCI_GRAPHICS_CONTROLS32_H
#define SCI_GRAPHICS_CONTROLS32_H

namespace Sci {

class GfxCache;
class GfxScreen;
class GfxText32;

struct TextEditor {
	/**
	 * The bitmap where the editor is rendered.
	 */
	reg_t bitmap;

	/**
	 * The width of the editor, in bitmap pixels.
	 */
	int16 width;

	/**
	 * The text in the editor.
	 */
	Common::String text;

	/**
	 * The rect where text should be drawn into the editor,
	 * in bitmap pixels.
	 */
	Common::Rect textRect;

	/**
	 * The color of the border. -1 indicates no border.
	 */
	int16 borderColor;

	/**
	 * The text color.
	 */
	uint8 foreColor;

	/**
	 * The background color.
	 */
	uint8 backColor;

	/**
	 * The transparent color.
	 */
	uint8 skipColor;

	/**
	 * The font used to render the text in the editor.
	 */
	GuiResourceId fontId;

	/**
	 * The current position of the cursor within the editor.
	 */
	uint16 cursorCharPosition;

	/**
	 * Whether or not the cursor is currently drawn to the
	 * screen.
	 */
	bool cursorIsDrawn;

	/**
	 * The rectangle for drawing the input cursor, in bitmap
	 * pixels.
	 */
	Common::Rect cursorRect;

	/**
	 * The maximum allowed text length, in characters.
	 */
	uint16 maxLength;
};

/**
 * Controls class, handles drawing of controls in SCI32 (SCI2, SCI2.1, SCI3) games
 */
class GfxControls32 {
public:
	GfxControls32(SegManager *segMan, GfxCache *cache, GfxText32 *text);

	reg_t kernelEditText(const reg_t controlObject);

private:
	SegManager *_segMan;
	GfxCache *_gfxCache;
	GfxText32 *_gfxText32;

	bool _overwriteMode;
	uint32 _nextCursorFlashTick;
	void drawCursor(TextEditor &editor);
	void eraseCursor(TextEditor &editor);
	void flashCursor(TextEditor &editor);
};

} // End of namespace Sci

#endif
