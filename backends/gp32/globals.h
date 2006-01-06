/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2006 Won Star - GP32 Backend
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

#ifndef __GLOBALS_H
#define __GLOBALS_H

enum {
	FM_QUALITY_LOW = 0,
	FM_QUALITY_MED,
	FM_QUALITY_HI
};

struct GlobalVars {
	uint16 cpuSpeed;
	uint16 gammaRamp;
	uint8 fmQuality;
	uint32 sampleRate;
};

extern GlobalVars g_vars;

#endif
