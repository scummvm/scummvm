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

#ifndef		__BOOT__
#define		__BOOT__

#include	"cge/jbw.h"

#define		BOOTSECT_SIZ	512
#define		BOOTHEAD_SIZ	62
#define		BOOTCODE_SIZ	BOOTSECT_SIZ-BOOTHEAD_SIZ
#define		FreeBoot(b)	free(b)

#ifndef		EC
  #define	EC
#endif

typedef struct {
		 uint8	Jmp[3];			// NEAR jump machine code
		 char	OEM_ID[8];		// OEM name and version
		 uint16	SectSize;		// bytes per sector
		 uint8	ClustSize;		// sectors per cluster
		 uint16	ResSecs;		// sectors before 1st FAT
		 uint8	FatCnt;			// number of FATs
		 uint16	RootSize;		// root directory entries
		 uint16	TotSecs;		// total sectors on disk
		 uint8	Media;			// media descriptor byte
		 uint16	FatSize;		// sectors per FAT
		 uint16	TrkSecs;		// sectors per track
		 uint16	HeadCnt;		// number of sufraces
		 uint16	HidnSecs;		// special hidden sectors
		 uint16	_;			//  (unknown: reserved?)
		 uint32	lTotSecs;		// total number of sectors
		 uint16	DriveNum;		// physical drive number
		 uint8	XSign;			// extended boot signature
		 uint32	Serial;			// volume serial number
		 char	Label[11];		// volume label
		 char	FileSysID[8];		// file system ID
		 char	Code[BOOTCODE_SIZ-8];	// 8 = length of following
		 uint32	Secret;			// long secret number
		 uint8	BootCheck;		// boot sector checksum
		 uint8	BootFlags;		// secret flags
		 uint16	BootSig;		// boot signature 0xAA55
		} Boot;


EC	Boot *		ReadBoot	(int drive);
EC	uint8		CheckBoot	(Boot * boot);
EC	bool		WriteBoot	(int drive, Boot * boot);


#endif