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

#include <cxxtest/TestSuite.h>

class RipperTarotCardsTestSuite : public CxxTest::TestSuite {
public:
	void testRetailInitialLayout() {
		Ripper::TarotCardsModel model;

		for (uint slot = 0; slot < Ripper::TarotCardsModel::kTargetSlotCount; ++slot)
			TS_ASSERT_EQUALS(model.cardAt(slot), 0U);
		for (uint card = 0; card < Ripper::TarotCardsModel::kCardCount; ++card)
			TS_ASSERT_EQUALS(model.cardAt(
				Ripper::TarotCardsModel::kTargetSlotCount + card), card + 1);
		TS_ASSERT_EQUALS(model.heldCard(), 0U);
		TS_ASSERT(!model.solved());
	}

	void testClickSwapsSlotWithHeldCard() {
		Ripper::TarotCardsModel model;

		TS_ASSERT(model.swap(13));
		TS_ASSERT_EQUALS(model.cardAt(13), 0U);
		TS_ASSERT_EQUALS(model.heldCard(), 8U);
		TS_ASSERT(model.swap(0));
		TS_ASSERT_EQUALS(model.cardAt(0), 8U);
		TS_ASSERT_EQUALS(model.heldCard(), 0U);
	}

	void testRetailTargetSequenceSolvesPuzzle() {
		Ripper::TarotCardsModel model;

		for (uint target = 0; target < Ripper::TarotCardsModel::kTargetSlotCount; ++target) {
			const uint card = Ripper::TarotCardsModel::targetCardAt(target);
			TS_ASSERT(model.swap(Ripper::TarotCardsModel::kTargetSlotCount + card - 1));
			TS_ASSERT(model.swap(target));
		}

		TS_ASSERT(model.solved());
		TS_ASSERT_EQUALS(model.heldCard(), 0U);
	}

	void testInvalidSlotDoesNotChangeState() {
		Ripper::TarotCardsModel model;

		TS_ASSERT(!model.swap(Ripper::TarotCardsModel::kSlotCount));
		TS_ASSERT_EQUALS(model.heldCard(), 0U);
		TS_ASSERT_EQUALS(model.cardAt(6), 1U);
	}
};
