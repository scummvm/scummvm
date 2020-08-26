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
namespace Magnetic {

const PlainGameDescriptor MAGNETIC_GAME_LIST[] = {
	{ "magnetic", "Magnetic Scrolls Game" },

	{ "corruption", "Corruption" },
	{ "fish", "Fish!" },
	{ "guild", "The Guild of Thieves" },
	{ "jinxter", "Jinxter" },
	{ "myth", "Myth" },
	{ "pawn", "the Pawn" },
	{ "wonderland", "Wonderland" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry MAGNETIC_GAMES[] = {
	DT_ENTRY0("corruption", "313880cbe0f15bfa259ebaf228b4d0e9", 167466),
	DT_ENTRY1("corruption", "Collection", "6fe35b357fa0311450d3a9c809e60ba8", 177185),
	DT_ENTRY0("fish", "2efb8118f4cb9a36bb54646ce41a950e", 162858),
	DT_ENTRY1("fish", "Collection", "cfe333306597d36c8aa3fc64f6be94ba", 172517),
	DT_ENTRY0("guild", "bab78740d39ee5e058faf4912fdbf33d", 130858),
	DT_ENTRY1("guild", "Collection", "36af907a4ec9db909148f308287586f1", 141766),
	DT_ENTRY0("jinxter", "998cf5c49a67409bd344e59307a5213e", 156458),
	DT_ENTRY0("myth", "9c2a5272a9c0b1e173401ba4df32567a", 99370),
	DT_ENTRY0("pawn", "4a7847980f9e942acd7aa51ea12a6586", 103466),
	DT_ENTRY0("wonderland", "2cea8fccf42d570be8836416c2802613", 183916),
	
	DT_END_MARKER
};

const gms_game_table_t GMS_GAME_TABLE[] = {
	{0x2100, 0x427e, "Corruption v1.11 (Amstrad CPC)"},
	{0x2100, 0x43a0, "Corruption v1.11 (Archimedes)"},
	{0x2100, 0x43a0, "Corruption v1.11 (DOS)"},
	{0x2100, 0x4336, "Corruption v1.11 (Commodore 64)"},
	{0x2100, 0x4222, "Corruption v1.11 (Spectrum +3)"},
	{0x2100, 0x4350, "Corruption v1.12 (Archimedes)"},
	{0x2500, 0x6624, "Corruption v1.12 (DOS, Magnetic Windows)"},

	{0x2300, 0x3fa0, "Fish v1.02 (DOS)"},
	{0x2400, 0x4364, "Fish v1.03 (Spectrum +3)"},
	{0x2300, 0x3f72, "Fish v1.07 (Commodore 64)"},
	{0x2200, 0x3f9c, "Fish v1.08 (Archimedes)"},
	{0x2a00, 0x583a, "Fish v1.10 (DOS, Magnetic Windows)"},

	{0x5000, 0x6c30, "Guild v1.0 (Amstrad CPC)"},
	{0x5000, 0x6cac, "Guild v1.0 (Commodore 64)"},
	{0x5000, 0x6d5c, "Guild v1.1 (DOS)"},
	{0x3300, 0x698a, "Guild v1.3 (Archimedes)"},
	{0x3200, 0x6772, "Guild v1.3 (Spectrum +3)"},
	{0x3400, 0x6528, "Guild v1.3 (DOS, Magnetic Windows)"},

	{0x2b00, 0x488c, "Jinxter v1.05 (Commodore 64)"},
	{0x2c00, 0x4a08, "Jinxter v1.05 (DOS)"},
	{0x2c00, 0x487a, "Jinxter v1.05 (Spectrum +3)"},
	{0x2c00, 0x4a56, "Jinxter v1.10 (DOS)"},
	{0x2b00, 0x4924, "Jinxter v1.22 (Amstrad CPC)"},
	{0x2c00, 0x4960, "Jinxter v1.30 (Archimedes)"},

	{0x1600, 0x3940, "Myth v1.0 (Commodore 64)"},
	{0x1500, 0x3a0a, "Myth v1.0 (DOS)"},

	{0x3600, 0x42cc, "Pawn v2.3 (Amstrad CPC)"},
	{0x3600, 0x4420, "Pawn v2.3 (Archimedes)"},
	{0x3600, 0x3fb0, "Pawn v2.3 (Commodore 64)"},
	{0x3600, 0x4420, "Pawn v2.3 (DOS)"},
	{0x3900, 0x42e4, "Pawn v2.3 (Spectrum 128)"},
	{0x3900, 0x42f4, "Pawn v2.4 (Spectrum +3)"},

	{0x3900, 0x75f2, "Wonderland v1.21 (DOS, Magnetic Windows)"},
	{0x3900, 0x75f8, "Wonderland v1.27 (Archimedes)"},
	{0, 0, NULL}
};

} // End of namespace Magnetic
} // End of namespace Glk
