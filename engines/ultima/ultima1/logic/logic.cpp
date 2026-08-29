
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

#include "ultima/ultima1/logic/logic.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

using namespace Shared::Messages;

void Logic::writeString(const Common::String &msg) {
	g_engine->findView("Commands")->send(GameMessage("TEXT", msg));
}

void Logic::prompt() {
	g_engine->findView("Commands")->send(GameMessage("PROMPT"));
}

void Logic::redrawMap() {
	g_engine->focusedView()->redraw();
}

void Logic::redrawStats() {
	g_engine->findView("Stats")->draw();
}

void Logic::playFX(int num) {
	g_engine->playFX(num);
}

int Logic::getRandomNumber(int minNumber, int maxNumber) {
	return g_engine->getRandomNumber(minNumber, maxNumber);
}

int Logic::getRandomNumber(int maxNumber) {
	return g_engine->getRandomNumber(maxNumber);
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
