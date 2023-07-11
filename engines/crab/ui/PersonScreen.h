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

#ifndef CRAB_PERSONSCREEN_H
#define CRAB_PERSONSCREEN_H

#include "crab/event/GameEventInfo.h"
#include "crab/ui/ImageData.h"
#include "crab/ui/TraitMenu.h"
#include "crab/event/gameevent.h"
#include "crab/level/level.h"
#include "crab/ui/menu.h"
#include "crab/people/person.h"
#include "crab/animation/sprite.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class PersonScreen {
	// The background
	ImageData bg;

	// The name of the character
	TextData name;

	// The place where the sprite should be drawn
	Element img;

	// The buttons for drawing traits
	TraitMenu menu;

	// Store the current person sprite temporarily
	pyrodactyl::anim::Sprite *cur_sp;

public:
	PersonScreen() { cur_sp = nullptr; }
	~PersonScreen() {}

	void load(const Common::String &filename);
	void Cache(pyrodactyl::event::Info &info, const Common::String &id, pyrodactyl::level::Level &level);

	void handleEvents(pyrodactyl::event::Info &info, const Common::String &id, const Common::Event &Event);
#if 0
	void handleEvents(pyrodactyl::event::Info &info, const Common::String &id, const SDL_Event &Event);
#endif

	void internalEvents();
	void draw(pyrodactyl::event::Info &info, const Common::String &id);

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_PERSONSCREEN_H
