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

#include "xeen/worldofxeen/worldofxeen_resources.h"

namespace Xeen {
namespace WorldOfXeen {

// FIXME: These should really go into into the xeen.ccs datafile
const char *const WorldOfXeenResources::CLOUDS_INTRO1 = "\f00\v082\t040\x3"
	"cKing Burlock\v190\t040Peasants\v082\t247"
	"Lord Xeen\v190\t258Xeen's Pet\v179\t150Crodo";

const char *const WorldOfXeenResources::DARKSIDE_ENDING1 = "\n\x3" "cCongratulations\n"
	"\n"
	"Your Final Score is:\n"
	"\n"
	"%010lu\n"
	"\x3" "l\n"
	"Please send this score to the Ancient's Headquarters "
	"where you'll be added to the Hall of Legends!\n"
	"\n"
	"Ancient's Headquarters\n"
	"New World Computing, Inc.\n"
	"P.O. Box 4302\n"
	"Hollywood, CA 90078";

const char *const WorldOfXeenResources::DARKSIDE_ENDING2 = "\n"
	"Adventurers,\n"
	"\n"
	"I will save your game in Castleview.\n"
	"\n"
	"The World of Xeen still needs you!\n"
	"\n"
	"Load your game afterwards and come visit me in the "
	"Great Pyramid for further instructions";

const char *const WorldOfXeenResources::PHAROAH_ENDING_TEXT1 = "\fd\v001\t001%s\x3" "c\t000\v180Press a Key!\x3" "l";
const char *const WorldOfXeenResources::PHAROAH_ENDING_TEXT2 = "\f04\v000\t000%s\x3" "c\t000\v180Press a Key!\x3" "l\fd";

} // End of namespace WorldOfXeen
} // End of namespace Xeen
