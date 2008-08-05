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
 * $URL$
 * $Id$
 */

#ifndef NEWGUI_H
#define NEWGUI_H

#include "common/scummsys.h"
#include "common/singleton.h"
#include "common/stack.h"
#include "common/str.h"
#include "graphics/fontman.h"
#include "gui/theme.h"
#include "gui/widget.h"

class OSystem;

namespace GUI {

class Dialog;
class Eval;
class ThemeEval;

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
	~NewGui();
public:

	// Main entry for the GUI: this will start an event loop that keeps running
	// until no dialogs are active anymore.
	void runLoop();

	bool isActive() const	{ return ! _dialogStack.empty(); }

	bool loadNewTheme(const Common::String &file);
	Theme *theme() { return _theme; }
	
	Eval *evaluator() { return _theme->_evaluator; }
	ThemeEval *xmlEval() { return (ThemeEval*)_theme->evaluator(); }

	const Graphics::Font &getFont(Theme::FontStyle style = Theme::kFontStyleBold) const { return *(_theme->getFont(style)); }
	int getFontHeight(Theme::FontStyle style = Theme::kFontStyleBold) const { return _theme->getFontHeight(style); }
	int getStringWidth(const Common::String &str, Theme::FontStyle style = Theme::kFontStyleBold) const { return _theme->getStringWidth(str, style); }
	int getCharWidth(byte c, Theme::FontStyle style = Theme::kFontStyleBold) const { return _theme->getCharWidth(c, style); }

	WidgetSize getWidgetSize();

	void clearDragWidget();

	void screenChange();

	enum RedrawStatus {
		kRedrawDisabled = 0,
		kRedrawOpenDialog,
		kRedrawCloseDialog,
		kRedrawTopDialog,
		kRedrawFull
	};

protected:
	OSystem			*_system;

	Theme		*_theme;

//	bool		_needRedraw;
	RedrawStatus _redrawStatus;
	int			_lastScreenChangeID;
	DialogStack	_dialogStack;

	bool		_stateIsSaved;

	bool		_useStdCursor;

	// position and time of last mouse click (used to detect double clicks)
	struct {
		int16 x, y;	// Position of mouse when the click occured
		uint32 time;	// Time
		int count;	// How often was it already pressed?
	} _lastClick;

	// mouse cursor state
	int		_cursorAnimateCounter;
	int		_cursorAnimateTimer;
	byte		_cursor[2048];

	bool _themeChange;

	void saveState();
	void restoreState();

	void openDialog(Dialog *dialog);
	void closeTopDialog();

	void redraw();

	void loop();

	void setupCursor();
	void animateCursor();

	Dialog *getTopDialog() const;
};

} // End of namespace GUI

#endif
