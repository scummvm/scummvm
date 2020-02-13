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

#ifndef MADS_MENU_NEBULAR_H
#define MADS_MENU_NEBULAR_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/menu_views.h"
#include "mads/msurface.h"
#include "mads/nebular/dialogs_nebular.h"

namespace MADS {

class MADSEngine;

namespace Nebular {

enum MADSGameAction {
	START_GAME, RESUME_GAME, SHOW_INTRO, CREDITS, QUOTES, EXIT,
	SETS, EVOLVE
};

class MainMenu: public MenuView {
private:
	SpriteAsset *_menuItems[7];
	int _menuItemIndexes[7];
	int _menuItemIndex;
	int _frameIndex;
	uint32 _delayTimeout;
	bool _skipFlag;
	bool _showEvolve, _showSets;

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

	/**
	 * Returns true if the Quotes item should be shown.
	 * i.e. if the player has completed the game
	 */
	bool shouldShowQuotes();

	/**
	 * Show the bonus item icons, if available
	 */
	void showBonusItems();
protected:
	/**
	 * Display the menu
	 */
	void display() override;

	/**
	 * Handle the menu item animations
	 */
	void doFrame() override;

	/**
	 * Event handler
	 */
	bool onEvent(Common::Event &event) override;
public:
	MainMenu(MADSEngine *vm);

	~MainMenu() override;
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
	bool onEvent(Common::Event &event) override;
public:
	AdvertView(MADSEngine *vm);

	~AdvertView() override {}

	/**
	 * Show the dialog
	 */
	void show();
};

class RexAnimationView : public AnimationView {
protected:
	void scriptDone() override;
public:
	RexAnimationView(MADSEngine *vm) : AnimationView(vm) {}
};

class RexTextView : public TextView {
public:
	RexTextView(MADSEngine *vm) : TextView(vm) {}
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_MENU_NEBULAR_H */
