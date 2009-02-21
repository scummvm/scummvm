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

#include "common/timer.h"
#include "engines/engine.h"
#include "sci/include/sfx_timer.h"

namespace Sci {

#define FREQ 60
#define DELAY (1000000 / FREQ)

typedef void (*scummvm_timer_callback_t)(void *);
static scummvm_timer_callback_t scummvm_timer_callback = NULL;
static void *scummvm_timer_callback_data = NULL;
extern ::Engine *g_engine;

void scummvm_timer_update_internal(void *ptr) {
	if (scummvm_timer_callback)
		scummvm_timer_callback(scummvm_timer_callback_data);
}

int scummvm_timer_start(void (*func)(void *), void *data) {
	if (scummvm_timer_callback) {
		fprintf(stderr,
		        "Error: Attempt to initialize gametick timer more than once\n");
		return SFX_ERROR;
	}

	if (!func) {
		fprintf(stderr,
		        "Error: Attempt to initialize gametick timer w/o callback\n");
		return SFX_ERROR;
	}

	scummvm_timer_callback = func;
	scummvm_timer_callback_data = data;

	::g_engine->getTimerManager()->installTimerProc(&scummvm_timer_update_internal, DELAY, NULL);
	return SFX_OK;
}

int scummvm_timer_stop() {
	scummvm_timer_callback = NULL;
	return SFX_OK;
}


sfx_timer_t sfx_timer_scummvm = {
	DELAY / 1000,
	&scummvm_timer_start,
	&scummvm_timer_stop
};

} // End of namespace Sci
