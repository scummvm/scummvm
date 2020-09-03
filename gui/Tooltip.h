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

#ifndef GUI_TOOLTIP_H
#define GUI_TOOLTIP_H

#include "common/keyboard.h"
#include "common/str-array.h"
#include "gui/dialog.h"

namespace GUI {

class Widget;

class Tooltip : public Dialog {
private:
	Dialog *_parent;

public:
	Tooltip();

	void setup(Dialog *parent, Widget *widget, int x, int y);

	void drawDialog(DrawLayer layerToDraw) override;

	void receivedFocus(int x = -1, int y = -1) override {}
protected:
	void handleMouseDown(int x, int y, int button, int clickCount) override {
		close();
		_parent->handleMouseDown(x + (getAbsX() - _parent->getAbsX()), y + (getAbsY() - _parent->getAbsY()), button, clickCount);
	}
	void handleMouseUp(int x, int y, int button, int clickCount) override {
		close();
		_parent->handleMouseUp(x + (getAbsX() - _parent->getAbsX()), y + (getAbsY() - _parent->getAbsY()), button, clickCount);
	}
	void handleMouseWheel(int x, int y, int direction) override {
		close();
		_parent->handleMouseWheel(x + (getAbsX() - _parent->getAbsX()), y + (getAbsX() - _parent->getAbsX()), direction);
	}
	void handleKeyDown(Common::KeyState state) override {
		close();
		_parent->handleKeyDown(state);
	}
	void handleKeyUp(Common::KeyState state) override {
		close();
		_parent->handleKeyUp(state);
	}
	void handleMouseMoved(int x, int y, int button) override {
		close();
	}

	int _maxWidth;
	int _xdelta, _ydelta;

	Common::U32StringArray _wrappedLines;
};

} // End of namespace GUI

#endif // GUI_TOOLTIP_H
