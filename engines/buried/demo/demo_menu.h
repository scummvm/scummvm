/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_DEMO_MENU_H
#define BURIED_DEMO_MENU_H

#include "buried/window.h"

namespace Graphics {
struct Surface;
}

namespace Buried {

class DemoMainMenuWindow : public Window {
public:
	DemoMainMenuWindow(BuriedEngine *vm, Window *parent);
	~DemoMainMenuWindow();

	void showWithSplash();

	void onPaint();
	bool onEraseBackground();
	void onLButtonUp(const Common::Point &point, uint flags);
	void onLButtonDown(const Common::Point &point, uint flags);

private:
	Common::Rect _overview;
	Common::Rect _trailer;
	Common::Rect _interactive;
	Common::Rect _gallery;
	Common::Rect _quit;
	int _curButton;
	//bool _buttonDrawnDown;
	Graphics::Surface *_background;
};

} // End of namespace Buried

#endif
