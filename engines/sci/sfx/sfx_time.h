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

#ifndef SCI_SFX_SFX_TIME_H
#define SCI_SFX_SFX_TIME_H

namespace Sci {

struct sfx_timestamp_t {
	long secs;
	long usecs;
	int frame_rate;
	int frame_offset;
	/* Total time: secs + usecs + frame_offset/frame_rate */
};


sfx_timestamp_t sfx_new_timestamp(long secs, long usecs, int frame_rate);
/* Creates a new mutable timestamp
** Parameters: (long x long) (secs, usecs): Initial timestamp
**             (int) frame_rate: Frame rate, for increasing the time stamp
*/

sfx_timestamp_t sfx_timestamp_add(sfx_timestamp_t timestamp, int frames);
/* Adds a number of frames to a timestamp
** Parameters: (sfx_timestampt_t *) timestamp: The timestamp to update
**             (int) frames: Number of frames to add
** Returns   : (sfx_timestamp_t) The increased timestamp
*/

sfx_timestamp_t sfx_timestamp_renormalise(sfx_timestamp_t timestamp, int new_freq);
/* Translates a timestamp to a new base frame frequency
** Parameters: (sfx_timestamp_t *) timestamp: The timestamp to normalise
**             (int) new_freq: The new frequency to normalise to
** Returns   : (sfx_timestamp_t) The re-normalised timestamp
** The translation looses accuracy in the order of magnitude of milliseconds
** for "usual" sampling frequencies.
*/

int sfx_timestamp_frame_diff(sfx_timestamp_t a, sfx_timestamp_t b);
/* Computes the difference (# of frames) between two timestamps
** Parameters: (sfx_timestamp) a: See below
**             (sfx_timestamp) b: See below
** Returns   : (int) a-b
*/

long sfx_timestamp_usecs_diff(sfx_timestamp_t a, sfx_timestamp_t b);
/* Computes the difference (# of microseconds) between two timestamps
** Parameters: (sfx_timestamp) a: See below
**             (sfx_timestamp) b: See below
** Returns   : (long) a-b
*/

void sfx_timestamp_gettime(sfx_timestamp_t *timestamp, long *secs, long *usecs);
/* Determines the time described by a given timestamp
** Parameters: (sfx_timestamp_t *) timestamp: Timestamp to read from
** Returns   : (int * x int *) (secs, usecs): Seconds and microseconds since
**                                            the epoch described there
*/

} // End of namespace Sci

#endif // SCI_SFX_SFX_TIME_H
