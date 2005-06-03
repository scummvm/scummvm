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

#ifndef NEWGUI_H
#define NEWGUI_H

#include "common/scummsys.h"
#include "common/singleton.h"
#include "common/stack.h"
#include "common/str.h"
#include "graphics/fontman.h"

class OSystem;

namespace GUI {

class Dialog;

#define g_gui	(GUI::NewGui::instance())


// Height of a single text line
#define kLineHeight	(g_gui.getFontHeight() + 2)


using Graphics::TextAlignment;
using Graphics::kTextAlignCenter;
using Graphics::kTextAlignLeft;
using Graphics::kTextAlignRight;


// Simple dialog stack class
// Anybody nesting dialogs deeper than 4 is mad anyway
typedef Common::FixedStack<Dialog *> DialogStack;


/**
 * GUI manager singleton.
 */
class NewGui : public Common::Singleton<NewGui> {
	typedef Common::String String;
	friend class Dialog;
	friend class Common::Singleton<SingletonBaseType>;
	NewGui();
public:

	// Main entry for the GUI: this will start an event loop that keeps running
	// until no dialogs are active anymore.
	void runLoop();

	bool isActive() const	{ return ! _dialogStack.empty(); }

protected:
	OSystem			*_system;
	Graphics::Surface	_screen;
	int			_screenPitch;

	bool		_needRedraw;
	DialogStack	_dialogStack;

	bool		_stateIsSaved;
	
	const Graphics::Font *_font;

	// for continuous events (keyDown)
	struct {
		uint16 ascii;
		byte flags;
		int keycode;
	} _currentKeyDown;
	uint32		_keyRepeatTime;

	// position and time of last mouse click (used to detect double clicks)
	struct {
		int16 x, y;	// Position of mouse when the click occured
		uint32 time;	// Time
		int count;	// How often was it already pressed?
	} _lastClick;

	// mouse cursor state
	bool		_oldCursorMode;
	int		_cursorAnimateCounter;
	int		_cursorAnimateTimer;
	byte		_cursor[2048];

	void saveState();
	void restoreState();

	void openDialog(Dialog *dialog);
	void closeTopDialog();

	void loop();

	void animateCursor();
	void updateColors();

	void updateScaleFactor();

	OverlayColor *getBasePtr(int x, int y);

public:
	// Theme colors
	OverlayColor _color, _shadowcolor;
	OverlayColor _bgcolor;
	OverlayColor _textcolor;
	OverlayColor _textcolorhi;

	// Font
	const Graphics::Font &getFont() const;

	// Screen surface
	Graphics::Surface &getScreen() { return _screen; }

	// Drawing primitives
	void box(int x, int y, int width, int height, OverlayColor colorA, OverlayColor colorB);
	void hLine(int x, int y, int x2, OverlayColor color);
	void vLine(int x, int y, int y2, OverlayColor color);

	/**
	 * Copy the specified screen rectangle into a new graphics surfaces.
	 * New memory for the GFX data is allocated via malloc; it is the
	 * callers responsibilty to free that data.
	 */
	void copyToSurface(Graphics::Surface *s, int x, int y, int w, int h);
	
	/**
	 * Draw the graphics contained in the given surface at the specified coordinates.
	 */
	void drawSurface(const Graphics::Surface &s, int x, int y);

	void blendRect(int x, int y, int w, int h, OverlayColor color, int level = 3);
	void fillRect(int x, int y, int w, int h, OverlayColor color);
	void frameRect(int x, int y, int w, int h, OverlayColor color);

	void drawChar(byte c, int x, int y, OverlayColor color, const Graphics::Font *font = 0);
	void drawString(const String &str, int x, int y, int w, OverlayColor color, Graphics::TextAlignment align = Graphics::kTextAlignLeft, int deltax = 0, bool useEllipsis = true);

	int getStringWidth(const String &str) const;
	int getCharWidth(byte c) const;
	int getFontHeight() const;

	void addDirtyRect(int x, int y, int w, int h);
};

} // End of namespace GUI

#endif
