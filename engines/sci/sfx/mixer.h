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

/**
 * Subscribes the mixer to a new feed.
 * @param feed	The feed to subscribe to
 */
void mixer_subscribe(sfx_pcm_feed_t *feed);

/**
 * Processes all feeds, mixes their results, and passes everything to the output device.
 * Returns  : (int) SFX_OK on success, SFX_ERROR otherwise (output device error or
 *                  internal assertion failure)
 * Effects  : All feeds are poll()ed, and the device is asked to output(). Buffer size
 *            depends on the time that has passed since the last call to process(), if
 *            any.
 */
int mixer_process();

} // End of namespace Sci

#endif // SCI_SFX_MIXER_H
