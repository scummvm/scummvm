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

namespace Glk {
namespace AdvSys {

const PlainGameDescriptor ADVSYS_GAME_LIST[] = {
	{ "advsys", "AdvSys Game" },

	{ "bustedadvsys", "Busted!" },
	{ "starshipcolumbus", "Starship Columbus" },
	{ "elves87", "Elves '87" },
	{ "keytotime", "The Key to Time" },
	{ "onehand", "The Sound of One Hand Clapping" },
	{ "pirating", "Pirating" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry ADVSYS_GAMES[] = {
	DT_ENTRY0("bustedadvsys", "2246a2686a07c714868680eaf980ece9", 79091),
	DT_ENTRY0("starshipcolumbus", "120d7041dfa000c9a313a8b0ae9cef33", 76032),
	DT_ENTRY0("elves87", "746963e82552f95b5e743fe24ecd1ec3", 77947),
	DT_ENTRY0("keytotime", "892217ab8d902a732e82c55efd22931d", 24941),
	DT_ENTRY0("onehand", "3a2a3cc24709ff3272f3a15d09b5e63e", 95762),
	DT_ENTRY0("pirating", "e55fff2ac51a8a16b979541e8d3210d8", 29529),

	DT_END_MARKER
};

} // End of namespace AdvSys
} // End of namespace Glk
