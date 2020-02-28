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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#ifndef MACVENTURE_GUI_H
#define MACVENTURE_GUI_H

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macmenu.h"

#include "graphics/font.h"

#include "common/timer.h"

#include "macventure/macventure.h"
#include "macventure/container.h"
#include "macventure/image.h"
#include "macventure/prebuilt_dialogs.h"
#include "macventure/dialog.h"
#include "macventure/controls.h"
#include "macventure/windows.h"

namespace MacVenture {

using namespace Graphics::MacGUIConstants;
using namespace Graphics::MacWindowConstants;
class MacVentureEngine;
typedef uint32 ObjID;

class Cursor;
class ConsoleText;
class CommandButton;
class ImageAsset;
class Dialog;

BorderBounds borderBounds(MVWindowType type);

enum MenuAction {
	kMenuActionAbout,
	kMenuActionNew,
	kMenuActionOpen,
	kMenuActionSave,
	kMenuActionSaveAs,
	kMenuActionQuit,
	kMenuActionUndo,
	kMenuActionCut,
	kMenuActionCopy,
	kMenuActionPaste,
	kMenuActionClear,
	kMenuActionCleanUp,
	kMenuActionMessUp,

	kMenuActionCommand
};

struct DraggedObj {
	ObjID id;
	Common::Point pos;
	Common::Point mouseOffset;
	Common::Point startPos;
	WindowReference startWin;
	bool hasMoved;
};

class Gui {

public:
	Gui(MacVentureEngine *engine, Common::MacResManager *resman);
	~Gui();

	void reloadInternals();

	void draw();
	void drawMenu();
	void drawTitle();

	void clearControls();
	bool processEvent(Common::Event &event);
	void handleMenuAction(MenuAction action);
	void updateWindow(WindowReference winID, bool containerOpen);
	void invertWindowColors(WindowReference winID);

	WindowReference createInventoryWindow(ObjID objRef);
	bool tryCloseWindow(WindowReference winID);

	Common::Point getObjMeasures(ObjID obj);

	WindowReference getObjWindow(ObjID objID);
	WindowReference findObjWindow(ObjID objID);

	// Event processors
	bool processCommandEvents(WindowClick click, Common::Event &event);
	bool processMainGameEvents(WindowClick click, Common::Event &event);
	bool processOutConsoleEvents(WindowClick click, Common::Event &event);
	bool processSelfEvents(WindowClick click, Common::Event &event);
	bool processExitsEvents(WindowClick click, Common::Event &event);
	bool processDiplomaEvents(WindowClick click, Common::Event &event);
	bool processInventoryEvents(WindowClick click, Common::Event &event);

	const WindowData& getWindowData(WindowReference reference);

	const Graphics::Font& getCurrentFont();

	// Clicks
	void selectForDrag(Common::Point cursorPosition);
	void handleSingleClick();
	void handleDoubleClick();

	// Modifiers
	void bringToFront(WindowReference window);
	void setWindowTitle(WindowReference winID, Common::String string);
	void updateWindowInfo(WindowReference ref, ObjID objID, const Common::Array<ObjID> &children);
	void ensureInventoryOpen(WindowReference reference, ObjID id);

	void addChild(WindowReference target, ObjID child);
	void removeChild(WindowReference target, ObjID child);

	void clearExits();
	void unselectExits();
	void updateExit(ObjID id);

	void printText(const Common::String &text);

	//Dialog interactions
	void showPrebuiltDialog(PrebuiltDialogs type);
	bool isDialogOpen();

	void getTextFromUser();
	void setTextInput(Common::String str);
	void closeDialog();

	void loadGame();
	void saveGame();
	void newGame();
	void quitGame();

	void createInnerSurface(Graphics::ManagedSurface *innerSurface, Graphics::ManagedSurface *outerSurface, const BorderBounds &borders);


private: // Attributes

	MacVentureEngine *_engine;
	Common::MacResManager *_resourceManager;

	Graphics::ManagedSurface _screen;
	Graphics::MacWindowManager _wm;

	Common::List<WindowData> *_windowData;
	Common::Array<CommandButton> *_controlData;
	Common::Array<CommandButton> *_exitsData;

	Graphics::MacWindow *_controlsWindow;
	Graphics::MacWindow *_mainGameWindow;
	Graphics::MacWindow *_outConsoleWindow;
	Graphics::MacWindow *_selfWindow;
	Graphics::MacWindow *_exitsWindow;
	Graphics::MacWindow *_diplomaWindow;
	Common::Array<Graphics::MacWindow*> _inventoryWindows;
	Graphics::MacMenu *_menu;
	Dialog *_dialog;

	Container *_graphics;
	Common::HashMap<ObjID, ImageAsset*> _assets;

	Graphics::ManagedSurface _draggedSurface;
	DraggedObj _draggedObj;

	Cursor *_cursor;

	ConsoleText *_consoleText;

private: // Methods

	// Initializers
	void initGUI();
	void initWindows();
	void assignObjReferences(); // Mainly guesswork

	// Loaders
	bool loadMenus();
	bool loadWindows();
	bool loadControls();
	void loadBorders(Graphics::MacWindow *target, MVWindowType type);
	void loadBorder(Graphics::MacWindow *target, MVWindowType type, bool active);
	void loadGraphics();
	void clearAssets();

	// Drawers
	void drawWindows();
	void drawCommandsWindow();
	void drawMainGameWindow();
	void drawSelfWindow();
	void drawInventories();
	void drawExitsWindow();
	void drawConsoleWindow();

	void drawDraggedObject();
	void drawObjectsInWindow(const WindowData &targetData, Graphics::ManagedSurface *surface);
	void drawWindowTitle(WindowReference target, Graphics::ManagedSurface *surface);
	void drawDialog();

	void moveDraggedObject(Common::Point target);

	// Finders
	WindowReference findWindowAtPoint(Common::Point point);
	Common::Point getGlobalScrolledSurfacePosition(WindowReference reference);
	WindowData& findWindowData(WindowReference reference);
	Graphics::MacWindow *findWindow(WindowReference reference);

	// Utils
	void checkSelect(const WindowData &data, Common::Point pos, const Common::Rect &clickRect, WindowReference ref);
	bool canBeSelected(ObjID obj, const Common::Rect &clickRect, WindowReference ref);
	bool isRectInsideObject(Common::Rect target, ObjID obj);
	void selectDraggable(ObjID child, WindowReference origin, Common::Point startPos);
	void handleDragRelease(bool shiftPressed, bool isDoubleClick);
	Common::Rect calculateClickRect(Common::Point clickPos, Common::Rect windowBounds);
	Common::Point localizeTravelledDistance(Common::Point point, WindowReference origin, WindowReference target);
	void removeInventoryWindow(WindowReference ref);

	void ensureAssetLoaded(ObjID obj);

};

enum ClickState {
	kCursorIdle = 0,
	kCursorSCStart = 1,
	kCursorSCDrag = 2,
	kCursorDCStart = 3,
	kCursorDCDo = 4,
	kCursorSCSink = 5,
	kCursorStateCount
};

enum CursorInput { // Columns for the FSM transition table
	kButtonDownCol = 0,
	kButtonUpCol = 1,
	kTickCol = 2,
	kCursorInputCount
};

class Cursor {

public:
	Cursor(Gui *gui);
	~Cursor();

	void tick();
	bool processEvent(const Common::Event &event);
	Common::Point getPos();
	bool canSelectDraggable();

private:

	void changeState(CursorInput input);
	void executeStateIn();
	void executeStateOut();


private:
	Gui *_gui;

	Common::Point _pos;
	ClickState _state;
};



enum {
	kConsoleLeftOffset = 2
};

class ConsoleText {

public:

	ConsoleText(Gui *gui) {
		_gui = gui;
		_lines.push_back("");
		updateScroll();
	}

	~ConsoleText() {

	}

	void printLine(const Common::String &str, int maxW) {
		Common::StringArray wrappedLines;
		int textW = maxW;
		const Graphics::Font *font = &_gui->getCurrentFont();

		font->wordWrapText(str, textW, wrappedLines);

		if (wrappedLines.empty()) // Sometimes we have empty lines
			_lines.push_back("");

		for (Common::StringArray::const_iterator j = wrappedLines.begin(); j != wrappedLines.end(); ++j) {
			_lines.push_back(*j);
		}

		updateScroll();
	}

	void renderInto(Graphics::ManagedSurface *target, const BorderBounds borders, int textOffset) {
		target->fillRect(target->getBounds(), kColorWhite);

		Graphics::ManagedSurface *composeSurface = new Graphics::ManagedSurface();
		_gui->createInnerSurface(composeSurface, target, borders);
		composeSurface->clear(kColorGreen);

		const Graphics::Font *font = &_gui->getCurrentFont();
		int y = target->h - font->getFontHeight();
		for (uint i = _scrollPos; i != 0; i--) {
			font->drawString(target, _lines[i], textOffset, y, font->getStringWidth(_lines[i]), kColorBlack);

			if (y < font->getFontHeight())	// Do not draw off-screen
				break;

			y -= font->getFontHeight();
		}

		Common::Point composePosition = Common::Point(borders.leftOffset, borders.topOffset);
		target->transBlitFrom(*composeSurface, composePosition, kColorGreen);
		delete composeSurface;
	}

	void updateScroll() {
		_scrollPos = _lines.size() - 1;
	}

	void scrollDown() {
		if (_scrollPos < (int)(_lines.size() - 1)) {
			_scrollPos++;
		}
	}

	void scrollUp() {
		if (_scrollPos > 0) {
			_scrollPos--;
		}
	}


private:

	Gui *_gui;

	Common::StringArray _lines;
	int _scrollPos;

};

} // End of namespace MacVenture

#endif
