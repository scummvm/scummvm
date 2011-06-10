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

#ifndef		__VGA13H__
#define		__VGA13H__

#include	"cge/general.h"
#include	<stddef.h>
#include	<dir.h>
#include	"cge/bitmap.h"
#include	"cge/snail.h"


#define		TMR_RATE1	16
#define		TMR_RATE2	4
#define		TMR_RATE        (TMR_RATE1*TMR_RATE2)

#define		MAX_NAME	20
#define		VIDEO   	0x10

#define		NO_CLEAR	0x80
#define		TEXT_MODE	0x03
#define		M13H		0x13

#ifndef	SCR_WID
  #define	SCR_WID	320
#endif

#ifndef	SCR_HIG
  #define	SCR_HIG	200
#endif


#if 0
 #define		LIGHT		0xFF
 #define		DARK		0x00
 #define		DGRAY		0xF6
 #define		GRAY		0xFC
 #define		LGRAY		0xFF
#else
 #define		LIGHT		0xFF
 #define		DARK		207
 #define		DGRAY		225 /*219*/
 #define		GRAY		231
 #define		LGRAY		237
#endif

#define		NO_SEQ		(-1)
#define		NO_PTR		((byte)-1)

#define		SPR_EXT		".SPR"

#define		IsFile(s)	(access(s,0)==0)
#define		IsWrit(s)	(access(s,2)==0)



typedef	struct	{ word r : 2; word R : 6;
		  word g : 2; word G : 6;
		  word b : 2; word B : 6;
		} RGB;

typedef	union	{
		  DAC dac;
		  RGB rgb;
		} TRGB;

typedef	struct	{ byte idx, adr; byte clr, set; } VgaRegBlk;

typedef	struct	{ byte Now, Next; signed char Dx, Dy; int Dly; } SEQ;

extern	SEQ	Seq1[];
extern	SEQ	Seq2[];
//extern	SEQ *	Compass[];
//extern	SEQ	TurnToS[];


#define		PAL_CNT		256
#define		PAL_SIZ		(PAL_CNT*sizeof(DAC))

#define	VGAATR_	0x3C0
#define	VGAMIw_	0x3C0
#define	VGASEQ_	0x3C4
#define	VGAMIr_	0x3CC
#define	VGAGRA_	0x3CE
#define	VGACRT_	0x3D4
#define	VGAST1_	0x3DA
#define	VGAATR	(VGAATR_ & 0xFF)
#define	VGAMIw	(VGAMIw_ & 0xFF)
#define	VGASEQ	(VGASEQ_ & 0xFF)
#define	VGAMIr	(VGAMIr_ & 0xFF)
#define	VGAGRA	(VGAGRA_ & 0xFF)
#define	VGACRT	(VGACRT_ & 0xFF)
#define	VGAST1	(VGAST1_ & 0xFF)






class HEART : public ENGINE
{
  friend ENGINE;
public:
  static Boolean Enable;
  static word * XTimer;
  static void SetXTimer (word * ptr);
  static void SetXTimer (word * ptr, word time);
  HEART (void);
};





class SPREXT
{
public:
  int x0, y0;
  int x1, y1;
  BMP_PTR b0, b1;
  BMP_PTR * ShpList;
  SEQ * Seq;
  char * Name;
  SNAIL::COM * Near, * Take;
  SPREXT (void) :
     x0(0), y0(0),
     x1(0), y1(0),
     b0(NULL), b1(NULL),
     ShpList(NULL), Seq(NULL),
     Name(NULL), Near(NULL), Take(NULL)
     {}
};




class SPRITE
{
protected:
  SPREXT * Ext;
public:
  int Ref;
  signed char Cave;
  struct FLAGS { word Hide : 1;		// general visibility switch
		 word Near : 1;		// Near action lock
		 word Drag : 1;		// sprite is moveable
		 word Hold : 1;		// sprite is held with mouse
		 word ____ : 1;		// intrrupt driven animation
		 word Slav : 1;		// slave object
		 word Syst : 1;		// system object
		 word Kill : 1;		// dispose memory after remove
		 word Xlat : 1;		// 2nd way display: xlat table
		 word Port : 1;		// portable
		 word Kept : 1;		// kept in pocket
		 word East : 1;		// talk to east (in opposite to west)
		 word Shad : 1;		// shadow
		 word Back : 1;		// 'send to background' request
		 word BDel : 1;		// delete bitmaps in ~SPRITE
		 word Tran : 1;		// transparent (untouchable)
	       } Flags;
  int X, Y;
  signed char Z;
  word W, H;
  word Time;
  byte NearPtr, TakePtr;
  int SeqPtr;
  int ShpCnt;
  char File[MAXFILE];
  SPRITE * Prev, * Next;
  Boolean Works (SPRITE * spr);
  Boolean SeqTest (int n);
  inline Boolean Active (void) { return Ext != NULL; }
  SPRITE (BMP_PTR * shp);
  virtual ~SPRITE (void);
  BMP_PTR Shp (void);
  BMP_PTR * SetShapeList (BMP_PTR * shp);
  void MoveShapes (byte far * buf);
  SPRITE * Expand (void);
  SPRITE * Contract (void);
  SPRITE * BackShow (Boolean fast = FALSE);
  void SetName(char * n);
  inline char * Name(void) { return (Ext) ? Ext->Name : NULL; }
  void Goto (int x, int y);
  void Center (void);
  void Show (void);
  void Hide (void);
  BMP_PTR Ghost (void);
  void Show (word pg);
  void MakeXlat (byte far * x);
  void KillXlat (void);
  void Step (int nr = -1);
  SEQ * SetSeq (SEQ * seq);
  SNAIL::COM * SnList(SNLIST type);
  virtual void Touch (word mask, int x, int y);
  virtual void Tick (void);
};






class QUEUE
{
  SPRITE * Head, * Tail;
public:
  Boolean Show;
  QUEUE (Boolean show = FALSE);
  ~QUEUE (void);
  void Append (SPRITE * spr);
  void Insert (SPRITE * spr, SPRITE * nxt);
  void Insert (SPRITE * spr);
  SPRITE * Remove (SPRITE * spr);
  void ForAll (void (*fun)(SPRITE *));
  SPRITE * First (void) { return Head; }
  SPRITE * Last (void) { return Tail; }
  SPRITE * Locate (int ref);
  void Clear (void);
};






class VGA
{
  static word OldMode;
  static word far * OldScreen;
  static word StatAdr;
  static Boolean SetPal;
  static DAC far * OldColors, far * NewColors;
  static int SetMode (int mode);
  static void UpdateColors (void);
  static void SetColors (void);
  static const char * Msg;
  static const char * Nam;
  static void SetStatAdr (void);
  static void WaitVR (Boolean on = TRUE);
public:
  dword FrmCnt;
  static QUEUE ShowQ, SpareQ;
  static int Mono;
  static byte far * Page[4];
  VGA (int mode = M13H);
  ~VGA (void);
  void Setup (VgaRegBlk * vrb);
  static void GetColors (DAC far * tab);
  static void SetColors (DAC far * tab, int lum);
  static void Clear (byte color = 0);
  static void Exit (const char * txt = NULL, const char * name = NULL);
  static void Exit (int tref, const char * name = NULL);
  static void CopyPage (word d, word s = 3);
  static void Sunrise (DAC far * tab);
  static void Sunset (void);
  void Show (void);
  void Update (void);
};



DAC		MkDAC		(byte r, byte g, byte b);
RGB		MkRGB		(byte r, byte g, byte b);




template <class CBLK>
byte Closest (CBLK far * pal, CBLK x)
{
  #define f(col,lum) ((((word)(col))<<8)/lum)
  word i, dif = 0xFFFF, found;
  word L = x.R + x.G + x.B; if (! L) ++ L;
  word R = f(x.R, L), G = f(x.G, L), B = f(x.B, L);
  for (i = 0; i < 256; i ++)
    {
      word l = pal[i].R + pal[i].G + pal[i].B; if (! l) ++ l;
      int  r = f(pal[i].R, l), g = f(pal[i].G, l), b = f(pal[i].B, l);
      word D = ((r > R) ? (r - R) : (R - r)) +
	       ((g > G) ? (g - G) : (G - g)) +
	       ((b > B) ? (b - B) : (B - b)) +
	       ((l > L) ? (l - L) : (L - l)) * 10 ;

      if (D < dif)
	{
	  found = i;
	  dif = D;
	  if (D == 0) break;	// exact!
	}
    }
  return found;
  #undef f
};







		char *		NumStr		(char * str, int num);
		void		Video		(void);
		word far *	SaveScreen	(void);
		void		RestoreScreen	(word far * &sav);
		SPRITE *	SpriteAt	(int x, int y);
		SPRITE *	Locate		(int ref);

extern		Boolean		SpeedTest;


#endif
