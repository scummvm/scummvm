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

// WARNING!!
// Type "PPG" in the game to enter cheat mode.

#define COPYRIGHT   "Copyright 1989-1997 David P Gray, All Rights Reserved."

// Started code on 04/01/95
// VER "1.0"  // 10/01/95 Initial Release
// VER "1.1"  // 10/06/95 Restore system volume levels on exit
// VER "v1.2" // 10/12/95 Added "background music" checkbox in volume dlg
// VER "v1.3" // 10/23/95 Support game 1 as shareware
// VER "v1.4" // 12/06/95 Faster graphics, logical palette
// VER "v1.5" // 10/07/97 Added order form, new web site

// Game specific equates
#define TAKE_TEXT      "Picked up the %s ok."

// Only for non-database
#define BKGEXT         ".PCX"                       // Extension of background files
#define OBJEXT         ".PIX"                       // Extension of object picture files

// Name scenery and objects picture databases
#define OBJECTS_FILE   "objects.dat"
#define STRING_FILE    "strings.dat"
#define SOUND_FILE     "sounds.dat"

// User interface database (Windows Only)
// This file contains, between others, the bitmaps of the fonts used in the application
#define UIF_FILE   "uif.dat"

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

/**
* Following defines the action types and action list
*/
enum action_t {                                     // Parameters:
	ANULL              = 0xff,                      // Special NOP used to 'delete' events in DEL_EVENTS
	ASCHEDULE          = 0,                         //  0 - Ptr to action list to be rescheduled
	START_OBJ,                                      //  1 - Object number
	INIT_OBJXY,                                     //  2 - Object number, x,y
	PROMPT,                                         //  3 - index of prompt & response string, ptrs to action
	                                                //      lists.  First if response matches, 2nd if not.
	BKGD_COLOR,                                     //  4 - new background color
	INIT_OBJVXY,                                    //  5 - Object number, vx, vy
	INIT_CARRY,                                     //  6 - Object number, carried status
	INIT_HF_COORD,                                  //  7 - Object number (gets hero's 'feet' coordinates)
	NEW_SCREEN,                                     //  8 - New screen number
	INIT_OBJSTATE,                                  //  9 - Object number, new object state
	INIT_PATH,                                      // 10 - Object number, new path type
	COND_R,                                         // 11 - Conditional on object state - req state, 2 act_lists
	TEXT,                                           // 12 - Simple text box
	SWAP_IMAGES,                                    // 13 - Swap 2 object images
	COND_SCR,                                       // 14 - Conditional on current screen
	AUTOPILOT,                                      // 15 - Set object to home in on another (stationary) object
	INIT_OBJ_SEQ,                                   // 16 - Object number, sequence index to set curr_seq_p to
	SET_STATE_BITS,                                 // 17 - Objnum, mask to OR with obj states word
	CLEAR_STATE_BITS,                               // 18 - Objnum, mask to ~AND with obj states word
	TEST_STATE_BITS,                                // 19 - Objnum, mask to test obj states word
	DEL_EVENTS,                                     // 20 - Action type to delete all occurrences of
	GAMEOVER,                                       // 21 - Disable hero & commands.  Game is over
	INIT_HH_COORD,                                  // 22 - Object number (gets hero's actual coordinates)
	EXIT,                                           // 23 - Exit game back to DOS
	BONUS,                                          // 24 - Get score bonus for an action
	COND_BOX,                                       // 25 - Conditional on object within bounding box
	SOUND,                                          // 26 - Set currently playing sound
	ADD_SCORE,                                      // 27 - Add object's value to current score
	SUB_SCORE,                                      // 28 - Subtract object's value from current score
	COND_CARRY,                                     // 29 - Conditional on carrying object
	INIT_MAZE,                                      // 30 - Start special maze hotspot processing
	EXIT_MAZE,                                      // 31 - Exit special maze processing
	INIT_PRIORITY,                                  // 32 - Initialize fbg field
	INIT_SCREEN,                                    // 33 - Initialise screen field of object
	AGSCHEDULE,                                     // 34 - Global schedule - lasts over new screen
	REMAPPAL,                                       // 35 - Remappe palette - palette index, color
	COND_NOUN,                                      // 36 - Conditional on noun appearing in line
	SCREEN_STATE,                                   // 37 - Set new screen state - used for comments
	INIT_LIPS,                                      // 38 - Position lips object for supplied object
	INIT_STORY_MODE,                                // 39 - Set story mode TRUE/FALSE (user can't type)
	WARN,                                           // 40 - Same as TEXT but can't dismiss box by typing
	COND_BONUS,                                     // 41 - Conditional on bonus having been scored
	TEXT_TAKE,                                      // 42 - Issue text box with "take" info string
	YESNO,                                          // 43 - Prompt user for Yes or No
	STOP_ROUTE,                                     // 44 - Skip any route in progress (hero still walks)
	COND_ROUTE,                                     // 45 - Conditional on route in progress
	INIT_JUMPEXIT,                                  // 46 - Initialize status.jumpexit
	INIT_VIEW,                                      // 47 - Initialize viewx, viewy, dir
	INIT_OBJ_FRAME,                                 // 48 - Object number, seq,frame to set curr_seq_p to
	OLD_SONG           = 49                         // Added by Strangerke - Set currently playing sound, old way: that is, using a string index instead of a reference in a file
};

struct hugo_boot_t {                                // Common HUGO boot file
	char checksum;                                  // Checksum for boot structure (not exit text)
	char registered;                                // TRUE if registered version, else FALSE
	char pbswitch[8];                               // Playback switch string
	char distrib[32];                               // Distributor branding string
	uint16 exit_len;                                // Length of exit text (next in file)
};

struct uif_hdr_t {                                  // UIF font/image look up
	uint16  size;                                   // Size of uif item
	uint32  offset;                                 // Offset of item in file
};

/**
* Game specific type definitions
*/
typedef byte *image_pt;                             // ptr to an object image (sprite)
typedef byte *sound_pt;                             // ptr to sound (or music) data

/**
* Following are points for achieving certain actions.
*/
struct point_t {
	byte score;                                     // The value of the point
	bool scoredFl;                                  // Whether scored yet
};

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
* The following determines how a verb is acted on, for an object
*/
struct cmd {
	uint16 verbIndex;                               // the verb
	uint16 reqIndex;                                // ptr to list of required objects
	uint16 textDataNoCarryIndex;                    // ptr to string if any of above not carried
	byte   reqState;                                // required state for verb to be done
	byte   newState;                                // new states if verb done
	uint16 textDataWrongIndex;                      // ptr to string if wrong state
	uint16 textDataDoneIndex;                       // ptr to string if verb done
	uint16 actIndex;                                // Ptr to action list if verb done
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

/**
* Following is structure of verbs and nouns for 'background' objects
* These are objects that appear in the various screens, but nothing
* interesting ever happens with them.  Rather than just be dumb and say
* "don't understand" we produce an interesting msg to keep user sane.
*/
struct background_t {
	uint16 verbIndex;
	uint16 nounIndex;
	int    commentIndex;                            // Index of comment produced on match
	bool   matchFl;                                 // TRUE if noun must match when present
	byte   roomState;                               // "State" of room. Comments might differ.
	byte   bonusIndex;                              // Index of bonus score (0 = no bonus)
};

typedef background_t *objectList_t;

struct target_t {                                   // Secondary target for action
	uint16 nounIndex;                               // Secondary object
	uint16 verbIndex;                               // Action on secondary object
};

struct uses_t {                                     // Define uses of certain objects
	int16     objId;                                // Primary object
	uint16    dataIndex;                            // String if no secondary object matches
	target_t *targets;                              // List of secondary targets
};

// Global externs
extern maze_t      _maze;                           // Maze control structure
extern hugo_boot_t _boot;                           // Boot info structure

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
