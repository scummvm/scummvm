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

#ifndef PELROCK_BACKGROUNDBOOK_H
#define PELROCK_BACKGROUNDBOOK_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/managed_surface.h"

#include "pelrock/pelrock.h"

namespace Pelrock {

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
	Buttons isButtonClicked(int x, int y);
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
		Common::Rect(Common::Point(238, 104), 28, 44), // Previous
		Common::Rect(Common::Point(238, 178), 28, 44), // Next
	};
	Common::StringArray _roomNames;
};

} // End of namespace Pelrock

#endif
