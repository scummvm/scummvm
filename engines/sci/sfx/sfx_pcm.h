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

#ifndef SCI_SFX_SFX_PCM_H
#define SCI_SFX_SFX_PCM_H

#include "sci/sfx/sfx.h"
#include "sound/timestamp.h"

namespace Sci {

#define SFX_PCM_MONO 0
#define SFX_PCM_STEREO_LR 1 /* left sample, then right sample */

/* The following are used internally by the mixer */
#define SFX_PCM_FORMAT_8  0
#define SFX_PCM_FORMAT_16 2


/* Pick one of these formats (including the _NATIVE) ones for your PCM feed */
#define SFX_PCM_FORMAT_U8     (0x0080 | SFX_PCM_FORMAT_8)			/* Unsigned (bias 128) 8 bit format */
#define SFX_PCM_FORMAT_S16_NATIVE (0x0000 | SFX_PCM_FORMAT_16)


struct sfx_pcm_config_t {
	int rate;   /* Sampling rate */
	int stereo; /* The stereo mode used (SFX_PCM_MONO or SFX_PCM_STEREO_*) */
	unsigned int format; /* Sample format (SFX_PCM_FORMAT_*) */
};


int sfx_pcm_available();
/* Determines whether a PCM device is available and has been initialised
** Returns   : (int) zero iff no PCM device is available
*/

} // End of namespace Sci

#endif // SCI_SFX_SFX_PCM_H
