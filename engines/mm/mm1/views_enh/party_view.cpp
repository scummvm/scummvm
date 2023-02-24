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

#include "mm/mm1/views_enh/party_view.h"
#include "mm/mm1/views_enh/game_party.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

bool PartyView::msgFocus(const FocusMessage &msg) {
	// Turn on highlight for selected character
	if (!g_globals->_currCharacter && selectCharByDefault())
		g_globals->_currCharacter = &g_globals->_party[0];

	g_events->send("GameParty", GameMessage("CHAR_HIGHLIGHT", (int)true));
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_PARTY_MENUS);
	return true;
}

bool PartyView::msgUnfocus(const UnfocusMessage &msg) {
	// Turn off highlight for selected character
	g_events->send("GameParty", GameMessage("CHAR_HIGHLIGHT", (int)false));

	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return true;
}

bool PartyView::msgMouseDown(const MouseDownMessage &msg) {
	if (canSwitchChar()) {
		if (send("GameParty", msg))
			return true;
	}

	return ScrollView::msgMouseDown(msg);
}

bool PartyView::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		draw();
		return true;
	}

	return true;
}

bool PartyView::msgAction(const ActionMessage &msg) {
	if (msg._action >= KEYBIND_VIEW_PARTY1 &&
			msg._action <= KEYBIND_VIEW_PARTY6 && canSwitchChar())
		return send("GameParty", msg);
	return false;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
