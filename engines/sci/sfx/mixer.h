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

#ifndef SCI_SFX_MIXER_H
#define SCI_SFX_MIXER_H

#include "sci/sfx/sfx_pcm.h"

namespace Sci {

struct sfx_pcm_mixer_t {
	/* Mixers are the heart of all matters PCM. They take PCM data from subscribed feeds,
	** mix it (hence the name) and ask the pcm device they are attached to to play the
	** result.  */

	const char *name;
	const char *version;

	int (*init)(sfx_pcm_mixer_t *self);
	/* Initialises the mixer
	** Parameters: (sfx_pcm_mixer_t *) self: Self reference
	** Returns   : (int) SFX_OK on success, SFX_ERROR otherwise
	*/

	void (*exit)(sfx_pcm_mixer_t *self);
	/* Uninitialises the mixer
	** Parameters: (sfx_pcm_mixer_t *) self: Self reference
	** Also uninitialises all feeds and the attached output device.
	*/

	void (*subscribe)(sfx_pcm_mixer_t *self, sfx_pcm_feed_t *feed);
	/* Subscribes the mixer to a new feed
	** Parameters: (sfx_pcm_mixer_t *) self: Self reference
	**             (sfx_pcm_feed_t *) feed: The feed to subscribe to
	*/

	int (*process)(sfx_pcm_mixer_t *self);
	/* Processes all feeds, mixes their results, and passes everything to the output device
	** Returns  : (int) SFX_OK on success, SFX_ERROR otherwise (output device error or
	**                  internal assertion failure)
	** Effects  : All feeds are poll()ed, and the device is asked to output(). Buffer size
	**            depends on the time that has passed since the last call to process(), if
	**            any.
	*/

	void *private_bits;
};

sfx_pcm_mixer_t *sfx_pcm_find_mixer(char *name);
/* Looks up a mixer by name, or a default mixer
** Parameters: (char *) name: Name of the mixer to look for, or NULL to
**                            take a default
*/

extern sfx_pcm_mixer_t *mixer; /* _THE_ global pcm mixer */

sfx_pcm_mixer_t *getMixer();

} // End of namespace Sci

#endif // SCI_SFX_MIXER_H
