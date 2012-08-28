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
 *
 */
/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef __MPALDLL_H
#define __MPALDLL_H

#include "common/file.h"
#include "tony/mpal/memory.h"
#include "tony/mpal/loadmpc.h"
#include "tony/mpal/expr.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Defines
\****************************************************************************/

#define HEX_VERSION             0x0170


#define MAX_ACTIONS_PER_ITEM    40
#define MAX_COMMANDS_PER_ITEM   128
#define MAX_COMMANDS_PER_ACTION 128
#define MAX_DESCRIBE_SIZE		64


#define MAX_MOMENTS_PER_SCRIPT  256
#define MAX_COMMANDS_PER_SCRIPT 256
#define MAX_COMMANDS_PER_MOMENT 32


#define MAX_GROUPS_PER_DIALOG   128
#define MAX_COMMANDS_PER_DIALOG 480
#define MAX_COMMANDS_PER_GROUP  64
#define MAX_CHOICES_PER_DIALOG  64
#define MAX_SELECTS_PER_CHOICE  64
#define MAX_PLAYGROUPS_PER_SELECT 9
#define MAX_PERIODS_PER_DIALOG  400


#define NEED_LOCK_MSGS


/****************************************************************************\
*       Structures
\****************************************************************************/

#include "common/pack-start.h"

/**
 * MPAL global variables
 */
struct MPALVAR {
	uint32 dwVal;				// Variable value
	char lpszVarName[33];		// Variable name
} PACKED_STRUCT;
typedef MPALVAR *LPMPALVAR;
typedef LPMPALVAR *LPLPMPALVAR;


/**
 * MPAL Messages
 */
struct MPALMSG {
  HGLOBAL _hText;				// Handle to the message text
  uint16 _wNum;					// Message number
} PACKED_STRUCT;
typedef MPALMSG *LPMPALMSG;
typedef LPMPALMSG *LPLPMPALMSG;


/**
 * MPAL Locations
 */
struct MPALLOCATION {
	uint32 nObj;				// Location number
	uint32 dwXlen, dwYlen;		// Dimensions
	uint32 dwPicRes;			// Resource that contains the image
} PACKED_STRUCT;
typedef MPALLOCATION *LPMPALLOCATION;
typedef LPMPALLOCATION *LPLPMPALLOCATION;


/**
 * All the data for a command, ie. tags used by OnAction in the item, the time
 * in the script, and in the group dialog.
 */
struct command {
  /*
   * Types of commands that are recognized
   *
   *   #1 -> Custom function call		(ITEM, SCRIPT, DIALOG)
   *   #2 -> Variable assignment		(ITEM, SCRIPT, DIALOG)
   *   #3 -> Making a choice			(DIALOG)
   *
   */
	byte  type;						// Type of control

	union {
		int32 _nCf;                 // Custom function call			[#1]
		char *lpszVarName;			// Variable name				[#2]
		int32 nChoice;              // Number of choice you make	[#3]
	};

	union {
		int32 _arg1;                // Argument for custom function			[#1]
		HGLOBAL expr;				// Expression to assign to a variable	[#2]
	};

	int32 _arg2, _arg3, _arg4;		// Arguments for custom function		[#1]
} PACKED_STRUCT;


/**
 * MPAL dialog
 */
struct MPALDIALOG {
	uint32 nObj;                    // Dialog number

	struct command _command[MAX_COMMANDS_PER_DIALOG];

	struct {
		uint16 num;

		byte nCmds;
		uint16 CmdNum[MAX_COMMANDS_PER_GROUP];

	} _group[MAX_GROUPS_PER_DIALOG];

	struct {
		// The last choice has nChoice == 0
		uint16 nChoice;

		// The select number (we're pretty stingy with RAM). The last select has dwData == 0
		struct {
			HGLOBAL when;
			uint32 dwData;
			uint16 wPlayGroup[MAX_PLAYGROUPS_PER_SELECT];

			// Bit 0=endchoice   Bit 1=enddialog
			byte attr;

			// Modified at run-time: 0 if the select is currently disabled,
			// and 1 if currently active
			byte curActive;
		} _select[MAX_SELECTS_PER_CHOICE];

	} _choice[MAX_CHOICES_PER_DIALOG];

	uint16 _periodNums[MAX_PERIODS_PER_DIALOG];
	HGLOBAL _periods[MAX_PERIODS_PER_DIALOG];

} PACKED_STRUCT;
typedef MPALDIALOG *LPMPALDIALOG;
typedef LPMPALDIALOG *LPLPMPALDIALOG;


/**
 * MPAL Item
 */
struct ItemAction {
	byte	num;                // Action number
	uint16	wTime;              // If idle, the time which must pass
    byte	perc;               // Percentage of the idle run
    HGLOBAL	when;               // Expression to compute. If != 0, then
								// action can be done
    uint16	wParm;              // Parameter for action

    byte	nCmds;				// Number of commands to be executed
    uint32	CmdNum[MAX_COMMANDS_PER_ACTION]; // Commands to execute
} PACKED_STRUCT;

struct MPALITEM {
	uint32 nObj;				// Item number

	byte lpszDescribe[MAX_DESCRIBE_SIZE]; // Name
	byte nActions;				// Number of managed actions
	uint32 dwRes;				// Resource that contains frames and patterns

	struct command _command[MAX_COMMANDS_PER_ITEM];

	// Pointer to array of structures containing various managed activities. In practice, of
	// every action we know what commands to run, including those defined in structures above
	struct ItemAction *Action;

} PACKED_STRUCT;
typedef MPALITEM *LPMPALITEM;
typedef LPMPALITEM *LPLPMPALITEM;


/**
 * MPAL Script
 */
struct MPALSCRIPT {
	uint32 nObj;

	uint32 nMoments;

	struct command _command[MAX_COMMANDS_PER_SCRIPT];

	struct {
		int32 dwTime;

		byte nCmds;
		uint32 CmdNum[MAX_COMMANDS_PER_MOMENT];

	} Moment[MAX_MOMENTS_PER_SCRIPT];

} PACKED_STRUCT;
typedef MPALSCRIPT   *LPMPALSCRIPT;
typedef LPMPALSCRIPT *LPLPMPALSCRIPT;

#include "common/pack-end.h"


/****************************************************************************\
*       Function prototypes
\****************************************************************************/

/**
 * Returns the current value of a global variable
 *
 * @param lpszVarName		Name of the variable
 * @returns		Current value
 * @remarks		Before using this method, you must call LockVar() to
 * lock the global variablves for use. Then afterwards, you will
 * need to remember to call UnlockVar()
 */
extern int32 varGetValue(const char *lpszVarName);


/**
 * Sets the value of a MPAL global variable
 * @param lpszVarName       Name of the variable
 * @param val				Value to set
 */
extern void varSetValue(const char *lpszVarName, int32 val);

} // end of namespace MPAL

} // end of namespace Tony

#endif

