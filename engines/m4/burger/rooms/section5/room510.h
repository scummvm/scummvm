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

#ifndef M4_BURGER_ROOMS_SECTION5_ROOM510_H
#define M4_BURGER_ROOMS_SECTION5_ROOM510_H

#include "m4/burger/rooms/section5/section5_room.h"
#include "m4/burger/gui/inventory.h"

namespace M4 {
namespace Burger {
namespace Rooms {

#define BUTTON_COUNT 7

class Room510 : public Section5Room {
private:
	struct ButtonDef {
		int _x1;
		int _y1;
		int _x2;
		int _y2;
		int _index0;
		int _index1;
		int _index2;
		int _index3;
		int _state;
		int _val10;
	};
	struct Button : public ButtonDef {
		machine *_series = nullptr;
		Button() : ButtonDef() {}
		Button(const ButtonDef &src) : ButtonDef(src) {}
	};
	static const ButtonDef _BUTTONS[BUTTON_COUNT];
	Button _buttons[BUTTON_COUNT];
	//machine *_buttonSeries[BUTTON_COUNT];
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;

	void setupButton(int index);
	void setButtonState(int index, GUI::ControlStatus state);
	void buttonAction(int index, int firstFrame, int lastFrame);
	void updateButtons();
	int getHighlightedButton() const;

public:
	Room510();
	~Room510() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
