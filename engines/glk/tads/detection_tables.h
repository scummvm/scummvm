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
namespace TADS {

const PlainGameDescriptor TADS2_GAME_LIST[] = {
	{ "tads2", "TADS 2 Game" },

	// TADS 2 - Free games
	{ "oncefuture", "Once and Future" },

	// TADS 2 - Adventions games
	{ "ccr", "Colossal Cave Revisited" },
	{ "legendlives", "The Legend Lives!" },
	{ "rylvania", "The Horror of Rylvania" },
	{ "u0", "Unnkulia Zero: The Search for Amanda" },
	{ "uhalf", "Unnkulia One-Half: The Salesman Triumphant" },
	{ "uu1", "Unnkulian Underworld: The Unknown Unventure" },
	{ "uu2", "Unnkulian Unventure II: The Secret of Acme" },

	{ nullptr, nullptr }
};

const PlainGameDescriptor TADS3_GAME_LIST[] = {
	{ "tads3", "TADS 3 Game" },
	{ nullptr, nullptr }
};

const GlkDetectionEntry TADS_GAMES[] = {
	// TADS 2 - Free games

	// TADS 2 - Commercial games
	DT_ENTRY0("oncefuture", "4ed995d0784520ca6f0ec5391d92f4d8", 909993),

	// TADS 2 - Adventions games
	DT_ENTRY0("ccr", "2da7dba524075aed0167ae02e6484a32", 291305),
	DT_ENTRY0("legendlives", "03a651ef3d904dbef6a80cd2c041423c", 622989),
	DT_ENTRY0("rylvania", "98027d23f0da0e38c02f1326a2357713", 302903),
	DT_ENTRY1("u0", "Demo", "d9615e08336cc0d50984cdc0879f2006", 500949),
	DT_ENTRY0("uhalf", "80c18f27f656d7e83d1f3f8075115720", 242269),
	DT_ENTRY0("uu1", "705b91f9e54c591e361d6c2e15bff8ef", 367506),
	DT_ENTRY0("uu2", "69a4f7cd914bd32bd7a3bef5c68f9f3a", 400113),

	DT_END_MARKER
};

} // End of namespace Frotz
} // End of namespace Glk
