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

#include "engines/game.h"
#include "common/gui_options.h"
#include "common/language.h"

namespace Glk {
namespace Alan3 {

const PlainGameDescriptor ALAN3_GAME_LIST[] = {
	{ "alan3", "Alan3 Game" },

	{ "catchcat", "Catch That Cat" },
	{ "christmasparty", "The Christmas Party" },
	{ "deadleaves", "City of Dead Leaves" },
	{ "enterthedark", "Enter The Dark" },
	{ "fishmess", "A Very Hairy Fish-Mess" },
	{ "forbidden", "The Ngah Angah School of Forbidden Wisdom" },
	{ "hwmurders", "Hollywood Murders" },
	{ "indigosoul", "IN-D-I-GO SOUL" },
	{ "misguided", "Mis/Guided" },
	{ "onaar", "Onaar" },
	{ "room206", "Room 206" },
	{ "tedpaladin", "Ted Paladin And The Case Of The Abandoned House" },
	{ "tedstrikesback", "Ted Strikes Back" },
	{ "thesealedroom", "The Sealed Room" },
	{ "vrgambler", "VR Gambler" },
	{ "waldospie", "Waldo's Pie" },
	{ "wyldkyndproject",  "The Wyldkynd Project" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry ALAN3_GAMES[] = {
	DT_ENTRY0("catchcat", "758325a2c46f72218a82f22736ea7017", 43878),
	DT_ENTRY0("christmasparty", "86b87969d124c213632398980ec87c23", 94892),
	DT_ENTRY0("deadleaves", "7c228698507508043d1d3938695e28cd", 90139),
	DT_ENTRY0("enterthedark", "739add0fec3dbd18a63389826ffeba4d", 78900),
	DT_ENTRY0("fishmess", "e9952cfbe2adef5dcef82abd57661f60", 312561),
	DT_ENTRY0("forbidden", "8ce2b96851d43a72c84144632d6a084f", 122072),
	DT_ENTRY0("hwmurders", "abadbb15faf7f0b7324222fdea6bd495", 213539),
	DT_ENTRY0("indigosoul", "2c87486c4a44f5ef5a704a0b55763c61", 163903),
	DT_ENTRY0("misguided", "cc2c6e724d599e731efa9b7a34ae4f51", 672613),
	DT_ENTRY0("onaar", "f809531286f1757ab113fe3592a49d3b", 1516301),
	DT_ENTRY0("room206", "eb5711ecfad102ee4d9fda7fcb3ddf78", 364156),
	DT_ENTRY0("tedpaladin", "6132d401ea7c05b474e598a37b642bd9", 292486),
	DT_ENTRY0("tedstrikesback", "6085ede584b3705370b2a689cc96419a", 407634),
	DT_ENTRY0("thesealedroom", "06a81cb30efff9c63e7f7d3aff554d3d", 306937),
	DT_ENTRY0("vrgambler", "c29b8276a9a1be1444193eb3724bf0fd", 938147),
	DT_ENTRY0("waldospie", "fcd53ac4669f85845c1cbbaad6082724", 511956),
	DT_ENTRY0("wyldkyndproject", "35b2f168cb78d7df46638dcb583f5e15", 1333195),
	DT_END_MARKER
};

} // End of namespace Alan3
} // End of namespace Glk
