#include	<wind.h>
#include	<mem.h>
#include	<dos.h>


extern	void	(*MenuProc)	(int);





int MenuChoice (Wind *W)
{
  int i;
  Keys k;

  while (TRUE)
    {
      if (MenuProc != NULL) MenuProc(W->Vp);
      RepaintMenuWind(W);
      k = GetKey();
      switch (k)
	{
	  case MouseLeft:
	  case TwiceLeft: if (! MenuMouse(W)) break;
	  case Enter : return W->Vp;
	  case -1    : break;
	  case Left  :
	  case Right :
	  case Esc   : return -1;
	  case Up    : MenuUp(W); break;
	  case Down  : MenuDown(W); break;
	  case PgUp  : MenuPgUp(W); break;
	  case CtrlPgUp :
	  case Home  : MenuHome(W); break;
	  case PgDn  : MenuPgDn(W); break;
	  case CtrlPgDn :
	  case End   : MenuEnd(W); break;
	  default    : if (W->Flags.KeyChoice && IsAlpha(k))
			 {
			   char *s = Image(W);
			   int w = W->Wid;
			   k = Upper(k);
			   for (i = 0; i < W->Hig; i ++)
			     {
			       if (memchr(s, k, w) != NULL)
				 {
				   W->Vp = i;
				   W->Y = i;
				   SetRefreshWind(W);
				   Refresh();
				   return i;
				 }
			       s += w;
			     }
			 }
		       else return -1;
		       break;
	}
    }
}
