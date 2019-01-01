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

/**
 * Game description
 */
struct TADSGameDescription {
	const char *const _gameId;
	const char *const _extra;
	const char *const _md5;
	size_t _filesize;
	Common::Language _language;
};

const PlainGameDescriptor TADS2_GAME_LIST[] = {
	{ "tads2", "TADS 2 Game" },
	{ "oncefuture", "Once and Future" },
	{ nullptr, nullptr }
};

const PlainGameDescriptor TADS3_GAME_LIST[] = {
	{ "tads3", "TADS 3 Game" },
	{ nullptr, nullptr }
};

#define ENTRY0(ID, MD5, FILESIZE) { ID, "", MD5, FILESIZE, Common::EN_ANY }
#define TABLE_END_MARKER { nullptr, nullptr, nullptr, 0, Common::EN_ANY }

const TADSGameDescription TADS_GAMES[] = {
	ENTRY0("oncefuture", "4ed995d0784520ca6f0ec5391d92f4d8", 909993),
	TABLE_END_MARKER
};

} // End of namespace Frotz
} // End of namespace Glk
