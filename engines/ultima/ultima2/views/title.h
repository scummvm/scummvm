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

#ifndef ULTIMA2_VIEWS_TITLE_H
#define ULTIMA2_VIEWS_TITLE_H

#include "ultima/shared/gfx/view.h"
#include "ultima/shared/gfx/text_input.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

using namespace Shared::Messages;

class Title : public Shared::Gfx::View {
private:
	Shared::Gfx::TextInput _cursor = Shared::Gfx::TextInput("Cursor", this,
		Common::Point(12, 19), 1);

public:
	Title() : View("Title") {}
	~Title() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;

	bool msgGame(const GameMessage &msg) override;
};

} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
