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

#include "engines/game.h"
#include "common/gui_options.h"
#include "common/language.h"

namespace Glk {
namespace Alan3 {

const PlainGameDescriptor ALAN3_GAME_LIST[] = {
	{ "alan3", "Alan3 Game" },

	{ "christmasparty", "The Christmas Party" },
	{ "deadleaves", "City of Dead Leaves" },
	{ "enterthedark", "Enter The Dark" },
	{ "fishmess", "Fish Mess" },
	{ "forbidden", "The Ngah Angah School of Forbidden Wisdom" },
	{ "hwmurders", "Hollywood Murders" },
	{ "misguided", "Mis/Guided" },
	{ "room206", "Room 206" },
	{ "tedpaladin", "Ted Paladin And The Case Of The Abandoned House" },
	{ "thesealedroom", "The Sealed Room" },
	{ "waldospie", "Waldo's Pie" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry ALAN3_GAMES[] = {
	DT_ENTRY0("christmasparty", "86b87969d124c213632398980ec87c23", 94892),
	DT_ENTRY0("deadleaves", "7c228698507508043d1d3938695e28cd", 90139),
	DT_ENTRY0("enterthedark", "739add0fec3dbd18a63389826ffeba4d", 78900),
	DT_ENTRY0("fishmess", "e9952cfbe2adef5dcef82abd57661f60", 312561),
	DT_ENTRY0("forbidden", "8ce2b96851d43a72c84144632d6a084f", 122072),
	DT_ENTRY0("hwmurders", "abadbb15faf7f0b7324222fdea6bd495", 213539),
	DT_ENTRY0("misguided", "cc2c6e724d599e731efa9b7a34ae4f51", 672613),
	DT_ENTRY0("room206", "eb5711ecfad102ee4d9fda7fcb3ddf78", 364156),
	DT_ENTRY0("tedpaladin", "6132d401ea7c05b474e598a37b642bd9", 292486),
	DT_ENTRY0("thesealedroom", "06a81cb30efff9c63e7f7d3aff554d3d", 306937),
	DT_ENTRY0("waldospie", "fcd53ac4669f85845c1cbbaad6082724", 511956),
	DT_END_MARKER
};

} // End of namespace Alan3
} // End of namespace Glk
