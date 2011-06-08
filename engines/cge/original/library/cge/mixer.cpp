#include	"mixer.h"
#include	"text.h"
#include	"snail.h"
#include	"mouse.h"
#include	<snddrv.h>
#include	<string.h>
#include	<alloc.h>

//--------------------------------------------------------------------------


extern	MOUSE		Mouse;

	Boolean		MIXER::Appear	= FALSE;



MIXER::MIXER (int x, int y)
: SPRITE(NULL), Fall(MIX_FALL)
{
  int i;
  Appear = TRUE;
  mb[0] = new BITMAP("VOLUME");
  mb[1] = NULL;
  SetShapeList(mb);
  SetName(Text[MIX_NAME]);
  Flags.Syst = TRUE;
  Flags.Kill = TRUE;
  Flags.BDel = TRUE;
  Goto(x, y);
  Z = MIX_Z;

  // slaves

  for (i = 0; i < MIX_MAX; i ++)
    {
      static char fn[] = "V00";
      wtom(i, fn+1, 10, 2);
      lb[i] = new BITMAP(fn);
      ls[i].Now = ls[i].Next = i;
      ls[i].Dx = ls[i].Dy = ls[i].Dly = 0;
    }
  lb[i] = NULL;

  for (i = 0; i < ArrayCount(Led); i ++)
    {
      register SPRITE * spr = new SPRITE(lb);
      spr->SetSeq(ls);
      spr->Goto(x+2+12*i, y+8);
      spr->Flags.Tran = TRUE;
      spr->Flags.Kill = TRUE;
      spr->Flags.BDel = FALSE;
      spr->Z = MIX_Z;
      Led[i] = spr;
    }
  Led[ArrayCount(Led)-1]->Flags.BDel = TRUE;

  VGA::ShowQ.Insert(this);
  for (i = 0; i < ArrayCount(Led); i ++) VGA::ShowQ.Insert(Led[i]);

  //--- reset balance
  i = (SNDDrvInfo.VOL4.ML + SNDDrvInfo.VOL4.MR) / 2;
  SNDDrvInfo.VOL4.ML = i;
  SNDDrvInfo.VOL4.MR = i;
  i = (SNDDrvInfo.VOL4.DL + SNDDrvInfo.VOL4.DR) / 2;
  SNDDrvInfo.VOL4.DL = i;
  SNDDrvInfo.VOL4.DR = i;
  Update();
  Time = MIX_DELAY;
}




MIXER::~MIXER (void)
{
  Appear = FALSE;
}



#pragma argsused
void MIXER::Touch (word mask, int x, int y)
{
  SPRITE::Touch(mask, x, y);
  if (mask & L_UP)
    {
      byte * vol = (&SNDDrvInfo.VOL2.D) + (x < W/2);
      if (y < MIX_BHIG) { if (*vol < 0xFF) *vol += 0x11; }
      else if (y >= H-MIX_BHIG) { if (*vol > 0x00) *vol -= 0x11; }
      Update();
    }
}



void MIXER::Tick (void)
{
  int x = Mouse.X, y = Mouse.Y;
  if (SpriteAt(x, y) == this)
    {
      Fall = MIX_FALL;
      if (Flags.Hold) Touch(L_UP, x-X, y-Y);
    }
  else
    {
      if (Fall) -- Fall;
      else
	{
	  int i;
	  for (i = 0; i < ArrayCount(Led); i ++)
	    {
	      SNPOST_(SNKILL, -1, 0, Led[i]);
	    }
	  SNPOST_(SNKILL, -1, 0, this);
	}
    }
  Time = MIX_DELAY;
}




void MIXER::Update (void)
{
  Led[0]->Step(SNDDrvInfo.VOL4.ML);
  Led[1]->Step(SNDDrvInfo.VOL4.DL);
  SNPOST_(SNEXEC, -1, 0, SNDSetVolume);
}
