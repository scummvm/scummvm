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

#ifndef ULTIMA0_VIEWS_CREATE_CHARACTER_H
#define ULTIMA0_VIEWS_CREATE_CHARACTER_H

#include "ultima/ultima0/views/view.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

class CreateCharacter : public View {
private:
	enum Mode { LUCKY_NUMBER, LEVEL, STATS, CLASS };
	Mode _mode = LUCKY_NUMBER;
	Common::String _input;

	void characterDone();

public:
	CreateCharacter() : View("CreateCharacter") {}
	~CreateCharacter() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace Views
} // namespace Ultima0
} // namespace Ultima

#endif
