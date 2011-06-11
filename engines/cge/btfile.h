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

#ifndef	__BTFILE__
#define	__BTFILE__

#include	"cge/general.h"

namespace CGE {

#define		BT_SIZE		K(1)
#define		BT_KEYLEN	13
#define		BT_LEVELS	2

#define		BT_NONE		0xFFFF
#define		BT_ROOT		0


struct BT_KEYPACK
{
  char Key[BT_KEYLEN];
  uint32 Mark;
  uint16 Size;
};



struct BT_PAGE
{
  struct HEA
    {
      uint16 Count;
      uint16 Down;
    } Hea;
  union
    {
      // dummy filler to make proper size of union
      uint8 Data[BT_SIZE-sizeof(HEA)];
      // inner version of data: key + word-sized page link
      struct INNER
	{
	  uint8 Key[BT_KEYLEN];
	  uint16 Down;
	} Inn[(BT_SIZE-sizeof(HEA))/sizeof(INNER)];
      // leaf version of data: key + all user data
      BT_KEYPACK Lea[(BT_SIZE-sizeof(HEA))/sizeof(BT_KEYPACK)];
    };
};





class BTFILE : public IOHAND
{
  struct
    {
      BT_PAGE * Page;
      uint16 PgNo;
      int Indx;
      bool Updt;
    } Buff[BT_LEVELS];
  void PutPage (int lev, bool hard = FALSE);
  BT_PAGE * GetPage (int lev, uint16 pgn);
public:
  BTFILE (const char * name, IOMODE mode = REA, CRYPT * crpt = NULL);
  virtual ~BTFILE (void);
  BT_KEYPACK * Find(const char * key);
  BT_KEYPACK * Next(void);
  void Make(BT_KEYPACK * keypack, uint16 count);
};

} // End of namespace CGE

#endif
