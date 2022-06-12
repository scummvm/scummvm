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
		ButtonContainer("GameCommands", owner) {
	Xeen::SpriteResource *spr = &g_globals->_mainIcons;
	addButton(Common::Rect(235, 75, 259, 95), Common::KEYCODE_s, spr);
	addButton(Common::Rect(260, 75, 284, 95), Common::KEYCODE_c, spr);
	addButton(Common::Rect(286, 75, 310, 95), Common::KEYCODE_r, spr);
	addButton(Common::Rect(235, 96, 259, 116), Common::KEYCODE_b, spr);
	addButton(Common::Rect(260, 96, 284, 116), Common::KEYCODE_d, spr);
	addButton(Common::Rect(286, 96, 310, 116), Common::KEYCODE_v, spr);
	addButton(Common::Rect(235, 117, 259, 137), Common::KEYCODE_m, spr);
	addButton(Common::Rect(260, 117, 284, 137), Common::KEYCODE_i, spr);
	addButton(Common::Rect(286, 117, 310, 137), Common::KEYCODE_q, spr);
	addButton(Common::Rect(109, 137, 122, 147), Common::KEYCODE_TAB, spr);
	addButton(Common::Rect(235, 148, 259, 168), Common::KEYCODE_LEFT, spr);
	addButton(Common::Rect(260, 148, 284, 168), Common::KEYCODE_UP, spr);
	addButton(Common::Rect(286, 148, 310, 168), Common::KEYCODE_RIGHT, spr);
	addButton(Common::Rect(235, 169, 259, 189), (Common::KBD_CTRL << 16) | Common::KEYCODE_LEFT, spr);
	addButton(Common::Rect(260, 169, 284, 189), Common::KEYCODE_DOWN, spr);
	addButton(Common::Rect(286, 169, 310, 189), (Common::KBD_CTRL << 16) | Common::KEYCODE_RIGHT, spr);
	addButton(Common::Rect(236, 11, 308, 69), Common::KEYCODE_EQUALS);
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
