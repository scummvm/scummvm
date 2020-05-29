/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/game_tm.h"

namespace Glk {
namespace Comprehend {

/* FIXME - This is broken */

TalismanGame::TalismanGame() : ComprehendGame() {
	game_name = "Talisman, Challenging the Sands of Time (broken)";
	short_name = "tm";
	game_data_file = "G0";

	location_graphic_files.push_back("RA");
	location_graphic_files.push_back("RB");
	location_graphic_files.push_back("RC");
	location_graphic_files.push_back("RD");
	location_graphic_files.push_back("RE");
	location_graphic_files.push_back("RF");
	location_graphic_files.push_back("RG");
	item_graphic_files.push_back("OA");
	item_graphic_files.push_back("OB");
	item_graphic_files.push_back("OE");
	item_graphic_files.push_back("OF");
}

} // namespace Comprehend
} // namespace Glk
