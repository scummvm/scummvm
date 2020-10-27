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
#include "glk/comprehend/dictionary.h"

namespace Glk {
namespace Comprehend {

static bool word_match(Word *word, const char *string) {
	/* Words less than 6 characters must match exactly */
	if (strlen(word->_word) < 6 && strlen(string) != strlen(word->_word))
		return false;

	return strncmp(word->_word, string, strlen(word->_word)) == 0;
}

Word *dict_find_word_by_string(ComprehendGame *game,
                               const char *string) {
	uint i;

	if (!string)
		return NULL;

	for (i = 0; i < game->_words.size(); i++)
		if (word_match(&game->_words[i], string))
			return &game->_words[i];

	return NULL;
}

Word *dict_find_word_by_index_type(ComprehendGame *game,
                                   uint8 index, uint8 type) {
	uint i;

	for (i = 0; i < game->_words.size(); i++) {
		if (game->_words[i]._index == index &&
		        game->_words[i]._type == type)
			return &game->_words[i];
	}

	return NULL;
}

Word *find_dict_word_by_index(ComprehendGame *game,
                              uint8 index, uint8 type_mask) {
	uint i;

	for (i = 0; i < game->_words.size(); i++) {
		if (game->_words[i]._index == index &&
		        (game->_words[i]._type & type_mask) != 0)
			return &game->_words[i];
	}

	return NULL;
}

bool dict_match_index_type(ComprehendGame *game, const char *word,
                           uint8 index, uint8 type_mask) {
	uint i;

	for (i = 0; i < game->_words.size(); i++)
		if (game->_words[i]._index == index &&
		        ((game->_words[i]._type & type_mask) != 0) &&
		        word_match(&game->_words[i], word))
			return true;

	return false;
}

} // namespace Comprehend
} // namespace Glk
