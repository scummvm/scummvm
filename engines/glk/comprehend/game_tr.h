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

#ifndef GLK_COMPREHEND_GAME_TR_H
#define GLK_COMPREHEND_GAME_TR_H

#include "glk/comprehend/game.h"

namespace Glk {
namespace Comprehend {

struct TransylvaniaMonster {
	uint8 object;
	uint8 dead_flag;
	unsigned min_turns_before;
	unsigned room_allow_flag;
	unsigned randomness;
};

class TransylvaniaGame : public ComprehendGame {
private:
	static const TransylvaniaMonster WEREWOLF;
	static const TransylvaniaMonster VAMPIRE;

	void update_monster(const TransylvaniaMonster *monster_info);

public:
	TransylvaniaGame();
	~TransylvaniaGame() override {}

	void beforeGame() override;
	bool beforeTurn() override;
	int roomIsSpecial(unsigned room_index, unsigned *room_desc_string) override;
	void handleSpecialOpcode(uint8 operand) override;
};

} // namespace Comprehend
} // namespace Glk

#endif
