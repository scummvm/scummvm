/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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

#ifndef INIT_SONY_H
#define INIT_SONY_H

#define SONY_PORTRAIT	1
#define SONY_LANDSCAPE	2

UInt16 SilkInit(UInt32 *retVersion);
void SilkRelease(UInt16 slkRefNum);

UInt16 SonyHRInit(UInt32 depth);
void SonyHRRelease(UInt16 HRrefNum);

UInt8 SonyScreenSize(UInt16 HRrefNum, Coord *stdw, Coord *stdh, Coord *fullw, Coord *fullh);

#endif