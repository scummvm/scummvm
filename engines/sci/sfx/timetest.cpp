/***************************************************************************
 timetest.c  Copyright (C) 2003 Christoph Reichenbach


 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence as
 published by the Free Software Foundaton; either version 2 of the
 Licence, or (at your option) any later version.

 It is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 merchantibility or fitness for a particular purpose. See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with this program; see the file COPYING. If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.


 Please contact the maintainer for any program-related bug reports or
 inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include <stdio.h>
#include <sfx_time.h>
#include <assert.h>

sfx_timestamp_t a,b,c;

int
main(int argc, char **argv)
{
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
