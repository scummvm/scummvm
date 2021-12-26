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

#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_palette.h"
#include "ags/shared/util/wgt2_allg.h"

namespace AGS3 {






void CyclePalette(int strt, int eend) {
	// hi-color game must invalidate screen since the palette changes
	// the effect of the drawing operations
	if (_GP(game).color_depth > 1)
		invalidate_screen();

	if ((strt < 0) || (strt > 255) || (eend < 0) || (eend > 255))
		quit("!CyclePalette: start and end must be 0-255");

	if (eend > strt) {
		// forwards
		wcolrotate(strt, eend, 0, _G(palette));
		set_palette_range(_G(palette), strt, eend, 0);
	} else {
		// backwards
		wcolrotate(eend, strt, 1, _G(palette));
		set_palette_range(_G(palette), eend, strt, 0);
	}

}
void SetPalRGB(int inndx, int rr, int gg, int bb) {
	if (_GP(game).color_depth > 1)
		invalidate_screen();

	wsetrgb(inndx, rr, gg, bb, _G(palette));
	set_palette_range(_G(palette), inndx, inndx, 0);
}
/*void scSetPal(color*pptr) {
wsetpalette(0,255,pptr);
}
void scGetPal(color*pptr) {
get_palette(pptr);
}*/

void UpdatePalette() {
	if (_GP(game).color_depth > 1)
		invalidate_screen();

	if (!_GP(play).fast_forward)
		setpal();
}

} // namespace AGS3
