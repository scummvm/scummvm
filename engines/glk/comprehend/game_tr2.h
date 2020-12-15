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

#ifndef GLK_COMPREHEND_GAME_TR2_H
#define GLK_COMPREHEND_GAME_TR2_H

#include "glk/comprehend/game_opcodes.h"

namespace Glk {
namespace Comprehend {

struct TransylvaniaMonster;

class TransylvaniaGame2 : public ComprehendGameV2 {
private:
	static const TransylvaniaMonster WEREWOLF;
	static const TransylvaniaMonster VAMPIRE;
	bool _miceReleased;

	bool updateMonster(const TransylvaniaMonster *monsterInfo);
	bool isMonsterInRoom(const TransylvaniaMonster *monsterInfo);
public:
	TransylvaniaGame2();
	~TransylvaniaGame2() override {}

	void beforeGame() override;
	void beforeTurn() override;
	void synchronizeSave(Common::Serializer &s) override;
	int roomIsSpecial(unsigned room_index, unsigned *roomDescString) override;
	void handleSpecialOpcode() override;
};

} // namespace Comprehend
} // namespace Glk

#endif
