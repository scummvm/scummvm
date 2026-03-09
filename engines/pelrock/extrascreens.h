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

#include "graphics/managed_surface.h"

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
	Graphics::ManagedSurface _backgroundScreen;
	Graphics::ManagedSurface _compositeScreen;
	byte *_palette;

	Spell *_spell = nullptr;

	Spell *_selectedSpell = nullptr;
	void init();
	void selectPage(int page);
	void drawScreen();
	void loadBackground();
	void cleanup();
	bool checkMouse(int x, int y);
};


class CDPlayer {

enum CDControls {
	STOP_BUTTON,
	PAUSE_BUTTON,
	PLAY_BUTTON,
	PREVIOUS_BUTTON,
	NEXT_BUTTON,
	NO_CDBUTTON
};

public:
	CDPlayer(PelrockEventManager *eventMan, ResourceManager *res, SoundManager *sound);
	~CDPlayer();

	void run();

private:
	void init();
	void loadTrackNames();
	void drawScreen();
	void drawButtons();
	void loadBackground();
	void loadControls();
	void checkMouse(int x, int y);
	void cleanup();
	CDControls isButtonClicked(int x, int y);
	ResourceManager *_res;
	SoundManager *_sound;
	PelrockEventManager *_events;
	Graphics::ManagedSurface _backgroundScreen;
	Graphics::ManagedSurface _compositeScreen;
	byte *_palette;
	byte *_controls;
	Common::String trackNames[31];
	byte *buttons[5][2];
	Common::Rect _buttonRects[5] = {
		Common::Rect(Common::Point(17, 46), 37, 26), // Stop
		Common::Rect(Common::Point(57, 48), 33, 23), // Pause
		Common::Rect(Common::Point(92, 44), 34, 28), // Play
		Common::Rect(Common::Point(128, 45), 38, 24), // Previous
		Common::Rect(Common::Point(168, 44), 41, 28) // Next
	};
	int _selectedTrack = 2;
	CDControls _selectedButton = NO_CDBUTTON;

};


class BackgroundBook {
enum Buttons {
	PREVIOUS_BUTTON,
	NEXT_BUTTON,
	NO_BG_BUTTON
};
	int kItemsPerPage = 22;

public:
	BackgroundBook(PelrockEventManager *eventMan, ResourceManager *res, RoomManager *room);
	~BackgroundBook();

	void run();

private:
	void init();
	void loadRoomNames();
	void drawScreen();
	void drawButtons();
	void loadButtons();
	void loadBackground();
	void checkMouse(int x, int y);
	BackgroundBook::Buttons isButtonClicked(int x, int y);
	void showRoom(int roomIndex);
	void cleanup();

	PelrockEventManager *_events;
	ResourceManager *_res;
	RoomManager *_room;
	Graphics::ManagedSurface _backgroundScreen;
	Graphics::ManagedSurface _compositeScreen;
	Graphics::ManagedSurface *thumbSurface = nullptr;
	byte *_palette;
	byte *_buttons[2][2];
	Buttons _selectedButton = NO_BG_BUTTON;
	int _selectedPage = 0;

	Common::Rect _buttonRects[2] = {
		Common::Rect(Common::Point(238, 104), 28, 44), // Stop
		Common::Rect(Common::Point(238, 178), 28, 44), // Pause
	};
	Common::StringArray _roomNames;
};

} // End of namespace Pelrock

#endif // PELROCK_EXTRASCREENS_H
