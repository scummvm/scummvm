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

#ifndef ULTIMA3_VIEWS_FORM_PARTY_H
#define ULTIMA3_VIEWS_FORM_PARTY_H

#include "ultima/ultima3/data/roster.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/views/window_view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

/**
 * Selects up to four roster characters to form the party. Entering 0 ends
 * the selection early
 */
class FormParty : public WindowView {
private:
	enum State {
		ENTRY,
		MESSAGE
	};

	State _state = ENTRY;
	NumberInput _numbers[Data::PARTY_MAX];
	bool _showSelection = true;
	const char *_message = nullptr;
	int _messageCol = 0, _messageRow = 0;

	void showMessage(const char *message, int col, int row);
	void fail(const char *message);
	void finish();

public:
	FormParty();
	~FormParty() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
