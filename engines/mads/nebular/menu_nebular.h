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
private:
	void handleEvents();
protected:
	bool _breakFlag;
	bool _redrawFlag;

	virtual void doFrame() = 0;

	virtual bool onEvent(Common::Event &event) = 0;

	virtual void display();
public:
	MenuView(MADSEngine *vm);

	virtual ~MenuView() {}

	virtual void show();
};

class MainMenu: public MenuView {
private:
	Common::Point _itemPosList[6];
	SpriteAsset *_menuItems[7];
	int _menuItemIndexes[7];
	int _menuItemIndex;
	int _frameIndex;
	uint32 _delayTimeout;
	bool _skipFlag;
	int _highlightedIndex;

	int getHighlightedItem(int x, int y);
	void handleAction(MADSGameAction action);
	void addSpriteSlot();
protected:
	virtual void display();

	virtual void doFrame();

	virtual bool onEvent(Common::Event &event);
public:
	MainMenu(MADSEngine *vm);
	virtual ~MainMenu();
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_MENU_NEBULAR_H */
