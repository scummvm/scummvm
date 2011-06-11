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
//#include	<dir.h>
#include	"cge/bitmap.h"
#include	"cge/snail.h"

namespace CGE {

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
#define		NO_PTR		((uint8)-1)

#define		SPR_EXT		".SPR"

#define		IsFile(s)	(access(s,0)==0)
#define		IsWrit(s)	(access(s,2)==0)



typedef	struct	{ uint16 r : 2; uint16 R : 6;
		  uint16 g : 2; uint16 G : 6;
		  uint16 b : 2; uint16 B : 6;
		} RGB;

typedef	union	{
		  DAC dac;
		  RGB rgb;
		} TRGB;

typedef	struct	{ uint8 idx, adr; uint8 clr, set; } VgaRegBlk;

typedef	struct	{ uint8 Now, Next; signed char Dx, Dy; int Dly; } SEQ;

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
  static bool Enable;
  static uint16 * XTimer;
  static void SetXTimer (uint16 * ptr);
  static void SetXTimer (uint16 * ptr, uint16 time);
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
  struct FLAGS { uint16 Hide : 1;		// general visibility switch
		 uint16 Near : 1;		// Near action lock
		 uint16 Drag : 1;		// sprite is moveable
		 uint16 Hold : 1;		// sprite is held with mouse
		 uint16 ____ : 1;		// intrrupt driven animation
		 uint16 Slav : 1;		// slave object
		 uint16 Syst : 1;		// system object
		 uint16 Kill : 1;		// dispose memory after remove
		 uint16 Xlat : 1;		// 2nd way display: xlat table
		 uint16 Port : 1;		// portable
		 uint16 Kept : 1;		// kept in pocket
		 uint16 East : 1;		// talk to east (in opposite to west)
		 uint16 Shad : 1;		// shadow
		 uint16 Back : 1;		// 'send to background' request
		 uint16 BDel : 1;		// delete bitmaps in ~SPRITE
		 uint16 Tran : 1;		// transparent (untouchable)
	       } Flags;
  int X, Y;
  signed char Z;
  uint16 W, H;
  uint16 Time;
  uint8 NearPtr, TakePtr;
  int SeqPtr;
  int ShpCnt;
  char File[MAXFILE];
  SPRITE * Prev, * Next;
  bool Works (SPRITE * spr);
  bool SeqTest (int n);
  inline bool Active (void) { return Ext != NULL; }
  SPRITE (BMP_PTR * shp);
  virtual ~SPRITE (void);
  BMP_PTR Shp (void);
  BMP_PTR * SetShapeList (BMP_PTR * shp);
  void MoveShapes (uint8 * buf);
  SPRITE * Expand (void);
  SPRITE * Contract (void);
  SPRITE * BackShow (bool fast = false);
  void SetName(char * n);
  inline char * Name(void) { return (Ext) ? Ext->Name : NULL; }
  void Goto (int x, int y);
  void Center (void);
  void Show (void);
  void Hide (void);
  BMP_PTR Ghost (void);
  void Show (uint16 pg);
  void MakeXlat (uint8 * x);
  void KillXlat (void);
  void Step (int nr = -1);
  SEQ * SetSeq (SEQ * seq);
  SNAIL::COM * SnList(SNLIST type);
  virtual void Touch (uint16 mask, int x, int y);
  virtual void Tick (void);
};






class QUEUE
{
  SPRITE * Head, * Tail;
public:
  bool Show;
  QUEUE (bool show = false);
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
  static uint16 OldMode;
  static uint16 * OldScreen;
  static uint16 StatAdr;
  static bool SetPal;
  static DAC * OldColors, * NewColors;
  static int SetMode (int mode);
  static void UpdateColors (void);
  static void SetColors (void);
  static const char * Msg;
  static const char * Nam;
  static void SetStatAdr (void);
  static void WaitVR (bool on = true);
public:
  uint32 FrmCnt;
  static QUEUE ShowQ, SpareQ;
  static int Mono;
  static uint8 * Page[4];
  VGA (int mode = M13H);
  ~VGA (void);
  void Setup (VgaRegBlk * vrb);
  static void GetColors (DAC * tab);
  static void SetColors (DAC * tab, int lum);
  static void Clear (uint8 color = 0);
  static void Exit (const char * txt = NULL, const char * name = NULL);
  static void Exit (int tref, const char * name = NULL);
  static void CopyPage (uint16 d, uint16 s = 3);
  static void Sunrise (DAC * tab);
  static void Sunset (void);
  void Show (void);
  void Update (void);
};



DAC		MkDAC		(uint8 r, uint8 g, uint8 b);
RGB		MkRGB		(uint8 r, uint8 g, uint8 b);




template <class CBLK>
uint8 Closest (CBLK * pal, CBLK x)
{
  #define f(col,lum) ((((uint16)(col))<<8)/lum)
  uint16 i, dif = 0xFFFF, found = 0;
  uint16 L = x.R + x.G + x.B; if (! L) ++ L;
  uint16 R = f(x.R, L), G = f(x.G, L), B = f(x.B, L);
  for (i = 0; i < 256; i ++)
    {
      uint16 l = pal[i].R + pal[i].G + pal[i].B; if (! l) ++ l;
      int  r = f(pal[i].R, l), g = f(pal[i].G, l), b = f(pal[i].B, l);
      uint16 D = ((r > R) ? (r - R) : (R - r)) +
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
		//static void		Video		(void);
		uint16 *	SaveScreen	(void);
		void		RestoreScreen	(uint16 * &sav);
		SPRITE *	SpriteAt	(int x, int y);
		SPRITE *	Locate		(int ref);

extern		bool		SpeedTest;

} // End if namespace CGE

#endif
