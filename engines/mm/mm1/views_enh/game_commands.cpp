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

#include "mm/mm1/views_enh/game_commands.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

GameCommands::GameCommands(UIElement *owner) :
		ButtonContainer("GameCommands", owner),
		_minimap(this) {
	Shared::Xeen::SpriteResource *spr = &g_globals->_mainIcons;
	_iconSprites.load("view.icn");

	addButton(Common::Rect(235, 75, 259, 95), KEYBIND_SPELL, 2, spr);
	addButton(Common::Rect(260, 75, 284, 95), KEYBIND_PROTECT, 10, spr);
	addButton(Common::Rect(286, 75, 310, 95), KEYBIND_REST, 4, spr);

	addButton(Common::Rect(235, 96, 259, 116), KEYBIND_BASH, 6, spr);
	addButton(Common::Rect(260, 96, 284, 116), KEYBIND_SEARCH, 40, &_iconSprites);
	addButton(Common::Rect(286, 96, 310, 116), KEYBIND_ORDER, 44, &_iconSprites);

	addButton(Common::Rect(235, 117, 259, 137), KEYBIND_MAP, 12, spr);
	addButton(Common::Rect(260, 117, 284, 137), KEYBIND_UNLOCK, 14, spr);
	addButton(Common::Rect(286, 117, 310, 137), KEYBIND_QUICKREF, 16, spr);

	addButton(Common::Rect(109, 137, 122, 147), KEYBIND_MENU, 18, spr);
	addButton(Common::Rect(235, 148, 259, 168), KEYBIND_TURN_LEFT, spr);
	addButton(Common::Rect(260, 148, 284, 168), KEYBIND_FORWARDS, spr);
	addButton(Common::Rect(286, 148, 310, 168), KEYBIND_TURN_RIGHT, spr);
	addButton(Common::Rect(235, 169, 259, 189), KEYBIND_STRAFE_LEFT, spr);
	addButton(Common::Rect(260, 169, 284, 189), KEYBIND_BACKWARDS, spr);
	addButton(Common::Rect(286, 169, 310, 189), KEYBIND_STRAFE_RIGHT, spr);

	addButton(_minimap.getBounds(), KEYBIND_MINIMAP);
}

bool GameCommands::msgAction(const ActionMessage & msg) {
	switch (msg._action) {
	case KEYBIND_MINIMAP:
		_minimap.toggleMinimap();
		return true;
	default:
		// Other button actions are handled by outer Game view
		break;
	}

	return false;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
