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

#include "mm/mm1/views/spells/spell_view.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

void SpellView::spellFailed() {
	// Spell failed
	clearSurface();
	writeString(10, 2, STRING["spells.failed"]);

	Sound::sound(SOUND_2);
	delaySeconds(3);
}

void SpellView::spellDone() {
	clearSurface();
	writeString(14, 2, STRING["spells.done"]);
	g_globals->_party.updateAC();

	Sound::sound(SOUND_2);
	delaySeconds(3);
}

void SpellView::timeout() {
	close();
}

bool SpellView::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return true;
}

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM
