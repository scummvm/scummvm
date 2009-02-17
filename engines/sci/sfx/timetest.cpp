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

#include <stdio.h>
#include <sfx_time.h>
#include <assert.h>

sfx_timestamp_t a, b, c;

int
main(int argc, char **argv) {
	int i;
	a = sfx_new_timestamp(10, 0, 1000);
	b = sfx_new_timestamp(10, 1000, 1000);
	c = sfx_new_timestamp(10, 2000, 1000);

	assert(sfx_timestamp_sample_diff(a, b) == -1);
	assert(sfx_timestamp_sample_diff(b, a) == 1);
	assert(sfx_timestamp_sample_diff(c, a) == 2);
	assert(sfx_timestamp_sample_diff(sfx_timestamp_add(b, 2000), a) == 2001);
	assert(sfx_timestamp_sample_diff(sfx_timestamp_add(b, 2000), sfx_timestamp_add(a, -1000)) == 3001);

	for (i = -10000; i < 10000; i++) {
		int v = sfx_timestamp_sample_diff(sfx_timestamp_add(c, i), c);
		if (v != i) {
			fprintf(stderr, "After adding %d samples: Got diff of %d\n", i, v);
			return 1;
		}
	}

	return 0;
}
