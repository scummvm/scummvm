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

#ifndef ARCHETYPE_GAME_STAT
#define ARCHETYPE_GAME_STAT

/* Functions and procedures that help with the saving and loading of
 * game states.  Only the attribute records of a given object list are ever
 * saved or loaded; statements and other object information, such as
 * the Dynamic pointer, are really constant across a game; as long as
 * we know that the game states belong to a particular game, we don't
 * need to save any more.
 */

#include "glk/archetype/array.h"
#include "common/stream.h"

namespace Glk {
namespace Archetype {

extern void save_game_state(Common::WriteStream *bfile, XArrayType &objects);
extern bool load_game_state(Common::ReadStream *bfile, XArrayType &objects);

} // End of namespace Archetype
} // End of namespace Glk

#endif
