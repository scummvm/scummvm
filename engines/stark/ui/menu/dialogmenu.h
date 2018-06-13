/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_UI_MENU_DIALOG_H
#define STARK_UI_MENU_DIALOG_H

#include "engines/stark/ui/menu/locationscreen.h"

namespace Stark {

class DialogScreen : public StaticLocationScreen {
public:
	DialogScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~DialogScreen();

	// StaticLocationScreen API
	void open() override;

private:
	void backHandler();
};

} // End of namespace Stark

#endif // STARK_UI_MENU_DIALOG_H
