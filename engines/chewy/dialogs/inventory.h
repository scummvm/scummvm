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

#ifndef CHEWY_DIALOGS_INVENTORY_H
#define CHEWY_DIALOGS_INVENTORY_H

#include "chewy/ngstypes.h"

namespace Chewy {
namespace Dialogs {

class Inventory {
private:
	static int16 inv_rand_x;
	static int16 inv_rand_y;

	static bool calc_use_invent(int16 invNr);
	static void showDiary();
public:
	static void plot_menu();
	static void menu();
	static int16 look(int16 invent_nr, int16 mode, int16 ats_nr);
	static void look_screen(int16 txt_mode, int16 txt_nr);
};

} // namespace Dialogs
} // namespace Chewy

#endif
