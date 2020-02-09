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
#include "sherlock/image_file.h"
#include "sherlock/surface.h"
#include "sherlock/resources.h"
#include "sherlock/saveload.h"

namespace Sherlock {

#define VGA_COLOR_TRANS(x) ((x) * 255 / 63)
#define BG_GREYSCALE_RANGE_END 229
#define BLACK 0

class SherlockEngine;

class Screen : public BaseSurface {
private:
	uint32 _transitionSeed;

	// Rose Tattoo fields
	int _fadeBytesRead, _fadeBytesToRead;
	int _oldFadePercent;
protected:
	SherlockEngine *_vm;
	Surface _backBuffer;

public:
	Surface _backBuffer1, _backBuffer2;
	bool _fadeStyle;
	byte _cMap[PALETTE_SIZE];
	byte _sMap[PALETTE_SIZE];
	byte _tMap[PALETTE_SIZE];
	bool _flushScreen;
	Common::Point _currentScroll;
public:
	static Screen *init(SherlockEngine *vm);
	Screen(SherlockEngine *vm);
	~Screen() override;

	/**
	 * Obtain the currently active back buffer.
	 */
	Surface *getBackBuffer() { return &_backBuffer; }

	/**
	 * Makes first back buffer active.
	 */
	void activateBackBuffer1();

	/**
	 * Makes second back buffer active.
	 */
	void activateBackBuffer2();

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
	void print(const Common::Point &pt, uint color, const char *formatStr, ...) GCC_PRINTF(4, 5);

	/**
	 * Print a strings onto the back buffer without blitting it to the screen
	 */
	void gPrint(const Common::Point &pt, uint color, const char *formatStr, ...) GCC_PRINTF(4, 5);

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
	void flushImage(ImageFrame *frame, const Common::Point &pt, int16 *xp, int16 *yp,
		int16 *width, int16 *height);

	/**
	 * Similar to flushImage, this method takes in an extra parameter for the scale proporation,
	 * which affects the calculated bounds accordingly
	 */
	void flushScaleImage(ImageFrame *frame, const Common::Point &pt, int16 *xp, int16 *yp,
		int16 *width, int16 *height, int scaleVal);

	/**
	 * Variation of flushImage/flushScaleImage that takes in and updates a rect
	 */
	void flushImage(ImageFrame *frame, const Common::Point &pt, Common::Rect &newBounds, int scaleVal);

	/**
	 * Copies data from the back buffer to the screen
	 */
	void blockMove(const Common::Rect &r);

	/**
	 * Copies the entire screen from the back buffer
	 */
	void blockMove();

	/**
	 * Fills an area on the back buffer, and then copies it to the screen
	 */
	void vgaBar(const Common::Rect &r, int color);

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

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Serializer &s);

	/**
	 * Draws the given string into the back buffer using the images stored in _font
	 */
	virtual void writeString(const Common::String &str, const Common::Point &pt, uint overrideColor);


	// Rose Tattoo specific methods
	void initPaletteFade(int bytesToRead);

	int fadeRead(Common::SeekableReadStream &stream, byte *buf, int totalSize);

	/**
	 * Translate a palette from 6-bit RGB values to full 8-bit values suitable for passing
	 * to the underlying palette manager
	 */
	static void translatePalette(byte palette[PALETTE_SIZE]);
};

} // End of namespace Sherlock

#endif
