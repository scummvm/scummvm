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
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "avalanche/enhanced2.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Avalanche {

	namespace Enhanced {

	bool isenh() {
		warning("STUB: Enhanced::isenh()");
	}

	void readkeye() {
		warning("STUB: Enhanced::readkeye()");
	}

	bool keypressede() {
	/*
	 function fancystuff:boolean;
	  inline( $B4/ $11/  { MOV AH,11 }
			  $CD/ $16/  { INT 16 }
			  $B8/ $00/ $00/ { MOV AX, 0000 }
			  $74/ $01/  { JZ 0112 (or wherever- the next byte after $40, anyway) }
			  $40);      { INC AX }
	*/
	
		warning("STUB: Enhanced::keypressede()");
	}

	} // End of namespace Enhanced

} // End of namespace Avalanche
