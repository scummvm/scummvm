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

/**
 * Game description
 */
struct Alan3GameDescription {
	const char *const _gameId;
	const char *const _extra;
	const char *const _md5;
	size_t _filesize;
	Common::Language _language;
};

const PlainGameDescriptor ALAN3_GAME_LIST[] = {
	{ "alan3", "Alan3 Game" },

	{ "christmasparty", "The Christmas Party" },
	{ "deadleaves", "City of Dead Leaves" },
	{ "fishmess", "Fish Mess" },
	{ "hwmurders", "Hollywood Murders" },
	{ "misguided", "Mis/Guided" },
	{ "room206", "Room 206" },

	{ nullptr, nullptr }
};

#define ENTRY0(ID, MD5, FILESIZE) { ID, nullptr, MD5, FILESIZE, Common::EN_ANY }
#define TABLE_END_MARKER { nullptr, nullptr, nullptr, 0, Common::EN_ANY }

const Alan3GameDescription ALAN3_GAMES[] = {
	ENTRY0("christmasparty", "86b87969d124c213632398980ec87c23", 94892),
	ENTRY0("deadleaves", "7c228698507508043d1d3938695e28cd", 90139),
	ENTRY0("fishmess", "e9952cfbe2adef5dcef82abd57661f60", 312561),
	ENTRY0("hwmurders", "abadbb15faf7f0b7324222fdea6bd495", 213539),
	ENTRY0("misguided", "cc2c6e724d599e731efa9b7a34ae4f51", 672613),
	ENTRY0("room206", "eb5711ecfad102ee4d9fda7fcb3ddf78", 364156),

	TABLE_END_MARKER
};

} // End of namespace Alan3
} // End of namespace Glk
