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

#include	"cge/general.h"
#include	"cge/talk.h"
#include	"cge/vol.h"
#include	"cge/game.h"
#include	"cge/mouse.h"
#include	<dos.h>
#include	<alloc.h>
#include	<mem.h>

#define		WID_SIZ		256
#define		POS_SIZ		256
#define		MAP_SIZ		(256*8)


//--------------------------------------------------------------------------



//byte	FONT::Wid[WID_SIZ];
//word	FONT::Pos[POS_SIZ];
//byte	FONT::Map[MAP_SIZ];







FONT::FONT (const char * name)
{
  Map = farnew(byte, MAP_SIZ);
  Pos = farnew(word, POS_SIZ);
  Wid = farnew(byte, WID_SIZ);
  if (Map == NULL || Pos == NULL || Wid == NULL) DROP("No core", NULL);
  MergeExt(Path, name, FONT_EXT);
  Load();
}




FONT::~FONT (void)
{
  farfree(Map);
  farfree(Pos);
  farfree(Wid);
}




void FONT::Load (void)
{
  INI_FILE f(Path);
  if (! f.Error)
    {
      f.Read(Wid, WID_SIZ);
      if (! f.Error)
	{
	  word i, p = 0;
	  for (i = 0; i < POS_SIZ; i ++)
	    {
	      Pos[i] = p;
	      p += Wid[i];
	    }
	  f.Read(Map, p);
	}
    }
}





word FONT::Width (const char * text)
{
  word w = 0;
  if (text) while (* text) w += Wid[*(text ++)];
  return w;
}



/*
void FONT::Save (void)
{
  CFILE f((const char *) Path, WRI);
  if (! f.Error)
    {
      f.Write(Wid, WID_SIZ);
      if (! f.Error)
	{
	  f.Write(Map, Pos[POS_SIZ-1] + Wid[WID_SIZ-1]);
	}
    }
}
*/




//--------------------------------------------------------------------------



FONT	TALK::Font(ProgName());



TALK::TALK (const char * tx, TBOX_STYLE mode)
: SPRITE(NULL), Mode(mode)
{
  TS[0] = TS[1] = NULL;
  Flags.Syst = TRUE;
  Update(tx);
}





TALK::TALK (void)
: SPRITE(NULL), Mode(PURE)
{
  TS[0] = TS[1] = NULL;
  Flags.Syst = TRUE;
}





/*
TALK::~TALK (void)
{
  word i;
  for (i = 0; i < ShpCnt; i ++)
    {
      if (FP_SEG(ShpList[i]) != _DS) // small model: always FALSE
	{
	  delete ShpList[i];
	  ShpList[i] = NULL;
	}
    }
}
*/



void TALK::Update (const char * tx)
{
  word vmarg = (Mode) ? TEXT_VM : 0;
  word hmarg = (Mode) ? TEXT_HM : 0;
  word mw, mh, ln = vmarg;
  const char * p;
  byte * m;

  if (! TS[0])
    {
      word k = 2 * hmarg;
      mh = 2 * vmarg + FONT_HIG;
      mw = 0;
      for (p = tx; *p; p ++)
	{
	  if (*p == '|' || *p == '\n')
	    {
	      mh += FONT_HIG + TEXT_LS;
	      if (k > mw) mw = k;
	      k = 2 * hmarg;
	    }
	  else k += Font.Wid[*p];
	}
      if (k > mw) mw = k;
      TS[0] = Box(mw, mh);
    }

  m = TS[0]->M + ln * mw + hmarg;

  while (* tx)
    {
      if (*tx == '|' || *tx == '\n')
	m = TS[0]->M + (ln += FONT_HIG + TEXT_LS) * mw + hmarg;
      else
	{
	  int cw = Font.Wid[*tx], i;
	  char * f = Font.Map + Font.Pos[*tx];
	  for (i = 0; i < cw; i ++)
	    {
	      char * p = m;
	      word n;
	      register word b = * (f ++);
	      for (n = 0; n < FONT_HIG; n ++)
		{
		  if (b & 1) * p = TEXT_FG;
		  b >>= 1;
		  p += mw;
		}
	      ++ m;
	    }
	}
      ++ tx;
    }
  TS[0]->Code();
  SetShapeList(TS);
}




BITMAP * TALK::Box (word w, word h)
{
  byte * b, * p, * q;
  word n, r = (Mode == ROUND) ? TEXT_RD : 0;
  int i;

  if (w < 8) w = 8;
  if (h < 8) h = 8;
  b = farnew(byte, n = w * h);
  if (! b) VGA::Exit("No core");
  _fmemset(b, TEXT_BG, n);

  if (Mode)
    {
      p = b; q = b + n - w;
      _fmemset(p, LGRAY, w);
      _fmemset(q, DGRAY, w);
      while (p < q)
	{
	  p += w;
	  * (p-1) = DGRAY;
	  * p = LGRAY;
	}
      p = b;
      for (i = 0; i < r; i ++)
	{
	  int j;
	  for (j = 0; j < r-i; j ++)
	    {
	      p[  j  ] = TRANS;
	      p[w-j-1] = TRANS;
	      q[  j  ] = TRANS;
	      q[w-j-1] = TRANS;
	    }
	  p[  j  ] = LGRAY;
	  p[w-j-1] = DGRAY;
	  q[  j  ] = LGRAY;
	  q[w-j-1] = DGRAY;
	  p += w;
	  q -= w;
	}
    }
  return new BITMAP(w, h, b);
}





void TALK::PutLine (int line, const char * text)
// Note: (TS[0].W % 4) have to be 0
{
  word w = TS[0]->W, h = TS[0]->H;
  byte * v = TS[0]->V, * p;
  word dsiz = w >> 2;		// data size (1 plane line size)
  word lsiz = 2 + dsiz + 2;	// word for line header, word for gap
  word psiz = h * lsiz;		// - last gap, but + plane trailer
  word size = 4 * psiz;		// whole map size
  word rsiz = FONT_HIG * lsiz;	// length of whole text row map

  // set desired line pointer
  v += (TEXT_VM + (FONT_HIG + TEXT_LS) * line) * lsiz;

  // clear whole rectangle
  p = v;				// assume blanked line above text
  _fmemcpy(p, p-lsiz, rsiz); p += psiz;	// tricky replicate lines for plane 0
  _fmemcpy(p, p-lsiz, rsiz); p += psiz;	// same for plane 1
  _fmemcpy(p, p-lsiz, rsiz); p += psiz;	// same for plane 2
  _fmemcpy(p, p-lsiz, rsiz);		// same for plane 3

  // paint text line
  if (text)
    {
      byte * q;
      p = v + 2 + TEXT_HM/4 + (TEXT_HM%4)*psiz;
      q = v + size;

      while (* text)
	{
	  word cw = Font.Wid[*text], i;
	  byte * fp = Font.Map + Font.Pos[*text];

	  for (i = 0; i < cw; i ++)
	    {
	      register word b = fp[i];
	      word n;
	      for (n = 0; n < FONT_HIG; n ++)
		{
		  if (b & 1) *p = TEXT_FG;
		  b >>= 1;
		  p += lsiz;
		}
	      p = p - rsiz + psiz;
	      if (p >= q) p = p - size + 1;
	    }
	  ++ text;
	}
    }
}






//--------------------------------------------------------------------------




INFO_LINE::INFO_LINE (word w)
: OldTxt(NULL)
{
  TS[0] = new BITMAP(w, FONT_HIG, TEXT_BG);
  SetShapeList(TS);
}






void INFO_LINE::Update (const char * tx)
{
  if (tx != OldTxt)
    {
      word w = TS[0]->W, h = TS[0]->H;
      byte * v = (byte *) TS[0]->V;
      word dsiz = w >> 2;		// data size (1 plane line size)
      word lsiz = 2 + dsiz + 2;		// word for line header, word for gap
      word psiz = h * lsiz;		// - last gape, but + plane trailer
      word size = 4 * psiz;		// whole map size

      // claer whole rectangle
      memset(v+2, TEXT_BG, dsiz);	// data bytes
      memcpy(v + lsiz, v, psiz - lsiz);	// tricky replicate lines
      * (word *) (v + psiz - 2) = EOI;	// plane trailer word
      memcpy(v + psiz, v, 3 * psiz);	// tricky replicate planes

      // paint text line
      if (tx)
	{
	  byte * p = v + 2, * q = p + size;

	  while (* tx)
	    {
	      word cw = Font.Wid[*tx], i;
	      byte * fp = Font.Map + Font.Pos[*tx];

	      for (i = 0; i < cw; i ++)
		{
		  register word b = fp[i];
		  word n;
		  for (n = 0; n < FONT_HIG; n ++)
		    {
		      if (b & 1) *p = TEXT_FG;
		      b >>= 1;
		      p += lsiz;
		    }
		  if (p >= q) p = p - size + 1;
		}
	      ++ tx;
	    }
	}
      OldTxt = tx;
    }
}
