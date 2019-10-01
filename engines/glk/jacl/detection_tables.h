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
namespace JACL {

const PlainGameDescriptor JACL_GAME_LIST[] = {
	{ "prisonbreak", "Prisoner Break" },
	{ "unholygrail", "The Unholy Grail" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry JACL_GAMES[] = {
	DT_ENTRY0("prisonbreak", "e2e85c5e60a63575bf0cd0481f0f3958", 199403),
	DT_ENTRY0("unholygrail", "7d40e485c8cf8c9d5c4958a79337d6c7", 447833),

	DT_END_MARKER
};

} // End of namespace JACL
} // End of namespace Glk
