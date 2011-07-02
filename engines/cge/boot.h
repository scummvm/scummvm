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

#ifndef __CGE_BOOT__
#define __CGE_BOOT__

#include "cge/jbw.h"

namespace CGE {

#define BOOTSECT_SIZ  512
#define BOOTHEAD_SIZ  62
#define BOOTCODE_SIZ  BOOTSECT_SIZ-BOOTHEAD_SIZ
#define FreeBoot(b)   free(b)

struct Boot {
	uint8  _jmp[3];                                    // NEAR jump machine code
	char   _idOEM[8];                                  // OEM name and version
	uint16 _sectSize;                                  // bytes per sector
	uint8  _clustSize;                                 // sectors per cluster
	uint16 _resSecs;                                   // sectors before 1st FAT
	uint8  _fatCnt;                                    // number of FATs
	uint16 _rootSize;                                  // root directory entries
	uint16 _totSecs;                                   // total sectors on disk
	uint8  _media;                                     // media descriptor byte
	uint16 _fatSize;                                   // sectors per FAT
	uint16 _trkSecs;                                   // sectors per track
	uint16 _headCnt;                                   // number of sufraces
	uint16 _hidnSecs;                                  // special hidden sectors
	uint16 __;                                         // (unknown: reserved?)
	uint32 _lTotSecs;                                  // total number of sectors
	uint16 _driveNum;                                  // physical drive number
	uint8  _xSign;                                     // extended boot signature
	uint32 _serial;                                    // volume serial number
	char   _label[11];                                 // volume label
	char   _fileSysID[8];                              // file system ID
	char   _code[BOOTCODE_SIZ - 8];                    // 8 = length of following
	uint32 _secret;                                    // long secret number
	uint8  _bootCheck;                                 // boot sector checksum
	uint8  _bootFlags;                                 // secret flags
	uint16 _bootSig;                                   // boot signature 0xAA55
};


Boot  *readBoot(int drive);
uint8  checkBoot(Boot *boot);
bool   writeBoot(int drive, Boot *boot);

} // End of namespace CGE

#endif
