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

#ifndef MM1_VIEWS_LOCATIONS_TRAINING_H
#define MM1_VIEWS_LOCATIONS_TRAINING_H

#include "mm/mm1/data/character.h"
#include "mm/mm1/views/locations/location.h"
#include "mm/mm1/data/locations.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

class Training : public Location, public TrainingData {
private:
	int _currLevel = 0;
	CharacterClass _class = KNIGHT;
	int _expTotal = 0;
	int _remainingExp = 0;
	int _expAmount = 0;
	int _cost = 0, _cost2;
	bool _canTrain = false;
	bool _canAfford = false;
private:
	/**
	 * Checks whether a character can train
	 */
	void checkCharacter();

	/**
	 * Validates if training is allowed, then trains.
	 */
	void train();

protected:
	/**
	 * Change character
	 */
	void changeCharacter(uint index) override;

	/**
	 * Called when an active timeout countdown expired
	 */
	void timeout() override;

public:
	Training();
	virtual ~Training() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	void draw() override;
};

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM

#endif
