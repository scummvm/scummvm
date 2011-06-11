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

#include	"cge/gettext.h"
#include	"cge/keybd.h"
#include	"cge/mouse.h"
#include	<string.h>

namespace CGE {

GET_TEXT *	GET_TEXT::Ptr	= NULL;



GET_TEXT::GET_TEXT (const char * info, char * text, int size, void (*click)(void))
: Text(text), Size(min<int>(size, GTMAX)), Len(min<int>(Size, strlen(text))),
  Cntr(GTBLINK), Click(click), OldKeybClient(KEYBOARD::SetClient(this))
{
  int i = 2 * TEXT_HM + Font.Width(info);
  Ptr = this;
  Mode = RECT;
  TS[0] = Box((i + 3) & ~3, 2 * TEXT_VM + 2 * FONT_HIG + TEXT_LS);
  SetShapeList(TS);
  Flags.BDel = true;
  Flags.Kill = true;
  memcpy(Buff, text, Len);
  Buff[Len] = ' ';
  Buff[Len+1] = '\0';
  PutLine(0, info);
  Tick();
}






GET_TEXT::~GET_TEXT (void)
{
  KEYBOARD::SetClient(OldKeybClient);
  Ptr = NULL;
}






void GET_TEXT::Tick (void)
{
  if (++ Cntr >= GTBLINK)
    {
      Buff[Len] ^= (' ' ^ '_');
      Cntr = 0;
    }
  PutLine(1, Buff);
  Time = GTTIME;
}




void GET_TEXT::Touch (uint16 mask, int x, int y)
{
  static char ogon[] = "èïêú•£ò†°";
  static char bezo[] = "ACELNOSXZ";
  char * p;

  if (mask & KEYB)
    {
      if (Click) Click();
      switch (x)
	{
	  case Enter : Buff[Len] = '\0'; strcpy(Text, Buff);
		       for (p = Text; *p; p ++)
			 {
			   char * q = strchr(ogon, *p);
			   if (q) *p = bezo[q-ogon];
			 }
	  case Esc   : SNPOST_(SNKILL, -1, 0, this); break;
	  case BSp   : if (Len)
			 {
			   -- Len;
			   Buff[Len] = Buff[Len+1];
			   Buff[Len+1] = Buff[Len+2];
			 }
		       break;
	  default    : if (x < 'A' || x > 'Z')
			 {
			   if (OldKeybClient)
			     OldKeybClient->Touch(mask, x, y);
			 }
		       else
			 {
			   if (KEYBOARD::Key[ALT])
			     {
			       p = strchr(bezo, x);
			       if (p) x = ogon[p-bezo];
			     }
			   if (Len < Size && 2 * TEXT_HM + Font.Width(Buff) + Font.Wid[x] <= W)
			     {
			       Buff[Len+2] = Buff[Len+1];
			       Buff[Len+1] = Buff[Len];
			       Buff[Len ++] = x;
			     }
			 }
		       break;
	}
    }
  else SPRITE::Touch(mask, x, y);
}

} // End of namespace CGE
