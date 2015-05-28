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

#ifndef SHERLOCK_SCREEN_H
#define SHERLOCK_SCREEN_H

#include "common/list.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "sherlock/surface.h"
#include "sherlock/resources.h"

namespace Sherlock {

#define PALETTE_SIZE 768
#define PALETTE_COUNT 256
#define VGA_COLOR_TRANS(x) ((x) * 255 / 63)

enum {
	INFO_BLACK			= 1,
	INFO_FOREGROUND		= 11,
	INFO_BACKGROUND		= 1,
	BORDER_COLOR		= 237,
	INV_FOREGROUND		= 14,
	INV_BACKGROUND		= 1,
	COMMAND_HIGHLIGHTED	= 10,
	COMMAND_FOREGROUND	= 15,
	COMMAND_BACKGROUND	= 4,
	COMMAND_NULL		= 248,
	BUTTON_TOP			= 233,
	BUTTON_MIDDLE		= 244,
	BUTTON_BOTTOM		= 248,
	TALK_FOREGROUND		= 12,
	TALK_NULL			= 16,
	PEN_COLOR			= 250
};

class SherlockEngine;

class Screen : public Surface {
private:
	SherlockEngine *_vm;
	int _fontNumber;
	Common::List<Common::Rect> _dirtyRects;
	uint32 _transitionSeed;
	ImageFile *_font;
	int _fontHeight;
	Surface _sceneSurface;

	// Rose Tattoo fields
	int _fadeBytesRead, _fadeBytesToRead;
	int _oldFadePercent;
	byte _lookupTable[PALETTE_COUNT];
	byte _lookupTable1[PALETTE_COUNT];
private:
	/**
	 * Merges together overlapping dirty areas of the screen
	 */
	void mergeDirtyRects();

	/**
	 * Returns the union of two dirty area rectangles
	 */
	bool unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2);

	/**
	 * Draws the given string into the back buffer using the images stored in _font
	 */
	void writeString(const Common::String &str, const Common::Point &pt, byte color);
protected:
	/**
	 * Adds a rectangle to the list of modified areas of the screen during the
	 * current frame
	 */
	virtual void addDirtyRect(const Common::Rect &r);
public:
	Surface _backBuffer1, _backBuffer2;
	Surface *_backBuffer;
	bool _fadeStyle;
	byte _cMap[PALETTE_SIZE];
	byte _sMap[PALETTE_SIZE];
	byte _tMap[PALETTE_SIZE];
	int _currentScroll, _targetScroll;
	int _scrollSize, _scrollSpeed;
	bool _flushScreen;
public:
	Screen(SherlockEngine *vm);
	virtual ~Screen();

	/**
	 * Set the font to use for writing text on the screen
	 */
	void setFont(int fontNumber);

	/**
	 * Handles updating any dirty areas of the screen Surface object to the physical screen
	 */
	void update();

	/**
	 * Return the currently active palette
	 */
	void getPalette(byte palette[PALETTE_SIZE]);

	/**
	 * Set the palette
	 */
	void setPalette(const byte palette[PALETTE_SIZE]);

	/**
	 * Fades from the currently active palette to the passed palette
	 */
	int equalizePalette(const byte palette[PALETTE_SIZE]);

	/**
	 * Fade out the palette to black
	 */
	void fadeToBlack(int speed = 2);

	/**
	 * Fade in a given palette
	 */
	void fadeIn(const byte palette[PALETTE_SIZE], int speed = 2);

	/**
	 * Do a random pixel transition in from _backBuffer surface to the screen
	 */
	void randomTransition();

	/**
	 * Transition to the surface from _backBuffer using a vertical transition
	 */
	void verticalTransition();

	/**
	 * Prints the text passed onto the back buffer at the given position and color.
	 * The string is then blitted to the screen
	 */
	void print(const Common::Point &pt, byte color, const char *formatStr, ...) GCC_PRINTF(4, 5);

	/**
	 * Print a strings onto the back buffer without blitting it to the screen
	 */
	void gPrint(const Common::Point &pt, byte color, const char *formatStr, ...) GCC_PRINTF(4, 5);

	/**
	 * Copies a section of the second back buffer into the main back buffer
	 */
	void restoreBackground(const Common::Rect &r);

	/**
	 * Copies a given area to the screen
	 */
	void slamArea(int16 xp, int16 yp, int16 width, int16 height);

	/**
	 * Copies a given area to the screen
	 */
	void slamRect(const Common::Rect &r);

	/**
	 * Copy an image from the back buffer to the screen, taking care of both the
	 * new area covered by the shape as well as the old area, which must be restored
	 */
	void flushImage(ImageFrame *frame, const Common::Point &pt,
		int16 *xp, int16 *yp, int16 *width, int16 *height);

	void flushScaleImage(ImageFrame *frame, const Common::Point &pt,
		int16 *xp, int16 *yp, int16 *width, int16 *height, int scaleVal);

	/**
	 * Returns the width of a string in pixels
	 */
	int stringWidth(const Common::String &str);

	/**
	 * Returns the width of a character in pixels
	 */
	int charWidth(char c);

	/**
	 * Fills an area on the back buffer, and then copies it to the screen
	 */
	void vgaBar(const Common::Rect &r, int color);

	/**
	 * Draws a button for use in the inventory, talk, and examine dialogs.
	 */
	void makeButton(const Common::Rect &bounds, int textX, const Common::String &str);

	/**
	 * Prints an interface command with the first letter highlighted to indicate
	 * what keyboard shortcut is associated with it
	 */
	void buttonPrint(const Common::Point &pt, byte color, bool slamIt, const Common::String &str);

	/**
	 * Draw a panel in the back buffer with a raised area effect around the edges
	 */
	void makePanel(const Common::Rect &r);

	/**
	 * Draw a field in the back buffer with a raised area effect around the edges,
	 * suitable for text input.
	 */
	void makeField(const Common::Rect &r);

	/**
	 * Sets the active back buffer pointer to a restricted sub-area of the first back buffer
	 */
	void setDisplayBounds(const Common::Rect &r);

	/**
	 * Resets the active buffer pointer to point back to the full first back buffer
	 */
	void resetDisplayBounds();

	/**
	 * Return the size of the current display window
	 */
	Common::Rect getDisplayBounds();

	int fontNumber() const { return _fontNumber; }

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Common::Serializer &s);

	// Rose Tattoo specific methods
	void initPaletteFade(int bytesToRead);

	int fadeRead(Common::SeekableReadStream &stream, byte *buf, int totalSize);

	void setupBGArea(const byte cMap[PALETTE_SIZE]);

	void initScrollVars();

	/**
	 * Translate a palette from 6-bit RGB values to full 8-bit values suitable for passing
	 * to the underlying palette manager
	 */
	static void translatePalette(byte palette[PALETTE_SIZE]);
};

} // End of namespace Sherlock

#endif
