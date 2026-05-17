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

#ifndef MADSV2_ANIMVIEW_FUNCTIONS_H
#define MADSV2_ANIMVIEW_FUNCTIONS_H

#include "common/stream.h"
#include "mads/madsv2/core/pal.h"

namespace MADS {
namespace MADSV2 {
namespace AnimView {

struct AnimEntry {
	char name[16];
	bool bg_load_status;
	uint8 sound_mode;
	uint8 show_bars;
	uint8 fx;
};
constexpr int MAX_ANIM = 40;

extern int anim_count;
extern AnimEntry anim_list[MAX_ANIM];
extern bool background_load_status;
extern int16 sound_interrupts_mode;
extern bool show_white_bars;
extern bool resync_timer1, resync_timer2;
extern bool exit_immediately_at_end;
extern bool do_not_clear_screen;


/**
 * Initialize globals
 */
extern void functions_init();

/**
 * Adds an animation to the list of .aa files to show in sequence
 * @param name		Animation resource name
 */
extern void add_anim(const char *name);

/**
 * Parses a flag from an animation line in the resource file
 */
extern void flag_parse(const char *param);

/**
 * Reads the contents of the resource file stream, and adds
 * entries to the anim_list for what to display
 */
extern void read_resource(Common::SeekableReadStream *src);

extern void anim_peel();

extern void anim_setup_cycle(int fx);
extern bool anim_fade(Palette *pal, int fadeAmount);

} // namespace AnimView
} // namespace MADSV2
} // namespace MADS

#endif
