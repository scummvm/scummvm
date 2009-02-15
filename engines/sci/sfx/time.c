/***************************************************************************
 time.c  Copyright (C) 2003 Christoph Reichenbach


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

#include <sfx_time.h>
#include <resource.h>

sfx_timestamp_t
sfx_new_timestamp(long secs, long usecs, int frame_rate)
{
	sfx_timestamp_t r;
	r.secs = secs;
	r.usecs = usecs;
	r.frame_rate = frame_rate;
	r.frame_offset = 0;

	return r;
}


sfx_timestamp_t
sfx_timestamp_add(sfx_timestamp_t timestamp, int frames)
{
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
sfx_timestamp_frame_diff(sfx_timestamp_t a, sfx_timestamp_t b)
{
	long usecdelta = 0;

	if (a.frame_rate != b.frame_rate) {
		fprintf(stderr, "Fatal: The semantics of subtracting two timestamps with a different base from each other is not defined!\n");
		BREAKPOINT();
	}

	if (a.usecs != b.usecs) {
#if (SIZEOF_LONG >= 8)
		usecdelta = (a.usecs * a.frame_rate) / 1000000
			- (b.usecs * b.frame_rate) / 1000000;
#else
		usecdelta = ((a.usecs/1000) * a.frame_rate) / 1000
			- ((b.usecs/1000) * b.frame_rate) / 1000;
#endif
	}

	return usecdelta
		+ (a.secs - b.secs) * a.frame_rate
		+ a.frame_offset - b.frame_offset;
}

long
sfx_timestamp_usecs_diff(sfx_timestamp_t t1, sfx_timestamp_t t2)
{
	long secs1, secs2;
	long usecs1, usecs2;

	sfx_timestamp_gettime(&t1, &secs1, &usecs1);
	sfx_timestamp_gettime(&t2, &secs2, &usecs2);

	return (usecs1 - usecs2) + ((secs1 - secs2) * 1000000);
} 

sfx_timestamp_t
sfx_timestamp_renormalise(sfx_timestamp_t timestamp, int new_freq)
{
	sfx_timestamp_t r;
	sfx_timestamp_gettime(&timestamp, &r.secs, &r.usecs);
	r.frame_rate = new_freq;
	r.frame_offset = 0;

	return r;
}

void
sfx_timestamp_gettime(sfx_timestamp_t *timestamp, long *secs, long *usecs)
{
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

