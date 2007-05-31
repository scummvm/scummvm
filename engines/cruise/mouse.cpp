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
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/cruise/mouse.cpp $
 * $Id:mouse.cpp 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#include "cruise/cruise_main.h"

namespace Cruise {

int16 main10;

void getMouseStatus(int16 *pMouseVar, int16 *pMouseX, int16 *pMouseButton,
	    int16 *pMouseY) {
	// mouseStatusStruct localStatus;

	// OSystem_GetMouseStatus(&localStatus);

	*pMouseX = 0;
	*pMouseY = 0;

	*pMouseButton = 0;
/*
  if(localStatus.left)
    *pMouseButton |= 1;
  if(localStatus.right)
    *pMouseButton |= 2;
  if(localStatus.middle)
    *pMouseButton |= 4;
	*/
}

} // End of namespace Cruise
