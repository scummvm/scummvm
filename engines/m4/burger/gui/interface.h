
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
#include "m4/adv_r/adv_hotspot.h"
#include "m4/graphics/graphics.h"
#include "m4/graphics/gr_buff.h"
#include "m4/burger/gui/inventory.h"

namespace M4 {
namespace Burger {
namespace GUI {

struct Interface : public M4::Interface {
private:
	void setup();

	void trackIcons();
	ControlStatus trackHotspots(int event, int x, int y);
	void dispatch_command();
	void handleState(ControlStatus status);

public:
	int _sprite = 22; // main_interface_sprite;
	bool _shown = false;
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
	const HotSpotRec *_hotspot = nullptr;
	int _savedX = 0, _savedY = 0;
	char _prepText[40] = { 0 };
	char _nounText[40] = { 0 };
	char _verbText[40] = { 0 };
	bool _iconSelected = false;
	int _state = 0;

	Interface();
	~Interface() override;

	bool init(int arrow, int wait, int look, int grab, int use) override;

	void cancel_sentence() override;

	void freshen_sentence() override;

	bool set_interface_palette(RGB8 *myPalette) override;

	void track_hotspots_refresh() override;

	bool eventHandler(void *bufferPtr, int32 eventType, int32 event, int32 x, int32 y, bool *z) override;

	void show() override;

	void refresh_right_arrow();
	void refresh_left_arrow();

	void l_cb();
	void u_cb();
	void t_cb();
	void a_cb();
};

} // namespace Interface
} // namespace Burger
} // namespace M4

#endif
