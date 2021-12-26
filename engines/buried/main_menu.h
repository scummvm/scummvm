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

#ifndef BURIED_MAIN_MENU_H
#define BURIED_MAIN_MENU_H

#include "buried/window.h"

namespace Graphics {
struct Surface;
}

namespace Buried {

class MainMenuWindow : public Window {
public:
	MainMenuWindow(BuriedEngine *vm, Window *parent);
	~MainMenuWindow();

	bool showMainMenu();

	void onPaint();
	bool onEraseBackground();
	void onLButtonUp(const Common::Point &point, uint flags);
	void onLButtonDown(const Common::Point &point, uint flags);
	void onMouseMove(const Common::Point &point, uint flags);

private:
	Common::Rect _buttons[5];
	Common::Rect _playMode[2];
	Common::Rect _intro[2];

	int _curButton;
	bool _showIntro;
	bool _walkthrough;
	bool _buttonDrawnDown;
	bool _disableDrawing;

	Graphics::Surface *_background;
	Graphics::Surface *_depressedPlayMode;
	Graphics::Surface *_depressedPlayIntro;
	Graphics::Surface *_depressedOverview;
	Graphics::Surface *_depressedNewGame;
	Graphics::Surface *_depressedRestoreGame;
	Graphics::Surface *_depressedCredits;
	Graphics::Surface *_depressedQuit;
};

} // End of namespace Buried

#endif
