/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mutationofjb/guiscreen.h"

#include "mutationofjb/widgets/widget.h"

#include "graphics/screen.h"

namespace MutationOfJB {

GuiScreen::GuiScreen(Game &game, Graphics::Screen *screen)
	: _game(game),
	  _screen(screen) {}

GuiScreen::~GuiScreen() {
	for (Common::Array<Widget *>::iterator it = _widgets.begin(); it != _widgets.end(); ++it) {
		delete *it;
	}
}

Game &GuiScreen::getGame() {
	return _game;
}

void GuiScreen::markDirty() {
	for (Common::Array<Widget *>::iterator it = _widgets.begin(); it != _widgets.end(); ++it) {
		if ((*it)->isVisible()) {
			(*it)->markDirty();
		}
	}
}

void GuiScreen::handleEvent(const Common::Event &event) {
	for (Common::Array<Widget *>::iterator it = _widgets.begin(); it != _widgets.end(); ++it) {
		if ((*it)->isVisible()) {
			(*it)->handleEvent(event);
		}
	}
}

void GuiScreen::update() {
	for (Common::Array<Widget *>::iterator it = _widgets.begin(); it != _widgets.end(); ++it) {
		if ((*it)->isVisible()) {
			(*it)->update(*_screen);
		}
	}
}

void GuiScreen::addWidget(Widget *widget) {
	_widgets.push_back(widget);
	widget->markDirty();
}

const GuiScreen::Widgets &GuiScreen::getWidgets() const {
	return _widgets;
}

}
