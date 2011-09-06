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
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE_SNDDRV_H
#define CGE_SNDDRV_H

namespace CGE {

// ******************************************************
// *  Constants                                         *
// ******************************************************

// sample info
struct SmpInfo {
	const uint8  *_saddr;                              // address
	uint16  _slen;                                     // length
	uint16  _span;                                     // left/right pan (0-15)
	int     _counter;                                  // number of time the sample should be played
};

// ******************************************************
// *  Data                                              *
// ******************************************************

// midi player flag (1 means we are playing)
extern uint16 _midiPlayFlag;

// midi song end flag (1 means we have crossed end mark)
extern uint16 _midiEndFlag;

// ******************************************************
// *  Driver Code                                       *
// ******************************************************
// Set Volume
void sndSetVolume();

} // End of namespace CGE

#endif
