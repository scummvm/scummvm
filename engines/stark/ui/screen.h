/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_UI_SCREEN_H
#define STARK_UI_SCREEN_H

#include "common/array.h"
#include "common/rect.h"

#include "engines/stark/ui/window.h"

namespace Stark {

/**
 * Screens are individual pages of the user interface
 */
class Screen {
public:
	enum Name {
		kScreenMainMenu,
		kScreenGame,
		kScreenFMV,
		kScreenDiaryIndex,
		kScreenSettingsMenu,
		kScreenSaveMenu,
		kScreenLoadMenu,
		kScreenFMVMenu,
		kScreenDiaryPages,
		kScreenDialog
	};

	explicit Screen(Name name) : _name(name) {};
	virtual ~Screen() {};

	/** Obtain the name of the screen */
	Name getName() const { return _name; }

	/** The screen is being made active, prepare it to be drawn */
	virtual void open() {}

	/** The screen is no longer active, release resources to free memory */
	virtual void close() {}

	/** Draw the screen */
	virtual void render() = 0;

	/** Called once per game loop when the screen is active. */
	virtual void handleGameLoop() {}

	/** Called when the screen resolution changes */
	virtual void onScreenChanged() {}

	virtual void handleMouseMove() = 0;
	virtual void handleClick() = 0;
	virtual void handleRightClick() = 0;
	virtual void handleDoubleClick() = 0;

private:
	Name _name;
};

class SingleWindowScreen : public Screen, public Window {
public:
	SingleWindowScreen(Name name, Gfx::Driver *gfx, Cursor *cursor) :
			Screen(name), Window(gfx, cursor) {}

	void handleGameLoop() override { Window::handleGameLoop(); }
	void render() override { Window::render(); }

	void handleMouseMove() override { Window::handleMouseMove(); }
	void handleClick() override { Window::handleClick(); }
	void handleRightClick() override { Window::handleRightClick(); }
	void handleDoubleClick() override { Window::handleDoubleClick(); }
};

} // End of namespace Stark

 #endif // STARK_UI_SCREEN_H
