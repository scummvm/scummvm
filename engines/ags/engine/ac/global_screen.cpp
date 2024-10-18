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
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_screen.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/screen.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/media/audio/audio.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/gfx/bitmap.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;


void FlipScreen(int amount) {
	if ((amount < 0) | (amount > 3)) quit("!FlipScreen: invalid argument (0-3)");
	_GP(play).screen_flipped = amount;
}

void ShakeScreen(int severe) {
	EndSkippingUntilCharStops();

	if (_GP(play).fast_forward)
		return;

	severe = data_to_game_coord(severe);

	// TODO: support shaking room viewport separately
	// TODO: rely on game speed setting? and/or provide frequency and duration args
	// TODO: unify blocking and non-blocking shake update

	_GP(play).shakesc_length = 10;
	_GP(play).shakesc_delay = 2;
	_GP(play).shakesc_amount = severe;
	_GP(play).mouse_cursor_hidden++;

	// FIXME: we have to sync audio here explicitly, because ShakeScreen
	// does not call any game update function while it works
	sync_audio_playback();
	if (_G(gfxDriver)->RequiresFullRedrawEachFrame()) {
		for (int hh = 0; hh < 40; hh++) {
			_G(loopcounter)++;
			_G(platform)->Delay(50);

			render_graphics();

			update_polled_stuff();
		}
	} else {
		// Optimized variant for software render: create game scene once and shake it
		construct_game_scene();
		_G(gfxDriver)->RenderToBackBuffer();
		for (int hh = 0; hh < 40; hh++) {
			_G(platform)->Delay(50);
			const int yoff = hh % 2 == 0 ? 0 : severe;
			_GP(play).shake_screen_yoff = yoff;
			render_to_screen();
			update_polled_stuff();
		}
		clear_letterbox_borders();
		render_to_screen();
	}
	sync_audio_playback();

	_GP(play).mouse_cursor_hidden--;
	_GP(play).shakesc_length = 0;
	_GP(play).shakesc_delay = 0;
	_GP(play).shakesc_amount = 0;
}

void ShakeScreenBackground(int delay, int amount, int length) {
	if (delay < 2)
		quit("!ShakeScreenBackground: invalid delay parameter");

	amount = data_to_game_coord(amount);

	if (amount < _GP(play).shakesc_amount) {
		// from a bigger to smaller shake, clear up the borders
		clear_letterbox_borders();
	}

	_GP(play).shakesc_amount = amount;
	_GP(play).shakesc_delay = delay;
	_GP(play).shakesc_length = length;
}

void TintScreen(int red, int grn, int blu) {
	if ((red < 0) || (grn < 0) || (blu < 0) || (red > 100) || (grn > 100) || (blu > 100))
		quit("!TintScreen: RGB values must be 0-100");

	invalidate_screen();

	if ((red == 0) && (grn == 0) && (blu == 0)) {
		_GP(play).screen_tint = -1;
		return;
	}
	red = (red * 25) / 10;
	grn = (grn * 25) / 10;
	blu = (blu * 25) / 10;
	_GP(play).screen_tint = red + (grn << 8) + (blu << 16);
}

void FadeOut(int sppd) {
	EndSkippingUntilCharStops();

	if (_GP(play).fast_forward)
		return;

	// FIXME: we have to sync audio here explicitly, because FadeOut
	// does not call any game update function while it works
	sync_audio_playback();
	fadeout_impl(sppd);
	sync_audio_playback();
}

void fadeout_impl(int spdd) {
	if (_GP(play).screen_is_faded_out == 0) {
		_G(gfxDriver)->FadeOut(spdd, _GP(play).fade_to_red, _GP(play).fade_to_green, _GP(play).fade_to_blue, RENDER_SHOT_SKIP_ON_FADE);
		_GP(play).screen_is_faded_out = 1;
	}
}

void SetScreenTransition(int newtrans) {
	if ((newtrans < 0) || (newtrans > FADE_LAST))
		quit("!SetScreenTransition: invalid transition type");

	_GP(play).fade_effect = newtrans;

	debug_script_log("Screen transition changed");
}

void SetNextScreenTransition(int newtrans) {
	if ((newtrans < 0) || (newtrans > FADE_LAST))
		quit("!SetNextScreenTransition: invalid transition type");

	_GP(play).next_screen_transition = newtrans;

	debug_script_log("SetNextScreenTransition engaged");
}

void SetFadeColor(int red, int green, int blue) {
	if ((red < 0) || (red > 255) || (green < 0) || (green > 255) ||
	        (blue < 0) || (blue > 255))
		quit("!SetFadeColor: Red, Green and Blue must be 0-255");

	_GP(play).fade_to_red = red;
	_GP(play).fade_to_green = green;
	_GP(play).fade_to_blue = blue;
}

void FadeIn(int sppd) {
	EndSkippingUntilCharStops();

	if (_GP(play).fast_forward)
		return;

	// Update drawables, prepare them for the transition-in
	// in case this is called after the game state change but before any update was run
	SyncDrawablesState();
	// FIXME: we have to sync audio here explicitly, because FadeIn
	// does not call any game update function while it works
	sync_audio_playback();
	fadein_impl(_G(palette), sppd);
	sync_audio_playback();
}

} // namespace AGS3
