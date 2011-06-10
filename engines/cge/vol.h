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

#ifndef		__VOL__
#define		__VOL__


#include	<dir.h>
#include	"cge/btfile.h"
#include	"cge/cfile.h"

#define		CAT_NAME	"VOL.CAT"
#define		DAT_NAME	"VOL.DAT"

#ifndef	CRP
  #define	CRP		XCrypt
#endif

#define		XMASK		0xA5

#ifdef	VOL_UPD
#define		VOLBASE		IOHAND
#else
#define		VOLBASE		CFILE
#endif



class DAT
{
  friend VFILE;
  static VOLBASE File;
public:
  static bool Append (uint8 * buf, uint16 len);
  static bool Write (CFILE& f);
  static bool Read (long org, uint16 len, uint8 * buf);
};







class	VFILE : public IOBUF
{
  static DAT Dat;
  static BTFILE Cat;
  static VFILE * Recent;
  long BegMark, EndMark;
  void ReadBuff (void);
  void WriteBuff (void) { }
  void Make(const char * fspec);
public:
  VFILE (const char * name, IOMODE mode = REA);
  ~VFILE (void);
  static bool Exist (const char * name);
  static const char * Next (void);
  long Mark (void) { return (BufMark+Ptr) - BegMark; }
  long Size (void) { return EndMark - BegMark; }
  long Seek (long pos) { Recent = NULL; Lim = 0; return (BufMark = BegMark+pos); }
};




#endif
