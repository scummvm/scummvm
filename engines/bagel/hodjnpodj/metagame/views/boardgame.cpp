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

#include "bagel/hodjnpodj/metagame/views/boardgame.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

Boardgame::Boardgame() : Dialog("Boardgame") {
}

bool Boardgame::msgOpen(const OpenMessage &msg) {
	Dialog::msgOpen(msg);
	initBfcInfo();

	return true;
}

bool Boardgame::msgClose(const CloseMessage &msg) {
	Dialog::msgClose(msg);
	return true;
}

bool Boardgame::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		replaceView("TitleMenu", true);
		return true;
	}

	return false;
}

bool Boardgame::msgGame(const GameMessage &msg) {
	return false;
}

void Boardgame::initBfcInfo() {
	//auto &bfc = *lpMetaGame;

}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
