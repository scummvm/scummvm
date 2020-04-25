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

#include "common/array.h"

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

class Codex {
private:
	Common::Array<Common::String> _virtueQuestions;
	Common::Array<Common::String> _endgameText1;
	Common::Array<Common::String> _endgameText2;
private:
	/**
	 * Initializes the Chamber of the Codex sequence (runs from codexStart())
	 */
	int init();

	/**
	 * Frees all memory associated with the Codex sequence
	 */
	void deinit();

	/**
	 * Ejects you from the chamber of the codex (and the Abyss, for that matter)
	 * with the correct message.
	 */
	void eject(CodexEjectCode code);

	/**
	 * Handles entering the Word of Passage
	 */
	void handleWOP(const Common::String &word);

	/**
	 * Handles naming of virtues in the Chamber of the Codex
	 */
	void handleVirtues(const Common::String &virtue);

	void handleInfinity(const Common::String &answer);

	/**
	 * Pretty self-explanatory
	 */
	void impureThoughts();

	/**
	 * Key handlers
	 */
	static bool handleInfinityAnyKey(int key, void *data);
	static bool handleEndgameAnyKey(int key, void *data);
public:
	/**
	 * Constructor
	 */
	Codex();

	/**
	 * Destructor
	 */
	~Codex();

	/**
	 * Begins the Chamber of the Codex sequence
	 */
	void start();
};

extern Codex *g_codex;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
