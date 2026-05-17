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

#include "common/textconsole.h"
#include "graphics/palette.h"
#include "mads/madsv2/animview/functions.h"
#include "mads/madsv2/animview/animview.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/pal.h"

namespace MADS {
namespace MADSV2 {
namespace AnimView {

int anim_count;
AnimEntry anim_list[MAX_ANIM];
bool background_load_status;
int16 sound_interrupts_mode;
bool show_white_bars;
bool resync_timer1, resync_timer2;
bool exit_immediately_at_end;
bool do_not_clear_screen;

/**
 * Initialize globals
 */
void functions_init() {
	anim_count = 0;
	background_load_status = true;
	sound_interrupts_mode = 0;
	show_white_bars = false;
	resync_timer1 = true;
	resync_timer2 = false;
	exit_immediately_at_end = false;
	do_not_clear_screen = false;
}

/**
 * Adds an animation to the list of .aa files to show in sequence
 * @param name		Animation resource name
 */
void add_anim(const char *name) {
	static char buf[16];

	if (strlen(name) > 0 && anim_count < MAX_ANIM) {
		Common::strcpy_s(buf, name);
		if (!strchr(buf, '.'))
			Common::strcat_s(buf, ".aa");

		Common::strcpy_s(anim_list[anim_count].name, buf);
		anim_list[anim_count].bg_load_status = background_load_status;
		anim_list[anim_count].sound_mode = sound_interrupts_mode;
		anim_list[anim_count].show_bars = show_white_bars;
		++anim_count;
	}
}

/**
 * Parses a flag from an animation line in the resource file
 */
void flag_parse(const char *param) {
	switch (tolower(*param++)) {
	case 'o':
		// Specify opening special effect
		assert(anim_count < MAX_ANIM);
		if (*param == ':')
			anim_list[anim_count].fx = atoi(param + 1);
		break;

	case 'r':
		// -r[:abn] Resynch timer (always, beginning, never)
		if (*param == ':') {
			switch (tolower(*++param)) {
			case 'n':
				resync_timer1 = true;
				resync_timer2 = false;
				break;
			case 'a':
				resync_timer1 = false;
				break;
			case 'b':
				resync_timer1 = true;
				resync_timer2 = true;
				break;
			default:
				break;
			}
		}
		break;

	case 'w':
		// Toggle white bars on or off
		show_white_bars = !show_white_bars;
		break;

	case 'x':
		// Exit immediately after last frame
		exit_immediately_at_end = true;
		break;

	case 'y':
		// Do not clear screen at start
		do_not_clear_screen = true;
		break;

	default:
		error("Unsupported animview flag - %c", *param);
		break;
	}
}

/**
 * Reads the contents of the resource file stream, and adds
 * entries to the anim_list for what to display
 */
void read_resource(Common::SeekableReadStream *src) {
	while (!src->eos()) {
		Common::String line = src->readLine();
		line.trim();
		if (line.empty())
			continue;

		// Handle any flags at the start of the line
		const char *lineP = line.c_str();
		while (strchr("/-", *lineP)) {
			// It's a flag
			++lineP;
			const char *switchEnd = strchr(lineP, ' ');
			Common::String param;

			if (switchEnd) {
				// There's more line after the flag
				param = Common::String(lineP, switchEnd);
				for (lineP = switchEnd; *lineP == ' '; ++lineP) {
				}
			} else {
				// This is the last flag of the line
				param = Common::String(lineP);
				lineP = lineP + strlen(lineP);
			}

			flag_parse(param.c_str());
		}

		// As long as we're not at the end of the line, any remainder
		// should be the name of the animation resource to play
		if (*lineP)
			add_anim(lineP);
	}
}

void anim_peel() {
	int peelX = current_anim->misc_peel_x;
	if (peelX) {
		buffer_peel_horiz(&scr_work, peelX);
		matte_refresh_work();
	}

	int peelY = current_anim->misc_peel_y;
	if (peelY) {
		buffer_peel_vert(&scr_work, peelY, scr_inter_orig.data, 320 * 200);
		matte_refresh_work();
	}
}

void anim_setup_cycle(int fx) {
	cycling_active = false;
	memcpy(cycling_palette, master_palette, sizeof(Palette));

	if (fx)
		mcga_setpal(&master_palette);

	cycle_init(&anim_cycle_list, has_cycles && !fx);
}

/**
 * Handleas incremental fading by subtracting an amount from each palette
 * entries' RGB values down towards zero
 */
bool anim_fade(Palette *pal, int fadeAmount) {
	bool palChanged = false;
	byte *rgb = (byte *)pal;
	for (int count = 0; count < Graphics::PALETTE_SIZE; ++count, ++rgb) {
		if (*rgb)
			palChanged = true;
		*rgb = MAX((int)*rgb - fadeAmount, 0);
	}

	return palChanged;
}

} // namespace AnimView
} // namespace MADSV2
} // namespace MADS
