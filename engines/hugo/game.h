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
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_GAME_H
#define HUGO_GAME_H

#include "common/keyboard.h"

namespace Common {
class WriteStream;
class SeekableReadStream;
}

namespace Hugo {

// Game specific equates
#define TAKE_TEXT   "Picked up the %s ok."

enum {LOOK_NAME = 1, TAKE_NAME};                    // Index of name used in showing takeables and in confirming take

// Definitions of 'generic' commands: Max # depends on size of gencmd in
// the object_t record since each requires 1 bit.  Currently up to 16
enum {LOOK = 1, TAKE = 2, DROP = 4, LOOK_S = 8};

enum TEXTCOLORS {
	_TBLACK,    _TBLUE,         _TGREEN,       _TCYAN,
	_TRED,      _TMAGENTA,      _TBROWN,       _TWHITE,
	_TGRAY,     _TLIGHTBLUE,    _TLIGHTGREEN,  _TLIGHTCYAN,
	_TLIGHTRED, _TLIGHTMAGENTA, _TLIGHTYELLOW, _TBRIGHTWHITE
};

enum uif_t {U_FONT5, U_FONT6, U_FONT8, UIF_IMAGES, NUM_UIF_ITEMS};
static const int kFirstFont = U_FONT5;

/**
 * Enumerate ways of cycling a sequence of frames
 */
enum cycle_t {kCycleInvisible, kCycleAlmostInvisible, kCycleNotCycling, kCycleForward, kCycleBackward};

/**
 * Enumerate sequence index matching direction of travel
 */
enum {RIGHT, LEFT, DOWN, _UP};

enum font_t {LARGE_ROMAN, MED_ROMAN, NUM_GDI_FONTS, INIT_FONTS, DEL_FONTS};

/**
 * Enumerate the different path types for an object
 */
enum path_t {
	kPathUser,                                      // User has control of object via cursor keys
	kPathAuto,                                      // Computer has control, controlled by action lists
	kPathQuiet,                                     // Computer has control and no commands allowed
	kPathChase,                                     // Computer has control, object is chasing hero
	kPathChase2,                                    // Same as CHASE, except keeps cycling when stationary
	kPathWander,                                    // Computer has control, object is wandering randomly
	kPathWander2                                    // Same as WANDER, except keeps cycling when stationary
};

struct hugo_boot_t {                                // Common HUGO boot file
	char checksum;                                  // Checksum for boot structure (not exit text)
	char registered;                                // TRUE if registered version, else FALSE
	char pbswitch[8];                               // Playback switch string
	char distrib[32];                               // Distributor branding string
	uint16 exit_len;                                // Length of exit text (next in file)
} PACKED_STRUCT;

/**
 * Game specific type definitions
 */
typedef byte *image_pt;                             // ptr to an object image (sprite)
typedef byte *sound_pt;                             // ptr to sound (or music) data

/**
 * Structure for initializing maze processing
 */
struct maze_t {
	bool enabledFl;                                 // TRUE when maze processing enabled
	byte size;                                      // Size of (square) maze matrix
	int  x1, y1, x2, y2;                            // maze hotspot bounding box
	int  x3, x4;                                    // north, south x entry coordinates
	byte firstScreenIndex;                          // index of first screen in maze
};

/**
 * The following is a linked list of images in an animation sequence
 * The image data is in 8-bit DIB format, i.e. 1 byte = 1 pixel
 */
struct seq_t {                                      // Linked list of images
	byte   *imagePtr;                               // ptr to image
	uint16  bytesPerLine8;                          // bytes per line (8bits)
	uint16  lines;                                  // lines
	uint16  x1, x2, y1, y2;                         // Offsets from x,y: data bounding box
	seq_t  *nextSeqPtr;                             // ptr to next record
};

/**
 * The following is an array of structures of above sequences
 */
struct seqList_t {
	uint16 imageNbr;                                // Number of images in sequence
	seq_t *seqPtr;                                  // Ptr to sequence structure
};

#include "common/pack-start.h"                      // START STRUCT PACKING
struct sound_hdr_t {                                // Sound file lookup entry
	uint16 size;                                    // Size of sound data in bytes
	uint32 offset;                                  // Offset of sound data in file
} PACKED_STRUCT;
#include "common/pack-end.h"                        // END STRUCT PACKING

static const int kMaxSeqNumb = 4;                   // Number of sequences of images in object

/**
 * Following is definition of object attributes
 */
struct object_t {
	uint16     nounIndex;                           // String identifying object
	uint16     dataIndex;                           // String describing the object
	uint16     *stateDataIndex;                     // Added by Strangerke to handle the LOOK_S state-dependant descriptions
	path_t     pathType;                            // Describe path object follows
	int        vxPath, vyPath;                      // Delta velocities (e.g. for CHASE)
	uint16     actIndex;                            // Action list to do on collision with hero
	byte       seqNumb;                             // Number of sequences in list
	seq_t     *currImagePtr;                        // Sequence image currently in use
	seqList_t  seqList[kMaxSeqNumb];                // Array of sequence structure ptrs and lengths
	cycle_t    cycling;                             // Whether cycling, forward or backward
	byte       cycleNumb;                           // No. of times to cycle
	byte       frameInterval;                       // Interval (in ticks) between frames
	byte       frameTimer;                          // Decrementing timer for above
	int8       radius;                              // Defines sphere of influence by hero
	byte       screenIndex;                         // Screen in which object resides
	int        x, y;                                // Current coordinates of object
	int        oldx, oldy;                          // Previous coordinates of object
	int8       vx, vy;                              // Velocity
	byte       objValue;                            // Value of object
	int        genericCmd;                          // Bit mask of 'generic' commands for object
	uint16     cmdIndex;                            // ptr to list of cmd structures for verbs
	bool       carriedFl;                           // TRUE if object being carried
	byte       state;                               // state referenced in cmd list
	bool       verbOnlyFl;                          // TRUE if verb-only cmds allowed e.g. sit,look
	byte       priority;                            // Whether object fore, background or floating
	int16      viewx, viewy;                        // Position to view object from (or 0 or -1)
	int16      direction;                           // Direction to view object from
	byte       curSeqNum;                           // Save which seq number currently in use
	byte       curImageNum;                         // Save which image of sequence currently in use
	int8       oldvx;                               // Previous vx (used in wandering)
	int8       oldvy;                               // Previous vy
};
} // End of namespace Hugo

#endif
