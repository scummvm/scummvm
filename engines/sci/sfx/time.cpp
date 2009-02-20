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

#include "common/util.h"
#include "sci/include/sfx_time.h"
#include "sci/include/resource.h"

sfx_timestamp_t
sfx_new_timestamp(long secs, long usecs, int frame_rate) {
	sfx_timestamp_t r;
	r.secs = secs;
	r.usecs = usecs;
	r.frame_rate = frame_rate;
	r.frame_offset = 0;

	return r;
}


sfx_timestamp_t
sfx_timestamp_add(sfx_timestamp_t timestamp, int frames) {
	timestamp.frame_offset += frames;

	if (timestamp.frame_offset < 0) {
		int secsub = 1 + (-timestamp.frame_offset / timestamp.frame_rate);

		timestamp.frame_offset += timestamp.frame_rate * secsub;
		timestamp.secs -= secsub;
	}

	timestamp.secs += (timestamp.frame_offset / timestamp.frame_rate);
	timestamp.frame_offset %= timestamp.frame_rate;

	return timestamp;
}

int
sfx_timestamp_frame_diff(sfx_timestamp_t a, sfx_timestamp_t b) {
	long usecdelta = 0;

	if (a.frame_rate != b.frame_rate) {
		error("Fatal: The semantics of subtracting two timestamps with a different base from each other is not defined");
		BREAKPOINT();
	}

	if (a.usecs != b.usecs) {
#if (SIZEOF_LONG >= 8)
		usecdelta = (a.usecs * a.frame_rate) / 1000000
		            - (b.usecs * b.frame_rate) / 1000000;
#else
		usecdelta = ((a.usecs / 1000) * a.frame_rate) / 1000
		            - ((b.usecs / 1000) * b.frame_rate) / 1000;
#endif
	}

	return usecdelta
	       + (a.secs - b.secs) * a.frame_rate
	       + a.frame_offset - b.frame_offset;
}

long
sfx_timestamp_usecs_diff(sfx_timestamp_t t1, sfx_timestamp_t t2) {
	long secs1, secs2;
	long usecs1, usecs2;

	sfx_timestamp_gettime(&t1, &secs1, &usecs1);
	sfx_timestamp_gettime(&t2, &secs2, &usecs2);

	return (usecs1 - usecs2) + ((secs1 - secs2) * 1000000);
}

sfx_timestamp_t
sfx_timestamp_renormalise(sfx_timestamp_t timestamp, int new_freq) {
	sfx_timestamp_t r;
	sfx_timestamp_gettime(&timestamp, &r.secs, &r.usecs);
	r.frame_rate = new_freq;
	r.frame_offset = 0;

	return r;
}

void
sfx_timestamp_gettime(sfx_timestamp_t *timestamp, long *secs, long *usecs) {
	long ust = timestamp->usecs;
	/* On 64 bit machines, we can do an accurate computation */
#if (SIZEOF_LONG >= 8)
	ust += (timestamp->frame_offset * 1000000l) / (timestamp->frame_rate);
#else
	ust += (timestamp->frame_offset * 1000l) / (timestamp->frame_rate / 1000l);
#endif

	if (ust > 1000000) {
		ust -= 1000000;
		*secs = timestamp->secs + 1;
	} else
		*secs = timestamp->secs;

	*usecs = ust;
}

