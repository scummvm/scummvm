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

#ifndef BURIED_CREDITS_H
#define BURIED_CREDITS_H

#include "buried/window.h"

namespace Graphics {
struct Surface;
}

namespace Buried {

class AVIFrames;

class CreditsWindow : public Window {
public:
	CreditsWindow(BuriedEngine *vm, Window *parent);
	~CreditsWindow();

	bool showCredits();

	void onPaint();
	bool onEraseBackground();
	void onLButtonDown(const Common::Point &point, uint flags);

private:
	Graphics::Surface *_normalImage;
	Graphics::Surface *_highlightedImage;

	Common::Rect _buttons[5];
	Common::Rect _returnButton;

	int _curButton;
	int _curPage;

	AVIFrames *_stillFrames;
};

} // End of namespace Buried

#endif
