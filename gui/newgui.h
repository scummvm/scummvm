/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
#include "common/system.h"	// For events
#include "graphics/font.h"

// Uncomment the following to enable the new font code:
//#define NEW_FONT_CODE


namespace GUI {

class Dialog;

#define g_gui	(GUI::NewGui::instance())


// Height of a single text line
#ifdef NEW_FONT_CODE
#define		g_guifont		Graphics::g_sysfont
#else
#define		g_guifont		Graphics::g_scummfont
#endif
#define kLineHeight	(g_guifont.getFontHeight() + 2)


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
	friend class Common::Singleton<NewGui>;
	NewGui();
public:

	// Main entry for the GUI: this will start an event loop that keeps running
	// until no dialogs are active anymore.
	void runLoop();

	bool isActive()	{ return ! _dialogStack.empty(); }

protected:
	OSystem		*_system;
	Graphics::Surface		_screen;
	int			_screenPitch;
	
	bool		_needRedraw;
	DialogStack	_dialogStack;
	
	bool		_stateIsSaved;
	
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
	int			_cursorAnimateCounter;
	int			_cursorAnimateTimer;
	byte		_cursor[2048];

	void saveState();
	void restoreState();
	
	void openDialog(Dialog *dialog);
	void closeTopDialog();
	
	void loop();

	void animateCursor();
	void updateColors();

public:
	// Theme colors
	OverlayColor _color, _shadowcolor;
	OverlayColor _bgcolor;
	OverlayColor _textcolor;
	OverlayColor _textcolorhi;

	// Drawing primitives
	OverlayColor *getBasePtr(int x, int y);
	void box(int x, int y, int width, int height, OverlayColor colorA, OverlayColor colorB);
	void hLine(int x, int y, int x2, OverlayColor color);
	void vLine(int x, int y, int y2, OverlayColor color);
	void blendRect(int x, int y, int w, int h, OverlayColor color, int level = 3);
	void fillRect(int x, int y, int w, int h, OverlayColor color);
	void checkerRect(int x, int y, int w, int h, OverlayColor color);
	void frameRect(int x, int y, int w, int h, OverlayColor color);
	void drawChar(byte c, int x, int y, OverlayColor color);
	int getStringWidth(const String &str);
	int getCharWidth(byte c);
	void drawString(const String &str, int x, int y, int w, OverlayColor color, Graphics::TextAlignment align = Graphics::kTextAlignLeft, int deltax = 0, bool useEllipsis = true);

	void blitFromBuffer(int x, int y, int w, int h, const byte *buf, int pitch);
	void blitToBuffer(int x, int y, int w, int h, byte *buf, int pitch);

	void drawBitmap(uint32 *bitmap, int x, int y, OverlayColor color, int h = 8);

	void addDirtyRect(int x, int y, int w, int h);
};

} // End of namespace GUI

#endif
