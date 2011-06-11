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

#ifndef		__TALK__
#define		__TALK__

#include	"cge/vga13h.h"
#include	"cge/general.h"
#include	"cge/jbw.h"
//#include	<dir.h>

namespace CGE {

#define		TEXT_FG		DARK	// foreground color
#define		TEXT_BG		GRAY	// background color
#define		TEXT_HM		(6&~1)	// EVEN horizontal margins!
#define		TEXT_VM		5	// vertical margins
#define		TEXT_LS		2	// line spacing
#define		TEXT_RD		3	// rounded corners

#define		FONT_HIG	8
#define		FONT_EXT	".CFT"



#define MAXPATH  128

class	FONT
{
  char Path[MAXPATH];
  void Load (void);
public:
//  static uint8 Wid[256];
//  static uint16 Pos[256];
//  static uint8 Map[256*8];
  uint8 * Wid;
  uint16 * Pos;
  uint8 * Map;
  FONT (const char * name);
  ~FONT (void);
  uint16 Width (const char * text);
  void Save (void);
};





enum	TBOX_STYLE	{ PURE, RECT, ROUND };



class TALK : public SPRITE
{
protected:
  TBOX_STYLE Mode;
  BITMAP * TS[2];
  BITMAP * Box(uint16 w, uint16 h);
public:
  static FONT Font;
  TALK (const char * tx, TBOX_STYLE mode = PURE);
  TALK (void);
  //~TALK (void);
  virtual void Update (const char * tx);
  virtual void Update (void) {}
  void PutLine (int line, const char * text);
};







class INFO_LINE : public TALK
{
  const char * OldTxt;
public:
  INFO_LINE (uint16 wid);
  void Update (const char * tx);
};


} // End of namespace CGE

#endif
