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

#ifndef ULTIMA1_VIEWS_CREATE_CHARACTER_H
#define ULTIMA1_VIEWS_CREATE_CHARACTER_H

#include "ultima/ultima1/gfx/text_cursor.h"
#include "ultima/ultima1/gfx/text_input.h"
#include "ultima/ultima1/views/dialog.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Shared::Messages;

constexpr int ATTRIBUTE_COUNT = 6;
constexpr int STARTING_POINTS = 30;

/**
 * Character generation - attribute point-buy, then race/sex/class
 * selection, name entry, and a final save confirmation.
 */
class CreateCharacter : public Dialog {
private:
	enum State {
		ATTRIBUTES, RACE, SEX, CLASS, NAME, SAVE
	};
	State _state = ATTRIBUTES;
	int _pointsRemaining = STARTING_POINTS;
	int _selectedAttribute = 0;

	Gfx::TextCursor _cursor;
	Gfx::TextInput _nameInput;

	void drawAttributes(Shared::Gfx::GfxSurface &s);
	void drawHelp(Shared::Gfx::GfxSurface &s);
	void drawConfirmedRace(Shared::Gfx::GfxSurface &s);
	void drawConfirmedSex(Shared::Gfx::GfxSurface &s);
	void drawConfirmedClass(Shared::Gfx::GfxSurface &s);

	bool msgAttributesKey(const KeypressMessage &msg);
	bool msgRaceKey(const KeypressMessage &msg);
	bool msgSexKey(const KeypressMessage &msg);
	bool msgClassKey(const KeypressMessage &msg);
	bool msgSaveKey(const KeypressMessage &msg);

	void selectRace(int race);
	void selectSex(int sex);
	void selectClass(int charClass);

	int16 &attribute(int index);

	/**
	 * Resets to a freshly-generated character with default attributes
	 */
	void reset();

	/**
	 * Fills in the rest of a new character's starting stats/inventory
	 * and saves the game
	 */
	void confirmAndSave();

public:
	CreateCharacter();
	~CreateCharacter() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;

	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
