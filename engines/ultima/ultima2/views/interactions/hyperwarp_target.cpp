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

#include "ultima/ultima2/views/interactions/hyperwarp_target.h"
#include "ultima/ultima2/logic/space_logic.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

static const char *const PROMPTS[3] = { "XENO=", " YAKO=", " ZABO=" };

HyperwarpTarget::HyperwarpTarget() : Interaction("HyperwarpTarget") {
}

bool HyperwarpTarget::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	_count = 0;
	writeString(PROMPTS[0]);
	return Interaction::msgFocus(msg);
}

bool HyperwarpTarget::msgKeypress(const KeypressMessage &msg) {
	if (msg.ascii < '0' || msg.ascii > '9')
		return true;

	_digits[_count] = msg.ascii - '0';
	writeString("%d", _digits[_count]);

	if (++_count < 3) {
		writeString(PROMPTS[_count]);
		return true;
	}

	close();
	writeString("\nPREPARE FOR HYPERWARP!\n");

	auto *logic = dynamic_cast<Logic::SpaceLogic *>(_G(logic).get());
	if (logic)
		logic->hyperwarpTo(_digits[0], _digits[1], _digits[2]);
	return true;
}

bool HyperwarpTarget::tick() {
	// The stars keep streaming past while the coordinates are entered
	g_engine->findView("SpaceMap")->tick();
	return Interaction::tick();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
