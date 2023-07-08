
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

#include "m4/adv_r/adv_interface.h"
#include "m4/graphics/graphics.h"
#include "m4/graphics/gr_buff.h"
#include "m4/burger/gui/gui_cheapo.h"

namespace M4 {
namespace Burger {
namespace GUI {

struct Interface : public M4::Interface {
private:
	void setup();
public:
	int _sprite = 22; // main_interface_sprite;
	bool _visible = false;
	bool _shown = false;
	GrBuff *_gameInterfaceBuff = nullptr;
	GUI::InterfaceBox *_interfaceBox = nullptr;
	GUI::Inventory *_inventory = nullptr;
	GUI::TextField *_textField = nullptr;
	GUI::ButtonClass *_btnTake = nullptr;
	GUI::ButtonClass *_btnManipulate = nullptr;
	GUI::ButtonClass *_btnHandle = nullptr;
	GUI::ButtonClass *_btnAbductFail = nullptr;
	GUI::ButtonClass *_btnMenu = nullptr;
	GUI::ButtonClass *_btnScrollLeft = nullptr;
	GUI::ButtonClass *_btnScrollRight = nullptr;
	void *_hotspot = nullptr;
	uint32 _counter = 0;

	Interface();
	~Interface() override;

	void init(int arrow, int wait, int look, int grab, int use) override;

	void cancel_sentence() override;

	bool set_interface_palette(RGB8 *myPalette) override;

	void track_hotspots_refresh() override;

	bool eventHandler(void *bufferPtr, int32 eventType, int32 event, int32 x, int32 y, bool *z) override;

	void show() override;
};

} // namespace Interface
} // namespace Burger
} // namespace M4

#endif
