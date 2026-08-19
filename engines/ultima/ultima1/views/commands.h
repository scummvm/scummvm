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

#ifndef ULTIMA1_VIEWS_COMMANDS_H
#define ULTIMA1_VIEWS_COMMANDS_H

#include "ultima/shared/engine/events.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Shared::Messages;

class Commands : public Shared::UIElement {
private:
	Shared::Gfx::GfxSurface _surface;

public:
	Commands(UIElement *parent) : Shared::UIElement("Commands", parent) {}
	~Commands() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void writeString(const Common::String &msg);

	void prompt();
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
