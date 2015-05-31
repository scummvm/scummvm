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

#ifndef SHERLOCK_TATTOO_UI_H
#define SHERLOCK_TATTOO_UI_H

#include "common/scummsys.h"
#include "sherlock/user_interface.h"

namespace Sherlock {

namespace Tattoo {

class TattooUserInterface : public UserInterface {
private:
	Common::Rect _menuBounds;
	Common::Rect _oldMenuBounds;
	Common::Rect _invMenuBounds;
	Common::Rect _oldInvMenuBounds;
	Common::Rect _tagBounds;
	Common::Rect _oldTagBounds;
	Common::Rect _invGraphicBounds;
	Common::Rect _oldInvGraphicBounds;
	Surface *_menuBuffer;
	Surface *_invMenuBuffer;
	Surface *_tagBuffer;
	Surface *_invGraphic;
	Common::Array<Common::Rect> _grayAreas;
private:
	/**
	 * Draws designated areas of the screen that are meant to be grayed out using grayscale colors
	 */
	void drawGrayAreas();
public:
	Common::Point _currentScroll, _targetScroll;
	int _scrollSize, _scrollSpeed;
	bool _drawMenu;
public:
	TattooUserInterface(SherlockEngine *vm);

	/**
	 * Handles restoring any areas of the back buffer that were/are covered by UI elements
	 */
	void doBgAnimRestoreUI();

	/**
	 * Checks to see if the screen needs to be scrolled. If so, scrolls it towards the target position
	 */
	void doScroll();
public:
	virtual ~TattooUserInterface() {}

	/**
	 * Main input handler for the user interface
	 */
	virtual void handleInput();

	/**
	 * Draw the user interface onto the screen's back buffers
	 */	
	virtual void drawInterface(int bufferNum = 3);

	/**
	 * Initializes scroll variables
	 */
	virtual void initScrollVars();
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
