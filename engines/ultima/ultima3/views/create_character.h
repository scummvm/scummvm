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

#ifndef ULTIMA3_VIEWS_CREATE_CHARACTER_H
#define ULTIMA3_VIEWS_CREATE_CHARACTER_H

#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/views/window_view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

/**
 * Creates a character in an empty roster slot: name, sex, race and class,
 * then four attributes drawn from a pool of 50 points
 */
class CreateCharacter : public WindowView {
private:
	enum State {
		ENTRY,
		MESSAGE,
		NAME,
		SEX,
		RACE,
		CLASS,
		ATTRIBUTE,
		CONFIRM,
		FINISHED
	};

	State _state = ENTRY;
	NumberInput _entry;
	LineInput _name;
	MenuChoice _sex, _race, _class, _confirm;
	NumberInput _attributes[4];
	int _attribute = 0;
	int _values[4] = {};
	int _points = 0;
	int _number = 0;
	bool _showForm = false;
	const char *_message = nullptr;

	void drawForm(GfxSurface &s);
	bool acceptAttribute(int value);
	void createCharacter();

public:
	CreateCharacter();
	~CreateCharacter() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
