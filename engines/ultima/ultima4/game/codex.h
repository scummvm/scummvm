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

#ifndef ULTIMA4_GAME_CODEX_H
#define ULTIMA4_GAME_CODEX_H

namespace Ultima {
namespace Ultima4 {

enum CodexEjectCode {
	CODEX_EJECT_NO_3_PART_KEY,
	CODEX_EJECT_BAD_WOP,
	CODEX_EJECT_NO_FULL_PARTY,
	CODEX_EJECT_NO_FULL_AVATAR,
	CODEX_EJECT_HONESTY,
	CODEX_EJECT_COMPASSION,
	CODEX_EJECT_VALOR,
	CODEX_EJECT_JUSTICE,
	CODEX_EJECT_SACRIFICE,
	CODEX_EJECT_HONOR,
	CODEX_EJECT_SPIRITUALITY,
	CODEX_EJECT_HUMILITY,
	CODEX_EJECT_TRUTH,
	CODEX_EJECT_LOVE,
	CODEX_EJECT_COURAGE,
	CODEX_EJECT_BAD_INFINITY
};

/**
 * Begins the Chamber of the Codex sequence
 */
void codexStart();

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
