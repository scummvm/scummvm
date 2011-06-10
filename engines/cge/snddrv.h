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

// ******************************************************
// *  Sound Driver by Hedges (c) 1995 LK AVALON         *
// *  Ver 1.00: 01-Mar-95                               *
// *  Ver 1.10: 03-Mar-95                               *
// *  Ver 1.20: 07-Mar-95                               *
// *  Ver 1.30: 09-Mar-95                               *
// *  Ver 1.40: 11-Mar-95                               *
// ******************************************************

#ifndef __SNDDRV__
#define __SNDDRV__

// ******************************************************
// *  Constants                                         *
// ******************************************************
// available devices

enum  DEV_TYPE  { DEV_AUTO = -1,         // auto-detect mode
                  DEV_QUIET,             // disable sound
                  DEV_SB,                // sb/pro/16/awe32
		  DEV_GUS,               // gus/max
		  DEV_GM                 // general midi
                };

#define                SERR_OK           0        // no error
#define                SERR_INITFAIL     1        // couldn't initialize
#define                SERR_BADDDEV      128      // bad device

// driver info
struct DRVINFO
{
  DEV_TYPE            DDEV;              // digi device
  DEV_TYPE            MDEV;              // midi device
  WORD                DBASE;             // digi base port
  WORD                DDMA;              // digi dma no
  WORD                DIRQ;              // digi irq no
  WORD                MBASE;             // midi base port
  union
    {
      struct
	{
	  WORD        DR : 4;
	  WORD        DL : 4;
	  WORD        MR : 4;
	  WORD        ML : 4;
	} VOL4;
      struct
	{
	  BYTE        D;                 // digi volume
	  BYTE        M;                 // midi volume
	} VOL2;
    };
};

// sample info
struct SMPINFO
{
  BYTE *          saddr;             // address
  WORD                slen;              // length
  WORD                span;              // left/right pan (0-15)
  int                 sflag;             // flag
};

// ******************************************************
// *  Data                                              *
// ******************************************************
// driver info
extern  DRVINFO      SNDDrvInfo;

// midi player flag (1 means we are playing)
extern  WORD         MIDIPlayFlag;

// midi song end flag (1 means we have crossed end mark)
extern  WORD         MIDIEndFlag;

// ******************************************************
// *  Driver Code                                       *
// ******************************************************
// Init Digi Device
EC void SNDInit        (void);

// Close Digi Device
EC void SNDDone        (void);

// Set Volume
EC void SNDSetVolume   (void);

// Start Digi
EC void SNDDigiStart   (SMPINFO *PSmpInfo);

// Stop Digi
EC void SNDDigiStop    (SMPINFO *PSmpInfo);

// Start MIDI File
EC void SNDMIDIStart   (BYTE *MIDFile);

// Stop MIDI File
EC void SNDMIDIStop    (void);

// Play MIDI File (to be called while interrupting)
// WARNING: Uses ALL registers!
EC void SNDMIDIPlay    (void);

#endif
