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

#include "mm/mm1/views_enh/won_game.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

WonGame::WonGame() : ScrollView("WonGame") {
	setBounds(Common::Rect(0, 0, 234, 144));
}

void WonGame::draw() {
	ScrollView::draw();
	writeLine(0, STRING["maps.map18.gates"], ALIGN_MIDDLE);
	writeLine(1, STRING["maps.map18.congratulations"], ALIGN_MIDDLE);
	Sound::sound(SOUND_3);
}

bool WonGame::msgKeypress(const KeypressMessage &msg) {
	close();
	return true;
}

bool WonGame::msgAction(const ActionMessage &msg) {
	close();
	return true;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
