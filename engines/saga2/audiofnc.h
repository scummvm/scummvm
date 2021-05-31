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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_AUDIOFNC_H
#define SAGA2_AUDIOFNC_H

namespace Saga2 {

/* ===================================================================== *
   DECODERS
     Decoders are generally declared in groups of three functions
     - An Open/Seek routine to prepare for reading
     - A Read routine to move the actual data
     - A Flush/Close routine to clean up

     Three types of decoder currently exist
     - In Place Decoders : operate on a buffer in place
     - Buffered Decoders : require a temporary work buffer
     - Buffer Loaders    : read buffers from disk etc.
 * ===================================================================== */


/*******************************************************************/
/* Declaration Macros                                              */

#define INPLACEDEC( procname ) \
	int16 procname( Buffer &sb, soundDecoder *sd, soundSample &ss )
#define BUFFERDEC( procname ) \
	int16 procname( Buffer &sb, soundDecoder *sd, soundSample &ss, workBuffer *wb )
#define BUFFERLOD( procname ) \
	int16 procname( Buffer &sb, soundSample &ss )

/*******************************************************************/
/* LOADERS                                                         */

// static buffer source

BUFFERLOD(seekBuffer);
BUFFERLOD(readBuffer);
BUFFERLOD(flushBuffer);

// FILE* source

BUFFERLOD(readFile);
BUFFERLOD(seekFile);
BUFFERLOD(flushFile);

/*******************************************************************/
/* TRANSLATORS                                                     */

INPLACEDEC(stereoToMonoSeek);
INPLACEDEC(stereoToMono);
INPLACEDEC(stereoToMonoFlush);


/*******************************************************************/
/* BUFFERED DECODERS                                               */

BUFFERDEC(readDecompress);
BUFFERDEC(seekDecompress);
BUFFERDEC(flushDecompress);

} // end of namespace Saga2

#endif
