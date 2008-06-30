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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <string.h>
#include "nds/dma.h"
#include "osystem_ds.h"

/*
extern "C" time_t __wrap_time(time_t* t) {
	if (t) {
		*t = OSystem_DS::instance()->getMillis() / 1000;
	}

	return OSystem_DS::instance()->getMillis() / 1000;
}
*/



time_t DS_time(time_t) {
	if (OSystem_DS::instance()) {
		return 0xABCD1234 + (OSystem_DS::instance()->getMillis() / 1000);
	} else {
		return 0xABCD1234;
	}
}

time_t DS_time(long* t) {
	if (OSystem_DS::instance()) {
		if (t) *t = 0xABCD1234 + (OSystem_DS::instance()->getMillis() / 1000);
		return 0xABCD1234 + (OSystem_DS::instance()->getMillis() / 1000);
	} else {
		if (t) *t = 0xABCD1234;
		return 0xABCD1234;
	}
}

