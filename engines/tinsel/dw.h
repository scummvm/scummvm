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
 * $URL$
 * $Id$
 *
 */

#ifndef TINSEL_DW_H
#define TINSEL_DW_H

#include "common/scummsys.h"
#include "common/endian.h"

namespace Tinsel {

/** scene handle data type */
typedef uint32		SCNHANDLE;

/** polygon handle */
typedef int HPOLYGON;


#define	EOS_CHAR		'\0'		// string terminator
#define	LF_CHAR			'\x0a'		// line feed

// file names
#define	SAMPLE_FILE		"english.smp"	// all samples
#define	SAMPLE_INDEX		"english.idx"	// sample index filename
#define	MIDI_FILE		"midi.dat"	// all MIDI sequences
#define	INDEX_FILENAME		"index"		// name of index file

#define	SCNHANDLE_SHIFT		23		// amount to shift scene handles by
#define	NO_SCNHANDLES		300		// number of memory handles for scenes
#define	MASTER_SCNHANDLE	(0 << SCNHANDLE_SHIFT)	// master scene memory handle

// the minimum value a integer number can have
#define	MIN_INT   (1 << (8*sizeof(int) - 1))
#define	MIN_INT16 (-32767)

// the maximum value a integer number can have
#define	MAX_INT	(~MIN_INT)

// TODO: v1->v2 scene files
#ifdef FILE_SPLIT
// each scene is split into 2 files
#define	INV_OBJ_SCNHANDLE	(2 << SCNHANDLE_SHIFT)	// inventory object handle (if there are inventory objects)
#else
#define	INV_OBJ_SCNHANDLE	(1 << SCNHANDLE_SHIFT)	// inventory object handle (if there are inventory objects)
#endif


#define FIELD_WORLD	0
#define FIELD_STATUS	1




// We don't set the Z position for print and talk text
// i.e. it gets a Z position of 0

#define Z_INV_BRECT	10	// Inventory background rectangle
#define Z_INV_MFRAME	15	// Inventory window frame
#define Z_INV_HTEXT	15	// Inventory heading text
#define Z_INV_ICONS	16	// Icons in inventory
#define Z_INV_ITEXT	995	// Icon text

#define Z_INV_RFRAME	22	// Re-sizing frame

#define Z_CURSOR	1000	// Cursor
#define Z_CURSORTRAIL	999	// Cursor trails
#define Z_ACURSOR	990	// Auxillary cursor

#define Z_TAG_TEXT	995	// In front of auxillary cursor

#define Z_MDGROOVE	20
#define Z_MDSLIDER	21

#define Z_TOPPLAY	100

#define Z_TOPW_TEXT	Z_TAG_TEXT

// Started a collection of assorted maximum numbers here:
#define MAX_MOVERS	6	// Moving actors using path system
#define MAX_SAVED_ACTORS 32	// Saved 'Normal' actors
#define MAX_SAVED_ALIVES 512	// Saves actors'lives

// Legal non-existant entrance number for LoadScene()
#define NO_ENTRY_NUM	(-3458)	// Magic unlikely number


#define SAMPLETIMEOUT	(15*ONE_SECOND)

// Language for the resource strings
enum LANGUAGE {
	TXT_ENGLISH, TXT_FRENCH, TXT_GERMAN,
		TXT_ITALIAN, TXT_SPANISH
};

} // end of namespace Tinsel

#endif	// TINSEL_DW_H
