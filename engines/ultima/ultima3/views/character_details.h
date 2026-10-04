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

#ifndef ULTIMA3_VIEWS_CHARACTER_DETAILS_H
#define ULTIMA3_VIEWS_CHARACTER_DETAILS_H

#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/views/window_view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

/**
 * Prompts for a roster entry number and shows that character's details
 */
class CharacterDetails : public WindowView {
private:
	enum State {
		ENTRY,
		MESSAGE,
		DETAILS
	};

	State _state = ENTRY;
	NumberInput _entry;
	const char *_message = nullptr;
	int _messageCol = 0;

	void drawDetails(GfxSurface &s);

public:
	CharacterDetails();
	~CharacterDetails() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
