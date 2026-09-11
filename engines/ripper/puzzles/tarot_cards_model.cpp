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

#include "ripper/puzzles/tarot_cards_model.h"

namespace Ripper {

static const byte kTargetCards[TarotCardsModel::kTargetSlotCount] = {
	8, 3, 9, 7, 2, 1
};

TarotCardsModel::TarotCardsModel() : _heldCard(0) {
	for (uint slot = 0; slot < kTargetSlotCount; ++slot)
		_cards[slot] = 0;
	for (uint card = 0; card < kCardCount; ++card)
		_cards[kTargetSlotCount + card] = card + 1;
}

bool TarotCardsModel::swap(uint slot) {
	if (slot >= kSlotCount)
		return false;
	const byte previousCard = _cards[slot];
	_cards[slot] = _heldCard;
	_heldCard = previousCard;
	return true;
}

bool TarotCardsModel::solved() const {
	for (uint slot = 0; slot < kTargetSlotCount; ++slot) {
		if (_cards[slot] != kTargetCards[slot])
			return false;
	}
	return true;
}

uint TarotCardsModel::cardAt(uint slot) const {
	return slot < kSlotCount ? _cards[slot] : 0;
}

uint TarotCardsModel::targetCardAt(uint slot) {
	return slot < kTargetSlotCount ? kTargetCards[slot] : 0;
}

} // End of namespace Ripper
