/* AWE - Another World Engine
 * Copyright (C) 2004 Gregory Montoir
 * Copyright (C) 2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include "stdafx.h"

#include "intern.h"

namespace Awe {

enum {
	DBG_LOGIC = 1 << 0,
	DBG_BANK  = 1 << 1,
	DBG_VIDEO = 1 << 2,
	DBG_INFO  = 1 << 3
};

extern uint16 g_debugMask;

extern void debug(uint16 cm, const char *msg, ...);
extern void error(const char *msg, ...);
extern void warning(const char *msg, ...);

extern void string_lower(char *p);
extern void string_upper(char *p);

}

#endif
