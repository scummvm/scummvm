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

// WARNING!!  Run the program at least once before release to
// generate the initial save file!  (Using the -i cmd switch)
// Set EPISODE_NUM & build.  Build pictures.mak and run "Tools/Hugo N".
// Copy helpedit\hugow_?.hlp to .\hugowin?.hlp
// Type "PPG" in the game to enter cheat mode.

#define DEBUG       false                           // Allow me to do special things (EDIT, GOTO)
#define STORY       true                            // Skip any intro stuff if FALSE
//#define DATABASE    TRUE                          // Use database instead of individual files. Individual files are used by Hugo1 DOS!
//#define BETA        FALSE                         // Puts big msg in intro/about box
#define EPISODE_NUM         1                       // Episode we are building
#define TITLE       "Hugo for Windows"
#define COPYRIGHT   "Copyright © 1995-97, David P. Gray"
// Started code on 04/01/95
// Don't forget to update Hugowin.rc2 with version info
//#define VER "1.0" // 10/01/95 Initial Release
//#define VER "1.1" // 10/06/95 Restore system volume levels on exit
//#define VER "v1.2"// 10/12/95 Added "background music" checkbox in volume dlg
//#define VER "v1.3"// 10/23/95 Support game 1 as shareware
//#define VER "v1.4"// 12/06/95 Faster graphics, logical palette
#define VER "v1.5"  // 10/07/97 Added order form, new web site

// Game specific equates
#define MAX_TUNES      16                           // Max number of tunes
#define NORMAL_TPS     9                            // Number of ticks (frames) per second
#define TURBO_TPS      16                           // This many in turbo mode
#define DX             5                            // Num pixels moved in x by HERO per step
#define DY             4                            // Num pixels moved in y by HERO per step
#define XBYTES         40                           // number of bytes in a compressed line
#define XPIX           320                          // Width of pcx background file
#define YPIX           200                          // Height of pcx background file
#define VIEW_DX        XPIX                         // Width of window view
#define VIEW_DY        184                          // Height of window view
#define INV_DX         32                           // Width of an inventory icon
#define INV_DY         32                           // Height of inventory icon
#define DIBOFF_Y       8                            // Offset into dib SrcY (old status line area)
#define OVL_SIZE       (XBYTES * YPIX)              // Size of an overlay file
#define MAX_SEQUENCES  4                            // Number of sequences of images in object
#define MAX_CHARS      (XBYTES - 2)                 // Max length of user input line
#define NUM_ROWS       25                           // Number of text lines in display
#define MAX_BOX        (MAX_CHARS * NUM_ROWS)       // Max chars on screen
#define DONT_CARE      0xFF                         // Any state allowed in command verb
#define MAX_FPATH      256                          // Max length of a full path name
#define HERO_MAX_WIDTH 24                           // Maximum width of hero
#define HERO_MIN_WIDTH 16                           // Minimum width of hero
#define LOOK_NAME      1                            // Index of name used in showing takeables
#define TAKE_NAME      2                            // Index of name used in confirming take
#define TAKE_TEXT      "Picked up the %s ok."
#define REP_MASK       0xC0                         // Top 2 bits mean a repeat code
#define MAX_STRLEN     1024
#define WARNLEN        512
#define ERRLEN         512
#define STEP_DY        8                            // Pixels per step movement

// Only for non-database
#define BKGEXT         ".PCX"                       // Extension of background files
#define OBJEXT         ".PIX"                       // Extension of object picture files
#define NAME_LEN       12                           // Max length of a DOS file name

// Definitions of 'generic' commands: Max # depends on size of gencmd in
//   the object_t record since each requires 1 bit.  Currently up to 16
#define LOOK           1
#define TAKE           2
#define DROP           4
#define LOOK_S         8                            // Description depends on state of object

// Macros:
#define TPS           ((_config.turboFl) ? TURBO_TPS : NORMAL_TPS)


enum TEXTCOLORS {
	_TBLACK,    _TBLUE,         _TGREEN,       _TCYAN,
	_TRED,      _TMAGENTA,      _TBROWN,       _TWHITE,
	_TGRAY,     _TLIGHTBLUE,    _TLIGHTGREEN,  _TLIGHTCYAN,
	_TLIGHTRED, _TLIGHTMAGENTA, _TLIGHTYELLOW, _TBRIGHTWHITE
};

#define CRYPT "Copyright 1992, David P Gray, Gray Design Associates"

struct hugo_boot_t {                                // Common HUGO boot file
	char checksum;                                  // Checksum for boot structure (not exit text)
	char registered;                                // TRUE if registered version, else FALSE
	char pbswitch[8];                               // Playback switch string
	char distrib[32];                               // Distributor branding string
	uint16 exit_len;                                // Length of exit text (next in file)
};

#define MAX_UIFS   32                               // Max possible uif items in hdr
#define NUM_FONTS  3                                // Number of dib fonts
#define FIRST_FONT U_FONT5
#define FONT_LEN   128                              // Number of chars in font
#define FONTSIZE   1200                             // Max size of font data

struct uif_hdr_t {                                  // UIF font/image look up
	uint16  size;                                   // Size of uif item
	uint32  offset;                                 // Offset of item in file
};

enum uif_t {U_FONT5, U_FONT6, U_FONT8, UIF_IMAGES, NUM_UIF_ITEMS};

// Game specific type definitions
typedef byte *image_pt;                             // ptr to an object image (sprite)
typedef byte *sound_pt;                             // ptr to sound (or music) data

// Enumerate overlay file types
enum ovl_t {BOUNDARY, OVERLAY, OVLBASE};

// Enumerate error types
enum {
	GEN_ERR,   FILE_ERR, WRITE_ERR, PCCH_ERR, HEAP_ERR, EVNT_ERR,  SOUND_ERR
	//MOUSE_ERR, VID_ERR,  FONT_ERR,  ARG_ERR,  CHK_ERR,  TIMER_ERR, VBX_ERR
};

// Enumerate ways of cycling a sequence of frames
enum cycle_t {INVISIBLE, ALMOST_INVISIBLE, NOT_CYCLING, CYCLE_FORWARD, CYCLE_BACKWARD};

// Enumerate sequence index matching direction of travel
enum {RIGHT, LEFT, DOWN, _UP};

// Channel requirement during sound effect
enum stereo_t {BOTH_CHANNELS, RIGHT_CHANNEL, LEFT_CHANNEL};

// Priority for sound effect
enum priority_t {LOW_PRI, MED_PRI, HIGH_PRI};

// Enumerate the different path types for an object
enum path_t {
	USER,                                           // User has control of object via cursor keys
	AUTO,                                           // Computer has control, controlled by action lists
	QUIET,                                          // Computer has control and no commands allowed
	CHASE,                                          // Computer has control, object is chasing hero
	CHASE2,                                         // Same as CHASE, except keeps cycling when stationary
	WANDER,                                         // Computer has control, object is wandering randomly
	WANDER2                                         // Same as WANDER, except keeps cycling when stationary
};

// Enumerate whether object is foreground, background or 'floating'
// If floating, HERO can collide with it and fore/back ground is determined
// by relative y-coord of object base.  This is the general case.
// If fore or background, no collisions can take place and object is either
// behind or in front of all others, although can still be hidden by the
// the overlay plane.  OVEROVL means the object is FLOATING (to other
// objects) but is never hidden by the overlay plane
enum {FOREGROUND, BACKGROUND, FLOATING, OVEROVL};

// Game view state machine
enum vstate_t {V_IDLE, V_INTROINIT, V_INTRO, V_PLAY, V_INVENT, V_EXIT};

enum font_t {LARGE_ROMAN, MED_ROMAN, NUM_GDI_FONTS, INIT_FONTS, DEL_FONTS};

// Ways to dismiss a text/prompt box
enum box_t {BOX_ANY, BOX_OK, BOX_PROMPT, BOX_YESNO};

// Standard viewport sizes
enum wsize_t {SIZE_DEF, SIZE_1, SIZE_2, SIZE_3};

// Display list functions
enum dupdate_t {D_INIT, D_ADD, D_DISPLAY, D_RESTORE};

// General device installation commands
enum inst_t {INSTALL, RESTORE, RESET};

// Inventory icon bar states
enum istate_t {I_OFF, I_UP, I_DOWN, I_ACTIVE};

// Actions for Process_inventory()
enum invact_t {INV_INIT, INV_LEFT, INV_RIGHT, INV_GET};

// Purpose of an automatic route
enum go_t {GO_SPACE, GO_EXIT, GO_LOOK, GO_GET};

// Following are points for achieving certain actions.
struct point_t {
	byte score;                                     // The value of the point
	bool scoredFl;                                  // Whether scored yet
};

// Structure for initializing maze processing
struct maze_t {
	bool enabledFl;                                 // TRUE when maze processing enabled
	byte size;                                      // Size of (square) maze matrix
	int  x1, y1, x2, y2;                            // maze hotspot bounding box
	int  x3, x4;                                    // north, south x entry coordinates
	byte firstScreenIndex;                          // index of first screen in maze
};

// Following defines the action types and action list
enum action_t {                                     // Parameters:
	ANULL              = 0xff,                      // Special NOP used to 'delete' events in DEL_EVENTS
	ASCHEDULE          = 0,                         //  0 - Ptr to action list to be rescheduled
	START_OBJ,                                      //  1 - Object number
	INIT_OBJXY,                                     //  2 - Object number, x,y
	PROMPT,                                         //  3 - index of prompt & response string, ptrs to action
	// lists.  First if response matches, 2nd if not.
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


struct act0 {                                       // Type 0 - Schedule
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   actIndex;                              // Ptr to an action list
};

struct act1 {                                       // Type 1 - Start an object
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      cycleNumb;                             // Number of times to cycle
	cycle_t  cycle;                                 // Direction to start cycling
};

struct act2 {                                       // Type 2 - Initialise an object coords
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      x, y;                                  // Coordinates
};

struct act3 {                                       // Type 3 - Prompt user for text
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   promptIndex;                           // Index of prompt string
	int     *responsePtr;                           // Array of indexes to valid response
	// string(s) (terminate list with -1)
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
	bool     encodedFl;                             // (HUGO 1 DOS ONLY) Whether response is encoded or not
};

struct act4 {                                       // Type 4 - Set new background color
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	long     newBackgroundColor;                    // New color
};

struct act5 {                                       // Type 5 - Initialise an object velocity
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      vx, vy;                                // velocity
};

struct act6 {                                       // Type 6 - Initialise an object carrying
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	bool     carriedFl;                             // carrying
};

struct act7 {                                       // Type 7 - Initialise an object to hero's coords
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
};

struct act8 {                                       // Type 8 - switch to new screen
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      screenIndex;                           // The new screen number
};

struct act9 {                                       // Type 9 - Initialise an object state
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	byte     newState;                              // New state
};

struct act10 {                                      // Type 10 - Initialise an object path type
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      newPathType;                           // New path type
	int8     vxPath, vyPath;                        // Max delta velocities e.g. for CHASE
};

struct act11 {                                      // Type 11 - Conditional on object's state
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	byte     stateReq;                              // Required state
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
};

struct act12 {                                      // Type 12 - Simple text box
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      stringIndex;                           // Index (enum) of string in strings.dat
};

struct act13 {                                      // Type 13 - Swap first object image with second
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      obj1;                                  // Index of first object
	int      obj2;                                  // 2nd
};

struct act14 {                                      // Type 14 - Conditional on current screen
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The required object
	int      screenReq;                             // The required screen number
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
};

struct act15 {                                      // Type 15 - Home in on an object
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      obj1;                                  // The object number homing in
	int      obj2;                                  // The object number to home in on
	int8     dx, dy;                                // Max delta velocities
};
// Note: Don't set a sequence at time 0 of a new screen, it causes
// problems clearing the boundary bits of the object!  timer > 0 is safe
struct act16 {                                      // Type 16 - Set curr_seq_p to seq
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      seqIndex;                              // The index of seq array to set to
};

struct act17 {                                      // Type 17 - SET obj individual state bits
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      stateMask;                             // The mask to OR with current obj state
};

struct act18 {                                      // Type 18 - CLEAR obj individual state bits
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      stateMask;                             // The mask to ~AND with current obj state
};

struct act19 {                                      // Type 19 - TEST obj individual state bits
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      stateMask;                             // The mask to AND with current obj state
	uint16   actPassIndex;                          // Ptr to action list (all bits set)
	uint16   actFailIndex;                          // Ptr to action list (not all set)
};

struct act20 {                                      // Type 20 - Remove all events with this type of action
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	action_t actTypeDel;                            // The action type to remove
};

struct act21 {                                      // Type 21 - Gameover.  Disable hero & commands
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act22 {                                      // Type 22 - Initialise an object to hero's coords
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
};

struct act23 {                                      // Type 23 - Exit game back to DOS
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act24 {                                      // Type 24 - Get bonus score
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      pointIndex;                            // Index into points array
};

struct act25 {                                      // Type 25 - Conditional on bounding box
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The required object number
	int      x1, y1, x2, y2;                        // The bounding box
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
};

struct act26 {                                      // Type 26 - Play a sound
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int16    soundIndex;                            // Sound index in data file
};

struct act27 {                                      // Type 27 - Add object's value to score
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // object number
};

struct act28 {                                      // Type 28 - Subtract object's value from score
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // object number
};

struct act29 {                                      // Type 29 - Conditional on object carried
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The required object number
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
};

struct act30 {                                      // Type 30 - Start special maze processing
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	byte     mazeSize;                              // Size of (square) maze
	int      x1, y1, x2, y2;                        // Bounding box of maze
	int      x3, x4;                                // Extra x points for perspective correction
	byte     firstScreenIndex;                      // First (top left) screen of maze
};

struct act31 {                                      // Type 31 - Exit special maze processing
	action_t actType;                                   // The type of action
	int      timer;                                 // Time to set off the action
};

struct act32 {                                      // Type 32 - Init fbg field of object
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	byte     priority;                              // Value of foreground/background field
};

struct act33 {                                      // Type 33 - Init screen field of object
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      screenIndex;                           // Screen number
};

struct act34 {                                      // Type 34 - Global Schedule
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   actIndex;                              // Ptr to an action list
};

struct act35 {                                      // Type 35 - Remappe palette
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int16    oldColorIndex;                         // Old color index, 0..15
	int16    newColorIndex;                         // New color index, 0..15
};

struct act36 {                                      // Type 36 - Conditional on noun mentioned
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   nounIndex;                             // The required noun (list)
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
};

struct act37 {                                      // Type 37 - Set new screen state
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      screenIndex;                           // The screen number
	byte     newState;                              // The new state
};

struct act38 {                                      // Type 38 - Position lips
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      lipsObjNumb;                           // The LIPS object
	int      objNumb;                               // The object to speak
	byte     dxLips;                                // Relative offset of x
	byte     dyLips;                                // Relative offset of y
};

struct act39 {                                      // Type 39 - Init story mode
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	bool     storyModeFl;                           // New state of story_mode flag
};

struct act40 {                                      // Type 40 - Unsolicited text box
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      stringIndex;                           // Index (enum) of string in strings.dat
};

struct act41 {                                      // Type 41 - Conditional on bonus scored
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      BonusIndex;                            // Index into bonus list
	uint16   actPassIndex;                          // Index of the action list if scored for the first time
	uint16   actFailIndex;                          // Index of the action list if already scored
};

struct act42 {                                      // Type 42 - Text box with "take" string
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object taken
};

struct act43 {                                      // Type 43 - Prompt user for Yes or No
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      promptIndex;                           // index of prompt string
	uint16   actYesIndex;                           // Ptr to action list if YES
	uint16   actNoIndex;                            // Ptr to action list if NO
};

struct act44 {                                      // Type 44 - Stop any route in progress
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act45 {                                      // Type 45 - Conditional on route in progress
	action_t   actType;                             // The type of action
	int        timer;                               // Time to set off the action
	int        routeIndex;                          // Must be >= current status.rindex
	uint16     actPassIndex;                        // Ptr to action list if en-route
	uint16     actFailIndex;                        // Ptr to action list if not
};

struct act46 {                                      // Type 46 - Init status.jumpexit
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	bool     jumpExitFl;                            // New state of jumpexit flag
};

struct act47 {                                      // Type 47 - Init viewx,viewy,dir
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object
	int16    viewx;                                 // object.viewx
	int16    viewy;                                 // object.viewy
	int16    direction;                             // object.dir
};

struct act48 {                                      // Type 48 - Set curr_seq_p to frame n
	action_t actType;                                   // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      seqIndex;                              // The index of seq array to set to
	int      frameIndex;                            // The index of frame to set to
};

struct act49 {                                      // Added by Strangerke - Type 79 - Play a sound (DOS way)
	action_t actType;                                   // The type of action
	int      timer;                                 // Time to set off the action
	uint16   soundIndex;                            // Sound index in string array
};

union act {
	act0     a0;
	act1     a1;
	act2     a2;
	act3     a3;
	act4     a4;
	act5     a5;
	act6     a6;
	act7     a7;
	act8     a8;
	act9     a9;
	act10    a10;
	act11    a11;
	act12    a12;
	act13    a13;
	act14    a14;
	act15    a15;
	act16    a16;
	act17    a17;
	act18    a18;
	act19    a19;
	act20    a20;
	act21    a21;
	act22    a22;
	act23    a23;
	act24    a24;
	act25    a25;
	act26    a26;
	act27    a27;
	act28    a28;
	act29    a29;
	act30    a30;
	act31    a31;
	act32    a32;
	act33    a33;
	act34    a34;
	act35    a35;
	act36    a36;
	act37    a37;
	act38    a38;
	act39    a39;
	act40    a40;
	act41    a41;
	act42    a42;
	act43    a43;
	act44    a44;
	act45    a45;
	act46    a46;
	act47    a47;
	act48    a48;
	act49    a49;
};

// The following determines how a verb is acted on, for an object
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

// The following is a linked list of images in an animation sequence
// The image data is in 8-bit DIB format, i.e. 1 byte = 1 pixel
struct seq_t {                                      // Linked list of images
	byte   *imagePtr;                               // ptr to image
	uint16  bytesPerLine8;                          // bytes per line (8bits)
	uint16  lines;                                  // lines
	uint16  x1, x2, y1, y2;                         // Offsets from x,y: data bounding box
	seq_t  *nextSeqPtr;                             // ptr to next record
};

// The following is an array of structures of above sequences
struct seqList_t {
	uint16 imageNbr;                                // Number of images in sequence
	seq_t *seqPtr;                                  // Ptr to sequence structure
};

// Following is definition of object attributes
struct object_t {
	uint16     nounIndex;                           // String identifying object
	uint16     dataIndex;                           // String describing the object
	uint16     *stateDataIndex;                     // Added by Strangerke to handle the LOOK_S state-dependant descriptions
	path_t     pathType;                            // Describe path object follows
	int        vxPath, vyPath;                      // Delta velocities (e.g. for CHASE)
	uint16     actIndex;                            // Action list to do on collision with hero
	byte       seqNumb;                             // Number of sequences in list
	seq_t     *currImagePtr;                        // Sequence image currently in use
	seqList_t  seqList[MAX_SEQUENCES];              // Array of sequence structure ptrs and lengths
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

// Following is structure of verbs and nouns for 'background' objects
//   These are objects that appear in the various screens, but nothing
//   interesting ever happens with them.  Rather than just be dumb and say
//   "don't understand" we produce an interesting msg to keep user sane.
struct background_t {
	uint16 verbIndex;
	uint16 nounIndex;
	int    commentIndex;                            // Index of comment produced on match
	bool   matchFl;                                 // TRUE if noun must match when present
	byte   roomState;                               // "State" of room. Comments might differ.
	byte   bonusIndex;                              // Index of bonus score (0 = no bonus)
};

typedef background_t *objectList_t;

typedef byte overlay_t[OVL_SIZE];                   // Overlay file
typedef byte viewdib_t[(long)XPIX *YPIX];           // Viewport dib
typedef byte icondib_t[XPIX *INV_DY];               // Icon bar dib

typedef char command_t[MAX_CHARS + 8];              // Command line (+spare for prompt,cursor)
typedef char fpath_t[MAX_FPATH];                    // File path

// Structure to define an EXIT or other collision-activated hotspot
struct hotspot_t {
	int        screenIndex;                         // Screen in which hotspot appears
	int        x1, y1, x2, y2;                      // Bounding box of hotspot
	uint16     actIndex;                            // Actions to carry out if a 'hit'
	int16      viewx, viewy, direction;             // Used in auto-route mode
};

struct status_t {                                   // Game status (not saved)
	bool     initSaveFl;                            // Force save of initial game
	bool     storyModeFl;                           // Game is telling story - no commands
	bool     gameOverFl;                            // Game is over - hero knobbled
	bool     playbackFl;                            // Game is in playback mode
	bool     recordFl;                              // Game is in record mode
	bool     demoFl;                                // Game is in demo mode
	bool     debugFl;                               // Game is in debug mode
	bool     textBoxFl;                             // Game is (halted) in text box
//	Strangerke - Not used ?
//	bool     mmtimeFl;                               // Multimedia timer supported
	bool     lookFl;                                // Toolbar "look" button pressed
	bool     recallFl;                              // Toolbar "recall" button pressed
	bool     leftButtonFl;                          // Left mouse button pressed
	bool     rightButtonFl;                         // Right button pressed
	bool     newScreenFl;                           // New screen just loaded in dib_a
	bool     jumpExitFl;                            // Allowed to jump to a screen exit
	bool     godModeFl;                             // Allow DEBUG features in live version
	bool     helpFl;                                // Calling WinHelp (don't disable music)
	uint32   tick;                                  // Current time in ticks
	uint32   saveTick;                              // Time of last save in ticks
	vstate_t viewState;                             // View state machine
	istate_t inventoryState;                        // Inventory icon bar state
	int16    inventoryHeight;                       // Inventory icon bar height
	int16    inventoryObjId;                        // Inventory object selected, or -1
	int16    routeIndex;                            // Index into route list, or -1
	go_t     go_for;                                // Purpose of an automatic route
	int16    go_id;                                 // Index of exit of object walking to
	fpath_t  path;                                  // Alternate path for saved files
	long     saveSize;                              // Size of a saved game
	int16    saveSlot;                              // Current slot to save/restore game
	int16    screenWidth;                           // Desktop screen width
	int16    song;                                  // Current song
	int16    cx, cy;                                // Cursor position (dib coords)
};

struct config_t {                                   // User's config (saved)
	bool musicFl;                                   // State of Music button/menu item
	bool soundFl;                                   // State of Sound button/menu item
	bool turboFl;                                   // State of Turbo button/menu item
//	int16 wx, wy;                                    // Position of viewport
	int16 cx, cy;                                   // Size of viewport
	bool  backgroundMusicFl;                        // Continue music when task inactive
	byte  musicVolume;                              // Music volume percentage
	byte  soundVolume;                              // Sound volume percentage
	bool  playlist[MAX_TUNES];                      // Tune playlist
};

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
extern config_t    _config;                         // User's config
extern maze_t      _maze;                           // Maze control structure
extern hugo_boot_t _boot;                           // Boot info structure
extern char        _textBoxBuffer[];                // Useful box text buffer
extern command_t   _line;                           // Line of user text input

/* Structure of scenery file lookup entry */
struct sceneBlock_t {
	uint32 scene_off;
	uint32 scene_len;
	uint32 b_off;
	uint32 b_len;
	uint32 o_off;
	uint32 o_len;
	uint32 ob_off;
	uint32 ob_len;
};

/* Structure of object file lookup entry */
struct objBlock_t {
	uint32 objOffset;
	uint32 objLength;
};

#include "common/pack-start.h"						// START STRUCT PACKING
struct sound_hdr_t {								// Sound file lookup entry
	uint16 size;									// Size of sound data in bytes
	uint32 offset;									// Offset of sound data in file
} PACKED_STRUCT;
#include "common/pack-end.h"	// END STRUCT PACKING

}

#endif
