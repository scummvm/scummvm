#include	"fedit.h"
#include	"mouse.h"
#include	<string.h>




FEDIT *	FEDIT::Ptr	= NULL;
char *	FEDIT::BN[][2]	= { { "FE_LFT0", "FE_LFT1" },
			    { "FE_RGT0", "FE_RGT1" },
			    { "FE_XIT0", "FE_XIT1" },
			    { "FE_SAV0", "FE_SAV1" },
			  };


//--------------------------------------------------------------------------


FEDIT::FEDIT (void)
{
  int i;

  TS[0] = Panel = new BITMAP("FE", FALSE);
  SetShapeList(TS);
  Reset(Seq1);
  VGA::ShowQ.Insert(this, VGA::ShowQ.Last());
  Flags.BDel = TRUE;
  Flags.Kill = TRUE;
  Debug( Flags.Drag = FALSE; )
  SetName(strdup("Font Designer"));
  Ptr = this;

  for (i = 0; i < ArrayCount(BN); i ++)
    {
      Btn[i] = new BUTTON(BN[i][0], BN[i][1], FEDIT_HM+i*2*FEDIT_PT, FEDIT_VM+FONT_HIG*FEDIT_PT);
      Btn[i]->Flags.Slav = TRUE;
      Btn[i]->Flags.Kill = TRUE;
      Btn[i]->Flags.BDel = TRUE;
      VGA::ShowQ.Insert(Btn[i], VGA::ShowQ.Last());
    }

//  GetChr('A');
}






FEDIT::~FEDIT (void)
{
  Ptr = NULL;
}





void FEDIT::GetChr (byte chr)
{
  byte * p = Font.Map+Font.Pos[CurChr = chr];
  word w = Font.Wid[CurChr];

  memset(ChrBuf, 0, sizeof(ChrBuf));
  memcpy(ChrBuf, p, w);

  Update();
}





void FEDIT::PutChr (void)
{
  int i, w0 = Font.Wid[CurChr], w;
  word n = Font.Pos[CurChr];
  byte * p = Font.Map + n;

  for (i = 0; i < 8; i ++) if (ChrBuf[i]) break;
  if (i < 8)
    {
      if (i)		// space on left: strip it out
	{
	  w = 8 - i;
	  memcpy(ChrBuf, ChrBuf+i, w);
	  memset(ChrBuf+w, 0, i);
	}
      for (w = 8; ChrBuf[w-1] == 0; w --);
      if (w < 8) ++ w;
    }
  else w = 4;
  if (w != w0 && CurChr < 255)
    {
      memmove(p + w, p + w0, Font.Pos[255] + Font.Wid[255] - (n + w0));
      Font.Wid[CurChr] = w;
      w0 = w - w0;
      for (i = CurChr + 1; i < 256; i ++) Font.Pos[i] += w0;
    }
  memcpy(p, ChrBuf, w);
}





void FEDIT::Plot (byte colr, word x, word y)
{
  byte far * p = Panel->M + (W*(FEDIT_VM+y*FEDIT_PT+1)+FEDIT_HM+x*FEDIT_PT+1);
  word i;

  for (i = 0; i < FEDIT_PT-2; i ++)
    {
      _fmemset(p, colr, FEDIT_PT-2);
      p += W;
    }
}





void FEDIT::Update (void)
{
  word i, n;
  for (i = 0; i < 8; i ++)
    {
      byte b = ChrBuf[i];
      for (n = 0; n < FONT_HIG; n ++)
	{
	  Plot((b & 1) ? TEXT_FG : TEXT_BG, i, n);
	  b >>= 1;
	}
    }
  (TS[0] = Panel)->Code();
}





void FEDIT::Touch (word mask, int x, int y)
{
  SPRITE::Touch(mask, x, y);
  if (mask & L_UP)
    {
      x -= FEDIT_HM;
      y -= FEDIT_VM;

      if (x < 0 || x >= 8 * FEDIT_PT) return;	// outside left or right
      if (y < 0) return;			// top border
      x /= FEDIT_PT;				// raster h-units
      y /= FEDIT_PT;				// raster v-units
      if (y < FONT_HIG)				// raster field?
	{
	  ChrBuf[x] ^= (1 << y);
	  Update();
	}
    }
}





void FEDIT::Tick (void)
{
  if (Btn[0]->Clicked()) { PutChr(); GetChr(CurChr-1); }
  if (Btn[1]->Clicked()) { PutChr(); GetChr(CurChr+1); }
  if (Btn[2]->Clicked()) { PutChr(); SNPOST_(SNKILL, -1, 0, this); }
  if (Btn[3]->Clicked()) { PutChr(); Font.Save(); Btn[3]->Reset(); }
}
