/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sfx/sfx_time.h"
#include "sci/tools.h"

namespace Sci {

sfx_timestamp_t sfx_new_timestamp(const uint32 msecs, const int frame_rate) {
	sfx_timestamp_t r;
	r.msecs = msecs;
	r.frame_rate = frame_rate;
	r.frame_offset = 0;

	return r;
}

sfx_timestamp_t sfx_timestamp_add(sfx_timestamp_t timestamp, int frames) {
	timestamp.frame_offset += frames;

	if (timestamp.frame_offset < 0) {
		int secsub = 1 + (-timestamp.frame_offset / timestamp.frame_rate);

		timestamp.frame_offset += timestamp.frame_rate * secsub;
		timestamp.msecs -= secsub * 1000;
	}

	timestamp.msecs += (timestamp.frame_offset / timestamp.frame_rate) * 1000;
	timestamp.frame_offset %= timestamp.frame_rate;

	return timestamp;
}

int sfx_timestamp_frame_diff(sfx_timestamp_t a, sfx_timestamp_t b) {
	int msecdelta = 0;

	if (a.frame_rate != b.frame_rate) {
		fprintf(stderr, "Fatal: The semantics of subtracting two timestamps with a different base from each other is not defined!\n");
		BREAKPOINT();
	}

	if (a.msecs != b.msecs)
		msecdelta = (long(a.msecs) - long(b.msecs)) * a.frame_rate / 1000;

	return msecdelta + a.frame_offset - b.frame_offset;
}

int sfx_timestamp_msecs_diff(sfx_timestamp_t t1, sfx_timestamp_t t2) {
	uint32 msecs1, msecs2;

	sfx_timestamp_gettime(&t1, &msecs1);
	sfx_timestamp_gettime(&t2, &msecs2);

	return long(msecs1) - long(msecs2);
}

sfx_timestamp_t sfx_timestamp_renormalise(sfx_timestamp_t timestamp, int new_freq) {
	sfx_timestamp_t r;
	sfx_timestamp_gettime(&timestamp, &r.msecs);
	r.frame_rate = new_freq;
	r.frame_offset = 0;

	return r;
}

void sfx_timestamp_gettime(sfx_timestamp_t *timestamp, uint32 *msecs) {
	*msecs = timestamp->msecs +
			 timestamp->frame_offset * 1000l / timestamp->frame_rate;
}

} // End of namespace Sci
