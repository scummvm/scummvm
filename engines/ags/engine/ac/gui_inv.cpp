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

#include "ags/shared/gui/gui_inv.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/character_extras.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/gfx/bitmap.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

bool GUIInvWindow::HasAlphaChannel() const {
	// We would have to test every inventory item's graphic to tell precisely,
	// so just test game color depth instead:
	return _GP(game).GetColorDepth() == 32;
}

int GUIInvWindow::GetCharacterId() const {
	if (CharId < 0)
		return _GP(game).playercharacter;

	return CharId;
}

void GUIInvWindow::Draw(Bitmap *ds, int x, int y) {
	const bool enabled = IsGUIEnabled(this);
	if (!enabled && (GUI::Options.DisabledStyle == kGuiDis_Blackout))
		return;

	// backwards compatibility
	// TODO: find a way to not have this inside GUIInvWindow::Draw!
	_GP(play).inv_numinline = ColCount;
	_GP(play).inv_numdisp = RowCount * ColCount;
	_GP(play).inv_numorder = _GP(charextra)[_GP(game).playercharacter].invorder_count;
	// if the user changes top_inv_item, switch into backwards compat mode
	if (_GP(play).inv_top != 0)
		_GP(play).inv_backwards_compatibility = 1;
	if (_GP(play).inv_backwards_compatibility)
		TopItem = _GP(play).inv_top;

	// draw the items
	const int leftmost_x = x;
	int at_x = x;
	int at_y = y;
	int lastItem = TopItem + (ColCount * RowCount);
	if (lastItem > _GP(charextra)[GetCharacterId()].invorder_count)
		lastItem = _GP(charextra)[GetCharacterId()].invorder_count;

	for (int item = TopItem; item < lastItem; ++item) {
		// draw inv graphic
		draw_gui_sprite(ds, _GP(game).invinfo[_GP(charextra)[GetCharacterId()].invorder[item]].pic, at_x, at_y, true);
		at_x += data_to_game_coord(ItemWidth);

		// go to next row when appropriate
		if ((item - TopItem) % ColCount == (ColCount - 1)) {
			at_x = leftmost_x;
			at_y += data_to_game_coord(ItemHeight);
		}
	}

	if (!enabled &&
		GUI::Options.DisabledStyle == kGuiDis_Greyout &&
		_GP(play).inventory_greys_out == 1) {
		// darken the inventory when disabled
		GUI::DrawDisabledEffect(ds, RectWH(x, y, _width, _height));
	}
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
