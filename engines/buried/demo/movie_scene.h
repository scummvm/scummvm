/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#ifndef BURIED_DEMO_MOVIE_SCENE_H
#define BURIED_DEMO_MOVIE_SCENE_H

#include "buried/window.h"

namespace Graphics {
struct Surface;
}

namespace Buried {

class MovieDisplayWindow : public Window {
public:
	MovieDisplayWindow(BuriedEngine *vm, Window *parent, const Common::String &background, const Common::String &movie, int movieLeft, int movieTop);
	~MovieDisplayWindow();

	bool showMovieInWindow();

	void onPaint();
	bool onEraseBackground();
	void onLButtonUp(const Common::Point &point, uint flags);
	void onTimer(uint timer);

private:
	Graphics::Surface *_background;
	VideoWindow *_movie;
	uint _timer;
};

} // End of namespace Buried

#endif
