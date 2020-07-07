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

#ifndef GUIMANAGER_H
#define GUIMANAGER_H

#include "common/scummsys.h"
#include "common/singleton.h"
#include "common/stack.h"
#include "common/str.h"
#include "common/list.h"

#include "gui/ThemeEngine.h"

class OSystem;

namespace Graphics {
class Font;
}

namespace Common {
	struct Event;
	class Keymap;
}

namespace GUI {

class Dialog;
class ThemeEval;
class GuiObject;

#define g_gui	(GUI::GuiManager::instance())


// Height of a single text line
#define kLineHeight	(g_gui.getFontHeight() + 2)



// Simple dialog stack class
// Anybody nesting dialogs deeper than 4 is mad anyway
typedef Common::FixedStack<Dialog *> DialogStack;


/**
 * GUI manager singleton.
 */
class GuiManager : public Common::Singleton<GuiManager> {
	friend class Dialog;
	friend class Common::Singleton<SingletonBaseType>;
	GuiManager();
	~GuiManager() override;
public:

	// Main entry for the GUI: this will start an event loop that keeps running
	// until no dialogs are active anymore.
	void runLoop();

	void processEvent(const Common::Event &event, Dialog *const activeDialog);
	Common::Keymap *getKeymap() const;
	void scheduleTopDialogRedraw();

	bool isActive() const	{ return ! _dialogStack.empty(); }

	bool loadNewTheme(Common::String id, ThemeEngine::GraphicsMode gfx = ThemeEngine::kGfxDisabled, bool force = false);
	ThemeEngine *theme() { return _theme; }

	ThemeEval *xmlEval() { return _theme->getEvaluator(); }

	int getWidth() const { return _width; }
	int getHeight() const { return _height; }

	bool useRTL() const { return _useRTL; }
	void setLanguageRTL();

	void setDialogPaddings(int l, int r);
	int getOverlayOffset() { return _topDialogRightPadding - _topDialogLeftPadding; }

	const Graphics::Font &getFont(ThemeEngine::FontStyle style = ThemeEngine::kFontStyleBold) const { return *(_theme->getFont(style)); }
	int getFontHeight(ThemeEngine::FontStyle style = ThemeEngine::kFontStyleBold) const { return _theme->getFontHeight(style); }
	int getStringWidth(const Common::String &str, ThemeEngine::FontStyle style = ThemeEngine::kFontStyleBold) const { return _theme->getStringWidth(str, style); }
	int getCharWidth(byte c, ThemeEngine::FontStyle style = ThemeEngine::kFontStyleBold) const { return _theme->getCharWidth(c, style); }
	int getKerningOffset(byte left, byte right, ThemeEngine::FontStyle font = ThemeEngine::kFontStyleBold) const { return _theme->getKerningOffset(left, right, font); }

	/**
	 * Tell the GuiManager to check whether the screen resolution has changed.
	 * If that is the case, the GuiManager will reload/refresh the active theme.
	 *
	 * @return true if the a screen change indeed occurred, false otherwise
	 */
	bool checkScreenChange();

	/**
	 * Tell the GuiManager to delete the given GuiObject later. If a parent
	 * dialog is provided and is present in the DialogStack, the object will
	 * only be deleted when that dialog is the top level dialog.
	 */
	void addToTrash(GuiObject*, Dialog* parent = nullptr);
	void initTextToSpeech();

	bool _launched;

	void redrawFull();

protected:
	enum RedrawStatus {
		kRedrawDisabled = 0,
		kRedrawOpenDialog,
		kRedrawCloseDialog,
		kRedrawTopDialog,
		kRedrawFull
	};

	OSystem			*_system;

	ThemeEngine		*_theme;

//	bool		_needRedraw;
	RedrawStatus _redrawStatus;
	int			_lastScreenChangeID;
	int			_width, _height;
	DialogStack	_dialogStack;

	bool		_stateIsSaved;

	bool		_useStdCursor;

	bool		_useRTL;

	int			_topDialogLeftPadding;
	int			_topDialogRightPadding;

	// position and time of last mouse click (used to detect double clicks)
	struct MousePos {
		MousePos() : x(-1), y(-1), count(0) { time = 0; }
		int16 x, y;	// Position of mouse when the click occurred
		uint32 time;	// Time
		int count;	// How often was it already pressed?
	} _lastClick, _lastMousePosition, _globalMousePosition;

	// mouse cursor state
	int		_cursorAnimateCounter;
	int		_cursorAnimateTimer;
	byte	_cursor[2048];

	// delayed deletion of GuiObject
	struct GuiObjectTrashItem {
		GuiObject* object;
		Dialog* parent;
	};
	Common::List<GuiObjectTrashItem> _guiObjectTrash;

	void initKeymap();
	void enableKeymap(bool enabled);

	void saveState();
	void restoreState();

	void openDialog(Dialog *dialog);
	void closeTopDialog();

	void redraw();

	void setupCursor();
	void animateCursor();

	Dialog *getTopDialog() const;

	void screenChange();

	void giveFocusToDialog(Dialog *dialog);
	void setLastMousePos(int16 x, int16 y);
};

} // End of namespace GUI

#endif
