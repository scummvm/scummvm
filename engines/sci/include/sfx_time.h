/***************************************************************************
 sfx_time.h  Copyright (C) 2003,04 Christoph Reichenbach


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

#ifndef _SFX_TIME_H_
#define _SFX_TIME_H_

typedef struct {
	long secs;
	long usecs;
	int frame_rate;
	int frame_offset;
	/* Total time: secs + usecs + frame_offset/frame_rate */
} sfx_timestamp_t;


sfx_timestamp_t
sfx_new_timestamp(long secs, long usecs, int frame_rate);
/* Creates a new mutable timestamp
** Parameters: (long x long) (secs, usecs): Initial timestamp
**             (int) frame_rate: Frame rate, for increasing the time stamp
*/

sfx_timestamp_t
sfx_timestamp_add(sfx_timestamp_t timestamp, int frames);
/* Adds a number of frames to a timestamp
** Parameters: (sfx_timestampt_t *) timestamp: The timestamp to update
**             (int) frames: Number of frames to add
** Returns   : (sfx_timestamp_t) The increased timestamp
*/

sfx_timestamp_t
sfx_timestamp_renormalise(sfx_timestamp_t timestamp, int new_freq);
/* Translates a timestamp to a new base frame frequency
** Parameters: (sfx_timestamp_t *) timestamp: The timestamp to normalise
**             (int) new_freq: The new frequency to normalise to
** Returns   : (sfx_timestamp_t) The re-normalised timestamp
** The translation looses accuracy in the order of magnitude of milliseconds
** for "usual" sampling frequencies.
*/

int
sfx_timestamp_frame_diff(sfx_timestamp_t a, sfx_timestamp_t b);
/* Computes the difference (# of frames) between two timestamps
** Parameters: (sfx_timestamp) a: See below
**             (sfx_timestamp) b: See below
** Returns   : (int) a-b
*/

long
sfx_timestamp_usecs_diff(sfx_timestamp_t a, sfx_timestamp_t b);
/* Computes the difference (# of microseconds) between two timestamps
** Parameters: (sfx_timestamp) a: See below
**             (sfx_timestamp) b: See below
** Returns   : (long) a-b
*/

void
sfx_timestamp_gettime(sfx_timestamp_t *timestamp, long *secs, long *usecs);
/* Determines the time described by a given timestamp
** Parameters: (sfx_timestamp_t *) timestamp: Timestamp to read from
** Returns   : (int * x int *) (secs, usecs): Seconds and microseconds since
**                                            the epoch described there
*/



#endif /* !defined(_SFX_TIME_H_) */
