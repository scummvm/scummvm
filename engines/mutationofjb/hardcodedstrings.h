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

#ifndef MUTATIONOFJB_HARDCODEDSTRINGS_H
#define MUTATIONOFJB_HARDCODEDSTRINGS_H

#include "common/language.h"
#include "common/hashmap.h"

namespace MutationOfJB {

class Game;

/**
 * Provides access to hardcoded strings.
 *
 * Currently, we do not have any agreement with the original author of the game,
 * so the strings are loaded from the gmae executable file.
 */
class HardcodedStrings {
public:
	enum StringType {
		WALK,
		TALK,
		LOOK,
		USE,
		PICKUP,

		JOHNNY_CANNOT_USE_1,
		SKEPTO_CANNOT_USE_1,
		JOHNNY_CANNOT_USE_2,
		SKEPTO_CANNOT_USE_2,
		JOHNNY_CANNOT_USE_3,
		SKEPTO_CANNOT_USE_3,
		JOHNNY_CANNOT_USE_4,
		SKEPTO_CANNOT_USE_4,

		JOHNNY_CANNOT_TALK_1,
		SKEPTO_CANNOT_TALK_1,
		JOHNNY_CANNOT_TALK_2,
		SKEPTO_CANNOT_TALK_2,
		JOHNNY_CANNOT_TALK_3,
		SKEPTO_CANNOT_TALK_3,
		JOHNNY_CANNOT_TALK_4,
		SKEPTO_CANNOT_TALK_4,

		JOHNNY_CANNOT_LOOK_1,
		SKEPTO_CANNOT_LOOK_1,
		JOHNNY_CANNOT_LOOK_2,
		SKEPTO_CANNOT_LOOK_2,
		JOHNNY_CANNOT_LOOK_3,
		SKEPTO_CANNOT_LOOK_3,
		JOHNNY_CANNOT_LOOK_4,
		SKEPTO_CANNOT_LOOK_4,

		JOHNNY_CANNOT_PICKUP_1,
		SKEPTO_CANNOT_PICKUP_1,
		JOHNNY_CANNOT_PICKUP_2,
		SKEPTO_CANNOT_PICKUP_2,
		JOHNNY_CANNOT_PICKUP_3,
		SKEPTO_CANNOT_PICKUP_3,
		JOHNNY_CANNOT_PICKUP_4,
		SKEPTO_CANNOT_PICKUP_4,

		STRING_TYPES_TOTAL
	};

	HardcodedStrings(Game &game);

	/**
	 * Get hardcoded string.
	 *
	 * @param strType String type.
	 * @return Hardcoded string.
	 */
	const Common::String &getString(StringType strType) const;

private:
	typedef Common::Array<Common::String> StringArray;

	void loadStrings(Common::Language lang);

	StringArray _strings;
};

}

#endif
