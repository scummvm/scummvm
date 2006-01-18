/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2005 Chris Apers - PalmOS Backend
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
 * $Header$
 *
 */

#include "be_zodiac.h"
#include "be_os5.h"

OSystem *OSystem_PalmOS5_create() {
	return OSystem_PalmOS5::create();
}

OSystem *OSystem_PalmOS5::create() {
	return new OSystem_PalmOS5();
}

OSystem *OSystem_PalmZodiac_create() {
	return OSystem_PalmZodiac::create();
}

OSystem *OSystem_PalmZodiac::create() {
	return new OSystem_PalmZodiac();
}
