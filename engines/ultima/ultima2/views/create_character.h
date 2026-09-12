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

#ifndef ULTIMA2_VIEWS_CREATE_CHARACTER_H
#define ULTIMA2_VIEWS_CREATE_CHARACTER_H

#include "ultima/shared/gfx/view.h"
#include "ultima/ultima2/gfx/text_cursor.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

using namespace Shared::Messages;

/**
 * Point-buy character creation: six attributes distributed from a shared
 * pool of 90 points (each at least 10), sex, race, class - each of which
 * adds its own bonus to a particular attribute - then a name and a final
 * confirmation. Any invalid attribute entry (below 10, or more than the
 * remaining pool) discards all progress and starts over, matching the
 * harsh but simple validation of the original.
 */
class CreateCharacter : public Shared::Gfx::View {
private:
	enum State {
		STRENGTH, AGILITY, STAMINA, CHARISMA, WISDOM, INTELLIGENCE,
		SEX, RACE, CLASS, NAME, CONFIRM, DONE
	};
	State _state = STRENGTH;

	int _pointsRemaining = 0;
	int _digits[2] = { 0, 0 };
	int _digitCount = 0;
	Common::String _name;

	// Blinking cursor, tracking whichever field is currently being entered
	Gfx::TextCursor _cursor;

	void reset();
	void rejectEntry();
	void enterStat(int value);
	void positionCursor();
	void drawDone(Shared::Gfx::GfxSurface &s);

public:
	CreateCharacter();
	~CreateCharacter() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
