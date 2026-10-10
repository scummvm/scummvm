/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this program.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_PUZZLES_TAROT_CARDS_MODEL_H
#define RIPPER_PUZZLES_TAROT_CARDS_MODEL_H

#include "common/scummsys.h"

namespace Ripper {

class TarotCardsModel {
public:
	enum {
		kTargetSlotCount = 6,
		kCardCount = 9,
		kSlotCount = kTargetSlotCount + kCardCount
	};

	TarotCardsModel();

	bool swap(uint slot);
	bool solved() const;
	uint cardAt(uint slot) const;
	uint heldCard() const { return _heldCard; }

	static uint targetCardAt(uint slot);

private:
	byte _cards[kSlotCount];
	byte _heldCard;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_TAROT_CARDS_MODEL_H
