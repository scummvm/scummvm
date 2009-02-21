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

#ifndef _FREESCI_SFX_TIMER_H_
#define _FREESCI_SFX_TIMER_H_

#include "sci/sfx/sfx_core.h"

namespace Sci {

struct sfx_timer_t {
	int delay_ms; /* Approximate delay (in milliseconds) between calls */

	int (*init)(void (*callback)(void *data), void *data);
	/* Initializes the timer
	** Parameters: (void* -> void) callback:
	**                                   'data' must contain the next argument:
	**             (void *) data: Must always be passed to the callback
	** Returns   : (int) SFX_OK on success, SFX_ERROR on failure
	** This does not start the timer yet, it just specifies and initializes it.
	** This function is called exactly once (provided that the timer is used at all).
	*/

	int (*exit)();
	/* Stops the timer
	** Returns   : (int) SFX_OK on success, SFX_ERROR on failure
	** All resources allocated with the timer should be freed as an effect
	** of this.
	*/
};

} // End of namespace Sci

#endif /* !_FREESCI_SFX_TIMER_H_ */
