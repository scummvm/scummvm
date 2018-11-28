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
 * Game descriptor
 */
struct TADSGame {
	const char *_md5;
	const char *_gameId;
	int32 _filesize;
};

const PlainGameDescriptor TADS_GAME_LIST[] = {
	{ "tads",     "TADS" },
	{ nullptr, nullptr }
};

const TADSGame TADS_GAMES[] = {
	{ nullptr, nullptr, 0 }
};

} // End of namespace Frotz
} // End of namespace Glk
