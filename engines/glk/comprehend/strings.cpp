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
#include "glk/comprehend/game.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/strings.h"

namespace Glk {
namespace Comprehend {

static char bad_string[128];

const char *string_lookup(comprehend_game *game, uint16 index)
{
	uint16 string;
	uint8 table;

	/*
	 * There are two tables of strings. The first is stored in the main
	 * game data file, and the second is stored in multiple string files.
	 *
	 * In instructions string indexes are split into a table and index
	 * value. In other places such as the save files strings from the
	 * main table are occasionally just a straight 16-bit index. We
	 * convert all string indexes to the former case so that we can handle
	 * them the same everywhere.
	 */
	table = (index >> 8) & 0xff;
	string = index & 0xff;

	switch (table) {
	case 0x81:
	case 0x01:
		string += 0x100;
		/* Fall-through */
	case 0x00:
	case 0x80:
		if (string < game->info->strings.nr_strings)
			return game->info->strings.strings[string];
		break;

	case 0x83:
		string += 0x100;
		/* Fall-through */
	case 0x02:
	case 0x82:
		if (string < game->info->strings2.nr_strings)
			return game->info->strings2.strings[string];
		break;
	}

	snprintf(bad_string, sizeof(bad_string), "BAD_STRING(%.4x)", index);
	return bad_string;
}

const char *instr_lookup_string(struct comprehend_game *game, uint8 index,
				uint8 table)
{
	return string_lookup(game, table << 8 | index);
}

} // namespace Comprehend
} // namespace Glk
