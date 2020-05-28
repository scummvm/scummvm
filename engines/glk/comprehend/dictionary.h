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

#ifndef GLK_COMPREHEND_DICTIONARY_H
#define GLK_COMPREHEND_DICTIONARY_H

namespace Glk {
namespace Comprehend {

struct comprehend_game;
struct word;

struct word *find_dict_word_by_index(struct comprehend_game *game,
				     uint8 index, uint8 type_mask);
struct word *dict_find_word_by_index_type(struct comprehend_game *game,
					  uint8 index, uint8 type);
struct word *dict_find_word_by_string(struct comprehend_game *game,
				      const char *string);
bool dict_match_index_type(struct comprehend_game *game, const char *word,
			   uint8 index, uint8 type_mask);

} // namespace Comprehend
} // namespace Glk

#endif
