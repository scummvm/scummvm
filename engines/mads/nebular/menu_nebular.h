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
 */

#ifndef MADS_MENU_NEBULAR_H
#define MADS_MENU_NEBULAR_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/msurface.h"
#include "mads/nebular/dialogs_nebular.h"

namespace MADS {

class MADSEngine;

namespace Nebular {

enum MADSGameAction { START_GAME, RESUME_GAME, SHOW_INTRO, CREDITS, QUOTES, EXIT };

class MenuView: public FullScreenDialog {
protected:
	bool _breakFlag;
	bool _redrawFlag;

	virtual void doFrame() = 0;

	virtual void display();

	/**
	* Event handler
	*/
	virtual bool onEvent(Common::Event &event);
public:
	MenuView(MADSEngine *vm);

	virtual ~MenuView() {}

	virtual void show();
};

class MainMenu: public MenuView {
private:
	SpriteAsset *_menuItems[7];
	int _menuItemIndexes[7];
	int _menuItemIndex;
	int _frameIndex;
	uint32 _delayTimeout;
	bool _skipFlag;

	/**
	 * Currently highlighted menu item
	 */
	int _highlightedIndex;

	/**
	 * Flag for mouse button being pressed
	 */
	bool _buttonDown;

	/**
	 * Stores menu item selection
	 */
	int _selectedIndex;

	/**
	 * Get the highlighted menu item under the cursor
	 */
	int getHighlightedItem(const Common::Point &pt);

	/**
	 * Un-highlight a currently highlighted item
	 */
	void unhighlightItem();

	/**
	 * Execute a given menuitem
	 */
	void handleAction(MADSGameAction action);

	/**
	 * Add a sprite slot for the current menuitem frame
	 */
	void addSpriteSlot();
protected:
	/**
	 * Display the menu
	 */
	virtual void display();

	/**
	 * Handle the menu item animations
	 */
	virtual void doFrame();

	/**
	 * Event handler
	 */
	virtual bool onEvent(Common::Event &event);
public:
	MainMenu(MADSEngine *vm);

	virtual ~MainMenu();
};

class AdvertView : public EventTarget {
private:
	/**
	 * Engine reference
	 */
	MADSEngine *_vm;

	/**
	 * Signals when to close the dialog
	 */
	bool _breakFlag;
protected:
	/**
	* Event handler
	*/
	virtual bool onEvent(Common::Event &event);
public:
	AdvertView(MADSEngine *vm);

	virtual ~AdvertView() {}

	/**
	 * Show the dialog
	 */
	void show();
};

struct TextLine {
	Common::Point _pos;
	Common::String _line;
	int _textDisplayIndex;
};

/**
 * Scrolling text view
 */
class TextView : public MenuView {
private:
	static char _resourceName[100];

	bool _animating;
	Common::Array<TextLine> _textLines;
	Common::Point _pan;
	int _panSpeed;
	MSurface _spareScreens[4];
	int _scrollCount;
	int _lineY;
	uint32 _scrollTimeout;
	int _panCountdown;
	int _translationX;
	Common::File _script;
	char _currentLine[80];
	MSurface *_spareScreen;
	Font *_font;
private:
	/**
	 * Load the text resource
	 */
	void load();

	/**
	 * Process the lines
	 */
	void processLines();

	/**
	 * Process a command from the script file
	 */
	void processCommand();

	/**
	 * Process text from the script file
	 */
	void processText();

	/**
	 * Get a parameter from line
	 */
	int getParameter(const char **paramP);

	/**
	 * Called when the script is finished
	 */
	void scriptDone();

	/**
	 * Reset the game palette
	 */
	void resetPalette();
protected:
	virtual void display();

	virtual void doFrame();
public:
	/**
	 * Queue the given text resource for display
	 */
	static void execute(MADSEngine *vm, const Common::String &resName);

	TextView(MADSEngine *vm);

	virtual ~TextView();
};

enum ResyncMode { NEVER, ALWAYS, BEGINNING };

struct ResourceEntry {
	Common::String _resourceName;
	int _fx;
	bool _soundFlag;
	bool _bgFlag;
	bool _showWhiteBars;

	ResourceEntry() {}
	ResourceEntry(const Common::String &resName, int fx, bool soundFlag,
			bool bgFlag, bool showWhiteBars) {
		_resourceName = resName;
		_fx = fx;
		_soundFlag = soundFlag;
		_bgFlag = bgFlag;
		_showWhiteBars = showWhiteBars;
	}
};

struct ResIndexEntry {
	int _id;
	int _v;
	Common::String _resourceName;

	ResIndexEntry() {}
};

/**
* Animation cutscene view
*/
class AnimationView : public MenuView {
private:
	static char _resourceName[100];

	Common::File _script;
	uint32 _previousUpdate;
	Common::String _currentLine;
	bool _soundDriverLoaded;
	bool _resetPalette;
	ResyncMode _resyncMode;
	int _sfx;
	bool _soundFlag;
	bool _bgLoadFlag;
	bool _showWhiteBars;
	Common::Array<ResourceEntry> _resources;
	Common::Array<ResIndexEntry> _resIndex;
	int _v1;
	int _v2;
	int _resourceIndex;
	SceneInfo *_sceneInfo;
	Animation *_currentAnimation;
	int _manualFrameNumber;
	SpriteAsset *_manualSpriteSet;
	int _manualStartFrame, _manualEndFrame;
	int _manualFrame2;
	bool _hasManual;
	int _animFrameNumber;
private:
	void checkResource(const Common::String &resourceName);

	int scanResourceIndex(const Common::String &resourceName);

	void load();

	void processLines();

	void processCommand();

	int getParameter();

	void scriptDone();

	void loadNextResource();
protected:
	virtual void display();

	virtual void doFrame();

	virtual bool onEvent(Common::Event &event);
public:
	/**
	* Queue the given text resource for display
	*/
	static void execute(MADSEngine *vm, const Common::String &resName);

	AnimationView(MADSEngine *vm);

	virtual ~AnimationView();
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_MENU_NEBULAR_H */
