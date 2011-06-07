#include	<wind.h>
#include	<string.h>



extern	Boolean	Status = TRUE;


static const char * SP;
static	Wind *	SW;






const char * SetStatus (const char * s)
{
  const char * p = SP;
  SP = s;
  return p;
}






static void StatusRepaint (Wind * W)
{
  static const char * old = NULL;
  if (SP != old)
    {
      memset(W->Body.Near, ' ', WindBodyWid(W));
      WriteWindText(W, 1, 0, SP);
      SetRefreshWind(W);
      old = SP;
    }
  W->Flags.Repaint = TRUE;
}





Wind * MakeStatusWind (void)
{
  SW = CreateWind(0, ZoomBot, MaxScrWid-1, ZoomBot, HLP, TRUE, FALSE);
  if (SW != NULL)
    {
      SW->ShowProc = ShowPureChar;
      SW->ReptProc = StatusRepaint;
      if (Status)
	{
	  -- ZoomBot;
	  ShowWind(SW);
	}
    }
  return SW;
}





void FlipStatus (void)
{
  Status = ! Status;
  if (Status)
    {
      ShowWind(SW);
      -- ZoomBot;
    }
  else
    {
      HideWind(SW);
      ++ ZoomBot;
    }
  SetRefreshDesk();
}
