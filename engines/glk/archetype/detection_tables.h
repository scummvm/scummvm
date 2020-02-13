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
namespace Archetype {

const PlainGameDescriptor ARCHETYPE_GAME_LIST[] = {
	{ "archetype", "Archetype IF Game" },

	{ "guesstheanimal", "Guess the Animal" },
	{ "gorreven", "The Gorreven Papers" },
	{ "starshipsolitaire", "The Starship Solitaire adventure" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry ARCHETYPE_GAMES[] = {
	DT_ENTRY0("guesstheanimal", "f1c4d7ba35db9f217eacd84181b4bb33", 1266),
	DT_ENTRY0("gorreven", "073a996b158474a2c419bc1d9dc8d44b", 66793),
	DT_ENTRY0("starshipsolitaire", "6c86208d0d84fb11f81bf5b1f6fedb84", 55762),

	DT_END_MARKER
};

} // End of namespace Archetype
} // End of namespace Glk
