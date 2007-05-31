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
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/cruise/various.cpp $
 * $Id:various.cpp 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#include "cruise/cruise_main.h"

namespace Cruise {

uint16 var0 = 0;
uint16 fadeVar;
uint16 main15;

int16 readB16(void *ptr) {
	int16 temp;

	temp = *(int16 *) ptr;
	flipShort(&temp);

	return temp;
}

char *getText(int textIndex, int overlayIndex) {
	if (!overlayTable[overlayIndex].ovlData) {
		return NULL;
	}

	if (!overlayTable[overlayIndex].ovlData->stringTable) {
		return NULL;
	}

	return overlayTable[overlayIndex].ovlData->stringTable[textIndex].
	    string;
}

} // End of namespace Cruise
