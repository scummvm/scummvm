#ifndef		__GAME__
#define		__GAME__

#include	"vga13h.h"
#include	"bitmaps.h"



#define		PAN_HIG		40
#define		LBound(s)	(s->X <= 0)
#define		RBound(s)	(s->X+s->W >= SCR_WID)
#define		TBound(s)	(s->Y <= 0)
#define		BBound(s)	(s->Y+s->H >= SCR_HIG - PAN_HIG)



extern		SPRITE *	Sys;

int		Sinus		(long x);
byte *		Glass		(DAC far * pal, byte r, byte g, byte b);
byte *		Mark		(DAC far * pal);





class FLY : public SPRITE
{
  static int L, T, R, B;
public:
  int Tx, Ty;
  FLY (BITMAP ** shpl);
  void Tick (void);
};





#endif