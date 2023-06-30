
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

#ifndef M4_BURGER_INTERFACE_H
#define M4_BURGER_INTERFACE_H

#include "m4/m4_types.h"
#include "m4/graphics/graphics.h"
#include "m4/graphics/gr_buff.h"
#include "m4/burger/gui_cheapo.h"

namespace M4 {
namespace Burger {

struct Interface {
private:
	void setup();
public:
	int _sprite = 22; // main_interface_sprite;
	int _arrow = 0;
	int _wait = 0;
	int _look = 0;
	int _grab = 0;
	int _use = 0;
	bool _visible = false;
	bool _shown = false;
	int _x1 = 0;
	int _y1 = 374;
	int _x2 = SCREEN_WIDTH;
	int _y2 = SCREEN_HEIGHT;
	GrBuff *_gameInterfaceBuff = nullptr;
	InterfaceBox *_interfaceBox = nullptr;
	Inventory *_inventory = nullptr;
	TextField *_textField = nullptr;
	ButtonClass *_btnTake = nullptr;
	ButtonClass *_btnManipulate = nullptr;
	ButtonClass *_btnHandle = nullptr;
	ButtonClass *_btnAbductFail = nullptr;
	ButtonClass *_btnMenu = nullptr;
	ButtonClass *_btnScrollLeft = nullptr;
	ButtonClass *_btnScrollRight = nullptr;

	~Interface();
	void init(int arrow, int wait, int look, int grab, int use);

	void show();
};

} // namespace Burger
} // namespace M4

#endif
