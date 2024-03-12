
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

#ifndef M4_BURGER_HOTKEYS_H
#define M4_BURGER_HOTKEYS_H

#include "m4/m4_types.h"
#include "m4/gui/hotkeys.h"
#include "m4/core/mouse.h"

namespace M4 {
namespace Burger {

struct Hotkeys : public M4::Hotkeys {
private:
	static Dialog *_versionDialog;

private:
	static void t_cb(void *, void *);
	static void u_cb(void *, void *);
	static void l_cb(void *, void *);
	static void a_cb(void *, void *);

	/**
	 * Show version dialog
	 */
	static void show_version(void *a, void *b);
	static void version_ok_button(void *a, void *b);

public:
	Hotkeys();
	virtual ~Hotkeys() {}

	/**
	 * Called when the Escape key is pressed
	 */
	static void escape_key_pressed(void *, void *);

	void add_hot_keys() override;

	void toggle_through_cursors(CursorChange cursChange = CURSCHANGE_NEXT) override;
};

} // namespace Burger
} // namespace M4

#endif
