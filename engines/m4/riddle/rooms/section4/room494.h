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

#ifndef M4_RIDDLE_ROOMS_SECTION4_ROOM494_H
#define M4_RIDDLE_ROOMS_SECTION4_ROOM494_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room494 : public Room {
private:
	static void escapeFn(void *, void *);
	int getSelectedButton() const;
	void restoreHotkeys();

private:
	int _menuButtons = 0;
	int _selectedBtn2 = -1;
	int _selectedBtn1 = -1;
	machine *_machine1 = nullptr;
	machine *_machine2 = nullptr;
	machine *_buttons[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	HotkeyCB _hotkeyEscape = nullptr;
	HotkeyCB _hotkeySave = nullptr;
	HotkeyCB _hotkeyLoad = nullptr;
	bool _selectState = false;

public:
	Room494() : Room() {}
	~Room494() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void pre_parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
