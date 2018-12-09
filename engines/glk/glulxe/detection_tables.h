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
namespace Glulxe {

/**
 * Game description
 */
struct GlulxeGameDescription {
	const char *const _gameId;
	const char *const _extra;
	const char *const _md5;
	size_t _filesize;
	Common::Language _language;
};

const GlulxeDescriptor GLULXE_GAME_LIST[] = {
	{ "glulxe", "Glulxe Game" },
	{ "cragne", "Cragne Manor" },

	{ nullptr, nullptr }
};

#define ENTRY0(ID, MD5, FILESIZE) { ID, nullptr, MD5, FILESIZE, Common::EN_ANY }
#define TABLE_END_MARKER { nullptr, nullptr, nullptr, 0, Common::EN_ANY }

const GlulxeGameDescription GLULXE_GAMES[] = {
	ENTRY0("cragne", "082f518c0120d2323ce340bef8a2d5a9", 8869096),
	TABLE_END_MARKER
};

} // End of namespace Glulxe
} // End of namespace Glk
