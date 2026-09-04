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

#include "ultima/ultima1/views/interactions/interaction.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

Interaction::Interaction(const Common::String &name) : View(name) {
	setBounds(Common::Rect(0, 0, 0, 0));
}

bool Interaction::tick() {
	g_engine->findView("Commands")->tick();
	return Shared::Gfx::View::tick();
}

void Interaction::writeString(const Common::String &msg) {
	g_engine->findView("Commands")->send(GameMessage("TEXT", msg));
}

void Interaction::writeString(const char *format, ...) {
	va_list alist;

	// Start reading values
	va_start(alist, format);
	Common::String msg = Common::String::vformat(format, alist);
	va_end(alist);

	writeString(msg);
}

void Interaction::resetLine() {
	g_engine->findView("Commands")->send(GameMessage("RESET_LINE"));
}

void Interaction::showCursor() {
	g_engine->findView("Commands")->send(GameMessage("SHOW_CURSOR"));
}

void Interaction::prompt() {
	g_engine->findView("Commands")->send(GameMessage("PROMPT"));
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
