/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA3_VIEWS_INTERACTIONS_INSERT_CARD_H
#define ULTIMA3_VIEWS_INTERACTIONS_INSERT_CARD_H

#include "ultima/ultima3/views/interactions/log_input.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

/**
 * Puts a card into the Exodus machine. The four have to go in the right
 * order, with the right word said for each, and the last one finishes the game
 */
class InsertCard : public Interaction {
private:
	enum Stage {
		DIRECTION,
		CHOOSE_WORD,
		FLASH,
		SHATTER,
		TEXT,
		DONE
	};

	Stage _stage = DIRECTION;
	int _slot = 0;
	int _frames = 0;
	int _step = 0;
	int _targetX = 0, _targetY = 0;

	DirectionChooser _directions;
	LogMenu _menu;

	// Which of the two sets of alternate pixel columns are inverted
	bool _invertEven = false;
	bool _invertOdd = false;

	/**
	 * Works out which cell was chosen, and asks for a word if it's the machine
	 */
	bool chooseCell();

	/**
	 * Tries the word chosen at the machine
	 */
	bool tryWord();

	/**
	 * Starts the congratulations
	 */
	void startVictory();

public:
	/**
	 * Starts putting a card in
	 * @param slot		The party member holding the card
	 */
	void start(int slot);

	bool keypress(const KeypressMessage &msg) override;
	bool timeout() override;
	void draw(GfxSurface &s) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
