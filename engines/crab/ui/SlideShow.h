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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_SLIDESHOW_H
#define CRAB_SLIDESHOW_H

#include "crab/image/Image.h"
#include "crab/ui/ImageData.h"
#include "crab/ui/button.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class SlideShow {
	// The list of images and where they are located, and the current image
	Common::Array<Common::String> path;

	// We only load the current image in memory
	pyrodactyl::image::Image img;

	// The index of our current image
	unsigned int index;

	// The position at which map image has to be drawn
	Element pos;

	// Background image of the slide show
	ImageData bg;

	Button prev, next;
	bool usekeyboard;

public:
	SlideShow() {
		index = 0;
		usekeyboard = false;
	}
	~SlideShow() { Clear(); }

	void Refresh();
	void Clear() { img.Delete(); }

	void load(rapidxml::xml_node<char> *node);

	void HandleEvents(const Common::Event &Event);
#if 0
	void HandleEvents(const SDL_Event &Event);
#endif
	void draw();

	void setUI();
};

} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_SLIDESHOW_H
