#include	<wind.h>
#include	<dos.h>

extern	Wind *		WindStack;
extern	Wind *		Desk = NULL;






void SetRefreshDesk (void)
{
  Wind *w;
  for (w = WindStack; w != NULL; w = w->Next) SetRefreshWind(w);
}





Wind * SetDesk (Boolean on)
{
  static int OldCBrk;
  static word OldCursor = OFF_Cursor;

  if ((Desk == NULL) != on) return Desk;
  BreakFlag = 0;
  if (on)
    {
      GetScrSiz(); if (Mono == -1) Mono = IsMono();
      ZoomTop = 0; ZoomBot = MaxScrHig-1;
      Screen = ScrAdr(0, 0);
      if ((Desk = MakeField(0, 0, MaxScrWid-1, MaxScrHig-1, DSK, '±')) != NULL)
	{
	  OldCBrk = getcbrk();
	  setcbrk(0);
	  harderr(CriticalError);
	  OldCursor = SetCursor(Desk->Cursor.Shape);
	  MouseInit(); MouseGoto(0, 0);
	  SetKeyProc(WindKeyProc);
	  ShowWind(Desk);
	}
    }
  else
    {
      MouseCursor(FALSE);
      while (WindStack != Desk) CloseWind(WindStack);
      if (Desk->Body.Near == NULL)
	{
	  SetRefreshDesk();
	  Desk->ShowProc = ShowFieldChar;
	  Desk->X = ' ';
	  Desk->Color[FLD_N] = 0x07;
	  Refresh();
	}
      else RestoreScreen(Desk->Body.Near);

      CloseWind(Desk);
      Desk = NULL;
      setcbrk(OldCBrk);
      SetCursor(OldCursor);
    }
  return Desk;
}
