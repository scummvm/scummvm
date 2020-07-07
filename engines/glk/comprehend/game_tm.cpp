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
	_gameDataFile = "G0";

	_locationGraphicFiles.push_back("RA");
	_locationGraphicFiles.push_back("RB");
	_locationGraphicFiles.push_back("RC");
	_locationGraphicFiles.push_back("RD");
	_locationGraphicFiles.push_back("RE");
	_locationGraphicFiles.push_back("RF");
	_locationGraphicFiles.push_back("RG");
	_itemGraphicFiles.push_back("OA");
	_itemGraphicFiles.push_back("OB");
	_itemGraphicFiles.push_back("OE");
	_itemGraphicFiles.push_back("OF");
}

} // namespace Comprehend
} // namespace Glk
