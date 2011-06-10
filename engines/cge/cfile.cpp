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

#include	<cfile.h>
#include	<dos.h>
#include	<fcntl.h>
#include	<string.h>
#include	<alloc.h>

#ifdef	DROP_H
  #include	"cge/drop.h"
#else
  #include	<stdio.h>
  #include	<stdlib.h>
  #define	DROP(m,n)	{ printf("%s [%s]\n", m, n); _exit(1); }
#endif




IOBUF::IOBUF (IOMODE mode, CRYPT * crpt)
: IOHAND(mode, crpt),
  BufMark(0),
  Ptr(0),
  Lim(0)
{
  Buff = farnew(byte, IOBUF_SIZE);
  if (Buff == NULL) DROP("No core for I/O", NULL);
}









IOBUF::IOBUF (const char * name, IOMODE mode, CRYPT * crpt)
: IOHAND(name, mode, crpt),
  BufMark(0),
  Ptr(0),
  Lim(0)
{
  Buff = farnew(byte, IOBUF_SIZE);
  if (Buff == NULL) DROP("No core for I/O", name);
}









IOBUF::~IOBUF (void)
{
  if (Mode > REA) WriteBuff();
  if (Buff) farfree(Buff);
}






void IOBUF::ReadBuff (void)
{
  BufMark = IOHAND::Mark();
  Lim = IOHAND::Read(Buff, IOBUF_SIZE);
  Ptr = 0;
}





void IOBUF::WriteBuff (void)
{
  if (Lim)
    {
      IOHAND::Write(Buff, Lim);
      BufMark = IOHAND::Mark();
      Lim = 0;
    }
}





word IOBUF::Read (void far * buf, word len)
{
  word total = 0;
  while (len)
    {
      if (Ptr >= Lim) ReadBuff();
      word n = Lim - Ptr;
      if (n)
	{
	  if (len < n) n = len;
	  _fmemcpy(buf, Buff+Ptr, n);
	  (byte far *) buf += n;
	  len -= n;
	  total += n;
	  Ptr += n;
	}
      else break;
    }
  return total;
}






word IOBUF::Read (byte far * buf)
{
  word total = 0;

  while (total < LINE_MAX-2)
    {
      if (Ptr >= Lim) ReadBuff();
      byte far * p = Buff + Ptr;
      word n = Lim - Ptr;
      if (n)
	{
	  if (total + n >= LINE_MAX-2) n = LINE_MAX-2 - total;
	  byte far * eol = (byte far *) _fmemchr(p, '\r', n);
	  if (eol) n = (word) (eol - p);
	  byte far * eof = (byte far *) _fmemchr(p, '\32', n);
	  if (eof) // end-of-file
	    {
	      n = (word) (eof - p);
	      Ptr = (word) (eof - Buff);
	    }
	  if (n) _fmemcpy(buf, p, n);
	  buf += n;
	  total += n;
	  if (eof) break;
	  Ptr += n;
	  if (eol)
	    {
	      ++ Ptr;
	      * (buf ++) = '\n';
	      ++ total;
	      if (Ptr >= Lim) ReadBuff();
	      if (Ptr < Lim) if (Buff[Ptr] == '\n') ++ Ptr;
	      break;
	    }
	}
      else break;
    }
  *buf = '\0';
  return total;
}







word IOBUF::Write (void far * buf, word len)
{
  word tot = 0;
  while (len)
    {
      word n = IOBUF_SIZE - Lim;
      if (n > len) n = len;
      if (n)
	{
	  _fmemcpy(Buff+Lim, buf, n);
	  Lim += n;
	  len -= n;
	  (byte far *) buf += n;
	  tot += n;
	}
      else WriteBuff();
    }
  return tot;
}






word IOBUF::Write (byte far * buf)
{
  word len = 0;
  if (buf)
    {
      len = _fstrlen((const char far *) buf);
      if (len) if (buf[len-1] == '\n') -- len;
      len = Write(buf, len);
      if (len)
	{
	  static char EOL[] = "\r\n";
	  word n = Write(EOL, sizeof(EOL)-1);
	  len += n;
	}
    }
  return len;
}






int IOBUF::Read (void)
{
  if (Ptr >= Lim)
    {
      ReadBuff();
      if (Lim == 0) return -1;
    }
  return Buff[Ptr ++];
}






void IOBUF::Write (byte b)
{
  if (Lim >= IOBUF_SIZE)
    {
      WriteBuff();
    }
  Buff[Lim ++] = b;
}






	word	CFILE::MaxLineLen	= LINE_MAX;








CFILE::CFILE (const char near * name, IOMODE mode, CRYPT * crpt)
: IOBUF(name, mode, crpt)
{
}









CFILE::~CFILE (void)
{
}






void CFILE::Flush (void)
{
  if (Mode > REA) WriteBuff();
  else Lim = 0;
  _BX = Handle;
  _AH = 0x68;		// Flush buffer
  asm	int	0x21
}





long CFILE::Mark (void)
{
  return BufMark + ((Mode > REA) ? Lim : Ptr);
}





long CFILE::Seek (long pos)
{
  if (pos >= BufMark && pos < BufMark + Lim)
    {
      ((Mode == REA) ? Ptr : Lim) = (word) (pos - BufMark);
      return pos;
    }
  else
    {
      if (Mode > REA)
	{
	  WriteBuff();
	}
      else
	{
	  Lim = 0;
	}
      Ptr = 0;
      return BufMark = IOHAND::Seek(pos);
    }
}






void CFILE::Append (CFILE& f)
{
  Seek(Size());
  if (f.Error == 0)
    {
      while (TRUE)
	{
	  if ((Lim = f.IOHAND::Read(Buff, IOBUF_SIZE)) == IOBUF_SIZE) WriteBuff();
	  else break;
	  if ((Error = f.Error) != 0) break;
	}
    }
}
