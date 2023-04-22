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
namespace JACL {

const PlainGameDescriptor JACL_GAME_LIST[] = {
	{ "bloodyguns", "The Bloody Guns" },
	{ "eldor", "The Curse of Eldor" },		// Competition 96
	{ "prisonbreak", "Prisoner Break" },
	{ "unholygrail", "The Unholy Grail" },	// Competition 97

	{ nullptr, nullptr }
};

const GlkDetectionEntry JACL_GAMES[] = {
	DT_ENTRY0("bloodyguns", "7c4463e92b1c202f8be485030ef4ace5", 159505),
	DT_ENTRY1("eldor", "r3/crashing", "c1435d28e7eb577fa58c7456d4fa7009", 168119),
	DT_ENTRY0("prisonbreak", "e2e85c5e60a63575bf0cd0481f0f3958", 199403),
	DT_ENTRY1("unholygrail", "original/crashing", "2cf04cb897ba799c17cbeb407be67acb", 456730),
	DT_ENTRY1("unholygrail", "002", "7d40e485c8cf8c9d5c4958a79337d6c7", 447833),

	DT_END_MARKER
};

} // End of namespace JACL
} // End of namespace Glk
