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

#ifndef PELROCK_EXTRASCREENS_H
#define PELROCK_EXTRASCREENS_H

#include "pelrock/pelrock.h"

namespace Pelrock {

class PelrockEngine;

struct Spell {
	Common::StringArray text;
    int page;
	byte *image;
};

struct Bookmark {
	int16 x;
	int16 y;
	byte w;
	byte h;
	int page;
};

static const Bookmark _bookmarks[13] = {
	{244, 8, 23, 40, 0},
	{396, 17, 44, 20, 1},
	{480, 68, 40, 42, 4},
	{480, 129, 30, 25, 5},
	{480, 224, 28, 32, 8},
	{416, 344, 23, 17, 12},
	{368, 346, 28, 34, 11},
	{198, 340, 26, 28, 10},
	{164, 336, 33, 17, 9},
	{95, 277, 33, 24, 7},
	{105, 227, 27, 33, 6},
	{103, 118, 30, 26, 3},
    {101, 78, 36, 33, 2}
};

class SpellBook {

public:
	SpellBook(PelrockEventManager *eventMan, ResourceManager *res);
	~SpellBook();

	/**
	 * returns the spell the user selected
	 */
	Spell *run();

private:
	PelrockEventManager *_events;
	ResourceManager *_res;
	byte *_backgroundScreen;
	byte *_compositeScreen;
	byte *_palette;

	Spell *_spell = nullptr;

	Spell *_selectedSpell = nullptr;
	void init();
	void selectPage(int page);
	void drawScreen();
	void loadBackground();
	void cleanup();
	void checkMouse(int x, int y);
};

} // End of namespace Pelrock

#endif // PELROCK_EXTRASCREENS_H
