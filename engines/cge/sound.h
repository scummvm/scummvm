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

#ifndef	__SOUND__
#define	__SOUND__

#include	"cge/wav.h"
#include	"cge/snddrv.h"


#define		BAD_SND_TEXT	97
#define		BAD_MIDI_TEXT	98




class SOUND
{
public:
  SMPINFO smpinf;
  SOUND (void);
  ~SOUND (void);
  void Open (void);
  void Close (void);
  void Play (DATACK * wav, int pan, int cnt = 1);
  void Stop (void);
};





class FX
{
  EMM Emm;
  struct HAN { int Ref; DATACK * Wav; } * Cache;
  int Size;
  DATACK * Load (int idx, int ref);
  int Find (int ref);
public:
  DATACK * Current;
  FX (int size = 16);
  ~FX (void);
  void Clear (void);
  void Preload (int ref0);
  DATACK * operator[] (int ref);
};






extern	Boolean	Music;
extern	SOUND	Sound;
extern	FX	Fx;


void		LoadMIDI	(int ref);
void		KillMIDI	(void);


#endif

