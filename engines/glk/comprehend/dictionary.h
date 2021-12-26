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

#ifndef GLK_COMPREHEND_DICTIONARY_H
#define GLK_COMPREHEND_DICTIONARY_H

namespace Glk {
namespace Comprehend {

class ComprehendGame;
struct Word;

Word *find_dict_word_by_index(ComprehendGame *game,
							  uint8 index, uint8 type_mask);
Word *dict_find_word_by_index_type(ComprehendGame *game,
								   uint8 index, uint8 type);
Word *dict_find_word_by_string(ComprehendGame *game,
							   const char *string);
bool dict_match_index_type(ComprehendGame *game, const char *word,
						   uint8 index, uint8 type_mask);

} // namespace Comprehend
} // namespace Glk

#endif
