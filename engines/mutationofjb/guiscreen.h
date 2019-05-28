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

#ifndef MUTATIONOFJB_GUISCREEN_H
#define MUTATIONOFJB_GUISCREEN_H

#include "common/array.h"

namespace Common {
struct Event;
}

namespace Graphics {
class Screen;
}

namespace MutationOfJB {

class Game;
class Widget;

/**
 * Base class for GUI screens.
 *
 * GUI screen is a collection of widgets.
 */
class GuiScreen {
public:

	GuiScreen(Game &game, Graphics::Screen *screen);
	virtual ~GuiScreen();
	Game &getGame();

	/**
	 * Marks all visible widgets as dirty (needs redraw).
	 */
	void markDirty();

	/**
	 * Lets all visible widgets handle core events.
	 *
	 * @param event ScummVM event.
	 */
	virtual void handleEvent(const Common::Event &event);

	/**
	 * Updates all visible widgets.
	 */
	void update();

	/**
	 * Adds a widget to the GUI screen.
	 * The GUI screen will own the widget.
	 *
	 * @param widget Widget to add.
	 */
	void addWidget(Widget *widget);

protected:
	typedef Common::Array<Widget *> Widgets;

	Game &_game;
	Graphics::Screen *_screen;

	const Widgets &getWidgets() const;

private:
	Widgets _widgets;
};

}

#endif
