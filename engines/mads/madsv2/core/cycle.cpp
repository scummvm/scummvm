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

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/color.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/pal.h"

namespace MADS {
namespace MADSV2 {

Palette cycling_palette;

CycleList cycle_list;
int cycling_active = false;
int cycling_delay = 0;
int cycling_threshold;
int total_cycle_colors;

long cycle_timing[COLOR_MAX_CYCLES];
static RGBcolor temp_color;

void cycle_init(CycleListPtr new_cycle_list, int activate) {
	int count;
	long current_time;

	current_time = timer_read();
	for (count = 0; count < COLOR_MAX_CYCLES; count++) {
		cycle_timing[count] = current_time;
	}

	memcpy(&cycle_list, new_cycle_list, sizeof(CycleList));
	memcpy(cycling_palette, master_palette, sizeof(Palette));

	total_cycle_colors = 0;
	for (count = 0; count < cycle_list.num_cycles; count++) {
		total_cycle_colors += cycle_list.table[count].num_colors;
		cycle_list.table[count].first_list_color = 0;
	}

	cycling_threshold = 3;
	if (total_cycle_colors <= 16) cycling_threshold = 0;

	cycling_active = activate;
}

void cycle_colors(void) {
	int num;
	int first;
	int count;
	int any_changes;
	int mark;
	long this_timing;

	if (!cycling_active)
		return;

	cycling_delay++;
	if (cycling_delay < cycling_threshold)
		return;

	if (mcga_palette_update)
		return;

	this_timing = timer_read();
	any_changes = false;

	for (count = 0; count < cycle_list.num_cycles; count++) {
		if (this_timing >= (cycle_timing[count] + cycle_list.table[count].ticks)) {
			cycle_timing[count] = this_timing;
			num = cycle_list.table[count].num_colors;
			first = cycle_list.table[count].first_palette_color;
			mark = cycle_list.table[count].first_list_color;
			any_changes = true;

			if (num > 1) {
				RGBcolor *base = &cycling_palette[first];
				RGBcolor tmp;

				// Save the last color
				tmp = base[num - 1];

				// Shift all colors forward by one
				memmove(&base[1], &base[0], (num - 1) * sizeof(Palette));

				// Write saved last color into the first slot
				base[0] = tmp;

				// Advance and wrap the cycle position marker
				mark++;
				if (mark >= num)
					mark = 0;
			}

			cycle_list.table[count].first_list_color = (byte)mark;
		}
	}

	if (any_changes) {
		mcga_setpal_range(&cycling_palette,
			cycle_list.table[0].first_palette_color,
			total_cycle_colors);
	}

	cycling_delay = 0;
}

} // namespace MADSV2
} // namespace MADS
