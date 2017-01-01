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

#ifndef XEEN_SCREEN_H
#define XEEN_SCREEN_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/array.h"
#include "common/keyboard.h"
#include "common/rect.h"
#include "xeen/font.h"
#include "xeen/sprites.h"
#include "xeen/xsurface.h"

namespace Xeen {

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)
#define GAME_WINDOW 28

class XeenEngine;
class Screen;

struct DrawStruct {
	SpriteResource *_sprites;
	int _frame;
	int _x;
	int _y;
	int _scale;
	int _flags;

	DrawStruct(int frame, int x, int y, int scale = 0, int flags = 0) :
		_sprites(nullptr), _frame(frame), _x(x), _y(y), _scale(scale), _flags(flags) {}
	DrawStruct(): _sprites(nullptr), _frame(0), _x(0), _y(0), _scale(0), _flags(0) {}
};

class Window: public XSurface {
private:
	static XeenEngine *_vm;
	Common::Rect _bounds;
	Common::Rect _innerBounds;
	XSurface _savedArea;
	int _a;
	int _border;
	int _xLo, _xHi;
	int _ycL, _ycH;

	void open2();
public:
	bool _enabled;
public:
	static void init(XeenEngine *vm);
public:
	Window();
	Window(const Window &src);
	Window(const Common::Rect &bounds, int a, int border,
		int xLo, int ycL, int xHi, int ycH);

	virtual void addDirtyRect(const Common::Rect &r);

	void setBounds(const Common::Rect &r);

	const Common::Rect &getBounds() { return _bounds; }

	void open();

	void close();

	/**
	 * Update the window
	 */
	void update();

	void frame();

	/**
	 * Fill the content area of a window with the current background color
	 */
	void fill();

	const char *writeString(const Common::String &s);

	void drawList(DrawStruct *items, int count);

	int getString(Common::String &line, uint maxLen, int maxWidth);
};

class Screen: public FontSurface {
private:
	XeenEngine *_vm;
	Common::List<Common::Rect> _dirtyRects;
	byte _mainPalette[PALETTE_SIZE];
	byte _tempPalette[PALETTE_SIZE];
	XSurface _pages[2];
	XSurface _savedScreens[10];
	bool _fadeIn;

	void mergeDirtyRects();

	bool unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2);

	/**
	 * Mark the entire screen for drawing
	 */
	void drawScreen();

	void fadeInner(int step);

	void updatePalette();

	void updatePalette(const byte *pal, int start, int count16);
public:
	/**
	 * Adds an area that requires redrawing on the next frame update
	 */
	virtual void addDirtyRect(const Common::Rect &r);
public:
	Common::Array<Window> _windows;

	Common::Array<Window *> _windowStack;
public:
	Screen(XeenEngine *vm);

	virtual ~Screen();

	void setupWindows();

	void closeWindows();

	void update();

	/**
	 * Load a palette resource into the temporary palette
	 */
	void loadPalette(const Common::String &name);

	/**
	 * Load a background resource into memory
	 */
	void loadBackground(const Common::String &name);

	/**
	 * Copy a loaded background into a display page
	 */
	void loadPage(int pageNum);

	void freePages();

	/**
	 * Merge the two pages along a horizontal split point
	 */
	void horizMerge(int xp = 0);

	/**
	 * Merge the two pages along a vertical split point
	 */
	void vertMerge(int yp);

	void draw(void *data = nullptr);

	void fadeIn(int step = 4);

	void fadeOut(int step = 4);

	void saveBackground(int slot = 1);

	void restoreBackground(int slot = 1);

	void frameWindow(uint bgType);
};

} // End of namespace Xeen

#endif /* XEEN_SCREEN_H */
