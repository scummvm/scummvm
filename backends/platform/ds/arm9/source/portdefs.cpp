
/* ScummVMDS - Scumm Interpreter DS Port
 * Copyright (C) 2002-2004 The ScummVM project and Neil Millstone
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

#include "portdefs.h"
#include <string.h>
#include "nds/dma.h"
#include "osystem_ds.h"

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

/* ScummVMDS - Scumm Interpreter DS Port
 * Copyright (C) 2002-2004 The ScummVM project and Neil Millstone
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

#include "portdefs.h"
#include <string.h>
#include "nds/dma.h"
#include "osystem_ds.h"

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
