/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SKY_DEBUG_H
#define SKY_DEBUG_H

#include "stdafx.h"
#include "common/scummsys.h"

namespace Sky {

class SkyDebug {
public:
	static void fetchCompact(uint32 a);
	static void logic(uint32 logic);
	static void script(uint32 command, uint16 *scriptData);
	static void mcode(uint32 mcode, uint32 a, uint32 b, uint32 c);
};

} // End of namespace Sky

#endif
