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

#include	"cge/vol.h"
//#include	<alloc.h>
#include "common/system.h"
#include "common/str.h"
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>

#ifdef	DROP_H
  #include	"cge/drop.h"
#else
  #include	<stdio.h>
#endif

namespace CGE {

#ifndef	DROP_H
	// TODO Replace printf by scummvm equivalent

	#define	DROP(m,n)	{ }
	//#define	DROP(m,n)	{ printf("%s [%s]\n", (m), (n)); _exit(1); }
#endif


#ifdef VOL_UPD
BTFILE		VFILE::Cat(CAT_NAME, UPD, CRP);
VOLBASE		DAT::File(DAT_NAME, UPD, CRP);
#else
BTFILE		VFILE::Cat(CAT_NAME, REA, CRP);
VOLBASE		DAT::File(DAT_NAME, REA, CRP);
#endif
DAT		VFILE::Dat;
VFILE *		VFILE::Recent			= NULL;





VFILE::VFILE (const char * name, IOMODE mode)
: IOBUF(mode)
{
  if (mode == REA)
    {
      if (Dat.File.Error || Cat.Error) DROP("Bad volume data", NULL);
      BT_KEYPACK * kp = Cat.Find(name);
      if (scumm_stricmp(kp->Key, name) != 0) Error = 1;
      EndMark = (BufMark = BegMark = kp->Mark) + kp->Size;
    }
  #ifdef VOL_UPD
  else Make(name);
  #endif
}





VFILE::~VFILE (void)
{
  if (Recent == this) Recent = NULL;
}





bool VFILE::Exist (const char * name)
{
  return scumm_stricmp(Cat.Find(name)->Key, name) == 0;
}




void VFILE::ReadBuff (void)
{
  if (Recent != this)
    {
      Dat.File.Seek(BufMark + Lim);
      Recent = this;
    }
  BufMark = Dat.File.Mark();
  long n = EndMark - BufMark;
  if (n > IOBUF_SIZE) n = IOBUF_SIZE;
  Lim = Dat.File.Read(Buff, (uint16) n);
  Ptr = 0;
}

} // End of namespace CGE
